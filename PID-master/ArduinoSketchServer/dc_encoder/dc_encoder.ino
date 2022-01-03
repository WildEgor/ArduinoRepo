const int encoder_a = 3; // Pin 3
const int encoder_b = 2; // Pin 5
long encoder_pulse_counter = 0;
long direction = 1;

void encoderPinChangeA()
{
    encoder_pulse_counter += 1;
    direction = digitalRead(encoder_a) == digitalRead(encoder_b) ? -1 : 1;
}

void encoderPinChangeB()
{
    encoder_pulse_counter += 1;
    direction = digitalRead(encoder_a) != digitalRead(encoder_b) ? -1 : 1;
}

void setup() 
{
    Serial.begin(115200);
    pinMode(encoder_a, INPUT_PULLUP);
    pinMode(encoder_b, INPUT_PULLUP);
    digitalWrite(4,0);
    digitalWrite(5,1);
    analogWrite(9,0);
    attachInterrupt(0, encoderPinChangeA, CHANGE);
    attachInterrupt(1, encoderPinChangeB, CHANGE);
}

void loop()
{
    long speed = encoder_pulse_counter/1024.00*60; // For encoder plate with 1024 Pulses per Revolution
    Serial.print("RPM: ");
    Serial.println(direction*speed);
    encoder_pulse_counter = 0; // Clear variable just before counting again 
    delay(1000);
}
