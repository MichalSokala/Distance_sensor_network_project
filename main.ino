#include <Ultrasonic.h>

Ultrasonic ultrasonic(2, 3);

int distance;

void setup() {
    Serial.begin(9600);
}

void loop() {
    distance = ultrasonic.read();
    Serial.print("Distance in cm: ");
    Serial.println(distance);
    delay(1000);
}
