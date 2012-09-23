#include "AirSwimmersIR.h"

#define AIRSWIMMERS_IR_PACKET_SIZE 24

#define AIRSWIMMERS_IR_SIGNATURE 0b0110101010111101
#define AIRSWIMMERS_IR_CHECKSUM_BASE 0b1010

AirSwimmersIR::AirSwimmersIR(uint8_t rxPin, uint8_t txPin)
    : IR(rxPin, txPin) 
{
    this->irConfig.startPulseDuration = 0;
    this->irConfig.gapDuration        = 200000l;
    this->irConfig.pulseGapDuration   = 340;
    this->irConfig.shortPulseDuration = 220; // 150 for RX
    this->irConfig.longPulseDuration  = 720; // 650 for RX
    this->irConfig.pulseTolerance     = 100;
    this->irConfig.txLowOffset        = 0;
    this->irConfig.txHighOffset       = 0;
    this->irConfig.packetBits         = 24;
    this->irConfig.txFrequency        = 38;
    this->irConfig.pulseInType        = HIGH;
}

AirSwimmersIRPacket *AirSwimmersIR::getPacket(uint32_t *packet) 
{
  return (AirSwimmersIRPacket *)packet;
}

void AirSwimmersIR::sendPacket(AirSwimmersIRPacket *irPacket)
{
  irPacket->signature = AIRSWIMMERS_IR_SIGNATURE;
  irPacket->checksum  = irPacket->commands ^ AIRSWIMMERS_IR_CHECKSUM_BASE;
  
  this->tx((uint32_t *)irPacket);
}

uint8_t AirSwimmersIR::checksum(uint32_t *packet) 
{
  AirSwimmersIRPacket *irPacket = this->getPacket(packet);
  
  return irPacket->signature == AIRSWIMMERS_IR_SIGNATURE
           && (irPacket->commands ^ irPacket->checksum) == AIRSWIMMERS_IR_CHECKSUM_BASE;
}