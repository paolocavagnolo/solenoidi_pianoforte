#define solenoid_pin 5

void setup() {
  delay(100);
  pinMode(solenoid_pin, OUTPUT);
  delay(100);
}

void loop() {
  digitalWrite(solenoid_pin, HIGH);
  delay(1000);
  digitalWrite(solenoid_pin, LOW);
  delay(2000);
}
