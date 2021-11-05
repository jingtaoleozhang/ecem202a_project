# Project Proposal

## 1. Motivation & Objective

*What are you trying to do and why? (plain English without jargon)*

The goal of this project is to evaluate the effectiveness of using a networked neural network accelerator to improve the performance of neural networks for human activity recognition using IMU and acoustic data on an embedded device. The device and accelerator in question are the Arduino Nano 33 BLE Sense and the Coral Dev Board TPU.


## 2. State of the Art & Its Limitations
*How is it done today, and what are the limits of current practice?*

Running ML models on embedded devices is known as TinyML and has shown validity. However the resource constraints limit the possibilities to decision tree, SVM, or shallow MLP models. Data can be sent from the device to an edge server or IoT hub. The the edge server can make inferences using a more complicated model or processes data and send it to a core cloud for inference. This means data may have to go from the IoT device, to the edge cloud, through the internet, back to the edge cloud, then back to the IoT device which will introduce added latency. Remote inference computation is not a novel concept, however the setups used in current research papers involve GPU equipped PCs as edge servers and smartphones as devices. These devices are much more powerful than the Arduino and Coral Dev Board. Additionally, the papers test using nerual networks for image recognition rather than for human activity recognition.


## 3. Novelty & Rationale

*What is new in your approach and why do you think it will be successful?*

Embedded devices like the Arduino Nano 33 BLE Sense are resource limited and cannot run large DNN models. Using the cloud to host a model is quite common, however this incurs additional latency. Using a low(er) cost neural network accelerator on the edge is also common but current approaches use powerful GPU equipped PCs along with smartphones as the end device. My approach is to evaluate the effectiveness using microcontroller class end devices with a Tensor Processing Unit device as the edge server. I believe this can be successful because the overall concept is fairly proven, however this project takes component cost as low as reasonable.


## 4. Potential Impact

*If the project is successful, what difference will it make, both technically and broadly?*

If this project is successful and gets reasonable accuracy and latency, then it will be significant in that the setup cost is much lower than the existing experimental setups. It could allow better IoT setups to be more available.


## 5. Challenges

*What are the challenges and risks?*

The communication between device and server will be through bluetooth which is not known for its bandwidth, techniques such as compressive sampling can come into play here. There also exists the possibility of distributing the workload between device and server, the partioning can make it so the intermediate results that are sent to the edge server smaller than the raw data.


## 6. Requirements for Success

*What skills and resources are necessary to perform the project?*

This project will require the ability to train machine learning models, a large one that will run on the edge server and a small one that will run on the device. These models will be premade resources and not developed as part of the project. The required hardware are the Arduino and Coral boards.


## 7. Metrics of Success

*What are metrics by which you would check for success?*

There will be three kinds of setups. One where only the device is used, one where the device is used only for sensing and data preprocessing and the edge server is used for inference, and (depending on time) one where the inference is split between the device and server. The metrics used will be energy, latency, and accuracy for each of the three setups.


## 8. Execution Plan

*Describe the key tasks in executing your project, and in case of team project describe how will you partition the tasks.*

The key tasks are:
<ol>
  <li>Get Arduino and Coral board running and learn how to program them (hello world).</li>
  <li>Find models for human activity recognition that use IMU and/or sound data.</li>
  <li>Run the models on the appropriate platform.</li>
  <li>Evaluate the performance metrics.</li>
</ol>


## 9. Related Work

### 9.a. Papers

*List the key papers that you have identified relating to your project idea, and describe how they related to your project. Provide references (with full citation in the References section below).*

"TinyML-Enabled Frugal Smart Objects: Challenges and Opportunities"[1] covers running ML models on microcontrollers.

"Emotion recognition using secure edge and cloud computing"[2] and "Deep Learning With Edge Computing: A Review"[3] cover use edge clouds to run inference.

"Machine Learning at the Network Edge: A Survey"[4] covers many topics about edge computing, in particular vertically distributed inference.

### 9.b. Datasets

*List datasets that you have identified and plan to use. Provide references (with full citation in the References section below).*
The Dataset that will be used is the Smartphone-Based Recognition of Human Activities and Postural Transitions Data Set [8].

### 9.c. Software

*List softwate that you have identified and plan to use. Provide references (with full citation in the References section below).*
A deep CNN that will run on the Coral Dev Board is DeepConvLSTM [5] and the implementation is courtesy of Takumi Watanabe [6]

Simpler, tradidional ML approaches that will run on the Arduino will come from "Evaluate Machine Learning Algorithms for Human Activity Recognition" [7].

TensorFlow, TensorFlow Lite, and Keras are frameworks used to implement the machine learning models.

## 10. References

*List references corresponding to citations in your text above. For papers please include full citation and URL. For datasets and software include name and URL.*

[1]: R. Sanchez-Iborra and A. F. Skarmeta, "TinyML-Enabled Frugal Smart Objects: Challenges and Opportunities," in IEEE Circuits and Systems Magazine, vol. 20, no. 3, pp. 4-18, thirdquarter 2020, doi: 10.1109/MCAS.2020.3005467. https://ieeexplore.ieee.org/document/9166461

[2]:  M. Shamim Hossain, Ghulam Muhammad, Emotion recognition using secure edge and cloud computing, Information Sciences, Volume 504, 2019, Pages 589-601, ISSN 0020-0255, https://doi.org/10.1016/j.ins.2019.07.040.

[3]: J. Chen and X. Ran, "Deep Learning With Edge Computing: A Review," in Proceedings of the IEEE, vol. 107, no. 8, pp. 1655-1674, Aug. 2019, doi: 10.1109/JPROC.2019.2921977. https://ieeexplore.ieee.org/document/8763885

[4]: M. G. Sarwar Murshed, Christopher Murphy, Daqing Hou, Nazar Khan, Ganesh Ananthanarayanan, and Faraz Hussain. 2021. Machine Learning at the Network Edge: A Survey. ACM Comput. Surv. 54, 8, Article 170 (November 2022), 37 pages. DOI:https://doi.org/10.1145/3469029

[5]: Ordóñez, F.J.; Roggen, D. Deep Convolutional and LSTM Recurrent Neural Networks for Multimodal Wearable Activity Recognition. Sensors 2016, 16, 115. https://doi.org/10.3390/s16010115

[6]: Watanabe, Takumi, Deep Learning for Human Activity Recognition, (2020), GitHub repository, https://github.com/takumiw/Deep-Learning-for-Human-Activity-Recognition

[7]: Brownlee, Jason, Evaluate Machine Learning Algorithms for Human Activity Recognition, (2020), Webpage, https://machinelearningmastery.com/evaluate-machine-learning-algorithms-for-human-activity-recognition/

[8]: Smartphone-Based Recognition of Human Activities and Postural Transitions Data Set. 2015. Available online: http://archive.ics.uci.edu/ml/datasets/Smartphone-Based+Recognition+of+Human+Activities+and+Postural+Transitions
