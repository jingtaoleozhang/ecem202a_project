#include <ArduinoBLE.h>

//#include <valarray>

#include "imu_data.h"

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

int sample_data_idx = 0;

bool waiting_for_rcv = true;

bool was_read = false;
bool waiting_for_read = false;
int slice_idx = 0;
int transfer_count = 0;
float write_buf[FLOATS_PER_WRITE];

bool waiting_for_inference = false;

void setup() {
    Serial.begin(9600);
    while (!Serial)
        ;

    pinMode(LED_BUILTIN, OUTPUT);

    if (!BLE.begin()) {
        Serial.println("starting BLE failed!");
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

void loop() {
    BLEDevice central = BLE.central();

    if (central) {
        // Serial.print("Connected to central: ");
        // Serial.println(central.address());

        while (central.connected()) {
            if (millis() >= curr_time + period) {
                curr_time += period;

                if (waiting_for_rcv == true && was_read == false) {
                    //Serial.println("waiting");
                    // keep sending first slice of first sample while waiting for a receiever (pre connection)
                    for (size_t j = 0; j < FLOATS_PER_WRITE; j++) {
                        // fill buffer with slice from sample data
                        write_buf[j] = sample_imu_data[sample_data_idx][slice_idx * FLOATS_PER_WRITE + j];
                    }
                    txChr.writeValue(write_buf, BYTES_PER_WRITE);
                    waiting_for_read = true;
                } else if (was_read == true && waiting_for_inference == false) {
                    //Serial.print("slice:");
                    //Serial.println(slice_idx);

                    // fill buffer with slice from sample data
                    for (size_t j = 0; j < FLOATS_PER_WRITE; j++) {
                        write_buf[j] = sample_imu_data[sample_data_idx][slice_idx * FLOATS_PER_WRITE + j];
                    }
                    // send the buffer
                    txChr.writeValue(write_buf, BYTES_PER_WRITE);

                    // will need to send 6 slices for an inference
                    slice_idx++;

                    was_read = false;
                    waiting_for_read = true;
                    if (slice_idx == NUM_SLICES) {
                        waiting_for_inference = true;
                    }
                }
            }
        }
    }
}

void onRxWritten(BLEDevice central, BLECharacteristic chr) {
    int val = rxChr.value();
    /*
    Serial.print("Inference for ");
    Serial.print(sample_data_idx);
    Serial.print(":");
    Serial.println(val);
    */

    if (val == 0) {  // got ack
        //Serial.println("got ack");
        was_read = true;
        if (waiting_for_rcv == true) {
            slice_idx++;
            waiting_for_rcv = false;
        }
    } else if (val == 1) {  // got inference
        Serial.println("got inference");
        waiting_for_inference = false;
        sample_data_idx = (sample_data_idx + 1) % 5;
        slice_idx = 0;
    }
}

void onTxRead(BLEDevice central, BLECharacteristic chr) {
    /*
    Serial.println("tx read");
    was_read = true;

    if (waiting_for_rcv == true) {
        slice_idx++;
        waiting_for_rcv = false;
    }
    */
}

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
    sample_data_idx = 0;

    waiting_for_rcv = true;

    was_read = false;
    slice_idx = 0;
    transfer_count = 0;

    waiting_for_inference = false;
}
