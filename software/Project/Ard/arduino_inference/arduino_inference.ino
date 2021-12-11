#include <Arduino_LSM9DS1.h>
#include <TensorFlowLite.h>
#include <math.h>

#include <algorithm>

#include "MedianFilter.hpp"
#include "filtfilt.h"
//#include "imu_ex_data.h"
//#include "imu_data.h"
//#include "model_8_8.h"
#include "model_12_12.h"
//#include "model_16_16.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

#define WINDOW_SIZE 128
#define OUTPUT_SIZE 6

// TFLite Globals, used for compatibility with Arduino-style sketches.
namespace {
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;
int inference_count = 0;

constexpr int kTensorArenaSize = 100000;  // 100 kB
uint8_t tensor_arena[kTensorArenaSize];
}  // namespace

float g_x, g_y, g_z;
float a_x, a_y, a_z;
float acc_x[WINDOW_SIZE];
float acc_y[WINDOW_SIZE];
float acc_z[WINDOW_SIZE];
float gyr_x[WINDOW_SIZE];
float gyr_y[WINDOW_SIZE];
float gyr_z[WINDOW_SIZE];
// int meas_buf_idx = 0;
// const size_t window_size = 128;
#define WINDOW_SIZE 128
#define MEDIAN_FILTER_WINDOW 5

vectord butter_b{0.52762438, 1.58287315, 1.58287315, 0.52762438};
vectord butter_a{1.0, 1.76004188, 1.18289326, 0.27805992};

unsigned long curr_time = 0;
int period = 20;  // sample at 50 Hz

void setup() {
    // put your setup code here, to run once:

    Serial.println("sensor data processing Test Begin");

    // Initialize serial and wait for port to open:
    Serial.begin(9600);
    while (!Serial) {
        ;  // wait for serial port to connect. Needed for native USB port only
    }

    // Initialize IMU
    if (!IMU.begin()) {
        Serial.println("init IMU fail");
        while (1)
            ;
    }

    // Inintialize TFLite micro
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;

    // model = tflite::GetModel(cnn_16_16_model_tflite);
    model = tflite::GetModel(cnn_12_12_model_tflite);
    // model = tflite::GetModel(_content_cnn_8_8_model_tflite);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        TF_LITE_REPORT_ERROR(error_reporter,
                             "Model provided is schema version %d not equal "
                             "to supported version %d.",
                             model->version(), TFLITE_SCHEMA_VERSION);
        return;
    }

    static tflite::AllOpsResolver resolver;

    static tflite::MicroInterpreter static_interpreter(model, resolver, tensor_arena, kTensorArenaSize,
                                                       error_reporter);

    interpreter = &static_interpreter;

    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
        return;
    }

    input = interpreter->input(0);
    output = interpreter->output(0);
}

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

void med_3(float arr[], vectord& vec, int idx, int offset) {
    float sort_buf_3[3];
    for (size_t j = 0; j < 3; j++) {
        sort_buf_3[j] = arr[idx + j - offset];
    }
    std::sort(sort_buf_3, sort_buf_3 + 3);
    vec[idx] = sort_buf_3[1];
}

void med_4(float arr[], vectord& vec, int idx, int offset) {
    float sort_buf_4[4];
    for (size_t j = 0; j < 4; j++) {
        sort_buf_4[j] = arr[idx + j - offset];
    }
    std::sort(sort_buf_4, sort_buf_4 + 4);
    vec[idx] = (sort_buf_4[1] + sort_buf_4[2]) / 2;
}

