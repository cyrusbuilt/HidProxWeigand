# HidProxWeigand :: HID Proximity RFID reader library for Arduino
[![Build Status](https://github.com/cyrusbuilt/HidProxWeigand/actions/workflows/ci.yml/badge.svg)](https://github.com/cyrusbuilt/HidProxWeigand/actions?query=workflow%3APlatformIO)

## LICENSE

Licensed under GPL v3 (the "License"); you may not use this library except
in compliance with the License.  You may obtain a copy of the License at:

https://www.gnu.org/licenses/gpl-3.0.en.html

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

## Description
The goal of HidProxWeigand is to provide a hardware abstraction of an HID or
other Weigand-compatible RFID proximity card reader. The card is read using the
pulses received from the 2 data lines on the reader (DATA 0 and DATA 1) via
attached interrupts. The pulses are received and stored in a buffer. The buffer
is then processed to determine if a card was read.

## How to use
Each reader you add to the system will require at least 2 inputs to read a card
and at least to interrupts (one for each input). Depending on what Arduino (or
Arduino-compatible board) you are using may limit what interrupts are available
to you. As of this writing, the following is a list of known platforms and
their available interrupts:
* AVR ATmega1280 and ATmega2560-based: 6 interrupts = 3 possible readers.
* AVR ATmega168, ATmega168P, and ATmega328P: 2 interrupts = 1 possible reader.
* AVR ATmega32U4-based: 5 interrupts = 2 possible readers.
* Other than above, the library will assume the system is running on some other
platform that allows each input to have an interrupt such as the Zero, Due,
MKR1000, 101, etc.

Each reader also typically has 3 outputs: buzzer, green LED, and red LED. The
reader outputs are **not** handled by this library and should be implemented as
desired in your sketch. However, the ProxReaderInfo structure supports an event
callback that can be fired when a card is read and then how you want those
outputs to behave and can be handled in that callback method.

To use the library in your sketch:
Add the library header to your sketch. Declare a global ProxReaderInfo structure
reference for each reader attached to the system, declare a card read handler
method for each of those. You also want to declare an interrupt handler method
with no parameters for each interrupt being used. In your setup() method,
set each global ProxReaderInfo reference to the return of addReader() and pass
in both input pins and the card read handler method. In your interrupt handlers,
you should call the appropriate handler method for the input. Also in setup(),
call HidProxWeigand_AttachReaderInterrupts() to attach the declared interrupt
handlers for both inputs needed by the reader. The following is the code for
the basic example in the **examples** folder:

```cpp
#include <Arduino.h>
#include "HidProxWiegand.h"

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
    reader1 = HidProxWeigand.addReader(PIN_DATA0, PIN_DATA1, cardReadHandler);

    // Attach interrupt handlers for reader 1.
    HidProxWiegand_AttachReaderInterrupts(PIN_DATA0, PIN_DATA1, handleInterrupt0, handleInterrupt1);
}

void loop() {
    // Process any cards that have been read.
    HidProxWeigand.loop();
}
```

## How to install

If you are using PlatformIO (https://platformio.org/), you can simply run the
following command:
```
platformio lib install HidProxWeigand
```

Otherwise, download the zip from https://github.com/cyrusbuilt/HidProxWeigand/archive/master.zip
then extract its contents to a folder called "HidProxWeigand" and then move that
folder to your libraries folder.
