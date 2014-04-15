//arduino test file

unsigned long time;

// This method is called once
void setup(){
  // sets the transfer rate to 9600 baud
  Serial.begin(9600);
  
  // to access the LED light
  pinMode(13,OUTPUT);
}

// This method s called repeatedly
void loop(){

  // wait a second so as not to send massive amounts of data
  delay(1000);
  int data_read = Serial.read();
  //Serial.print(data_read);
  if (data_read == 97) { //when sends a 2 turn the light on
    //Serial.print("this is correct");
    digitalWrite(13,HIGH);
    
  }
  if (data_read == 2) {
    digitalWrite(13,LOW);
  }
  else {
    //Serial.print("hello");
  }
}