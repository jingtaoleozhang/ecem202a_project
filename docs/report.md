# Table of Contents
* Abstract
* [Introduction](#1-introduction)
* [Related Work](#2-related-work)
* [Technical Approach](#3-technical-approach)
* [Evaluation and Results](#4-evaluation-and-results)
* [Discussion and Conclusions](#5-discussion-and-conclusions)
* [References](#6-references)

# Abstract
<!-- Provide a brief overview of the project objectives, approach, and results. -->

The goal of this project is to investigate the performance of using edge servers to provide deep neural network inference on data gathered by embedded microcontrollers. 

# 1. Introduction
<!-- 
This section should cover the following items:
* Motivation & Objective: What are you trying to do and why? (plain English without jargon)
* State of the Art & Its Limitations: How is it done today, and what are the limits of current practice?
* Novelty & Rationale: What is new in your approach and why do you think it will be successful?
* Potential Impact: If the project is successful, what difference will it make, both technically and broadly?
* Challenges: What are the challenges and risks?
* Requirements for Success: What skills and resources are necessary to perform the project?
* Metrics of Success: What are metrics by which you would check for success? 
* -->

The goal of this project is to evaluate the effectiveness of using a networked CNN accelerator to improve the performance of deep neural networks when using data gathered by embedded microcontrollers.

Running ML models on embedded devices is known as TinyML and has shown validity [[1](#1)]. However, resource constraints limit the possibilities to traditinal ML methods such as SVM, and MLP or relatively shallow nerual networks. If the microcontroller is capable of wireless communication, data can be sent from the device to an edge server or IoT hub. The the edge server can make inferences using a more complicated model or processes data and send it to a core cloud for inference [[2](#2)]. In the second scenario, the data has to go from the embedded device to the edge server, through the internet to the core cloud, then back to the edge cloud, then back to the IoT device, adding a lot of latency. 

Computing inference remotely is not a novel concept and is quite common, however the setups used in current research papers involve GPU equipped PCs as edge servers and smartphones as devices [[3](#3)]. The system that will be evaluated in this project consists of the Arduino Nano 33 BLE Sense for the device, and the Coral Dev Board for the server. The Arduino is a microcontroller class embedded device and the Coral is a single board computer equipped with an edge tensor processing unit. The overall concept for this approach is fairly proven, however this project takes component cost as low as reasonable.

The communication between device and server will be through bluetooth low energy which is not known for its throughput and can take a significant portion of the runtime of an iteration. For success, the project requires basic knowledge of training and running neural networks. There will be a small model that can run on the device and a more complex one that will run on the edge server. A pre-existing network architecture and dataset will be used.

The inference latency when a model is run on the microcontroller vs the edge server will be the most important metric. The communication overhead will need to be profiled.


# 2. Related Work

## Netural Networks on Microcontrollers.
Giving neural net inference capabilities to micrcontrollers is an emerging area [[1](#1)]. Applications exist from recognizing voice commands (like for Google home and Amazon Alexa) or recognizing when someone is looking at a camera to turn on a larger processor for facial recognition [[9](#9)].

## Inference at the Edge
Using powerful networked devices to run inference is very widespread in machine learning applications [[10](#10)]. Using edge servers for inference rather than going through the internet is an area of interest due to latency and security concerns [[3](#3), [4](#4)].


# 3. Technical Approach

There will be three setups that will run inference. The first setup only involves the embedded device. The Arduino will capture data from its inertial measurement unit, process it, then use run inference using a small CNN. The second setup will using the embedded device for data gathering and a Coral Dev Board edge server for inference. The communication between the server and device will be through BLE. The server will run inference using a larger model. The third setup will be similar to the second one, however a GPU equipped PC will serve as the edge server. Preprocessing of data can be done on the device or server.

The application that will be used to compare the setups is human activity recognition using acclerometer and gyroscope data.

## Data
The dataset that the nerual networks are trained on is the UCI Smartphone-Based Recognition of Human Activities and Postural Transitions Data Set [[8](#8)]. The input to a model is a 768 length floating point vector of sets of 3 axis accelerometer and 3 axis gyroscope measuremnts sampled at 50 Hz. The time for the entire vector to be generated is 2.56 seconds.

The data is essentially 6 vectors of IMU measurements. Each of these vectors is median filtered, then filtered by a third degree low-pass butterworth filter to remove noise and gravity components. The vectors are then assembled into the final 768 size vector.

Median filter implementation is by Bogdan Anderu[[11](#11)]. Butterworth filter implementation is based on Darien Pardina's filter implementation [[12](#12)].

## Models
The model that will run on the Coral board is the baseline CNN used in [[5](#5)] that has been quantized for compatability with the edge TPU. It's 1364 kB in size and its architecture is shown below. 
![Model for Coral](media/cnn_baseline.png)

The model that will run on the Arduino is similar to the one that runs on the Coral however the number of filters in its convolutional layers and number units in its densely connected layers have been reduced. it's 364 kB in size and its architecture is shown below. Combined with the code for gathering and processing the data, the amount of memory required is close to the device's maximum.
![Model for Arduino](media/cnn_12_12.png)

The model that will run on the PC is the DeepConvLSTM from [[5](#5)]. It's 1785 kB in size and its architecture is shown below. The LSTM layers are supported on standard TensorFlow.
![Model for PC](media/deepconvlstm.png)

One thing of note is that the accuracies for these models on the dataset are strangely quite similar at around 95%; however, the accuracy of nerual network architectures for human activity recognition is beyond the scope of this project.

Model implementations are based on Takumi Watanabe's implementations [[6](#6)].

## Communication

Data is sent to the edge server for inference through bluetooth low energy version 4.2. The maximum packet size is 512 bytes meaning the input vector of 768 floats must be sent in 6 slices. This is done by the following protocol. The communication happens using two BLE charachteristics, TX and RX (from the persepective of the device). 

When the data is ready, the device will continuously send the first slice of 128 floats on the TX characthersitic until it reads an acknowledgement on RX from the edge server. After that it will go to the next slice and again wait for an acknowledgement, this repeats until all 6 slices are sent after which the device will wait for an inference on RX. When it receives the inference it will sample and process a new vector of inputs and repeat the process. It will only repeat the first slice on the first iteration before an acknowledgement is received. 

## Software System

For the setup that involves only the Arduino. The IMU sampled for the required 2.56 seconds, the median and butterworth fitlers are applied, then the vector is sent to inference.

For the setups that involve an edge server, the IMU is sampled, the raw data is sent to the edge server where it is processed and an inference is made. The classification is then sent back to the device. The edge servers use python.

# 4. Evaluation and Results

The time it takes to sample the data is always 2.56 seconds, as the dataset specifies. 

The setup with just the microcontroller has only data processing and inference times to profile. The setups involivng edge servers have communications, processing, and inference times to consider.

The results averaged over 100 iterations are tabulated below. Time are in seconds. In parentheses is the speedup over doing everything on the device.

| Setup           | Processing      | Communication | Inference          | Total              |
| --------------- | --------------- | ------------- | ------------------ | ------------------ |
| Arudino         | 0.05090         | N/A           | 1.20305            | 3.80777            |
| Arduino + Coral | 0.01258 (4.05x) | 2.92701*      | 0.01843   (66.76x) | 5.51802*  (-1.45x) |
| Arduino + PC    | 0.00312 (16.3x) | 2.92701       | 0.01428  (84.25x)  | 5.50441 (-1.45x)   |

\* Unfortunately, during development I damaged the bluetooth chip on the Coral board by static electricity discharge. I believe this is the case because bluetooth had previously functioned on it then stopped with no changes to code, also bluetooth devices can no longer be found when using linux commands. I will use the communication penatly from the Arduino + PC setup for both remote inference setups.

# 5. Discussion and Conclusions

## Results Analysis
For the Arduino only setup, data processing took 1.3% of the iteration time, the fixed sampling period took 67.2%, and the inference took 31.6%.

For the setups including the Coral and PC servers, data processing and inference took less than 1% of the iteration time while the fixed sampling time took 46.4% and 46.% of the iteration and communication took 53.0% and 53.2% respectively.

The communication penalty is large considerable and takes up most of the iteration time. The input vector to the model consists of 768 floating point numbers for a total of 3072 bytes. With a communication cost of 2.92701 seconds, this gives an approximate bitrate of the BLE setup of 1.05kB/s.

The time to process the data (median and butterworth filter) is lowered drastically by performing it on the powerful CPUs of the edge servers, however the absolute time saved is negligible considering the commication penalty.


## Future Directions.
compressed sensing, partition models, reduce comm overhead by using integers


# 6. References

<a id="1">[1]</a>: R. Sanchez-Iborra and A. F. Skarmeta, "TinyML-Enabled Frugal Smart Objects: Challenges and Opportunities," in IEEE Circuits and Systems Magazine, vol. 20, no. 3, pp. 4-18, thirdquarter 2020, doi: 10.1109/MCAS.2020.3005467. https://ieeexplore.ieee.org/document/9166461

<a id="2">[2]</a>:  M. Shamim Hossain, Ghulam Muhammad, Emotion recognition using secure edge and cloud computing, Information Sciences, Volume 504, 2019, Pages 589-601, ISSN 0020-0255, https://doi.org/10.1016/j.ins.2019.07.040.

<a id="3">[3]</a>: J. Chen and X. Ran, "Deep Learning With Edge Computing: A Review," in Proceedings of the IEEE, vol. 107, no. 8, pp. 1655-1674, Aug. 2019, doi: 10.1109/JPROC.2019.2921977. https://ieeexplore.ieee.org/document/8763885

<a id="4">[4]</a>: M. G. Sarwar Murshed, Christopher Murphy, Daqing Hou, Nazar Khan, Ganesh Ananthanarayanan, and Faraz Hussain. 2021. Machine Learning at the Network Edge: A Survey. ACM Comput. Surv. 54, 8, Article 170 (November 2022), 37 pages. DOI:https://doi.org/10.1145/3469029

<a id="5">[5]</a>: Ordóñez, F.J.; Roggen, D. Deep Convolutional and LSTM Recurrent Neural Networks for Multimodal Wearable Activity Recognition. Sensors 2016, 16, 115. https://doi.org/10.3390/s16010115

<a id="6">[6]</a>: Watanabe, Takumi, Deep Learning for Human Activity Recognition, (2020), GitHub repository, https://github.com/takumiw/Deep-Learning-for-Human-Activity-Recognition

<a id="7">[7]</a>: Brownlee, Jason, Evaluate Machine Learning Algorithms for Human Activity Recognition, (2020), Webpage, https://machinelearningmastery.com/evaluate-machine-learning-algorithms-for-human-activity-recognition/

<a id="8">[8]</a>: Smartphone-Based Recognition of Human Activities and Postural Transitions Data Set. 2015. Available online: http://archive.ics.uci.edu/ml/datasets/Smartphone-Based+Recognition+of+Human+Activities+and+Postural+Transitions

<a id="9">[9]</a>: Warden, Pete, and Daniel Situnayake. TinyML. O'Reilly Media, Incorporated, 2019.

<a id="10">[10]</a>: Kaur, Harkiran, Top Cloud Computing Platforms for Machine Learning, (2020), Webpage, https://www.geeksforgeeks.org/top-cloud-computing-platforms-for-machine-learning/

<a id="11">[11]</a>: Bogdan, Alexandru, Median Filter, (2019), GitHub repository, https://github.com/accabog/MedianFilter

<a id="12">[12]</a>: Pardians, Darien, C++ Implementation of MATLAB's filtfilt , Webpage, https://stackoverflow.com/a/27270420