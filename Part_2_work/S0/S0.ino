// Define all constants globally
#define FRAME_SIZE     5
#define START_MARKER   0b01111110   // 0x7E
#define END_MARKER     0b01111111   // 0x7F

// Addresses. The address is specified using 4 bits
#define S0_ID          0b0000
#define S1_ID          0b0001
#define S2_ID          0b0010
#define S3_ID          0b0011
#define S4_ID          0b0100
#define S5_ID          0b0101
#define W1_ID          0b1001
#define W2_ID          0b1010

uint16_t data_S1;
char     info_W1_port_D2[5];
char     info_W1_port_D3[5];
char     info_W2_port_D2[5];
char     info_W2_port_D3[5];
char     info_S1[16];
char     info_W1[16];
char     info_W2[16];

// The blinkLED function takes a parameter "count" – the number of LED blinks.
void BlinkLED(int count) {
  for (int i = 0; i < count; i++) {
    digitalWrite(LED_BUILTIN, HIGH); // Turn on the LED
    delay(200);                      // Wait 200 ms
    digitalWrite(LED_BUILTIN, LOW);  // Turn off the LED
    delay(200);                      // Wait 200 ms before the next blink
  }
}

void BlinkLEDERR(int count) { //blinkLED function used to show that an error has occured
  for (int i = 0; i < count; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }
}



// [Start Marker] [Source (4 bits) | Destination (4 bits)] [Data (16 bits)] [End Marker]
void buildPacket(byte packet[FRAME_SIZE], byte src, byte dest, uint16_t data16) {
  packet[0] = START_MARKER;
  packet[1] = (byte)((src & 0x0F) << 4) | (byte)(dest & 0x0F);
  packet[2] = (byte)(data16 >> 8);
  packet[3] = (byte)(data16 & 0xFF);
  packet[4] = END_MARKER;
}

void nibbleToCStr(byte value, char outBuf[5]) {// takes last 4 bits of every byte of data and transforms it into a string
  byte nib = value & 0x0F;
  for (int i = 0; i < 4; ++i) {
    outBuf[i] = '0' + ((nib >> (3 - i)) & 1);
  }
  outBuf[4] = '\0';  
}

void clearSerial1Rx() {
  while (Serial1.available()) {
    Serial1.read();
  }
}


void sendRequest(byte src, byte dest, uint16_t requestData, const char* msgSent, const char* msgFail) {
  byte packet[FRAME_SIZE];
  buildPacket(packet, src, dest, requestData);

  Serial1.write(packet, FRAME_SIZE);

  Serial.println(msgSent);

  unsigned long startTime = millis();
  bool responseReceived = false;
  byte response[FRAME_SIZE];
  byte respSrc = 0;

  while (millis() - startTime < 4000) {
    if (Serial1.available() >= FRAME_SIZE) {
      int bytesRead = Serial1.readBytes(response, FRAME_SIZE);
      if (bytesRead == FRAME_SIZE &&
          response[0] == START_MARKER &&
          response[4] == END_MARKER &&
          ((response[1] & 0x0F) == src)) {
        respSrc = (response[1] >> 4) & 0x0F;
        responseReceived = true;
        break;
      }
    }
  }

  if (responseReceived) {
    BlinkLED(1);
    uint16_t receivedData = ((uint16_t)response[2] << 8)
                          | ((uint16_t)response[3]);
    Serial.println("Get response");
    if (respSrc == S1_ID) {
      strcpy(info_S1, "S1");
      Serial.print("Response from S1: ");
      Serial.println(receivedData);
      data_S1 = receivedData;
      // BlinkLED(1);
    }
    else if (respSrc == W1_ID) {
      strcpy(info_W1, "W1");
      byte highByte = response[2];
      byte lowByte  = response[3];

      nibbleToCStr(highByte, info_W1_port_D2);
      nibbleToCStr(lowByte,  info_W1_port_D3);

      Serial.print("W1 response, first 4 bits: ");
      Serial.println(info_W1_port_D2);
      Serial.print("W1 response, last 4 bits: ");
      Serial.println(info_W1_port_D3);
      // BlinkLED(2);
    }
    else if (respSrc == W2_ID) {
      strcpy(info_W2, "W2");
      byte highByte_W2 = response[2];
      byte lowByte_W2  = response[3];

      nibbleToCStr(highByte_W2, info_W2_port_D2);
      nibbleToCStr(lowByte_W2,  info_W2_port_D3);

      Serial.print("W2 response, first 4 bits: ");
      Serial.println(info_W2_port_D2);
      Serial.print("W2 response, last 4 bits: ");
      Serial.println(info_W2_port_D3);
      // BlinkLED(3);
    }
  }
  else {
    if (dest == S1_ID) {
      Serial.println("S0: No reply from S1");
      data_S1 = 0;
      strcpy(info_S1, "XX");
      // BlinkLEDERR(1);
    }
    else if (dest == W1_ID) {
      Serial.println("S0: No reply from W1");
      strcpy(info_W1_port_D2, "0000");
      strcpy(info_W1_port_D3, "0000");
      strcpy(info_W1, "XX");
      // BlinkLEDERR(2);
    }
    else if (dest == W2_ID) {
      Serial.println("S0: No reply from W2");
      strcpy(info_W2_port_D2, "0000");
      strcpy(info_W2_port_D3, "0000");
      strcpy(info_W2, "XX");
      // BlinkLEDERR(3);
    }
  }

  clearSerial1Rx();
}




//-----------------------------------------------------------------------------------------------------------------------------------------
// SETUP and LOOP
//-----------------------------------------------------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);             // USB Serial for debugging
  Serial1.begin(4800);            // Hardware Serial1 for communication with S1
  pinMode(LED_BUILTIN, OUTPUT);   // LED for error indication
  delay(1000);
}

void loop() {
  sendRequest(S0_ID, S1_ID, 0b1111111111111111, "S0: Sent 0xAB to S1", "S0: No reply from S1");
  delay(1000);
  clearSerial1Rx();
  delay(1000);
  sendRequest(S0_ID, W1_ID, 0b1111111111111111, "S0: Sent 0xAB to W1", "S0: No reply from W1");
  delay(1000);
  clearSerial1Rx();
  delay(1000);
  sendRequest(S0_ID, W2_ID, 0b1111111111111111, "S0: Sent 0xAB to W2", "S0: No reply from W2");
  delay(1000);
  clearSerial1Rx();
  delay(1000);

  Serial.println("<====================================================================================================>");
  Serial.println(info_S1 + String(" = ") + data_S1 + String(" ================================================================================ " + String("S0-----> PC")));
  Serial.println("                         ||                                              ||                         ");
  Serial.println(String("                         ")+ info_W1 + String("                                              ")+ info_W2 + String("                         "));
  Serial.println("                        /  \\                                            /  \\                        ");
  Serial.println("                       /    \\                                          /    \\                       ");
  Serial.println("                      /      \\                                        /      \\                      ");
  Serial.println(String("                    ") + info_W1_port_D2 + String("    ") + info_W1_port_D3 + String("                                    ") + info_W2_port_D2 + String("    ") + info_W2_port_D3 + String("                     ")); 
  Serial.println("<====================================================================================================>");
}