/**
 * HidProxWeigand.cpp
 * Version 1.0b
 * Author
 *  Cyrus Brunner
 *  Joseph Selby
 *
 * This library provides a means of reading RFID cards (fobs) in both HID 35bit
 * Corporate 1000 format, 32bit Weigand and 26bit Weigand format.
 */

#include "HidProxWeigand.h"

void HidProxWeigand_AttachReaderInterrupts(uint8_t int0, uint8_t int1, void (*int0Handler)(), void (*int1Handler)()) {
    attachInterrupt(int0, int0Handler, FALLING);
    attachInterrupt(int1, int1Handler, FALLING);
}

HidProxWeigandClass::HidProxWeigandClass() {
    this->_readerCount = 0;
    this->_mallocSize = 0;
    this->_initialCapacity = sizeof(ProxReaderInfo);
}

void HidProxWeigandClass::setPosition(short position) {
    this->_currentReader = this->_readers + position;
}

ProxReaderInfo* HidProxWeigandClass::getCurrentReader() {
    return this->_currentReader;
}

ProxReaderInfo* HidProxWeigandClass::addReader(short pinData0, short pinData1, void (*onCardRead)(ProxReaderInfo* reader)) {
    if ((SUPPORTED_READERS > 0) && ((this->_readerCount + 1) > SUPPORTED_READERS)) {
        // We can't add any more readers because it would exceed the max number
        // of supported readers.
        return NULL;
    }

    if (this->_readerCount > 0) {
        // Determine if the buffer free space fits the next object.
        short newSize = (sizeof(ProxReaderInfo) * (this->_readerCount + 1));
        if (this->_mallocSize < newSize) {
            // Allocate more memory space.
            this->_mallocSize = newSize;
            this->_readers = (ProxReaderInfo*) realloc(this->_readers, this->_mallocSize);
        }
    }
    else {
        // Determine if initial capacity param fits the first object.
        if (this->_initialCapacity >= sizeof(ProxReaderInfo)) {
            this->_mallocSize = this->_initialCapacity;
        }
        else {
            this->_mallocSize = sizeof(ProxReaderInfo);
        }

        // Create buffer.
        this->_readers = (ProxReaderInfo*) malloc(this->_mallocSize);
    }

    this->setPosition(this->_readerCount);
    this->_currentReader->pinData0 = pinData0;
    this->_currentReader->pinData1 = pinData1;
    this->_currentReader->weigandCounter = WEIGAND_WAIT_TIME;
    this->_currentReader->onCardRead = onCardRead;
    pinMode(this->_currentReader->pinData0, INPUT);
    pinMode(this->_currentReader->pinData1, INPUT);

    this->_readerCount++;
    return this->_currentReader;
}

void HidProxWeigandClass::loop() {
    // Iterate over all of the registered readers and check to see if we've
    // gotten any card reads.
    for (this->_index = 0; this->_index < this->_readerCount; this->_index++) {
        this->setPosition(this->_index);

        // Wait to make sure that there have been no more data pulses before
        // processing data.
        if (!this->_currentReader->flagDone) {
            if (--this->_currentReader->weigandCounter == 0) {
                this->_currentReader->flagDone = true;
            }
        }

        // If we have bits and the weigand counter went out.
        bool unsupported = false;
        if ((this->_currentReader->bitCount > 0) && (this->_currentReader->flagDone)) {
            uint8_t facStartBit = 0;
            uint8_t facStopBit = 0;
            uint8_t cardStartBit = 0;
            uint8_t cardStopBit = 0;
            if (this->_currentReader->bitCount == CARD_FORMAT_CORPORATE_1000) {
                // 35bit HID Corporate 1000 format.
                // Facility code is bits 3 to 14.
                facStartBit = 2;
                facStopBit = 14;

                // Card code is bits 15 to 34.
                cardStartBit = 14;
                cardStopBit = 34;
            }
            else if (this->_currentReader->bitCount == CARD_FORMAT_WEIGAND_26) {
                // Standard 26bit weigand format.
                // Facility code is bits 2 to 9.
                facStartBit = 1;
                facStopBit = 9;

                // Card code is bits 10 to 25;
                cardStartBit = 9;
                cardStopBit = 25;
            }
            else if (this->_currentReader->bitCount == CARD_FORMAT_WEIGAND_32) {
                // Standard 32bit weigand format.
                // Facility code is bits 5 to 16.
                facStartBit = 4;
                facStopBit = 16;

                // Card code is bits 17 to 32;
                cardStartBit = 16;
                cardStopBit = 32;
            }
            else {
                // Unrecognized format.
                unsupported = true;

                // TODO add support for more card formats.
            }

            uint8_t i = 0;
            if (!unsupported) {
                // Get the facility code.
                for (i = facStartBit; i < facStopBit; i++) {
                    this->_currentReader->facilityCode <<= 1;
                    this->_currentReader->facilityCode |= this->_currentReader->databits[i];
                }

                // Get the card code.
                for (i = cardStartBit; i < cardStopBit; i++) {
                    this->_currentReader->cardCode <<= 1;
                    this->_currentReader->cardCode |= this->_currentReader->databits[i];
                }
            }

            // Fire the event if we have a handler callback.
            this->_currentReader->cardUnsupported = unsupported;
            if (this->_currentReader->onCardRead != NULL) {
                this->_currentReader->onCardRead(this->_currentReader);
            }

            // Cleanup and get ready for next card read.
            this->_currentReader->bitCount = 0;
            this->_currentReader->facilityCode = 0;
            this->_currentReader->cardCode = 0;
            for (i = 0; i < MAX_READ_BITS; i++) {
                this->_currentReader->databits[i] = 0;
            }
        }
    }
}

HidProxWeigandClass HidProxWeigand;
