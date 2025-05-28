#include <SPI.h>
#include <LoRa.h>
#include <Ultrasonic.h>

int counter = 0;

Ultrasonic ultrasonic(2, 3);
int distance;

void setup() {
  Serial.begin(9600);
  

  Serial.println("LoRa Sender");

  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.setTxPower(14);
}

void loop() {
  distance = ultrasonic.read();
  Serial.print("Sending packet: ");
  Serial.println(counter);
  Serial.println(distance);

  // send packet
  LoRa.beginPacket();
  LoRa.print("The distance is " + String(distance));
  LoRa.endPacket();

  counter++;

  delay(5000);
}
