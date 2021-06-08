#include <Wire.h>
const int Led = LED_BUILTIN;

void setup() {
  // put your setup code here, to run once:
  Wire.begin(4); 
  Wire.onReceive(dataGet);
  Wire.onRequest(giveData); 
  Serial.begin(9600); 
  pinMode(Led, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(100);
  //Wire.write(1);
}

void dataGet(int howMany){
  while(1 < Wire.available()){
    char c = Wire.read(); 
    Serial.print(c);
    if(c == 1){
      digitalWrite(Led, HIGH);
    }
    else{
      digitalWrite(Led, LOW);
    }
  }
  int x = Wire.read();
  Serial.print(x);
  if(x == 1){
    digitalWrite(Led, HIGH); 
  }
  else{
    digitalWrite(Led, LOW); 
  }
}

void giveData(int howMany){
  DDRB = DDRB | B00100000;
  PORTB = B00100000;
}
