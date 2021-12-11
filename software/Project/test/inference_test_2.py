import asyncio
import time
from bleak import BleakScanner, BleakClient
from bleak import exc
from bleak.exc import BleakError
from struct import *

import numpy as np
# import tflite_runtime.interpreter as tflite
import tensorflow as tf
from preprocessing import Preprocess
from scipy.signal import butter, filtfilt
import pandas as pd

print('ble receiver and inference test')

target_name = 'Ard_BLE_33_S'
svc_uuid = '00001234-0000-1000-8000-00805f9b34fb'
read_chr_uuid = '00001111-0000-1000-8000-00805f9b34fb'
write_chr_uuid = '00002222-0000-1000-8000-00805f9b34fb'


class Connection:
    def __init__(self, wanted_name):
        self.wanted_name = wanted_name
        self.client = None
        self.connected = False
        self.count = 0
        self.write_vals = [bytearray([0x00]), bytearray(
            [0x01]), bytearray([0x02])]
        self.write_idx = 0
        self.slices_received = 0
        self.slices_required = 6
        self.allow_read = False

        self.send_ack = False
        self.send_inf = False

        # self.inf_idx = 0
        self.data_buf = []

        self.interpreter = tf.lite.Interpreter(model_path='DeepConvLSTM_model.tflite')
        self.interpreter.allocate_tensors()
        self.in_spec = self.interpreter.get_input_details()
        self.out_spec = self.interpreter.get_output_details()

        self.num_inf = 0
        self.proc_sum = 0
        self.inf_sum = 0

        fc = 20  # cutoff frequency
        fs = 50
        w = fc / (fs / 2)  # Normalize the frequency
        self.b, self.a = butter(3, w, "low")  # 3rd order low-pass Butterworth filter

    # Runs once, displays service and characteristic info

    async def check_connection(self):
        if self.connected == False:
            self.device = await BleakScanner.find_device_by_filter(
                lambda d, ad: d.name and d.name.lower() == self.wanted_name.lower())
            print(self.device)

            if not self.device:
                raise BleakError(target_name + " not found")

            self.client = BleakClient(self.device, loop=loop)

            await self.client.connect()
            self.connected = await self.client.is_connected()

            await self.client.start_notify(read_chr_uuid, self.on_read_change)

    async def read_ble(self):
        while (True):
            if self.connected and \
                    self.slices_received < self.slices_required and \
                    self.allow_read == True:
                # print("reading")
                try:
                    val = await self.client.read_gatt_char(read_chr_uuid)
                    nums = unpack('128f', val)
                    self.data_buf.append(nums)
                    # self.inf_idx += 1
                    self.slices_received += 1
                    self.count += 1
                    self.allow_read = False
                    self.send_ack = True

                    # print(str(self.count) + '_' + str(self.slices_received) + ': ' + str(nums))
                    print(str(self.count) + '_' + str(self.slices_received))
                except Exception as e:
                    print('READ BLE EXCEPTION')
                    print(e)
            await asyncio.sleep(.01, loop=loop)

    def on_read_change(self, sender, data):
        try:
            # print('read changed' + str(len(data)))
            self.allow_read = True
        except Exception as e:
            print('ON READ CHANGE EXCEPTION')
            print(e)

    async def write_ble(self):
        while (True):
            if self.connected:
                if self.slices_received == self.slices_required:  # send inference
                    try:
                        start_time = time.monotonic()
                        inference_buf = np.asarray(self.data_buf[0:6], dtype=np.float32)
                        inference_buf = inference_buf.reshape((128, 6))

                        inference_buf = pd.DataFrame(inference_buf, dtype=np.float32)

                        # normalize
                        #inference_buf = StandardScaler().fit_transform(inference_buf)

                        mean = inference_buf.mean()
                        std = inference_buf.std()
                        inference_buf = (inference_buf - mean) / std

                        #print(sum(inference_buf - inf_buf_2))
                        inference_buf = pd.DataFrame(inference_buf, dtype=np.float32)

                        # median
                        inference_buf = inference_buf.rolling(window=5, center=True, min_periods=1).median()

                        # butterworth
                        inference_buf = filtfilt(self.b, self.a, inference_buf, axis=0)

                        inference_buf = inference_buf.astype(np.float32)
                        inference_buf = inference_buf.reshape((128, 6, 1))

                        proc_end = time.monotonic()

                        self.interpreter.set_tensor(self.in_spec[0]['index'], [inference_buf])
                        self.interpreter.invoke()

                        inf_end = time.monotonic()

                        self.proc_sum += proc_end - start_time
                        self.inf_sum += inf_end - proc_end
                        self.num_inf += 1
                        print(
                            "iter:" + str(self.num_inf) + ", Proc:" + str(self.proc_sum / self.num_inf) + "Inf:" + str(
                                self.inf_sum / self.num_inf))

                        inference_res = self.interpreter.get_tensor(self.out_spec[0]['index'])
                        print(inference_res.round(3))
                        classification = (np.argmax(inference_res) + 1).tobytes()

                        print("sent inference")
                        await self.client.write_gatt_char(write_chr_uuid, classification)

                        self.data_buf = []
                        self.slices_received = 0

                    except Exception as e:
                        print("WRITE BLE INF EXCEPTION")
                        print(e)

                elif self.send_ack == True:  # send ack
                    try:
                        await self.client.write_gatt_char(
                            write_chr_uuid,
                            self.write_vals[0])
                        self.send_ack = False
                    except Exception as e:
                        print("WRITE BLE ACK EXCEPTION")
                        print(e)

            await asyncio.sleep(.01, loop=loop)

    async def cleanup(self):
        if self.client:
            await self.client.stop_notify(read_chr_uuid)
            await self.client.disconnect()


if __name__ == "__main__":
    loop = asyncio.get_event_loop()
    connection = Connection(target_name)
    try:
        asyncio.ensure_future(connection.check_connection())
        asyncio.ensure_future(connection.read_ble())
        asyncio.ensure_future(connection.write_ble())
        loop.run_forever()
    except Exception as e:
        print('MAIN EXCEPTION')
        print(e)
    finally:
        print("disconnecting")
        loop.run_until_complete(connection.cleanup())
