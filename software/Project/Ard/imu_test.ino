#include <Arduino_LSM9DS1.h>
#include <ArduinoBLE.h>

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
}

float x, y, z;

void loop() {
    // put your main code here, to run repeatedly:
    
    if (IMU.gyroscopeAvailable()) {
        IMU.readGyroscope(x, y, z);

        Serial.print(x);
        Serial.print('\t');
        Serial.print(y);
        Serial.print('\t');
        Serial.println(z);
    }
    
    /*
    if (IMU.accelerationAvailable()) {
        IMU.readAcceleration(x, y, z);

        Serial.print(x);
        Serial.print('\t');
        Serial.print(y);
        Serial.print('\t');
        Serial.println(z);
    }
    */
}
