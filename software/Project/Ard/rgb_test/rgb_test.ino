void setup() {
    Serial.begin(9600);
    while (!Serial)
        ;

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(LEDR, OUTPUT);
    pinMode(LEDG, OUTPUT);
    pinMode(LEDB, OUTPUT);
}

int period = 1000;
unsigned long curr_time = 0;
int led_state = 0;

void loop() {
    if (millis() >= curr_time + period) {
        curr_time += period;
        switch (led_state) {
            case 0:
                digitalWrite(LEDR, LOW);
                digitalWrite(LEDG, HIGH);
                digitalWrite(LEDB, HIGH);
                Serial.println("R");
                led_state = 1;
                break;
            case 1:
                digitalWrite(LEDR, HIGH);
                digitalWrite(LEDG, LOW);
                digitalWrite(LEDB, HIGH);
                Serial.println("G");
                led_state = 2;
                break;
            case 2:
                digitalWrite(LEDR, HIGH);
                digitalWrite(LEDG, HIGH);
                digitalWrite(LEDB, LOW);
                Serial.println("B");
                led_state = 0;
                break;
            default:
                break;
        }
    }
}