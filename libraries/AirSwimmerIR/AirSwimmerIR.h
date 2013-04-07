/**
 * Air Swimmer IR
 *
 * This library provides functionality for sending and receiving IR packets from
 * the Air Swimmer remote-controlled shark (http://www.airswimmers.com/).
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
#ifndef AIR_SWIMMER_IR_H_
#define AIR_SWIMMER_IR_H_

#include <IR.h>

/**
 * Structure defining the layout of the AirSwimmer IR packet.
 *
 *         7654 3210 FEDC BA98 7654 3210
 * Btn     THES IGNA TURE BITS  DIRS  CKSM
 * Up    - 0110 1010 1011 1101 [1000] 0010
 * Down  - 0110 1010 1011 1101 [0100] 1110
 * Left  - 0110 1010 1011 1101 [0010] 1000
 * Right - 0110 1010 1011 1101 [0001] 1011
 * CKSM = DIRS ^ 1010
 */
struct AirSwimmerIRPacket {
  union {
    struct {
      unsigned commandPadding : 4;
      unsigned commandRight   : 1;
      unsigned commandLeft    : 1;
      unsigned commandDown    : 1;
      unsigned commandUp      : 1;
    };
    struct {
      unsigned checksum : 4;
      unsigned commands : 4;
    };
    uint8_t payloadByte;
  };
  uint16_t signature;
};

class AirSwimmerIR : public IR {
  public:
    AirSwimmerIR();
    
    void setSpeed(uint8_t);
    void prepareFlap(int8_t);
    void prepareDive(int8_t);
    void prepareSync(uint8_t);
    
  protected:
    volatile AirSwimmerIRPacket irPacket;
    volatile int8_t lastFlapDirection;
    volatile uint32_t currentFlapTime;
  
    uint8_t overrideDelay;
    int8_t flapDirection;
    int8_t diveDirection;
    uint8_t currentSpeed;  
    
    uint32_t lastCommandTime;
    uint8_t syncEnabled;
    
    
    virtual uint8_t checksum(uint32_t *);
    virtual void sendPacket();
};
#endif
