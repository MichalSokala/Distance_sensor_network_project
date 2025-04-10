void setup(){
    Serial1.begin(9600);


}


void loop(){
    if (Serial1.available() > 0){
      incoming_message = Serial1.read();
      Serial1.println("Received a message: ");
      Serial1.println(incoming_message);
    }

}