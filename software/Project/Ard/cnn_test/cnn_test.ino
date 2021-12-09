//test using tensorflow on arduino

#include <TensorFlowLite.h>

#include "imu_data.h"
//#include "model_8_8.h"
#include "model_16_16.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

// Globals, used for compatibility with Arduino-style sketches.
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

int size_used = 0;

// The name of this function is important for Arduino compatibility.
void setup() {
    // Set up logging. Google style is to avoid globals or statics because of
    // lifetime uncertainty, but since this has a trivial destructor it's okay.
    // NOLINTNEXTLINE(runtime-global-variables)
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;

    // Map the model into a usable data structure. This doesn't involve any
    // copying or parsing, it's a very lightweight operation.
    // model = tflite::GetModel(_content_cnn_8_8_model_tflite);
    model = tflite::GetModel(cnn_16_16_model_tflite);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        TF_LITE_REPORT_ERROR(error_reporter,
                             "Model provided is schema version %d not equal "
                             "to supported version %d.",
                             model->version(), TFLITE_SCHEMA_VERSION);
        return;
    }

    // This pulls in all the operation implementations we need.
    // NOLINTNEXTLINE(runtime-global-variables)
    static tflite::AllOpsResolver resolver;

    // Build an interpreter to run the model with.
    static tflite::MicroInterpreter static_interpreter(model, resolver, tensor_arena, kTensorArenaSize,
                                                       error_reporter);

    size_used = static_interpreter.arena_used_bytes();
    Serial.print("area size=");
    Serial.println(size_used);

    interpreter = &static_interpreter;

    // Allocate memory from the tensor_arena for the model's tensors.
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
        return;
    }

    // Obtain pointers to the model's input and output tensors.
    input = interpreter->input(0);
    output = interpreter->output(0);

    // Keep track of how many inferences we have performed.
    inference_count = 0;
}

bool firstrun = true;
int data_idx = 0;

int period = 100;
unsigned long curr_time = 0;

int num_loops = 0;
unsigned long copy_sum = 0;
unsigned long inference_sum = 0;

// The name of this function is important for Arduino compatibility.
void loop() {
    if (millis() >= curr_time + period) {
        curr_time += period;

        // Serial.print("Data idx:");
        // Serial.print(data_idx);
        // Serial.print(", Input Size: ");
        // Serial.print((input->bytes / sizeof(float)));
        // Serial.print(", Output size:");
        // Serial.println((output->bytes / sizeof(float)));

        unsigned long loop_start = micros();

        for (int i = 0; i < 128 * 6; i++) {
            input->data.f[i] = sample_imu_data[data_idx][i];
        }

        unsigned long copy_time = micros();

        // Run inference, and report any error
        TfLiteStatus invoke_status = interpreter->Invoke();
        if (invoke_status != kTfLiteOk) {
            TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed\n");
            return;
        }

        unsigned long inference_time = micros();

        copy_sum += copy_time - loop_start;
        inference_sum += inference_time - copy_time;
        num_loops++;

        Serial.println(num_loops);
        Serial.print("Inf=");
        Serial.println(inference_time - copy_time);
        Serial.print("Avg Inf=");
        Serial.println((float)inference_sum / (float)num_loops / 1000000);

        // Serial.print("didx:");
        // Serial.println(data_idx);
        // for (int i = 0; i < 6; i++) {
        //     Serial.print(interpreter->output(0)->data.f[i]);
        //     Serial.print(", ");
        // }
        // Serial.println();

        data_idx = (data_idx + 1) % 2;
    }
}