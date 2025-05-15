#include <wiring_private.h>    // for pinPeripheral()

// MKR WAN 1310 TX1 pin (use the built-in macro)
static const uint8_t TX1_PIN = PIN_SERIAL1_TX;

#define COMM_PIN_D2     2   // channel for first communication
#define COMM_PIN_D3     3   // channel for second communication

#define FRAME_SIZE     5
#define START_MARKER   0b01111110   // 0x7E
#define END_MARKER     0b01111111   // 0x7F

#define BAUD_DELAY      4           // delay (4 ms per bit, approximately 300 baud)

#define S0_ID          0b0000
#define S1_ID          0b0001
#define S2_ID          0b0010
#define S3_ID          0b0011
#define S4_ID          0b0100
#define S5_ID          0b0101
#define W1_ID          0b1001
#define W2_ID          0b1010

// The blinkLED function takes a parameter "count" – the number of LED blinks.
void BlinkLED(int count) {
  for (int i = 0; i < count; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
  }
}

// Disable the UART1 TX driver (put the pin in high-impedance/input)
void disableUart1Tx() {
  pinMode(TX1_PIN, INPUT);
}

// Enable the UART1 TX driver (recover the pin as UART TX, idle = HIGH)
void enableUart1Tx() {
  pinMode(TX1_PIN, OUTPUT);
  digitalWrite(TX1_PIN, HIGH);             // ensure idle = HIGH
  pinPeripheral(TX1_PIN, PIO_SERCOM_ALT);      // attach SERCOM UART to that pin
  delayMicroseconds(5);                    // give SERCOM a moment to re-mux
}



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


// The test() function takes as parameter the pin (channel) through which communication is done.
void live_pin(byte expectedDest, uint8_t comm_pin, uint8_t &outID) {
  Serial.print("W2: Sending packet to pin ");
  Serial.println(comm_pin);
  // Send a packet with our own ID (W1_ID)
  sendPacketOn(comm_pin, expectedDest);
  
  // Switch the pin to INPUT_PULLUP mode for receiving
  pinMode(comm_pin, INPUT_PULLUP);
  
  uint8_t receivedID;
  bool gotPkt = readPacketOn(comm_pin, receivedID, 1000);  // wait 1000 ms
  if (gotPkt) {
    Serial.print("W2: Received packet with sender ID: ");
    Serial.println(format4bit(receivedID));
    outID = receivedID;
  } else {
    Serial.println("W2: No response received.");
    outID = 0x0000;
  }
  
  // Return the pin to OUTPUT mode (idle HIGH)
  pinMode(comm_pin, OUTPUT);
  digitalWrite(comm_pin, HIGH);
}

void buildPacket(byte packet[FRAME_SIZE], byte src, byte dest, uint16_t data16) {
  packet[0] = START_MARKER;
  packet[1] = (byte)((src & 0x0F) << 4) | (byte)(dest & 0x0F);
  packet[2] = (byte)(data16 >> 8);
  packet[3] = (byte)(data16 & 0xFF);
  packet[4] = END_MARKER;
}




void processAndReply(byte myID, byte replyHigh, byte replyLow) {
  byte packet[FRAME_SIZE];
  if (Serial1.available() >= FRAME_SIZE) {
    int n = Serial1.readBytes(packet, FRAME_SIZE);
    if (n == FRAME_SIZE &&
        packet[0] == START_MARKER &&
        packet[4] == END_MARKER) {

      byte src  = (packet[1] >> 4) & 0x0F;
      byte dest =  packet[1]       & 0x0F;
      if (dest == myID) {
        uint16_t replyData16 = ((uint16_t)replyHigh << 8) | replyLow;
        byte resp[FRAME_SIZE];
        buildPacket(resp, myID, src, replyData16);

        enableUart1Tx();
        delay(100);

        Serial1.write(resp, FRAME_SIZE);

        delay(100);
        disableUart1Tx();

        BlinkLED(1);
      }
    }
  }
}

//-----------------------------------------------------------------------------------------------------------------------------------------
// SETUP and LOOP
//-----------------------------------------------------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  Serial1.begin(4800);
  pinMode(LED_BUILTIN, OUTPUT);
  // Initialize both channels
  pinMode(COMM_PIN_D2, OUTPUT);
  digitalWrite(COMM_PIN_D2, HIGH);
  pinMode(COMM_PIN_D3, OUTPUT);
  digitalWrite(COMM_PIN_D3, HIGH);
  delay(1000);
}

void loop() {
  byte sensor_pin_D2;   
  live_pin(W2_ID, COMM_PIN_D2, sensor_pin_D2);
  Serial.println(format4bit(sensor_pin_D2));
  delay(200);
  byte sensor_pin_D3;   
  live_pin(W2_ID, COMM_PIN_D3, sensor_pin_D3);
  Serial.println(format4bit(sensor_pin_D3));
  delay(200);
  // processAndReply(W2_ID, sensor_pin_D2, sensor_pin_D3);
  // delay(300);

  enableUart1Tx();
  delay(50);
  processAndReply(W2_ID, sensor_pin_D2, sensor_pin_D3);
  delay(50);
  disableUart1Tx();
  delay(100);


}
