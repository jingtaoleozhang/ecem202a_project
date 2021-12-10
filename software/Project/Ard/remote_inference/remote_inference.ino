#include <ArduinoBLE.h>
#include <Arduino_LSM9DS1.h>
#include <math.h>

#include <algorithm>

#include "MedianFilter.hpp"
#include "filtfilt.h"

const char* PERIPHERAL_NAME = "Ard_BLE_33_S";
const char* SERVICE_UUID = "00001234-0000-1000-8000-00805f9b34fb";
const char* TX_CHR_UUID = "00001111-0000-1000-8000-00805f9b34fb";
const char* RX_CHR_UUID = "00002222-0000-1000-8000-00805f9b34fb";

#define BYTES_SENT 3072
#define BYTES_PER_WRITE 512
#define FLOATS_PER_WRITE 128
#define NUM_SLICES 6

BLEService myService(SERVICE_UUID);
BLECharacteristic txChr(TX_CHR_UUID, BLERead | BLENotify, BYTES_SENT);
BLEIntCharacteristic rxChr(RX_CHR_UUID, BLEWriteWithoutResponse | BLEWrite);

int period = 100;
unsigned long curr_time = 0;

bool waiting_for_rcv = true;

bool was_read = false;
bool waiting_for_read = false;
int slice_idx = 0;
int transfer_count = 0;
float write_buf[FLOATS_PER_WRITE];

bool waiting_for_inference = false;

bool data_ready = false;

#define WINDOW_SIZE 128
float g_x, g_y, g_z;
float a_x, a_y, a_z;
float acc_x[WINDOW_SIZE];
float acc_y[WINDOW_SIZE];
float acc_z[WINDOW_SIZE];
float gyr_x[WINDOW_SIZE];
float gyr_y[WINDOW_SIZE];
float gyr_z[WINDOW_SIZE];
float data_buf[WINDOW_SIZE * 6];
// int meas_buf_idx = 0;
// const size_t window_size = 128;
#define MEDIAN_FILTER_WINDOW 5

vectord butter_b{0.52762438, 1.58287315, 1.58287315, 0.52762438};
vectord butter_a{1.0, 1.76004188, 1.18289326, 0.27805992};

unsigned long sample_start;
unsigned long sample_end;
unsigned long proc_end;
unsigned long transfer_end;
unsigned long inf_end;

