#include <Arduino_LSM9DS1.h>
#include <math.h>

#include <algorithm>
#include <iterator>

#include "MedianFilter.hpp"
#include "filtfilt.h"
#include "imu_ex_data.h"

void setup() {
    // put your setup code here, to run once:

    Serial.println("sensor data processing Test Begin");

    // Initialize serial and wait for port to open:
    Serial.begin(9600);
    while (!Serial) {
        ;  // wait for serial port to connect. Needed for native USB port only
    }
    if (!IMU.begin()) {
        Serial.println("init IMU fail");
        while (1)
            ;
    }
}

float g_x, g_y, g_z;
float a_x, a_y, a_z;
// const size_t window_size = 128;
#define WINDOW_SIZE 128
#define MEDIAN_FILTER_WINDOW 5
int sample_period = 20;  // sample at 50 Hz

vectord butter_b{0.52762438, 1.58287315, 1.58287315, 0.52762438};
vectord butter_a{1.0, 1.76004188, 1.18289326, 0.27805992};

unsigned long curr_time = 0;
int period = 1000;

float acc_x[WINDOW_SIZE];
float acc_y[WINDOW_SIZE];
float acc_z[WINDOW_SIZE];

float get_mean(float arr[]) {
    float sum = 0;
    for (size_t i = 0; i < WINDOW_SIZE; i++) {
        sum += arr[i];
    }
    return sum / WINDOW_SIZE;
}

float get_std(float arr[], float mean) {
    float sum = 0;
    for (size_t i = 0; i < WINDOW_SIZE; i++) {
        sum += pow((arr[i] - mean), 2);
    }
    return sqrt(sum / WINDOW_SIZE);
}

void print_first_5(float arr1[], float arr2[], float arr3[]) {
    for (size_t i = 0; i < 5; i++) {
        Serial.print("x=");
        Serial.print(arr1[i]);
        Serial.print(", y=");
        Serial.print(arr2[i]);
        Serial.print(", z=");
        Serial.println(arr3[i]);
    }
}

void print_last_5(float arr1[], float arr2[], float arr3[]) {
    for (size_t i = WINDOW_SIZE - 5; i < WINDOW_SIZE; i++) {
        Serial.print("x=");
        Serial.print(arr1[i]);
        Serial.print(", y=");
        Serial.print(arr2[i]);
        Serial.print(", z=");
        Serial.println(arr3[i]);
    }
}

void vec_print_test(vectord vec1, vectord vec2, vectord vec3) {
    for (size_t i = 0; i < 5; i++) {
        Serial.print("x=");
        Serial.print(vec1[i]);
        Serial.print(", y=");
        Serial.print(vec2[i]);
        Serial.print(", z=");
        Serial.println(vec3[i]);
    }
    Serial.println("=====");
    for (size_t i = WINDOW_SIZE - 5; i < WINDOW_SIZE; i++) {
        Serial.print("x=");
        Serial.print(vec1[i]);
        Serial.print(", y=");
        Serial.print(vec2[i]);
        Serial.print(", z=");
        Serial.println(vec3[i]);
    }
}

void med_3(float arr[], vectord &vec, int idx, int offset) {
    float sort_buf_3[3];
    for (size_t j = 0; j < 3; j++) {
        sort_buf_3[j] = arr[idx + j - offset];
    }
    std::sort(sort_buf_3, sort_buf_3 + 3);
    vec[idx] = sort_buf_3[1];
}

void med_4(float arr[], vectord &vec, int idx, int offset) {
    float sort_buf_4[4];
    for (size_t j = 0; j < 4; j++) {
        sort_buf_4[j] = arr[idx + j - offset];
    }
    std::sort(sort_buf_4, sort_buf_4 + 4);
    vec[idx] = (sort_buf_4[1] + sort_buf_4[2]) / 2;
}

void process_arr(float arr[], vectord &vec) {
    float mean = get_mean(arr);
    float std = get_std(arr, mean);

    // scale norm
    for (size_t i = 0; i < WINDOW_SIZE; i++) {
        arr[i] = (arr[i] - mean) / std;
    }

    // Median Filter
    vectord med(WINDOW_SIZE);
    MedianFilter<float, 5> medianFilter;
    for (size_t i = 0; i < WINDOW_SIZE; i++) {
        float res = medianFilter.Insert(arr[i]);
        if (i >= 4) {
            med[i - 2] = res;
        }
    }

    med_3(arr, med, 0, 0);    // i == 0
    med_4(arr, med, 1, 1);    // i == 1
    med_4(arr, med, 126, 2);  // i == 126
    med_3(arr, med, 127, 2);  // i == 127

    // Butterworth Filter
    // vectord butter(WINDOW_SIZE);
    filtfilt(butter_b, butter_a, med, vec);
}

void loop() {
    if (millis() >= curr_time + sample_period) {
        curr_time += period;

        Serial.println("iter start");
        float acc_x[WINDOW_SIZE];
        float acc_y[WINDOW_SIZE];
        float acc_z[WINDOW_SIZE];
        for (size_t i = 0; i < WINDOW_SIZE; i++) {
            acc_x[i] = example_imu_acc_x_data[i];
            acc_y[i] = example_imu_acc_y_data[i];
            acc_z[i] = example_imu_acc_z_data[i];
        }

        vectord proc_x(WINDOW_SIZE);
        vectord proc_y(WINDOW_SIZE);
        vectord proc_z(WINDOW_SIZE);

        process_arr(acc_x, proc_x);
        process_arr(acc_y, proc_y);
        process_arr(acc_z, proc_z);

        // Serial.println("buttered");
        vec_print_test(proc_x, proc_y, proc_z);
    }
}
