//arduino test file

unsigned long time;
  int on = 0;

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
  //delay(1000);
  int data_read = Serial.read();

  //Serial.print(data_read);
  //Serial.print("this is on ");
  //Serial.print(on);
  if (data_read == 97 && on == 0) { //if
    //Serial.print("this is correct");
    on = 1;
    digitalWrite(13,HIGH);
  }
  if (data_read == 98 && on == 1) {
    on = 0;
    digitalWrite(13,LOW);
  }
  else {
    //Serial.print("hello");
  }
}
