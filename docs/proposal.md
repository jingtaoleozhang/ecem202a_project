# Project Proposal

## 1. Motivation & Objective

What are you trying to do and why? (plain English without jargon)

The goal of this project is to evaluate the effectiveness of using a networked neural network accelerator to improve the performance of neural networks for human activity recognition using IMU and acoustic data on an embedded device. The device and accelerator in question are the Arduino Nano 33 BLE Sense and the Coral Dev Board TPU.

## 2. State of the Art & Its Limitations
How is it done today, and what are the limits of current practice?

Running ML models on embedded devices only is known as TinyML and has shown validity. However the resource constraints limit the possibilities to decision tree, SVM, or shallow MLP models.
https://ieeexplore.ieee.org/abstract/document/9166461

Data sent to an edge cloud from IoT devices using bluetooth. Edge cloud preprocesses data which is sent to core cloud for inference. This means your data must go from the IoT device, to the edge cloud, through the internet, back to the edge cloud, then back to the IoT device.
https://www.sciencedirect.com/science/article/pii/S0020025519306486#sec0003

Edge inference is not a novel concept, however the setups used in most of the related papers involve GPU equipped PCs as edge servers and smartphones as devices. These devices are much more powerful than the arduino and coral dev board. Additionally, these are tested using nerual networks for image recognition.
https://ieeexplore.ieee.org/document/8763885



## 3. Novelty & Rationale

What is new in your approach and why do you think it will be successful?

Embedded devices like the Arduino Nano 33 BLE Sense are resource limited and cannot run large DNN models. Using the cloud to host a model is quite common, however this incurs additional latency. Using a low(er) cost neural network accelerator on the edge is also common but current approaches use powerful GPU equipped PCs along with smartphones as the end device. My approach is to evaluate the effectiveness using microcontroller class end devices with a Tensor Processing Unit device as the edge server. I believe this can be successful because the overall concept is fairly proven, however my approach takes component cost almost to the minimum.

## 4. Potential Impact

If the project is successful, what difference will it make, both technically and broadly?

If this project is successful and gets reasonable accuracy and latency, then it will be significant in that the setup cost is much lower than the existing experimental setups. It could allow better IoT setups to be more available.

## 5. Challenges

What are the challenges and risks?
The communication between device and server will be through bluetooth which is not known for its bandwidth.

## 6. Requirements for Success

What skills and resources are necessary to perform the project?

## 7. Metrics of Success

What are metrics by which you would check for success?

## 8. Execution Plan

Describe the key tasks in executing your project, and in case of team project describe how will you partition the tasks.

## 9. Related Work

### 9.a. Papers

List the key papers that you have identified relating to your project idea, and describe how they related to your project. Provide references (with full citation in the References section below).

### 9.b. Datasets

List datasets that you have identified and plan to use. Provide references (with full citation in the References section below).

### 9.c. Software

List softwate that you have identified and plan to use. Provide references (with full citation in the References section below).

## 10. References

List references correspondign to citations in your text above. For papers please include full citation and URL. For datasets and software include name and URL.
