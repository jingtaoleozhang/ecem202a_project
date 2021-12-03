#include <ArduinoBLE.h>
const char* PERIPHERAL_NAME = "Ard_BLE_33_S";
const char* SERVICE_UUID = "00001234-0000-1000-8000-00805f9b34fb";
const char* TX_CHR_UUID = "00001111-0000-1000-8000-00805f9b34fb";
const char* RX_CHR_UUID = "00002222-0000-1000-8000-00805f9b34fb";

BLEService myService(SERVICE_UUID);
BLEUnsignedCharCharacteristic txChr(TX_CHR_UUID, BLERead | BLENotify);
BLEUnsignedCharCharacteristic rxChr(RX_CHR_UUID, BLEWriteWithoutResponse | BLEWrite);

void setup() {
    Serial.begin(9600);
    while (!Serial)
        ;

    pinMode(LED_BUILTIN, OUTPUT);
    // pinMode(LEDR, OUTPUT);
    // pinMode(LEDG, OUTPUT);
    // pinMode(LEDB, OUTPUT);

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
    rxChr.setEventHandler(BLEWritten, onRxWritten);

    BLE.advertise();
    Serial.println("Bluetooth device active, waiting for connections...");
}

int period = 1000;
unsigned long curr_time = 0;
bool gotResponse = true;
int led_state = 0;

void loop() {
    BLEDevice central = BLE.central();

    if (central) {
        // Serial.print("Connected to central: ");
        // Serial.println(central.address());

        while (central.connected()) {
            if (millis() >= curr_time + period) {
                curr_time += period;

                if (gotResponse) {
                    int battery = analogRead(A0);
                    int batteryLevel = map(battery, 0, 1023, 0, 100);
                    Serial.print("Battery Level % is now: ");
                    Serial.println(batteryLevel);
                    txChr.writeValue(batteryLevel);
                    //gotResponse = false;
                }
            }
        }
    }
}

void onRxWritten(BLEDevice central, BLECharacteristic chr) {
    Serial.print("RX read: ");
    //byte val = rxChr.readValue(val);
    byte val = rxChr.value();
    Serial.println(val);
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
}