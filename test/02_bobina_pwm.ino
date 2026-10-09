#define PIN_SOLENOIDE D3
#define MAX_TIME_VELOCITY_0 1000UL

void setup() {
  delay(1000);

  pinMode(PIN_SOLENOIDE, OUTPUT);
  digitalWrite(PIN_SOLENOIDE, LOW);
}

void loop() {
  noteOn(100);
  delay(1000);

  noteOff();
  delay(2000);
}


void noteOn(uint8_t velocity) {

  if (velocity >= 127) {
    analogWrite(PIN_SOLENOIDE, 255);
    return;
  }

  uint32_t max_dtime = MAX_TIME_VELOCITY_0 * 1000 / 51UL;
  uint32_t cal_dtime = map(velocity, 0, 126, max_dtime, 1);

  for (uint16_t i=0; i<255; i+=5) {
    analogWrite(PIN_SOLENOIDE, i);
    delayMicroseconds(cal_dtime);
  }

  digitalWrite(PIN_SOLENOIDE, HIGH);

}

void noteOff() {
  digitalWrite(PIN_SOLENOIDE, LOW);
}
