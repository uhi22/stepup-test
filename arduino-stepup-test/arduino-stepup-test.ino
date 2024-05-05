#define PIN_PWM 13

void setup() {
  digitalWrite(PIN_PWM, 0);
  pinMode(PIN_PWM, OUTPUT);
}

void loop() {
  digitalWrite(PIN_PWM, 1);
  delayMicroseconds(10);
  digitalWrite(PIN_PWM, 0);
  delayMicroseconds(10);
  //delay(100);
}
