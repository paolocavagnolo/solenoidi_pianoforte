// =========================================================================
// IMPOSTA QUI L'INDICE DELLA SCHEDA: 0 = Bassa, 1..5 = Centrali, 6 = Alta
// =========================================================================
#define BOARD_INDEX 1 

#define BUS_BAUD 11520
#define PWM_FREQ 20000
#define PWM_RESOLUTION 8
#define MIN_STRIKE_MS 10UL
#define MAX_STRIKE_MS 45UL
#define HOLD_DUTY 60

// Configurazione automatica in base a BOARD_INDEX
#if (BOARD_INDEX == 0)
  #define NUM_SOLENOIDS 15
  #define FIRST_NOTE 21
  #define LAST_NOTE  35
  const uint8_t pins[NUM_SOLENOIDS] = { 4, 13, 14, 16, 17, 21, 22, 25, 26, 27, 32, 33, 18, 19, 23 };
#elif (BOARD_INDEX == 6)
  #define NUM_SOLENOIDS 13
  #define FIRST_NOTE 96
  #define LAST_NOTE  108
  const uint8_t pins[NUM_SOLENOIDS] = { 4, 13, 14, 16, 17, 21, 22, 25, 26, 27, 32, 33, 18 };
#else
  #define NUM_SOLENOIDS 12
  #define FIRST_NOTE (36 + (BOARD_INDEX - 1) * 12)
  #define LAST_NOTE  (FIRST_NOTE + 11)
  const uint8_t pins[NUM_SOLENOIDS] = { 4, 13, 14, 16, 17, 21, 22, 25, 26, 27, 32, 33 };
#endif

enum SolenoidState { STATE_IDLE, STATE_STRIKE, STATE_HOLD };

struct Solenoid {
  uint8_t pin;
  SolenoidState state;
  unsigned long strikeStartTime;
  unsigned long strikeDuration;
};

Solenoid solenoids[NUM_SOLENOIDS];

void noteOn(uint8_t idx, uint8_t velocity) {
  if (idx >= NUM_SOLENOIDS) return;
  uint32_t strike_ms = map(velocity, 1, 127, MIN_STRIKE_MS, MAX_STRIKE_MS);
  solenoids[idx].strikeDuration = strike_ms;
  solenoids[idx].strikeStartTime = millis();
  solenoids[idx].state = STATE_STRIKE;
  ledcWrite(solenoids[idx].pin, 255);
}

void noteOff(uint8_t idx) {
  if (idx >= NUM_SOLENOIDS) return;
  solenoids[idx].state = STATE_IDLE;
  ledcWrite(solenoids[idx].pin, 0);
}

void updateSolenoids() {
  unsigned long now = millis();
  for (int i = 0; i < NUM_SOLENOIDS; i++) {
    if (solenoids[i].state == STATE_STRIKE) {
      if (now - solenoids[i].strikeStartTime >= solenoids[i].strikeDuration) {
        solenoids[i].state = STATE_HOLD;
        ledcWrite(solenoids[i].pin, HOLD_DUTY);
      }
    }
  }
}

// Parser MIDI seriale a macchina a stati non bloccante
void processSerialBus() {
  static uint8_t status = 0;
  static uint8_t note = 0;
  static uint8_t state = 0; // 0: attesa status, 1: attesa note, 2: attesa vel

  while (Serial.available()) {
    uint8_t b = Serial.read();

    if (b & 0x80) { // Byte di stato MIDI (bit 7 = 1)
      status = b;
      state = 1;
      continue;
    }

    if (state == 1) {
      note = b;
      state = 2;
    } else if (state == 2) {
      uint8_t velocity = b;
      state = 1; // Pronto per eventuale "running status"

      // Controlla se la nota appartiene a questa scheda
      if (note >= FIRST_NOTE && note <= LAST_NOTE) {
        uint8_t localIdx = note - FIRST_NOTE;
        if (status == 0x90 && velocity > 0) {
          noteOn(localIdx, velocity);
        } else if (status == 0x80 || (status == 0x90 && velocity == 0)) {
          noteOff(localIdx);
        }
      }
    }
  }
}

void setup() {
  Serial.begin(BUS_BAUD);

  for (int i = 0; i < NUM_SOLENOIDS; i++) {
    solenoids[i].pin = pins[i];
    solenoids[i].state = STATE_IDLE;
    ledcAttach(solenoids[i].pin, PWM_FREQ, PWM_RESOLUTION);
    ledcWrite(solenoids[i].pin, 0);
  }
}

void loop() {
  processSerialBus();
  updateSolenoids();
}
