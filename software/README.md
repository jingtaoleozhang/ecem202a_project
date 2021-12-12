* Setup 1 = Arduino Only, Setup 2 = Arduino + Coral, Setup 3 = Arduino + PC
* [Ard\arduino_inference](Project\Project\Ard\arduino_inference)
  * code for running setup 1
* [Ard\remote_inference](Project\Project\Ard\remote_inference)
  * code for the arduino portion of setups 2 and 3
* [ML\Deep-Learning-for-Human-Activity-Recognition\run_small_cnn.py](Project\ML\Deep-Learning-for-Human-Activity-Recognition\run_small_cnn.py)
  * code for training the model used in setup 1
* [ML\Deep-Learning-for-Human-Activity-Recognition\run_cnn.py](Project\ML\Deep-Learning-for-Human-Activity-Recognition\run_cnn.py)
  * code for training the model used in setup 2
* [ML\Deep-Learning-for-Human-Activity-Recognition\run_deep_conv_lstm.py](Project\ML\Deep-Learning-for-Human-Activity-Recognition\run_deep_conv_lstm.py)
  * code for training the model used in setup 3
* [test\inference_test_coral.py](Project\test\inference_test_coral.py)
  * code for edge server portion of setup 2
* [test\inference_test2.py](Project\test\inference_test2.py)
  * code for the edge server portion of setup 3
* [test\test_metrics.py](Project\test\test_metrics.py)
  * code for measuring data processing and inference time for setup 2 after I broke the bluetooth chip