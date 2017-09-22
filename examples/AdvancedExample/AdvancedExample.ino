/*
    Advanced HidProxWeigand usage example.
 */

 #include <Arduino.h>
 #include "HidProxWeigand.h"

 #define RDR_INT0 0         // Reader 1 interrupt for DATA 0.
 #define RDR_INT1 1         // Reader 1 interrupt for DATA 1.
 #define RDR_INT2 2         // Reader 2 interrupt for DATA 0.
 #define RDR_INT3 3         // Reader 2 interrupt for DATA 1.

 #define PIN_RDR1_DATA0 2        // The pin to use for reader 1, DATA 0.
 #define PIN_RDR1_DATA1 3        // The pin to use for reader 1, DATA 1.
 #define PIN_RDR2_DATA0 4        // The pin to use for reader 2, DATA 0.
 #define PIN_RDR2_DATA1 5        // The pin to use for reader 2, DATA 1.

 // Reader 1 outputs.
 #define PIN_RDR1_BUZZER 6
 #define PIN_RDR1_GREEN_LED 7
 #define PIN_RDR1_RED_LED 8

 //Reader 2 outputs.
 #define PIN_RDR2_BUZZER 9
 #define PIN_RDR2_GREEN_LED 10
 #define PIN_RDR2_RED_LED 11

 ProxReaderInfo* reader1;   // Card reader 1.
 ProxReaderInfo* reader2;

 // Callback for handling card reads from reader 1.
 void cardRead1Handler(ProxReaderInfo* reader) {
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
     digitalWrite(PIN_RDR1_BUZZER, HIGH);
     digitalWrite(PIN_RDR1_RED_LED, LOW);
     digitalWrite(PIN_RDR1_GREEN_LED, HIGH);
     delay(500);
     digitalWrite(PIN_RDR1_BUZZER, LOW);
     digitalWrite(PIN_RDR1_GREEN_LED, LOW);
     digitalWrite(PIN_RDR1_RED_LED, HIGH);
 }

 void cardRead2Handler(ProxReaderInfo* reader) {
     Serial.print(F("Card read: "));
     Serial.print(reader->facilityCode);
     Serial.print(F(":"));
     Serial.println(reader->cardCode);

     digitalWrite(PIN_RDR2_BUZZER, HIGH);
     digitalWrite(PIN_RDR2_RED_LED, LOW);
     digitalWrite(PIN_RDR2_GREEN_LED, HIGH);
     delay(500);
     digitalWrite(PIN_RDR2_BUZZER, LOW);
     digitalWrite(PIN_RDR2_GREEN_LED, LOW);
     digitalWrite(PIN_RDR2_RED_LED, HIGH);
 }

 // Handle interrupt for reader 1, DATA 0.
 void handleInterrupt0() {
     if (reader1 != NULL) {
         reader1->ISR_Data0();
     }
 }

 // Handle interrupt for reader 1, DATA 1.
 void handleInterrupt1() {
     if (reader1 != NULL) {
         reader1->ISR_Data1();
     }
 }

 // Handle interrupt for reader 2, DATA 0.
 void handleInterrupt2() {
     if (reader2 != NULL) {
         reader2->ISR_Data0();
     }
 }

 // Handle interrupt for reader 2, DATA 1.
 void handleInterrupt3() {
     if (reader2 != NULL) {
         reader2->ISR_Data1();
     }
 }

 void setup() {
     Serial.begin(9600);
     while (!Serial) {
         delay(10);
     }

     // Create the readers and attach them to the system.
     reader1 = HidProxWeigand.addReader(PIN_RDR1_DATA0, PIN_RDR1_DATA1, cardRead1Handler);
     reader2 = HidProxWeigand.addReader(PIN_RDR2_DATA0, PIN_RDR2_DATA1, cardRead2Handler);

     // Attach interrupt handlers for readers 1 and 2.
     HidProxWeigand_AttachReaderInterrupts(RDR_INT0, RDR_INT1, handleInterrupt0, handleInterrupt1);
     HidProxWeigand_AttachReaderInterrupts(RDR_INT2, RDR_INT3, handleInterrupt2, handleInterrupt3);

     // Init reader 1 outputs.
     pinMode(PIN_RDR1_BUZZER, OUTPUT);
     digitalWrite(PIN_RDR1_BUZZER, LOW);
     pinMode(PIN_RDR1_GREEN_LED, OUTPUT);
     digitalWrite(PIN_RDR1_GREEN_LED, LOW);
     pinMode(PIN_RDR1_RED_LED, OUTPUT);
     digitalWrite(PIN_RDR1_RED_LED, HIGH);

     // Init reader 2 outputs.
     pinMode(PIN_RDR2_BUZZER, OUTPUT);
     digitalWrite(PIN_RDR2_BUZZER, LOW);
     pinMode(PIN_RDR2_GREEN_LED, OUTPUT);
     digitalWrite(PIN_RDR2_GREEN_LED, LOW);
     pinMode(PIN_RDR2_RED_LED, OUTPUT);
     digitalWrite(PIN_RDR2_RED_LED, HIGH);
 }

 void loop() {
     // Process any cards that have been read.
     HidProxWeigand.loop();
 }
