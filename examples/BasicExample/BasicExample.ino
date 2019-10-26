/*
    Basic HidProxWiegand usage example.
 */

#include <Arduino.h>
#include "HidProxWiegand.h"

#define RDR_INT0 0         // Reader interrupt for DATA 0.
#define RDR_INT1 1         // Reader interrupt for DATA 1.

#define PIN_DATA0 2        // The pin to use for DATA 0.
#define PIN_DATA1 3        // The pin to use for DATA 1.

ProxReaderInfo* reader1;   // Card reader 1.

// Callback for handling card reads.
void cardReadHandler(ProxReaderInfo* reader) {
    Serial.print(F("Card read: "));
    Serial.print(reader->facilityCode);
    Serial.print(F(":"));
    Serial.println(reader->cardCode);

    // Here you can add logic to see if the card that was read was actually
    // valid or whatever. For example: reading stored card numbers from EEPROM
    // and looking for a match. Ideal for access control applications.
    //
    // This would also be where you would want to change state of the buzzer
    // and LEDs on the reader.
}

// Handle interrupt for DATA 0.
void handleInterrupt0() {
    if (reader1 != NULL) {
        reader1->ISR_Data0();
    }
}

// Handle interrupt for DATA 1.
void handleInterrupt1() {
    if (reader1 != NULL) {
        reader1->ISR_Data1();
    }
}

void setup() {
    Serial.begin(9600);
    while (!Serial) {
        delay(10);
    }

    // Create the reader and attach it to the system.
    reader1 = HidProxWiegand.addReader(PIN_DATA0, PIN_DATA1, cardReadHandler);

    // Attach interrupt handlers for reader 1.
    HidProxWiegand_AttachReaderInterrupts(RDR_INT0, RDR_INT1, handleInterrupt0, handleInterrupt1);
}

void loop() {
    // Process any cards that have been read.
    HidProxWiegand.loop();
}
