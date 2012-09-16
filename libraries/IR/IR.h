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
  uint16_t lowPulseDuration;
  
  /**
   * Duration of the pulse used to signify a high bit.
   */
  uint16_t highPulseDuration;
  
  /**
   * Idle time between packet reception.
   */
  uint16_t gapDuration;
  
  /**
   * Error tolerance allowed when measuring a pulse width.
   */
  uint16_t pulseTolerance;
  
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
    void tx(uint32_t *);
    uint8_t rx(uint32_t *);
    uint8_t rx(uint32_t *, uint32_t);
    
  private:
    uint8_t rxPin;
    uint8_t txPin;
  protected:
    IRConfig irConfig;
    virtual uint8_t checksum(uint32_t *) = 0;
    virtual uint8_t addChecksum(uint32_t *) = 0;
};

#endif
