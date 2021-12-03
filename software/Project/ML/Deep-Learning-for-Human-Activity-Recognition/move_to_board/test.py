## Check if using GPU
# import tensorflow as tf
#
# print("Num GPUs Available: ", len(tf.config.list_physical_devices('GPU')))
#
# tf.debugging.set_log_device_placement(True)
#
# from tensorflow.keras import backend as K
# K.tensorflow_backend._get_available_gpus()


## Reformat data
# import pandas as pd
# import numpy as np
# print('testing')
# y_test = np.load('data/my_dataset/y_test.npy')
# y_train = np.load('data/my_dataset/y_train.npy')
#
# y_test = y_test.astype(int)
# y_train = y_train.astype(int)
#
# np.savetxt('data/my_dataset/y_test.txt', y_test.astype(int), delimiter=',',fmt='%d')
# np.savetxt('data/my_dataset/y_train.txt', y_train.astype(int), delimiter=',',fmt='%d')
# print(y_test.shape)

## Make tflite from h5
# import tensorflow as tf
#
# model = tf.keras.models.load_model('logs/deep-conv-lstm-20211117-000754/trained_model_fold0.h5')
# converter = tf.lite.TFLiteConverter.from_keras_model(model)
# tflite_model = converter.convert()
# open("DCLSTM_model.tflite", "wb").write(tflite_model)

## make an inference using tflite model
import numpy as np
import tensorflow as tf

X_tr = np.load('X_debug.npy')
X_tr = X_tr.astype(np.float32)
y_tr = np.load('y_debug.npy')

interpreter = tf.lite.Interpreter(model_path='DCLSTM_model_quant_edgetpu.tflite')
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
    print(in_data_out.round(3))
    print(y_tr[i])
    print()

## Convert tflite model to TPU useable
# import tensorflow as tf
#
# # interpreter = tf.lite.Interpreter(model_path='DCLSTM_model.tflite')
# # interpreter.allocate_tensors()
#
# # in_spec = interpreter.get_input_details()
# # out_spec = interpreter.get_output_details()
# # in_shape = in_spec[0]['shape']
#
# model = tf.keras.models.load_model('logs/cnn-20211117-215319/trained_model_fold0.h5')
#
#
# converter = tf.lite.TFLiteConverter.from_keras_model(model)
# converter.optimizations = [tf.lite.Optimize.DEFAULT]
# tflite_model = converter.convert()
#
# open("DCLSTM_model_quant.tflite", "wb").write(tflite_model)
