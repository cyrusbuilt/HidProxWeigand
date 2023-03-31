/**
 * HidProxWiegand.h
 * Version 1.0.3
 * Author
 *  Cyrus Brunner
 *  Joseph Selby
 *
 * This library provides a means of reading RFID cards (fobs) in both HID 35bit
 * Corporate 1000 format, 32bit Wiegand and 26bit Wiegand format.
 */

#ifndef HidProxWiegand_h
#define HidProxWiegand_h

#include <Arduino.h>

#define MAX_READ_BITS 100             // Max number of read bits.
#define WIEGAND_WAIT_TIME 3000        // Time to wait for another Wiegand pulse.
#define CARD_FORMAT_CORPORATE_1000 35 // HID Corporate 100 card format (35bit).
#define CARD_FORMAT_WIEGAND_26    26  // Standard 26bit Wiegand format.
#define CARD_FORMAT_WIEGAND_32    32  // Standard 32bit Wiegand format.
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
 * @brief Helper method for attaching interrupt handlers for the two data lines the
 * reader uses to transmit data pulses.
 * @param pinData0        Pin for DATA 0.
 * @param pinData1        Pin for DATA 1.
 * @param int0Handler Callback method for handling interrupt for DATA 0.
 * @param int1Handler Callback method for handling interrupt for DATA 1.
 */
void HidProxWiegand_AttachReaderInterrupts(uint8_t pinData0, uint8_t pinData1, void (*int0Handler)(), void (*int1Handler)());

/**
 * @brief Proximity RFID info structure. This carries the necessary info needed for
 * each reader attached to the system and callback methods for updating the
 * necessary values.
 */
struct ProxReaderInfo {
    /**
     * @brief The pin for data line 0.
     */
    short pinData0;

    /**
     * @brief The pin for data line 1.
     */
    short pinData1;

    /**
     * @brief Flag to indicate the read is done.
     */
    bool flagDone;

    /**
     * @brief Flag to indicate that the card read is of an unsupported format.
     */
    bool cardUnsupported;

    /**
     * @brief The facility code. Will be zero unless a valid code is read.
     */
    unsigned long facilityCode;

    /**
     * @brief The card code. Will be zero unless a valid code is read.
     */
    unsigned long cardCode;

    /**
     * @brief The number of bits read.
     */
    uint8_t bitCount;

    /**
     * @brief Countdown until it is assumed there are no more bits to read.
     */
    uint16_t wiegandCounter;

    /**
     * @brief Buffer for storing the bits read.
     */
    unsigned char databits[MAX_READ_BITS];

    /**
     * @brief Event callback method that fires when a card is read.
     */
    void (*onCardRead)(ProxReaderInfo* reader);

    /**
     * @brief This should be called by an interrupt handler when DATA0 goes low (0 bit)
     */
    void ISR_Data0() {
        bitCount++;
        flagDone = false;
        wiegandCounter = WIEGAND_WAIT_TIME;
    }

    /**
     * @brief This should be called by an interrupt handler when DATA1 goes low (1 bit)
     */
    void ISR_Data1() {
        databits[bitCount] = 1;
        bitCount++;
        flagDone = false;
        wiegandCounter = WIEGAND_WAIT_TIME;
    }

    /**
     * @brief Default constructor. Sets default values.
     */
    ProxReaderInfo() {
        onCardRead = NULL;
        flagDone = false;
        cardUnsupported = false;
        facilityCode = 0;
        cardCode = 0;
        bitCount = 0;
        wiegandCounter = 0;
    }
};

/**
 * @brief HID card reader manager class. Provides facilities for adding card readers
 * and processing card reads.
 */
class HidProxWiegandClass {
public:
    /**
     * @brief Default class constructor.
     */
    HidProxWiegandClass();

    /**
     * @brief Creates a reader instance and adds it to the system.
     * @param pinData0 The DATA0 pin from the reader.
     * @param pinData1 The DATA1 pin from the reader.
     * @param onCardRead Callback method that fires when a card is read.
     * @return ProxReaderInfo A reference to a ProxReaderInfo representing the reader
     * that was added. Returns NULL if the reader could not be added due to
     * platform limitation.
     */
    ProxReaderInfo* addReader(short pinData0, short pinData1, void (*onCardRead)(ProxReaderInfo* reader));

    /**
     * @brief Processing loop. Checks the buffers for each attached reader and checks
     * to see if any cards have been read. Should be called in your sketch's
     * main program loop() method.
     */
    void loop();

    /**
     * @brief Gets a reference to the currently active reader.
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
     * @brief Sets the position in the reader array. This essentially selects the
     * reader to process.
     * @param position The position to set.
     */
    void setPosition(short position);
};

/**
 * @brief Global instance.
 */
extern HidProxWiegandClass HidProxWiegand;
#endif