void process_arr(float arr[], vectord& vec) {
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

unsigned long sample_start;
unsigned long sample_end;
unsigned long proc_end;
unsigned long copy_end;
unsigned long inf_end;

unsigned long loop_sum = 0;
unsigned long sample_sum = 0;
unsigned long proc_sum = 0;
unsigned long inf_sum = 0;
int num_loops = 0;

void loop() {
    if (millis() >= curr_time + period) {
        curr_time += period;
        Serial.println("Loop Start");

        sample_start = micros();

        int loop_start;
        for (size_t i = 0; i < WINDOW_SIZE; i++) {
            loop_start = millis();
            IMU.readAcceleration(a_x, a_y, a_z);
            IMU.readGyroscope(g_x, g_y, g_z);

            acc_x[i] = a_x;
            acc_y[i] = a_y;
            acc_z[i] = a_z;

            gyr_x[i] = g_x;
            gyr_y[i] = g_y;
            gyr_z[i] = g_z;
            delay(20 - (millis() - loop_start));
        }

        // sample_end = micros();

        vectord proc_acc_x(WINDOW_SIZE);
        vectord proc_acc_y(WINDOW_SIZE);
        vectord proc_acc_z(WINDOW_SIZE);

        vectord proc_gyr_x(WINDOW_SIZE);
        vectord proc_gyr_y(WINDOW_SIZE);
        vectord proc_gyr_z(WINDOW_SIZE);

        process_arr(acc_x, proc_acc_x);
        process_arr(acc_y, proc_acc_y);
        process_arr(acc_z, proc_acc_z);

        process_arr(gyr_x, proc_gyr_x);
        process_arr(gyr_y, proc_gyr_y);
        process_arr(gyr_z, proc_gyr_z);

        // proc_end = micros();

        for (size_t i = 0; i < 128; i++) {
            input->data.f[i] = proc_acc_x[i];
            input->data.f[i + 1] = proc_acc_y[i];
            input->data.f[i + 2] = proc_acc_z[i];
            input->data.f[i + 3] = proc_gyr_x[i];
            input->data.f[i + 4] = proc_gyr_y[i];
            input->data.f[i + 5] = proc_gyr_z[i];
        }

        // copy_end = micros();

        TfLiteStatus invoke_status = interpreter->Invoke();
        if (invoke_status != kTfLiteOk) {
            TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed\n");
            return;
        }

        // inf_end = micros();

        loop_sum += micros() - sample_start;
        num_loops++;
        Serial.print("Runtime=");
        Serial.println(((float)loop_sum) / num_loops / 1000000, 5);

        for (size_t i = 0; i < OUTPUT_SIZE; i++) {
            float e = interpreter->output(0)->data.f[i];
            switch (i) {
                case 0:
                    Serial.print("LAYING: ");
                    break;
                case 1:
                    Serial.print("WALKING: ");
                    break;
                case 2:
                    Serial.print("WALKING_UPSTAIRS: ");
                    break;
                case 3:
                    Serial.print("WALKING_DOWNSTAIRS: ");
                    break;
                case 4:
                    Serial.print("SITTING: ");
                    break;
                case 5:
                    Serial.print("STANDING: ");
                    break;
                default:
                    Serial.print("invalid index");
                    break;
            }
            Serial.print(e);
            Serial.print(", ");
        }

        // sample_sum += sample_end - sample_start;
        // proc_sum += proc_end - sample_end;
        // inf_sum += inf_end - copy_end;
        // num_loops++;
        // Serial.println();
        // Serial.print(num_loops);
        // Serial.print(": sample sum=");
        // Serial.print(((float)sample_sum) / num_loops / 1000000, 5);
        // Serial.print("data proc=");
        // Serial.print(((float)proc_sum) / num_loops / 1000000, 5);
        // Serial.print("inf=");
        // Serial.print(((float)inf_sum) / num_loops / 1000000, 5);
        // Serial.println();

        // Serial.println();
        // Serial.print("sampling=");
        // Serial.print((float)(sample_end - sample_start) / 1000000, 5);
        // Serial.print(", data proc=");
        // Serial.print((float)(proc_end - sample_end) / 1000000, 5);
        // Serial.print(", copy=");
        // Serial.print((float)(copy_end - proc_end) / 1000000, 5);
        // Serial.print(", inf=");
        // Serial.print((float)(inf_end - copy_end) / 1000000, 5);
        // Serial.println();
    }
}
