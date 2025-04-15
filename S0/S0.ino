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
      packet[2 + i] = 0;  // Fill the remainder with zeros
    }
  }
  
  // 4. End Marker (1 byte)
  packet[18] = END_MARKER;
}



void setup() {
  Serial.begin(9600);             // USB Serial for debugging
  Serial1.begin(9600);            // Hardware Serial1 for communication with S1
  pinMode(LED_BUILTIN, OUTPUT);   // LED for error indication
  delay(1000);
}

void loop() {
  // Form a request packet using the buildPacket() function.
  byte packet[FRAME_SIZE];
  buildPacket(packet, S0_ID, S1_ID, "Request");
  
  // Send the packet through Serial1 (binary)
  Serial1.write(packet, FRAME_SIZE);
  Serial.println("S0: Request packet sent.");
  
  // Wait up to 2000 ms for a response from S1
  unsigned long startTime = millis();
  bool responseReceived = false;
  byte response[FRAME_SIZE];
  
  while (millis() - startTime < 2000) {
    if (Serial1.available() >= FRAME_SIZE) {
      int bytesRead = Serial1.readBytes(response, FRAME_SIZE);
      // Check the received packet: start and end markers, and whether the recipient address is S0.
      if (bytesRead == FRAME_SIZE &&
          response[0] == START_MARKER &&
          response[18] == END_MARKER &&
          ((response[1] & 0x0F) == S0_ID)) {  // the lower nibble should equal the S0 address
        responseReceived = true;
        break;
      }
    }
  }
  
  if (responseReceived) {
    // Extract the data field (bytes 2-17)
    char dataField[17];
    for (int i = 0; i < 16; i++) {
      dataField[i] = (char)response[2 + i];
    }
    dataField[16] = '\0';
    Serial.print("S0: Response received: ");
    Serial.println(dataField);
  } else {
    Serial.println("S0: No valid response received from S1. Blinking LED 3 times.");
    for (int i = 0; i < 3; i++) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(200);
      digitalWrite(LED_BUILTIN, LOW);
      delay(200);
    }
  }
  
  delay(2000); // Delay before the next request
}