void setup() {
    // put your setup code here, to run once:
    Serial.println("Remote Inference Test Begin");

    // Initialize serial and wait for port to open:
    Serial.begin(9600);
    while (!Serial) {
        ;  // wait for serial port to connect. Needed for native USB port only
    }

    pinMode(LED_BUILTIN, OUTPUT);

    // Initialize BLE
    if (!BLE.begin()) {
        Serial.println("starting BLE failed!");
        while (1)
            ;
    }

    // Initialize IMU
    if (!IMU.begin()) {
        Serial.println("init IMU fail");
        while (1)
            ;
    }

    BLE.setLocalName(PERIPHERAL_NAME);
    BLE.setAdvertisedService(myService);
    myService.addCharacteristic(txChr);
    myService.addCharacteristic(rxChr);
    BLE.addService(myService);

    BLE.setEventHandler(BLEConnected, onBLEConnected);
    BLE.setEventHandler(BLEDisconnected, onBLEDisconnected);
    txChr.setEventHandler(BLERead, onTxRead);
    rxChr.setEventHandler(BLEWritten, onRxWritten);

    BLE.advertise();
    Serial.println("Bluetooth device active, waiting for connections...");
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

void gather_data() {
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
    filtfilt(butter_b, butter_a, med, vec);
}

void process_data() {
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

    for (size_t i = 0; i < 128; i++) {
        data_buf[i] = proc_acc_x[i];
        data_buf[i + 1] = proc_acc_y[i];
        data_buf[i + 2] = proc_acc_z[i];
        data_buf[i + 3] = proc_gyr_x[i];
        data_buf[i + 4] = proc_gyr_y[i];
        data_buf[i + 5] = proc_gyr_z[i];
    }
}

void loop() {
    BLEDevice central = BLE.central();
    if (central) {
        while (central.connected()) {
            if (millis() >= curr_time + period) {
                curr_time += period;

                if (data_ready == true) {
                    if (waiting_for_rcv == true && was_read == false) {
                        // Serial.println("waiting");
                        // keep sending first slice of first sample while waiting for a receiever (pre
                        // connection)
                        for (size_t j = 0; j < FLOATS_PER_WRITE; j++) {
                            // fill buffer with slice from sample data
                            write_buf[j] = data_buf[slice_idx * FLOATS_PER_WRITE + j];
                        }
                        txChr.writeValue(write_buf, BYTES_PER_WRITE);
                        waiting_for_read = true;
                    } else if (was_read == true && waiting_for_inference == false) {
                        // Serial.print("slice:");
                        // Serial.println(slice_idx);

                        // fill buffer with slice from sample data
                        for (size_t j = 0; j < FLOATS_PER_WRITE; j++) {
                            write_buf[j] = data_buf[slice_idx * FLOATS_PER_WRITE + j];
                        }
                        // send the buffer
                        txChr.writeValue(write_buf, BYTES_PER_WRITE);

                        // will need to send 6 slices for an inference
                        slice_idx++;

                        was_read = false;
                        waiting_for_read = true;
                        if (slice_idx == NUM_SLICES) {
                            waiting_for_inference = true;
                            data_ready = false;
                        }
                    }
                } else if (waiting_for_inference == false) { //try collecting while waiting for inference
                    Serial.println("Sampling Start");
                    // This should take 2.56 seconds
                    sample_start = micros();
                    gather_data();  // gather raw data from IMU
                    sample_end = micros();
                    Serial.println("Sampling End");
                    process_data();  // median and butterworth filter raw data
                    proc_end = micros();
                    data_ready = true;
                }
            }
        }
    }
}

void onRxWritten(BLEDevice central, BLECharacteristic chr) {
    int val = rxChr.value();
    if (val == 0) {  // got ack
        // Serial.println("got ack");
        was_read = true;
        if (waiting_for_rcv == true) {
            slice_idx++;
            waiting_for_rcv = false;
        }
    } else {  // got inference
        inf_end = micros();
        Serial.print("Inference: ");
        switch (val - 1) {
            case 0:
                Serial.println("LAYING");
                break;
            case 1:
                Serial.println("WALKING");
                break;
            case 2:
                Serial.println("WALKING_UPSTAIRS");
                break;
            case 3:
                Serial.println("WALKING_DOWNSTAIRS");
                break;
            case 4:
                Serial.println("SITTING");
                break;
            case 5:
                Serial.println("STANDING");
                break;
            default:
                Serial.println("invalid index");
                break;
        }

        Serial.print("sampling=");
        Serial.print((float)(sample_end - sample_start) / 1000000);
        Serial.print(", data proc=");
        Serial.print((float)(proc_end - sample_end) / 1000000);
        // Serial.print(", copy=");
        // Serial.print((float)(transfer_end - proc_end) / 1000000);
        Serial.print(", inf=");
        Serial.print((float)(inf_end - proc_end) / 1000000);
        Serial.println();
        Serial.println();
        waiting_for_inference = false;
        slice_idx = 0;
    }
}

void onTxRead(BLEDevice central, BLECharacteristic chr) {}

void onBLEConnected(BLEDevice central) {
    Serial.print("Connected Event, ");
    Serial.println(central.address());
    digitalWrite(LED_BUILTIN, HIGH);
}

void onBLEDisconnected(BLEDevice central) {
    Serial.print("Disconnect event, ");
    Serial.println(central.address());
    digitalWrite(LED_BUILTIN, LOW);
    waiting_for_rcv = true;

    waiting_for_rcv = true;

    was_read = false;
    slice_idx = 0;
    transfer_count = 0;

    waiting_for_inference = false;

    data_ready = false;
}