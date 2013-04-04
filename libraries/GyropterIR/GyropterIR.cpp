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
#include "GyropterIR.h"

/**
 * Initialize the Gyropter IR class. Configures the IR interface with the
 * necessary parameters as determined by reverse-engineering the protocol.
 *
 * @param rxPin Pin hooked up to the IR receiver's data line
 */
GyropterIR::GyropterIR(uint8_t rxPin)
    : IR(rxPin, 0) 
{
    this->irConfig.startPulseDuration = 5000l;
    this->irConfig.gapDuration        = 120000l;
    this->irConfig.pulseGapDuration   = 1000l;
    this->irConfig.shortPulseDuration = 1000l;
    this->irConfig.longPulseDuration  = 2800l;
    this->irConfig.pulseTolerance     = 200l;
    this->irConfig.packetBits         = 21;
    this->irConfig.txFrequency        = 38;
    this->irConfig.pulseInType        = LOW;
    this->irConfig.hasChecksum        = 0;
}

/**
 * Converts an incoming packet to a standardized command packet, where directions
 * are converted to percentages.
 *
 * @param packet Pointer to the IR packet
 * @param commandPacket Output variable where the command packet is stored
 */
void GyropterIR::getCommandPacket(uint32_t *packet, GyropterIRCommand *commandPacket)
{
  GyropterIRPacket *irPacket = (GyropterIRPacket *)packet;
  
  // Initialize the command packet
  commandPacket->upPercent    = 0;
  commandPacket->downPercent  = 0;
  commandPacket->leftPercent  = 0;
  commandPacket->rightPercent = 0;
  
  if (irPacket->joyRightVertical < 4) {
    commandPacket->upPercent = (4 - irPacket->joyRightVertical) * 100 / 4;
  } else {
    commandPacket->downPercent = (irPacket->joyRightVertical - 4) * 100 / 4;
  }
  
  if (irPacket->joyRightHorizontal < 115) {
    commandPacket->rightPercent = (115 - irPacket->joyRightHorizontal) * 100 / 115;
  } else if (irPacket->joyRightHorizontal <= 215) {
    commandPacket->leftPercent = (irPacket->joyRightHorizontal - 115);
  }
  
  commandPacket->throttlePercent = irPacket->joyLeft * 100 / 63;
  
  commandPacket->lightToggle = irPacket->light;
}

/**
 * The GyropterIR packet does not have a checksum, so this method
 * just returns a false value.
 */
uint8_t GyropterIR::checksum(uint32_t *packet) 
{
  return FALSE;
}

/**
 * This method is not implemented in the GyropterIR library.
 */
void GyropterIR::sendPacket()
{

}
