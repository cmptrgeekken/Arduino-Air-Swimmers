#ifndef AIR_SWIMMERS_IR_H_
#define AIR_SWIMMERS_IR_H_

#include <IR.h>

/**
 * Structure defining the layout of the AirSwimmers IR packet.
 *
 *         7654 3210 FEDC BA98 7654 3210
 * Btn     THES IGNA TURE BITS DIRS CKSM
 * Up    - 0110 1010 1011 1101 1000 0010
 * Down  - 0110 1010 1011 1101 0100 1110
 * Left  - 0110 1010 1011 1101 0010 1000
 * Right - 0110 1010 1011 1101 0001 1011
 * CKSM = DIRS ^ 1010
 */
struct AirSwimmersIRPacket {
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

class AirSwimmersIR : public IR {
  public:
    AirSwimmersIR(uint8_t, uint8_t);
    AirSwimmersIRPacket *getPacket(uint32_t *packet);
  protected:
    virtual uint8_t checksum(uint32_t *);
    virtual uint8_t addChecksum(uint32_t *);
};
#endif