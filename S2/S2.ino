/***************************************************
 * S2: Sensor Node (ID = 0b0011)
 * Uses COMM_PIN (Digital Pin 2) in half-duplex mode.
 * It continuously listens on COMM_PIN.
 * When it receives a packet and finds that the Address Byte equals 0b1001,
 * it prints that it received a packet from W1 and then sends a response packet:
 * [Start Marker] | [S2's ID (0b0011)] | [End Marker]
 ***************************************************/

 #define COMM_PIN        2

 #define START_MARKER    0b01111110  // 0x7E
 #define END_MARKER      0b01111111  // 0x7F
 
 #define W1_ID           0b1001
 #define S2_ID           0b0011
 
 #define BAUD_DELAY      4           // 4 ms per bit (~300 baud)
 
 void sendByte(uint8_t b) {
   pinMode(COMM_PIN, OUTPUT);
   digitalWrite(COMM_PIN, LOW);
   delay(BAUD_DELAY);
   for (int i = 0; i < 8; i++) {
     digitalWrite(COMM_PIN, (b >> i) & 1);
     delay(BAUD_DELAY);
   }
   digitalWrite(COMM_PIN, HIGH);
   delay(BAUD_DELAY);
 }
 
 uint8_t readByte() {
   while (digitalRead(COMM_PIN) == HIGH) { }
   delay(BAUD_DELAY / 2);
   uint8_t b = 0;
   for (int i = 0; i < 8; i++) {
     delay(BAUD_DELAY);
     uint8_t bit = digitalRead(COMM_PIN);
     b |= (bit << i);
   }
   delay(BAUD_DELAY);
   return b;
 }
 
 // Function to send a 3-byte packet with S2's ID
 void sendPacket(uint8_t id) {
   sendByte(START_MARKER);
   sendByte(id);  // Address Byte = id (for example, 0b0011 → 00000011)
   sendByte(END_MARKER);
 }
 
 // Function to read a packet (3 bytes)
 // If a packet with the correct markers is received,
 // returns the address (Address Byte) in idOut.
 bool readPacket(uint8_t &idOut, unsigned long timeout) {
   unsigned long startTime = millis();
   uint8_t packet[3];
   while (millis() - startTime < timeout) {
     if (digitalRead(COMM_PIN) == LOW) {
       for (int i = 0; i < 3; i++) {
         packet[i] = readByte();
       }
       if (packet[0] == START_MARKER && packet[2] == END_MARKER) {
         idOut = packet[1];
         return true;
       }
     }
   }
   return false;
 }
 
 String format4bit(byte value) {
   String s = "";
   for (int i = 3; i >= 0; i--) {
     s += String((value >> i) & 1);
   }
   return s;
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
 
 void setup() {
   Serial.begin(9600);  // For debugging via USB, if available
   pinMode(COMM_PIN, INPUT_PULLUP);
   pinMode(LED_BUILTIN, OUTPUT);
   delay(1000);
   Serial.println("S2 starting...");
 }
 
 void loop() {
   uint8_t receivedID;
   if (readPacket(receivedID, 500)) {  // Wait up to 500 ms
     Serial.print("S2: Received packet with sender ID: ");
     Serial.println(format4bit(receivedID));
     
     // If the packet is received from W1 (the address should be 0b1001)
     if (receivedID == W1_ID) {
       // Sending a response, which consists of S2's ID
       pinMode(COMM_PIN, OUTPUT);
       sendPacket(S2_ID);
       pinMode(COMM_PIN, INPUT_PULLUP);
       blinkLED(1);
       Serial.println("S2: Response sent."); 
     }
   }
 }
 