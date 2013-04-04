/**
 * Gyropter IR
 *
 * This library provides functionality for receiving IR packets from
 * the controller for the Propel Gyropter remote-controlled helicopter
 * (http://amzn.com/B00481GIDA/)
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
#ifndef GYROPTER_IR_H_
#define GYROPTER_IR_H_

#include <IR.h>

/**
 * IR packet structure for the Gyropter Remote
 * 
 * CH LFTJOY RGHTJOYSTICK 0 L
 * Packet Structure:
 * Bits 19-20: Channel (A = 0, B = 2, C = 1)
 * Bits 13-18: Joystick Left Position (0 = Full Down, 63 = Full Up)
 * Bits 5-12: Joystick Right Horizontal Position: (0 = Full Right, 115 = Middle, 215 = Full Left)
 * Bits 2-4: Joystick Right Vertical Position (Full Up = 0, Middle = 4, Full Down = 7)
 * Bit 1: Always 0
 * Bit 0: Light (1 = Toggle)
 *
 * Note: The additional padding at the end of the structure is so that the structure is
 * an even 24 bits in length.
 */
struct GyropterIRPacket {
  unsigned light              : 1;
  unsigned zeroPadding        : 1;
  unsigned joyRightVertical   : 3; // Up = 0, Mid = 4, Down = 7
  uint8_t  joyRightHorizontal : 8; // Right = 0, Mid = 115, Left = 215
  unsigned joyLeft            : 6; // Down = 0, Up = 63
  unsigned channel            : 2;
  unsigned unused             : 3; // Padding
};

/**
 * The GyropterIRCommand structure is used to abstract away the Gyropter IR packet,
 * so programs that utilize this library do not need to know the specific packet
 * structure.
 */
struct GyropterIRCommand {
   uint8_t upPercent;
   uint8_t downPercent;
   uint8_t leftPercent;
   uint8_t rightPercent;
   uint8_t throttlePercent;
   uint8_t lightToggle;
};

/**
 * The GyropterIR class encapsulates functionality for reading IR packets from the 
 * Gyropter helicopter remote. At this time, it is not possible to transmit
 * Gyropter IR packets.
 */
class GyropterIR 
  : public IR {
  public:
    GyropterIR(uint8_t);
    void getCommandPacket(uint32_t *, GyropterIRCommand *);
  protected:
    virtual uint8_t checksum(uint32_t *);
    virtual void sendPacket();
};
#endif