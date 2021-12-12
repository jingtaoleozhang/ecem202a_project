# make an inference using tflite model
import numpy as np
import time
import tflite_runtime.interpreter as tflite
from scipy.signal import butter, filtfilt
import pandas as pd

X_tr = np.load('X_debug.npy')
X_tr = X_tr.astype(np.float32)
y_tr = np.load('y_debug.npy')

interpreter = tflite.Interpreter('DCLSTM_model_quant_edgetpu.tflite', experimental_delegates=[
                                 tflite.load_delegate('libedgetpu.so.1')])
interpreter.allocate_tensors()

in_spec = interpreter.get_input_details()
out_spec = interpreter.get_output_details()

fc = 20  # cutoff frequency
fs = 50
w = fc / (fs / 2)  # Normalize the frequency
b, a = butter(3, w, "low")

proc_sum = 0
inf_sum = 0
num_inf = 0

for iter in range(100):
    for i in range(X_tr.shape[0]):
        inference_buf = X_tr[i]

        start_time = time.monotonic()

        inference_buf = inference_buf.reshape((128, 6))

        inference_buf = pd.DataFrame(
            inference_buf, dtype=np.float32)

        # normalize
        mean = inference_buf.mean()
        std = inference_buf.std()
        inference_buf = (inference_buf - mean) / std
        inference_buf = pd.DataFrame(
            inference_buf, dtype=np.float32)

        # median
        inference_buf = inference_buf.rolling(
            window=5, center=True, min_periods=1).median()

        # butterworth
        inference_buf = filtfilt(b, a, inference_buf, axis=0)

        inference_buf = inference_buf.astype(np.float32)
        
        inference_buf = inference_buf.reshape((128, 6, 1))

        proc_end = time.monotonic()

        interpreter.set_tensor(in_spec[0]['index'], [inference_buf])
        interpreter.invoke()

        inf_end = time.monotonic()

        proc_sum += proc_end - start_time
        inf_sum += inf_end - proc_end
        num_inf += 1
        print("iter:" + str(num_inf) + ", Proc:" + str(proc_sum / num_inf) + "Inf:" + str(
            inf_sum / num_inf))

        in_data_out = interpreter.get_tensor(out_spec[0]['index'])
        print(in_data_out.round(3))
        print(y_tr[i])
        print()
