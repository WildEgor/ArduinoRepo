#include <GyverPWM.h>
int buffers = 0;

void setup() 
{
 Serial.begin(115200);
  pinMode(4,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(9,OUTPUT);
  digitalWrite(4,HIGH);
  digitalWrite(5,LOW);
  PWM_resolution(9, 9, FAST_PWM);
}

void loop() {
  static int  s   = -1;  
  static int  pin = 13;  
  unsigned int  val =  0;          
  
  if (Serial.available() >0)
  {
  
  val = Serial.parseInt();

  switch (s) {
    case -1: 
      if (val>47 && val<90){
       s=10*(val-48);
     }
      if ((s>40 && s<210 && s!=90) || (s>250 && s!=340 && s!=400)){
       s=-1;
     }
    break;
    
    case 40:
      if (val>98 && val<167) {
        pin=val-97;              
        s=41; 
      }
      else {
        s=-1; 
      }
      break; 

      case 41:
      //buffers = buffers + val;
      PWM_set(pin, val);
      Serial.println(val);
      s=-1;
      break;
             
  }
  }
  /*
  int i;
  for (i = 0; i < 511; i += 2){
    Serial.println(i);
    PWM_set(9, i);
    delay(300);
  }
  i = 0;
  */
}
