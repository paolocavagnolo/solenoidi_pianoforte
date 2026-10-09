#include "pwm.h"

#define PIN_SOLENOIDE D3

PwmOut solenoide(PIN_SOLENOIDE);

#define MIN_STRIKE_MS 10UL   // Pianissimo
#define MAX_STRIKE_MS 45UL   // Fortissimo

#define PERC_MANTENIMENTO 25.0f

void setup() {
  delay(1000);

  solenoide.begin(20000.0f, 0.0f);
 
  delay(1000);
}

void loop() {
  noteOn(30);
  delay(5000);

  noteOff();
  delay(2000);

  noteOn(120);
  delay(5000);

  noteOff();
  delay(2000);
}

void noteOn(uint8_t velocity) {
  if (velocity == 0) {
    noteOff();
    return;
  }

  uint32_t strike_ms = map(velocity, 1, 127, MIN_STRIKE_MS, MAX_STRIKE_MS);

  solenoide.pulse_perc(100.0f);
  delay(strike_ms);

  solenoide.pulse_perc(PERC_MANTENIMENTO);
}

void noteOff() {
  solenoide.pulse_perc(0.0f);
}
