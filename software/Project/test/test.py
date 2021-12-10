# import numpy as np
# # generate c array
# lsb = np.arange(0, 128)
# file1 = open('test_data_gen_1.txt', 'w')
#
# file1.write('{')
# for i in range(5):
#     file1.write('{')
#     for j in range(6):
#         for k in lsb:
#             file1.write(str(i * 1000 + j * 128 + k) + ',')
#
#     file1.write('},\n')
# file1.write('}')
# file1.close()
#file1 = open('test_data_gen_2.txt', 'w')

import numpy as np
import tensorflow as tf

X_tr = np.load('X_debug.npy')
X_tr = X_tr.astype(np.float32)
y_tr = np.load('y_debug.npy')

interpreter = tf.lite.Interpreter(model_path='DeepConvLSTM_model.tflite')
interpreter.allocate_tensors()

in_spec = interpreter.get_input_details()
out_spec = interpreter.get_output_details()
#in_shape = in_spec[0]['shape']

# rand_data = np.array(np.random.random_sample(in_shape), dtype=np.float32)
# interpreter.set_tensor(in_spec[0]['index'], rand_data)
# interpreter.invoke()
# rand_data_out = interpreter.get_tensor(out_spec[0]['index'])

for i in range(X_tr.shape[0]):
    in_data = [X_tr[i]]
    interpreter.set_tensor(in_spec[0]['index'], in_data)
    interpreter.invoke()
    in_data_out = interpreter.get_tensor(out_spec[0]['index'])
    classification = np.argmax(in_data_out) + 1
    print(in_data_out.round(3))
    print(y_tr[i])
    print(classification)
    byte_class =bytearray(classification.tobytes())
    print()
