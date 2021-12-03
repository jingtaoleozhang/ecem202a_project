import asyncio
import time
from bleak import BleakScanner, BleakClient
from bleak.exc import BleakError
from struct import *

print('ble receiver test')

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
        self.write_vals = [bytearray([0x01]), bytearray([0x02])]
        self.write_idx = 0
        self.slices_received = 0
        self.slices_required = 6
        self.allow_read = False

    # Runs once, displays service and characteristic info
    async def check_connection(self):
        if self.connected == False:
            self.device = await BleakScanner.find_device_by_filter(
                lambda d, ad: d.name and d.name.lower() == self.wanted_name.lower()
            )
            print(self.device)

            if not self.device:
                raise BleakError(target_name + " not found")

            self.client = BleakClient(self.device, loop=loop)

            # async with self.client:
            #     svcs = await self.client.get_services()
            #     print("Services:")
            #     for service in svcs:
            #         print(service)
            #         for chr in service.characteristics:
            #             print("\t" + str(chr))

            await self.client.connect()
            self.connected = await self.client.is_connected()

            await self.client.start_notify(read_chr_uuid, self.on_read_change)

    async def read_ble(self):
        # print("try reading")
        while (True):
            if self.connected and \
                    self.slices_received < self.slices_required and \
                    self.allow_read == True:
                # print("reading")
                try:
                    val = await self.client.read_gatt_char(read_chr_uuid)
                    nums = unpack('128f', val)
                    print(str(self.count) + ': ' + str(nums))
                    self.slices_received += 1
                    self.count += 1
                    self.allow_read = False
                except Exception as e:
                    print('READ BLE EXCEPTION')
                    print(e)
            await asyncio.sleep(.1, loop=loop)

    def on_read_change(self, sender, data):
        try:
            #print('read changed' + str(len(data)))
            self.allow_read = True
        except Exception as e:
            print('ON READ CHANGE EXCEPTION')
            print(e)

        # print("got notification: " + str(data) + " from " + sender)

    async def write_ble(self):
        # print("try writing")
        while (True):
            if self.connected and self.slices_received == self.slices_required:
                print("writing")
                try:
                    await self.client.write_gatt_char(
                        write_chr_uuid,
                        self.write_vals[self.write_idx])
                    self.write_idx = (self.write_idx + 1) % 2
                    self.slices_received = 0
                except Exception as e:
                    print("WRITE BLE EXCEPTION")
                    print(e)
            await asyncio.sleep(1, loop=loop)


async def cleanup(self):
    if self.client:
        await self.client.stop_notify(read_chr_uuid)
        await self.client.disconnect()


#
# device = None
# client = None
# device_found = False
#
#
# async def check_connection(wanted_name):
#     device = await BleakScanner.find_device_by_filter(
#         lambda d, ad: d.name and d.name.lower() == wanted_name.lower()
#     )
#     print(device)
#
#     if not device:
#         raise BleakError(target_name + " not found")
#
#     client = BleakClient(device)
#
#     # async with BleakClient(device) as client:
#     async with client:
#         svcs = await client.get_services()
#         print("Services:")
#         for service in svcs:
#             print(service)
#             for chr in service.characteristics:
#                 print("\t" + str(chr))
#
#
# async def read_ble():
#     if device_found:
#         while (True):
#             try:
#                 val = await client.read_gatt_char(read_chr_uuid)
#                 print(str(count) + ': ' + str(val))
#             except Exception as e:
#                 print(e)
#             await asyncio.sleep(1)
#
#
# async def write_ble():
#     if device_found:
#         while (True):
#             try:
#                 await client.write_gatt_char(write_chr_uuid, write_base)
#             except Exception as e:
#                 print(e)
#             await asyncio.sleep(1)
#
#
# async def cleanup():
#     if device_found:
#         await client.stop_notify(read_chr_uuid)
#         await client.disconnect()
#
#
# async with BleakClient(device) as client:
#     svcs = await client.get_services()
#     for service in svcs:
#         for chr in service.characteristics:
#             if chr.uuid == target_chr:
#                 print(chr.properties)
#                 while (True):
#                     try:
#                         val = await client.read_gatt_char(chr.uuid)
#                         write_val = write_base
#                         print(str(count) + ': ' + str(val) + ', ' + str(write_val))
#                         await client.write_gatt_char(chr.uuid, write_val)
#
#                     except Exception as e:
#                         print(e)

# time.sleep(.5)

if __name__ == "__main__":
    loop = asyncio.get_event_loop()
    connection = Connection(target_name)
    try:
        asyncio.ensure_future(connection.check_connection())
        asyncio.ensure_future(connection.read_ble())
        asyncio.ensure_future(connection.write_ble())
        loop.run_forever()
    except Exception as e:
        print(e)
    finally:
        print("disconnecting")
        # loop.run_until_complete(connection.cleanup())
