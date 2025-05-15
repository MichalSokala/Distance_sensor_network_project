#include <Arduino.h>
#include <wiring_private.h>    // for pinPeripheral()

// MKR WAN 1310 TX1 pin (use the built-in macro)
static const uint8_t TX1_PIN = PIN_SERIAL1_TX;

// Disable the UART1 TX driver (put the pin in high-impedance/input)
void disableUart1Tx() {
  pinMode(TX1_PIN, INPUT);
}

// Enable the UART1 TX driver (recover the pin as UART TX, idle = HIGH)
void enableUart1Tx() {
  pinMode(TX1_PIN, OUTPUT);
  digitalWrite(TX1_PIN, HIGH);             // ensure idle = HIGH
  pinPeripheral(TX1_PIN, PIO_SERCOM);      // attach SERCOM UART to that pin
  delayMicroseconds(5);                    // give SERCOM a moment to re-mux
}

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);

  // Start with TX disabled
  disableUart1Tx();

  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println("TX tri-state test ready");
}

void loop() {
  // Enable TX lines
  Serial.println(">> ENABLING TX");
  enableUart1Tx();
  digitalWrite(LED_BUILTIN, HIGH);
  // (measure TX1_PIN here: you should see ~3.3 V idle)
  delay(1000);

  // Disable TX lines
  Serial.println(">> DISABLING TX");
  disableUart1Tx();
  digitalWrite(LED_BUILTIN, LOW);
  // (measure TX1_PIN here: it should float / go high-Z)
  delay(1000);
}
