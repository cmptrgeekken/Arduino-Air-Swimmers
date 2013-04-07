/**
 * IR
 *
 * This library provides base functionality for sending and receiving IR packets.
 *
 * This project serves as partial fulfillment of my Master's 
 * of Science in Computer Science at Rochester Institute of Technology. 
 *
 * Created: 2013-03-24
 * Author: Ken Beck (http://geekken.net/)
 *
 * An in-depth analysis of this project can be found at:
 * http://blog.geekken.net/2013/03/23/masters-project-final-report/
 */ 
#ifndef _IR_H_
#define _IR_H_
#include <WProgram.h>
#include <Arduino.h>
#include <inttypes.h>

/**
 * Configuration object for the IR transmitter / receiver.
 */
struct IRConfig {
  /**
   * Duration of the start pulse. Not used in every protocol. Set
   * to 0 if unused.
   */
  uint16_t startPulseDuration;
  
  /**
   * Duration of the pulse used to signify a low bit. 
   */
  uint16_t shortPulseDuration;
  
  /**
   * Duration of the pulse used to signify a high bit.
   */
  uint16_t longPulseDuration;
  
  /**
   * Duration of the gap between each pulse
   */
  uint16_t pulseGapDuration;
  
  /**
   * Idle time between packet reception.
   */
  uint32_t gapDuration;
  
  /**
   * Error tolerance allowed when measuring a pulse width.
   */
  uint16_t pulseTolerance;
 
  /**
   * Type to use for reading pulses (either HIGH or LOW)
   */
  uint8_t pulseInType;
  
  /**
   * Frequency of the IR transmission
   */
  uint8_t txFrequency;
  
  /**
   * Number of bits in the IR packet. Cannot exceed 32.
   */
  uint8_t  packetBits;

  /**
   * If true, IR packet has a valid checksum routine
   */
  uint8_t hasChecksum;
};

class IR {
  public:
    IR(uint8_t, uint8_t);
    uint8_t rx(uint32_t *, uint32_t);
    
    void handleTx();
    
    void irOn();
    void irOff();
    void markPulse();
    void markGap();
    
  private:
    uint8_t rxPin;
    uint32_t packetBuffer;
    
    volatile uint8_t initialized;
    volatile uint8_t txEnabled;
    volatile uint8_t currentPacketBit;
    volatile uint8_t inPulseGap;
    volatile uint8_t startPulseSent;
    volatile uint16_t currentPulseDelay;
    volatile uint32_t lastBitTime;
    volatile uint32_t lastPacketTime;
    
    void sendPulse(uint32_t);
  protected:
    IRConfig irConfig;
    
    void tx(uint32_t *);
    virtual uint8_t checksum(uint32_t *) = 0;
    virtual void sendPacket() = 0;
    
    
    void enableIROut(int);
    uint32_t readPulse(uint16_t, uint16_t, uint32_t);
};

#endif
