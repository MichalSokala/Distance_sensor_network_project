#include <Ultrasonic.h>

Ultrasonic ultrasonic(2, 3);

int distance;

void setup() {
    Serial1.begin(9600);
}

void loop() {
    distance = ultrasonic.read();
    Serial1.write(distance);
    DEBUG_PRINT("sent distance to receiver");
    delay(1000);
    //Serial.print("Distance in cm: ");
    //Serial.println(distance);

}
