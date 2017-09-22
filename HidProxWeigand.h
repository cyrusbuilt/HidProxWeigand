/**
 * HidProxWeigand.h
 * Version 1.0a
 * Author
 *  Cyrus Brunner
 *
 * This library provides a means of reading RFID cards (fobs) in both HID 35bit
 * Corporate 1000 format and 26bit Weigand format.
 */

#ifndef HidProxWeigand_h
#define HidProxWeigand_h

#include <Arduino.h>

#define MAX_READ_BITS 100             // Max number of read bits.
#define WEIGAND_WAIT_TIME 3000        // Time to wait for another weigand pulse.
#define CARD_FORMAT_CORPORATE_1000 35 // HID Corporate 100 card format (35bit).
#define CARD_FORMAT_WEIGAND_26    26  // Standard 26bit Weigand format.
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    // Mega 1280 and 2560 has 6 interrupts, thus can support 3 readers.
    #define SUPPORTED_READERS 3
#elif defined(__AVR_ATmega168__) ||defined(__AVR_ATmega168P__) ||defined(__AVR_ATmega328P__)
    // These (primarily Uno) only have 2 interrupts, thus can support 1 reader.
    #define SUPPORTED_READERS 1
#elif defined(__AVR_ATmega32U4__)
    // Teensy, Micro, Leonardo, etc, has 5 interrupts, thus can support 2 readers.
    #define SUPPORTED_READERS 2
#else
    // Here we assume we are running on some other board where any input pin
    // can be used as an interrupt (i.e. Due, Zero, MKR1000, 101, etc) and
    // therefore can attach as many readers as possible.
    #define SUPPORTED_READERS 0
#endif

/**
 * Helper method for attaching interrupt handlers for the two data lines the
 * reader uses to transmit data pulses.
 * @param int0        Interrupt for DATA 0.
 * @param int1        Interrupt for DATA 1.
 * @param int0Handler Callback method for handling interrupt for DATA 0.
 * @param int1Handler Callback method for handling interrupt for DATA 1.
 */
void HidProxWeigand_AttachReaderInterrupts(uint8_t int0, uint8_t int1, void (*int0Handler)(), void (*int1Handler)());

/**
 * Proximity RFID info structure. This carries the necessary info needed for
 * each reader attached to the system and callback methods for updating the
 * necessary values.
 */
struct ProxReaderInfo {
    /**
     * The pin for data line 0.
     */
    short pinData0;

    /**
     * The pin for data line 1.
     */
    short pinData1;

    /**
     * Flag to indicate the read is done.
     */
    bool flagDone;

    /**
     * The facility code. Will be zero unless a valid code is read.
     */
    unsigned long facilityCode;

    /**
     * The card code. Will be zero unless a valid code is read.
     */
    unsigned long cardCode;

    /**
     * The number of bits read.
     */
    uint8_t bitCount;

    /**
     * Countdown until it is assumed there are no more bits to read.
     */
    uint16_t weigandCounter;

    /**
     * Buffer for storing the bits read.
     */
    unsigned char databits[MAX_READ_BITS];

    /**
     * Event callback method that fires when a card is read.
     */
    void (*onCardRead)(ProxReaderInfo* reader);

    /**
     * This should be called by an interrupt handler when DATA0 goes low (0 bit)
     */
    void ISR_Data0() {
        bitCount++;
        flagDone = false;
        weigandCounter = WEIGAND_WAIT_TIME;
    }

    /**
     * This should be called by an interrupt handler when DATA1 goes low (1 bit)
     */
    void ISR_Data1() {
        databits[bitCount] = 1;
        bitCount++;
        flagDone = false;
        weigandCounter = WEIGAND_WAIT_TIME;
    }

    /**
     * Default constructor. Sets default values.
     */
    ProxReaderInfo() {
        onCardRead = NULL;
        flagDone = false;
        facilityCode = 0;
        cardCode = 0;
        bitCount = 0;
        weigandCounter = 0;
    }
};

/**
 * [HidProxWeigandClass description]
 */
class HidProxWeigandClass {
public:
    /**
     * Default class constructor.
     */
    HidProxWeigandClass();

    /**
     * Creates a reader instance and adds it to the system.
     * @param pinData0 The DATA0 pin from the reader.
     * @param pinData1 The DATA1 pin from the reader.
     * @param onCardRead Callback method that fires when a card is read.
     * @return A reference to a ProxReaderInfo representing the reader
     * that was added.
     */
    ProxReaderInfo* addReader(short pinData0, short pinData1, void (*onCardRead)(ProxReaderInfo* reader));

    /**
     * Processing loop. Checks the buffers for each attached reader and checks
     * to see if any cards have been read. Should be called in your sketch's
     * main program loop() method.
     */
    void loop();

    /**
     * Gets a reference to the currently active reader.
     * @return A reference to the currently active reader.
     */
    ProxReaderInfo* getCurrentReader();
private:
    short _readerCount;
    short _mallocSize;
    short _index;
    uint8_t _initialCapacity;
    ProxReaderInfo* _readers;
    ProxReaderInfo* _currentReader;

    /**
     * Sets the position in the reader array. This essentially selects the
     * reader to process.
     * @param position The position to set.
     */
    void setPosition(short position);
};

// Global instance.
extern HidProxWeigandClass HidProxWeigand;
#endif
