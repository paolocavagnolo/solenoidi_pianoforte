// =========================================================================
// IMPOSTA QUI L'INDICE DELLA SCHEDA: 
// 0 = Bassa (15 note), 1..5 = Centrali (12 note), 6 = Alta (13 note)
// =========================================================================
#define BOARD_INDEX 1 

#define BUS_BAUD 115200
#define PWM_FREQ 20000
#define PWM_RESOLUTION 8
#define MIN_STRIKE_MS 10UL
#define MAX_STRIKE_MS 45UL
#define HOLD_DUTY 60

#ifndef LED_BUILTIN
  #define PIN_LED 2 // Tipico LED blu/rosso montato sulle board ESP32 WROOM
#else
  #define PIN_LED LED_BUILTIN
#endif

// Assegnazione pin aggiornata: 23 al posto di 16, 5 al posto di 17
// Pin 16 è riservato esclusivamente a Serial2 RX
#if (BOARD_INDEX == 0)
  #define NUM_SOLENOIDS 15
  #define FIRST_NOTE 21
  #define LAST_NOTE  35
  // 12 pin base + 3 pin extra (18, 19 e 17 come GPIO libero)
  const uint8_t pins[NUM_SOLENOIDS] = { 4, 13, 14, 23, 5, 21, 22, 25, 26, 27, 32, 33, 18, 19, 17 };
#elif (BOARD_INDEX == 6)
  #define NUM_SOLENOIDS 13
  #define FIRST_NOTE 96
  #define LAST_NOTE  108
  // 12 pin base + 1 pin extra (18)
  const uint8_t pins[NUM_SOLENOIDS] = { 4, 13, 14, 23, 5, 21, 22, 25, 26, 27, 32, 33, 18 };
#else
  #define NUM_SOLENOIDS 12
  #define FIRST_NOTE (36 + (BOARD_INDEX - 1) * 12)
  #define LAST_NOTE  (FIRST_NOTE + 11)
  const uint8_t pins[NUM_SOLENOIDS] = { 4, 13, 14, 23, 5, 21, 22, 25, 26, 27, 32, 33 };
#endif

enum SolenoidState { STATE_IDLE, STATE_STRIKE, STATE_HOLD };

struct Solenoid {
  uint8_t pin;
  SolenoidState state;
  unsigned long strikeStartTime;
  unsigned long strikeDuration;
};

Solenoid solenoids[NUM_SOLENOIDS];

// Gestione non bloccante lampeggio LED
unsigned long ledOffTime = 0;

void triggerLed() {
  digitalWrite(PIN_LED, HIGH);
  ledOffTime = millis() + 30; // Rimane acceso 30 ms
}

void updateLed() {
  if (ledOffTime != 0 && millis() >= ledOffTime) {
    digitalWrite(PIN_LED, LOW);
    ledOffTime = 0;
  }
}

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

// Ricezione da bus seriale su Serial2
void processSerialBus() {
  static uint8_t status = 0;
  static uint8_t note = 0;
  static uint8_t state = 0;

  while (Serial2.available()) {
    uint8_t b = Serial2.read();

    if (b & 0x80) {
      status = b;
      state = 1;
      continue;
    }

    if (state == 1) {
      note = b;
      state = 2;
    } else if (state == 2) {
      uint8_t velocity = b;
      state = 1;

      // Filtro per questa scheda
      if (note >= FIRST_NOTE && note <= LAST_NOTE) {
        triggerLed(); // ACCENDE IL LED SOLO SE IL PACCHETTO È PER QUESTA BOARD

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
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  // Inizializza Serial2 (usa di default GPIO 16 come RX)
  Serial2.begin(BUS_BAUD);

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
  updateLed();
}
