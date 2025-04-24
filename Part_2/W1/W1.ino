/***************************************************
 * W1: Master Node (ID = 0b1001)
 * Packet frame: [Start Marker] | [Address Byte] | [End Marker]
 * In our case the Address Byte is simply W1's ID, padded with leading zeros.
 * W1 sends a packet and waits 500 ms for a response.
 ***************************************************/

// Pin definitions for channels:
#define COMM_PIN_D2     2   // channel for first communication
#define COMM_PIN_D3     3   // channel for second communication

#define START_MARKER    0b01111110  // 0x7E
#define END_MARKER      0b01111111  // 0x7F

#define W1_ID           0b1001      // W1 ID = 1001
#define S2_ID           0b0011      // S2 ID (expected response) = 0011

#define BAUD_DELAY      4           // delay (4 ms per bit, approximately 300 baud)

//-------------
// Bit-banging functions for a given pin
//-------------

// Send a single byte through the specified pin
void sendByteOn(uint8_t comm_pin, uint8_t b) {
  pinMode(comm_pin, OUTPUT);
  // Start bit (LOW)
  digitalWrite(comm_pin, LOW);
  delay(BAUD_DELAY);
  // 8 bits, sending LSB first
  for (int i = 0; i < 8; i++) {
    digitalWrite(comm_pin, (b >> i) & 1);
    delay(BAUD_DELAY);
  }
  // Stop bit (HIGH)
  digitalWrite(comm_pin, HIGH);
  delay(BAUD_DELAY);
}

// Read a single byte through the specified pin
uint8_t readByteOn(uint8_t comm_pin) {
  // Wait until the line goes LOW (start of the start bit)
  while (digitalRead(comm_pin) == HIGH) { }
  delay(BAUD_DELAY / 2);  // center the start bit
  uint8_t b = 0;
  for (int i = 0; i < 8; i++) {
    delay(BAUD_DELAY);
    uint8_t bit = digitalRead(comm_pin);
    b |= (bit << i);
  }
  delay(BAUD_DELAY);  // delay for the stop bit
  return b;
}

//-------------
// Packet functions for a given pin
//-------------

// Send a packet through the specified pin
// Form the packet: [Start Marker] | [Address Byte] | [End Marker]
// Here the Address Byte is simply id, where id is an 8-bit value in which
// the upper 4 bits are always 0, and the lower 4 bits represent the ID.
void sendPacketOn(uint8_t comm_pin, uint8_t id) {
  sendByteOn(comm_pin, START_MARKER);
  sendByteOn(comm_pin, id);  // Send our own ID
  sendByteOn(comm_pin, END_MARKER);
}

// Read a packet (3 bytes) from the specified pin within timeout ms.
// If successfully read, returns the received address byte through idOut.
bool readPacketOn(uint8_t comm_pin, uint8_t &idOut, unsigned long timeout) {
  unsigned long startTime = millis();
  uint8_t packet[3];
  while (millis() - startTime < timeout) {
    if (digitalRead(comm_pin) == LOW) {  // start bit begins
      for (int i = 0; i < 3; i++) {
        packet[i] = readByteOn(comm_pin);
      }
      if (packet[0] == START_MARKER && packet[2] == END_MARKER) {
        idOut = packet[1];
        return true;
      }
    }
  }
  return false;
}

// Helper function for formatting a 4-bit value (with leading zeros)
String format4bit(byte value) {
  String s = "";
  for (int i = 3; i >= 0; i--) {
    s += String((value >> i) & 1);
  }
  return s;
}

//-------------
// Test function for a given pin
//-------------
// The test() function takes as parameter the pin (channel) through which communication is done.
void test(uint8_t comm_pin) {
  Serial.print("W1: Sending packet to pin ");
  Serial.println(comm_pin);
  // Send a packet with our own ID (W1_ID)
  sendPacketOn(comm_pin, W1_ID);
  
  // Switch the pin to INPUT_PULLUP mode for receiving
  pinMode(comm_pin, INPUT_PULLUP);
  
  uint8_t receivedID;
  bool gotPkt = readPacketOn(comm_pin, receivedID, 1000);  // wait 1000 ms
  if (gotPkt) {
    Serial.print("W1: Received packet with sender ID: ");
    Serial.println(format4bit(receivedID));
  } else {
    Serial.println("W1: No response received.");
  }
  
  // Return the pin to OUTPUT mode (idle HIGH)
  pinMode(comm_pin, OUTPUT);
  digitalWrite(comm_pin, HIGH);
}

// The blinkLED function takes a parameter "count" – the number of LED blinks.
void blinkLED(int count) {
  for (int i = 0; i < count; i++) {
    digitalWrite(LED_BUILTIN, HIGH); // Turn on the LED
    delay(200);                      // Wait 200 ms
    digitalWrite(LED_BUILTIN, LOW);  // Turn off the LED
    delay(200);                      // Wait 200 ms before the next blink
  }
}

//-------------
// SETUP and LOOP
//-------------
void setup() {
  Serial.begin(9600);            // USB Serial for debugging
  pinMode(LED_BUILTIN, OUTPUT);
  // Initialize both channels
  pinMode(COMM_PIN_D2, OUTPUT);
  digitalWrite(COMM_PIN_D2, HIGH);
  pinMode(COMM_PIN_D3, OUTPUT);
  digitalWrite(COMM_PIN_D3, HIGH);
  delay(1000);
  Serial.println("W1 starting...");
}

void loop() {
  // Test the channel on COMM_PIN_D2
  blinkLED(1);
  test(COMM_PIN_D2);
  delay(3000);
  // Test the channel on COMM_PIN_D3
  blinkLED(2);
  test(COMM_PIN_D3);
  delay(3000);
}
