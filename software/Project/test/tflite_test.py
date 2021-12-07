# make an inference using tflite model
import numpy as np

import tensorflow as tflite

# import tflite_runtime.interpreter as tflite

# inf_buffer = np.empty((128, 6, 1))

X_tr = np.load('X_debug.npy')
X_tr = X_tr.astype(np.float32)
y_tr = np.load('y_debug.npy')

X_point = X_tr[0]

slices = []
for i in range(0, 6):
    new_data = list(range(128 * i, (i + 1) * 128))
    slices.append(new_data)

inf_buffer = np.asarray(slices[0:6])
inf_buffer = inf_buffer.reshape(128, 6, 1)

#in_data = [X_tr[0]]

interpreter = tflite.Interpreter('DCLSTM_model_quant_edgetpu.tflite',
                                 experimental_delegates=[tflite.load_delegate('libedgetpu.so.1')])
interpreter.allocate_tensors()

in_spec = interpreter.get_input_details()
out_spec = interpreter.get_output_details()

for i in range(X_tr.shape[0]):
    in_data = [X_tr[i]]
    interpreter.set_tensor(in_spec[0]['index'], in_data)
    interpreter.invoke()
    in_data_out = interpreter.get_tensor(out_spec[0]['index'])
    print(in_data_out.round(3))
    print(y_tr[i])
    print()
