#include <ArduinoBLE.h>
#include <Arduino_LSM9DS1.h>

#define WINDOW_SIZE 128

float g_x, g_y, g_z;
float a_x, a_y, a_z;

float acc_x[WINDOW_SIZE];
float acc_y[WINDOW_SIZE];
float acc_z[WINDOW_SIZE];

float gyr_x[WINDOW_SIZE];
float gyr_y[WINDOW_SIZE];
float gyr_z[WINDOW_SIZE];
int meas_buf_idx = 0;

unsigned long curr_time = 0;
int period = 20;  // 50 Hz

void print_arr(float arr[]) {
    for (size_t i = 0; i < WINDOW_SIZE; i++) {
        Serial.print(arr[i]);
        Serial.print(", ");
    }
    Serial.println();
}

void setup() {
    // put your setup code here, to run once:

    Serial.println("IMU Test Begin");

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
    meas_buf_idx = 0;
}

void loop() {
    if (millis() >= curr_time + period) {
        curr_time += period;
        if (IMU.gyroscopeAvailable()) {
            IMU.readGyroscope(g_x, g_y, g_z);
            gyr_x[meas_buf_idx] = g_x;
            gyr_y[meas_buf_idx] = g_y;
            gyr_z[meas_buf_idx] = g_z;
        }
        if (IMU.accelerationAvailable()) {
            IMU.readAcceleration(a_x, a_y, a_z);
            acc_x[meas_buf_idx] = a_x;
            acc_y[meas_buf_idx] = a_y;
            acc_z[meas_buf_idx] = a_z;
        }
        meas_buf_idx++;

        if (meas_buf_idx == WINDOW_SIZE) {
            Serial.println("ACC X");
            //print_arr(acc_x);
            Serial.println("ACC Y");
            //print_arr(acc_y);
            Serial.println("ACC Z");
            //print_arr(acc_z);

            Serial.println("GYR X");
            //print_arr(gyr_x);
            Serial.println("GYR Y");
            //print_arr(gyr_y);
            Serial.println("GYR Z");
            //print_arr(gyr_z);
            meas_buf_idx = 0;
        }
    }
}
