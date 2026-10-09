/* 
Impostazioni Arduino IDE per ESP32-S3:
Tools -> USB Mode: USB-OTG (TinyUSB)
Tools -> USB CDC On Boot: Enabled
*/

#include "USB.h"
#include "USBMIDI.h"

USBMIDI MIDI;

// Baud rate per il bus seriale tra Master e Slave
#define BUS_BAUD 115200 

// Invio pacchetto MIDI raw a 3 byte sul bus
void sendBusMidi(uint8_t status, uint8_t note, uint8_t velocity) {
  uint8_t packet[3] = { status, note, velocity };
  Serial0.write(packet, 3);
}

void onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
  // Filtra note esterne alla tastiera 88 tasti
  if (note < 21 || note > 108) return;

  if (velocity > 0) {
    sendBusMidi(0x90, note, velocity);
  } else {
    // Velocity 0 è interpretato come NoteOff
    sendBusMidi(0x80, note, 0);
  }
}

void onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
  if (note < 21 || note > 108) return;
  sendBusMidi(0x80, note, 0);
}

void setup() {
  // Serial0 usa i pin fisici TX (GPIO 43) e RX (GPIO 44)
  Serial0.begin(BUS_BAUD);

  // Configura USB MIDI nativo
  MIDI.setNoteOnCallback(onNoteOn);
  MIDI.setNoteOffCallback(onNoteOff);
  MIDI.begin();
  USB.begin();
}

void loop() {
}
