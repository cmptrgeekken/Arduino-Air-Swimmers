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
   * Offset used when sending out packets
   */
  uint16_t txLowOffset;
  
  uint16_t txHighOffset;
 
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
};

class IR {
  public:
    IR(
      uint8_t rxPin,
      uint8_t txPin
    );
    ~IR();
    uint8_t rx(uint32_t *);
    uint8_t rx(uint32_t *, uint32_t);
    
  private:
    uint8_t rxPin;
    uint8_t txPin;
    void sendPulse(uint32_t);
  protected:
    IRConfig irConfig;
    void tx(uint32_t *);
    virtual uint8_t checksum(uint32_t *) = 0;
    void mark(int);
    void space(int);
    void enableIROut(int);
};

#endif
