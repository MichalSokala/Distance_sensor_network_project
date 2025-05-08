#include <Ultrasonic.h>
// Define all constants globally
#define FRAME_SIZE     5
#define START_MARKER   0b01111110   // 0x7E
#define END_MARKER     0b01111111   // 0x7F


Ultrasonic ultrasonic(2, 3);
int distance;

// Addresses. The address is specified using 4 bits
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
    digitalWrite(LED_BUILTIN, HIGH); // Turn on the LED
    delay(200);                      // Wait 200 ms
    digitalWrite(LED_BUILTIN, LOW);  // Turn off the LED
    delay(200);                      // Wait 200 ms before the next blink
  }
}

// The buildPacket() function creates a packet according to the scheme:
// [Start Marker] [Source (4 bits) | Destination (4 bits)] [Data (16 bits)] [End Marker]
void buildPacket(byte packet[FRAME_SIZE], byte src, byte dest, uint16_t data16) {
  packet[0] = START_MARKER;
  packet[1] = (byte)((src & 0x0F) << 4) | (byte)(dest & 0x0F);
  packet[2] = (byte)(data16 >> 8);
  packet[3] = (byte)(data16 & 0xFF);
  packet[4] = END_MARKER;
}


void processAndReply(byte myID, uint16_t replyData16) {
  byte packet[FRAME_SIZE];
  if (Serial1.available() >= FRAME_SIZE) {
    int n = Serial1.readBytes(packet, FRAME_SIZE);
    if (n == FRAME_SIZE &&
        packet[0] == START_MARKER &&
        packet[4] == END_MARKER) {

      byte src  = (packet[1] >> 4) & 0x0F;
      byte dest =  packet[1]       & 0x0F;
      if (dest == myID) {
        byte resp[FRAME_SIZE];
        buildPacket(resp, myID, src, replyData16);
        Serial1.write(resp, FRAME_SIZE);
        BlinkLED(1);
      }
    }
  }
}


//-----------------------------------------------------------------------------------------------------------------------------------------
// SETUP and LOOP
//-----------------------------------------------------------------------------------------------------------------------------------------
void setup() {
  Serial1.begin(9600);  // Initialize Serial1 for communication with S0
  pinMode(LED_BUILTIN, OUTPUT);  // Set up the LED for indication
  // If S1 has USB Serial for debugging, initialize it here (but S1 may not have USB)
}

void loop() {
  distance = ultrasonic.read();
  processAndReply(S1_ID, (uint16_t)distance);
}
