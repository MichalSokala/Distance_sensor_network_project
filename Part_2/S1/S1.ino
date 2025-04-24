// Define all constants globally
#define FRAME_SIZE     19
#define START_MARKER   0b01111110   // 0x7E
#define END_MARKER     0b01111111   // 0x7F

// Addresses. The address is specified using 4 bits
#define S0_ID          0b0000  // main board
#define S1_ID          0b0001  // sensor board

// The buildPacket() function creates a packet according to the scheme:
// [Start Marker] [Source (4 bits) | Destination (4 bits)] [Data (16 bytes)] [End Marker]
void buildPacket(byte packet[FRAME_SIZE], byte src, byte dest, const char* msg) {
  // 1. Start Marker (1 byte)
  packet[0] = START_MARKER;
  
  // 2. Addresses. In one byte: higher nibble = src, lower nibble = dest.
  packet[1] = (src << 4) | (dest & 0x0F);
  
  // 3. Data Field (16 bytes).
  int msgLen = strlen(msg); // Length of the message string
  for (int i = 0; i < 16; i++) {
    if (i < msgLen) {
      packet[2 + i] = msg[i];
    } else {
      packet[2 + i] = 0;  // Fill the rest with zeros
    }
  }
  
  // 4. End Marker (1 byte)
  packet[18] = END_MARKER;
}





void setup() {
  Serial1.begin(9600);  // Initialize Serial1 for communication with S0
  pinMode(LED_BUILTIN, OUTPUT);  // Set up the LED for indication
  // If S1 has USB Serial for debugging, initialize it here (but S1 may not have USB)
}

void loop() {
  byte packet[FRAME_SIZE];
  // Check if there are enough bytes to receive a packet
  if (Serial1.available() >= FRAME_SIZE) {
    int bytesRead = Serial1.readBytes(packet, FRAME_SIZE);
    
    // Check the start and end markers
    if (bytesRead == FRAME_SIZE &&
        packet[0] == START_MARKER &&
        packet[18] == END_MARKER) {
      
      // Check whether the destination field (lower nibble of the second byte) matches S1 ID
      byte dest = packet[1] & 0x0F;
      if (dest == S1_ID) {
        // Reset the request indication (LED blinking)
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        digitalWrite(LED_BUILTIN, LOW);
        
        // Form the response. In the response, source becomes S1 and destination becomes S0.
        byte responsePacket[FRAME_SIZE];
        buildPacket(responsePacket, S1_ID, S0_ID, "Hello");
        
        // Send the response through Serial1
        Serial1.write(responsePacket, FRAME_SIZE);
      }
    }
  }
  delay(50);
}
