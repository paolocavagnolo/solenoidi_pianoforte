#define NUM_NOTES 12
#define PWM_FREQ 20000
#define PWM_RESOLUTION 8
#define MIN_STRIKE_MS 10UL // Pianissimo (velocity 1)
#define MAX_STRIKE_MS 45UL // Fortissimo (velocity 127)
#define HOLD_DUTY 60       // Potenza di tenuta (8 bit: 60 ≈ 24% del duty cycle)

enum SolenoidState {
  STATE_IDLE,
  STATE_STRIKE,
  STATE_HOLD
};

struct Solenoid {
  const char* name;
  uint8_t pin;
  SolenoidState state;
  unsigned long strikeStartTime;
  unsigned long strikeDuration;
};

Solenoid notes[NUM_NOTES] = {
  { "Do",   4,  STATE_IDLE, 0, 0 },
  { "Do#", 13,  STATE_IDLE, 0, 0 },
  { "Re",  14,  STATE_IDLE, 0, 0 },
  { "Re#", 16,  STATE_IDLE, 0, 0 }, // RX2
  { "Mi",  17,  STATE_IDLE, 0, 0 }, // TX2
  { "Fa",  21,  STATE_IDLE, 0, 0 },
  { "Fa#", 22,  STATE_IDLE, 0, 0 },
  { "Sol", 25,  STATE_IDLE, 0, 0 },
  { "Sol#",26,  STATE_IDLE, 0, 0 },
  { "La",  27,  STATE_IDLE, 0, 0 },
  { "La#", 32,  STATE_IDLE, 0, 0 },
  { "Si",  33,  STATE_IDLE, 0, 0 }
};

void setup() {
  delay(1000); 
  for (int i = 0; i < NUM_NOTES; i++) {
    ledcAttach(notes[i].pin, PWM_FREQ, PWM_RESOLUTION);
    ledcWrite(notes[i].pin, 0);
  }
  delay(1000);
}

void noteOn(uint8_t noteIndex, uint8_t velocity) {
  if (noteIndex >= NUM_NOTES) return;
  if (velocity == 0) {
    noteOff(noteIndex);
    return;
  }
  uint32_t strike_ms = map(velocity, 1, 127, MIN_STRIKE_MS, MAX_STRIKE_MS);
  notes[noteIndex].strikeDuration = strike_ms;
  notes[noteIndex].strikeStartTime = millis();
  notes[noteIndex].state = STATE_STRIKE;
  ledcWrite(notes[noteIndex].pin, 255);
}

void noteOff(uint8_t noteIndex) {
  if (noteIndex >= NUM_NOTES) return;
  notes[noteIndex].state = STATE_IDLE;
  ledcWrite(notes[noteIndex].pin, 0);
}

// Funzioni per comandare tutti i solenoidi insieme
void allNotesOn(uint8_t velocity) {
  for (int i = 0; i < NUM_NOTES; i++) {
    noteOn(i, velocity);
  }
}

void allNotesOff() {
  for (int i = 0; i < NUM_NOTES; i++) {
    noteOff(i);
  }
}

void updateSolenoids() {
  unsigned long now = millis();
  for (int i = 0; i < NUM_NOTES; i++) {
    if (notes[i].state == STATE_STRIKE) {
      if (now - notes[i].strikeStartTime >= notes[i].strikeDuration) {
        notes[i].state = STATE_HOLD;
        ledcWrite(notes[i].pin, HOLD_DUTY);
      }
    }
  }
}

unsigned long demoTimer = 0;
int demoStep = 0;
bool avanza = false;
int subStep = 0;

void loop() {
  updateSolenoids();

  unsigned long now = millis();
  if (now - demoTimer >= 100) {
    demoTimer = now;

    switch (demoStep) {
      case 0:
        // Rampa di accensione di tutti i solenoidi (1 nota ogni 100ms)
        noteOn(subStep, 100);
        subStep++;
        if (subStep >= NUM_NOTES) {
          avanza = true;
          subStep = 0;
        }
        break;

      case 1:
        // Aspetta 50x100 = 5000ms con tutti i solenoidi in HOLD
        subStep++;
        if (subStep >= 50) {
          avanza = true;
          subStep = NUM_NOTES - 1; // 11
        }
        break;

      case 2:
        // Rilascia con rampa a scendere (1 nota ogni 100ms)
        noteOff(subStep);
        subStep--;
        if (subStep < 0) {
          avanza = true;
          allNotesOff();
          subStep = 0;
        }
        break;

      case 3:
        // Pausa di riposo 10x100 = 1000ms 
        subStep++;
        if (subStep >= 10) {
          avanza = true;
          subStep = 0;
        }
        break;

      case 4:
        // BLINK LENTO TUTTI INSIEME (1s ON, 1s OFF) per 10 secondi
        // Ciclo da 20 tick (20 x 100ms = 2s):
        // tick 0: accende tutti (rimarranno in HOLD dopo lo strike)
        if (subStep % 20 == 0) {
          allNotesOn(100);
        }
        // tick 10: spegne tutti dopo 1 secondo (10 x 100ms)
        else if (subStep % 20 == 10) {
          allNotesOff();
        }

        subStep++;
        // 100 tick x 100ms = 10 secondi totali (5 cicli ON/OFF)
        if (subStep >= 100) {
          allNotesOff();
          avanza = true;
          subStep = 0;
        }
        break;

      case 5:
        // BLINK VELOCE TUTTI INSIEME (100ms ON / 100ms OFF) per 5 secondi
        // Alterna stato ad ogni tick da 100ms
        if (subStep % 2 == 0) {
          allNotesOn(100); // 100ms ON
        } else {
          allNotesOff();   // 100ms OFF
        }

        subStep++;
        // 50 tick x 100ms = 5000ms (5 secondi totali = 25 battute)
        if (subStep >= 50) {
          allNotesOff();
          avanza = true;
          subStep = 0;
        }
        break;
    }

    if (avanza) {
      avanza = false;
      demoStep = (demoStep + 1) % 6; // Riavvia dal case 0
    }
  }
}
