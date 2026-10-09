/* 
Impostazioni Arduino IDE per ESP32-S3:
Tools -> USB Mode: USB-OTG (TinyUSB)
Tools -> USB CDC On Boot: Enabled
*/

#include "USB.h"
#include "USBMIDI.h"

USBMIDI MIDI;

#define BUS_BAUD 115200

#ifndef LED_BUILTIN
  #define PIN_LED 2 // Pin LED predefinito
#else
  #define PIN_LED LED_BUILTIN
#endif

// Gestione non bloccante del lampeggio LED
unsigned long ledOffTime = 0;

void triggerLed() {
  digitalWrite(PIN_LED, HIGH);
  ledOffTime = millis() + 30; // Rimane acceso 30 ms (ben visibile)
}

void updateLed() {
  if (ledOffTime != 0 && millis() >= ledOffTime) {
    digitalWrite(PIN_LED, LOW);
    ledOffTime = 0;
  }
}

// Invia il pacchetto MIDI raw a 3 byte sul bus
void sendBusMidi(uint8_t status, uint8_t note, uint8_t velocity) {
  uint8_t packet[3] = { status, note, velocity };
  Serial0.write(packet, 3);
  triggerLed(); // Feedback visivo sul Master
}

void onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
  if (note < 21 || note > 108) return;

  if (velocity > 0) {
    sendBusMidi(0x90, note, velocity);
  } else {
    sendBusMidi(0x80, note, 0);
  }
}

void onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
  if (note < 21 || note > 108) return;
  sendBusMidi(0x80, note, 0);
}

void setup() {
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  // Serial0 usa i pin fisici TX (GPIO 43) ed RX (GPIO 44)
  Serial0.begin(BUS_BAUD);

  // Avvio USB-MIDI nativo
  MIDI.setNoteOnCallback(onNoteOn);
  MIDI.setNoteOffCallback(onNoteOff);
  MIDI.begin();
  USB.begin();
}

void loop() {
  updateLed();
}
