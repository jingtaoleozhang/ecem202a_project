#include <Arduino_LSM9DS1.h>
#include <math.h>

#include <algorithm>

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

void loop() {
    if (millis() >= curr_time + sample_period) {
        curr_time += period;

        Serial.println("iter start");
        float mean_x = get_mean(example_imu_acc_x_data);
        float mean_y = get_mean(example_imu_acc_y_data);
        float mean_z = get_mean(example_imu_acc_z_data);

        float std_x = get_std(example_imu_acc_x_data, mean_x);
        float std_y = get_std(example_imu_acc_y_data, mean_y);
        float std_z = get_std(example_imu_acc_z_data, mean_z);

        // Scale Norm
        for (size_t i = 0; i < WINDOW_SIZE; i++) {
            acc_x[i] = (example_imu_acc_x_data[i] - mean_x) / std_x;
            acc_y[i] = (example_imu_acc_y_data[i] - mean_y) / std_y;
            acc_z[i] = (example_imu_acc_z_data[i] - mean_z) / std_z;
        }

        // Serial.println("scaled:");
        // print_first_5(acc_x, acc_y, acc_z);
        // print_last_5(acc_x, acc_y, acc_z);

        // Median Filter
        vectord med_acc_x(WINDOW_SIZE);
        vectord med_acc_y(WINDOW_SIZE);
        vectord med_acc_z(WINDOW_SIZE);

        vectord butter_acc_x(WINDOW_SIZE);
        vectord butter_acc_y(WINDOW_SIZE);
        vectord butter_acc_z(WINDOW_SIZE);

        // bad median method
        // float sort_buf_3[3];
        // float sort_buf_4[4];
        // float sort_buf_5[5];
        // for (size_t i = 0; i < WINDOW_SIZE; i++) {
        //     if (i == 0) {
        //         for (size_t j = 0; j < 3; j++) {
        //             sort_buf_3[j] = acc_x[i + j];
        //         }
        //         std::sort(sort_buf_3, sort_buf_3 + 3);
        //         med_acc_x[i] = sort_buf_3[1];
        //
        //         for (size_t j = 0; j < 3; j++) {
        //             sort_buf_3[j] = acc_y[i + j];
        //         }
        //         std::sort(sort_buf_3, sort_buf_3 + 3);
        //         med_acc_y[i] = sort_buf_3[1];
        //
        //         for (size_t j = 0; j < 3; j++) {
        //             sort_buf_3[j] = acc_z[i + j];
        //         }
        //         std::sort(sort_buf_3, sort_buf_3 + 3);
        //         med_acc_z[i] = sort_buf_3[1];
        //
        //     } else if (i == 1) {
        //         for (size_t j = 0; j < 4; j++) {
        //             sort_buf_4[j] = acc_x[i + j - 1];
        //         }
        //         std::sort(sort_buf_4, sort_buf_4 + 4);
        //         med_acc_x[i] = (sort_buf_4[1] + sort_buf_4[2]) / 2;
        //
        //         for (size_t j = 0; j < 4; j++) {
        //             sort_buf_4[j] = acc_y[i + j - 1];
        //         }
        //         std::sort(sort_buf_4, sort_buf_4 + 4);
        //         med_acc_y[i] = (sort_buf_4[1] + sort_buf_4[2]) / 2;
        //
        //         for (size_t j = 0; j < 4; j++) {
        //             sort_buf_4[j] = acc_z[i + j - 1];
        //         }
        //         std::sort(sort_buf_4, sort_buf_4 + 4);
        //         med_acc_z[i] = (sort_buf_4[1] + sort_buf_4[2]) / 2;
        //
        //     } else if (i == 126) {
        //         for (size_t j = 0; j < 4; j++) {
        //             sort_buf_4[j] = acc_x[i + j - 2];
        //         }
        //         std::sort(sort_buf_4, sort_buf_4 + 4);
        //         med_acc_x[i] = (sort_buf_4[1] + sort_buf_4[2]) / 2;
        //
        //         for (size_t j = 0; j < 4; j++) {
        //             sort_buf_4[j] = acc_y[i + j - 2];
        //         }
        //         std::sort(sort_buf_4, sort_buf_4 + 4);
        //         med_acc_y[i] = (sort_buf_4[1] + sort_buf_4[2]) / 2;
        //
        //         for (size_t j = 0; j < 4; j++) {
        //             sort_buf_4[j] = acc_z[i + j - 2];
        //         }
        //         std::sort(sort_buf_4, sort_buf_4 + 4);
        //         med_acc_z[i] = (sort_buf_4[1] + sort_buf_4[2]) / 2;
        //     } else if (i == 127) {
        //         for (size_t j = 0; j < 3; j++) {
        //             sort_buf_3[j] = acc_x[125 + j];
        //         }
        //         std::sort(sort_buf_3, sort_buf_3 + 3);
        //         med_acc_x[i] = sort_buf_3[1];
        //
        //         for (size_t j = 0; j < 3; j++) {
        //             sort_buf_3[j] = acc_y[125 + j];
        //         }
        //         std::sort(sort_buf_3, sort_buf_3 + 3);
        //         med_acc_y[i] = sort_buf_3[1];
        //
        //         for (size_t j = 0; j < 3; j++) {
        //             sort_buf_3[j] = acc_z[125 + j];
        //         }
        //         std::sort(sort_buf_3, sort_buf_3 + 3);
        //         med_acc_z[i] = sort_buf_3[1];
        //     } else {
        //         for (size_t j = 0; j < 5; j++) {
        //             sort_buf_5[j] = acc_x[i + j - 2];
        //         }
        //         std::sort(sort_buf_5, sort_buf_5 + 5);
        //         med_acc_x[i] = sort_buf_5[2];
        //
        //         for (size_t j = 0; j < 5; j++) {
        //             sort_buf_5[j] = acc_y[i + j - 2];
        //         }
        //         std::sort(sort_buf_5, sort_buf_5 + 5);
        //         med_acc_y[i] = sort_buf_5[2];
        //
        //         for (size_t j = 0; j < 5; j++) {
        //             sort_buf_5[j] = acc_z[i + j - 2];
        //         }
        //         std::sort(sort_buf_5, sort_buf_5 + 5);
        //         med_acc_z[i] = sort_buf_5[2];
        //     }
        // }

        MedianFilter<float, 5> medianFilter;
        for (size_t i = 0; i < WINDOW_SIZE; i++) {
            if (i >= 4) {
                med_acc_x[i - 2] = medianFilter.Insert(acc_x[i]);

            } else {
                medianFilter.Insert(acc_x[i]);
            }
        }
        for (size_t i = 0; i < WINDOW_SIZE; i++) {
            if (i >= 4) {
                med_acc_y[i - 2] = medianFilter.Insert(acc_y[i]);

            } else {
                medianFilter.Insert(acc_y[i]);
            }
        }
        for (size_t i = 0; i < WINDOW_SIZE; i++) {
            if (i >= 4) {
                med_acc_z[i - 2] = medianFilter.Insert(acc_z[i]);

            } else {
                medianFilter.Insert(acc_z[i]);
            }
        }

        // i == 0
        med_3(acc_x, med_acc_x, 0, 0);
        med_3(acc_y, med_acc_y, 0, 0);
        med_3(acc_z, med_acc_z, 0, 0);

        // i == 1
        med_4(acc_x, med_acc_x, 1, 1);
        med_4(acc_y, med_acc_y, 1, 1);
        med_4(acc_z, med_acc_z, 1, 1);

        // i == 126
        med_4(acc_x, med_acc_x, 126, 2);
        med_4(acc_y, med_acc_y, 126, 2);
        med_4(acc_z, med_acc_z, 126, 2);

        // i == 127
        med_3(acc_x, med_acc_x, 127, 2);
        med_3(acc_y, med_acc_y, 127, 2);
        med_3(acc_z, med_acc_z, 127, 2);

        // Serial.println("medianed");
        // vec_print_test(med_acc_x, med_acc_y, med_acc_z);

        // Butterworth Filter
        filtfilt(butter_b, butter_a, med_acc_x, butter_acc_x);
        filtfilt(butter_b, butter_a, med_acc_y, butter_acc_y);
        filtfilt(butter_b, butter_a, med_acc_z, butter_acc_z);

        // Serial.println("buttered");
        // vec_print_test(butter_acc_x, butter_acc_y, butter_acc_z);

        // Serial.print("mean x=");
        // Serial.print(mean_x);
        // Serial.print("mean y=");
        // Serial.print(mean_y);
        // Serial.print("mean z=");
        // Serial.print(mean_z);
        // Serial.print("std x=");
        // Serial.print(std_x);
        // Serial.print("std y=");
        // Serial.print(std_y);
        // Serial.print("std z=");
        // Serial.println(std_z);
    }

    // if (IMU.gyroscopeAvailable()) {
    //     IMU.readGyroscope(g_x, g_y, g_z);
    //     IMU.readyAcceleration(a_x, a_y, a_z)

    //     Serial.print(x);
    //     Serial.print('\t');
    //     Serial.print(y);
    //     Serial.print('\t');
    //     Serial.println(z);
    // }

    // if (IMU.accelerationAvailable()) {
    //     IMU.readAcceleration(x, y, z);

    //     Serial.print(x);
    //     Serial.print('\t');
    //     Serial.print(y);
    //     Serial.print('\t');
    //     Serial.println(z);
    // }
}
