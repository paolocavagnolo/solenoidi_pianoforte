#define PIN_SOLENOIDE 18

#define PWM_FREQ 20000
#define PWM_RESOLUTION 8

#define MIN_STRIKE_MS 10UL
#define MAX_STRIKE_MS 45UL

#define HOLD_DUTY 64

void setup() {

  ledcSetup(0, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(PIN_SOLENOIDE, 0);

  ledcWrite(PIN_SOLENOIDE, 0);
  delay(1000);
}

void loop() {

  noteOn(20);
  delay(1000);

  noteOff();
  delay(2000);

  noteOn(120);
  delay(1000);

  noteOff();
  delay(2000);
}

void noteOn(uint8_t velocity) {
  if (velocity == 0) {
    noteOff();
    return;
  }

  uint32_t strike_ms = map(velocity, 1, 127, MIN_STRIKE_MS, MAX_STRIKE_MS);

  ledcWrite(PIN_SOLENOIDE, 255);
  delay(strike_ms);

  ledcWrite(PIN_SOLENOIDE, HOLD_DUTY);
}

void noteOff() {
  ledcWrite(PIN_SOLENOIDE, 0);
}
