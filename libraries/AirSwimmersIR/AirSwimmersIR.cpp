#include "AirSwimmersIR.h"

#define AIRSWIMMERS_IR_PACKET_SIZE 24

#define AIRSWIMMERS_IR_SIGNATURE 0b0110101010111101
#define AIRSWIMMERS_IR_CHECKSUM_BASE 0b1010

AirSwimmersIR::AirSwimmersIR(uint8_t rxPin, uint8_t txPin)
    : IR(rxPin, txPin) 
{
    this->irConfig.startPulseDuration = 0;
    this->irConfig.gapDuration        = 0;
    this->irConfig.lowPulseDuration   = 150;
    this->irConfig.highPulseDuration  = 650;
    this->irConfig.pulseTolerance     = 100;
    this->irConfig.packetBits         = 24;
}

AirSwimmersIRPacket *AirSwimmersIR::getPacket(uint32_t *packet) 
{
  return (AirSwimmersIRPacket *)packet;
}

uint8_t AirSwimmersIR::checksum(uint32_t *packet) 
{
  AirSwimmersIRPacket *irPacket = this->getPacket(packet);
  
  return irPacket->signature == AIRSWIMMERS_IR_SIGNATURE
           && (irPacket->commands ^ irPacket->checksum) == AIRSWIMMERS_IR_CHECKSUM_BASE;
}

uint8_t AirSwimmersIR::addChecksum(uint32_t *packet) 
{
  
  AirSwimmersIRPacket *irPacket = this->getPacket(packet);
  
  irPacket->checksum = irPacket->commands ^ AIRSWIMMERS_IR_CHECKSUM_BASE;
}