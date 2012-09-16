#include "IR.h"

IR::IR(uint8_t rxPin, uint8_t txPin) 
{
  this->rxPin = rxPin;
  this->txPin = txPin;
  
  if (this->rxPin > 0) {
    pinMode(this->rxPin, INPUT); 
  }
  
  if (this->txPin > 0) {
    pinMode(this->txPin, OUTPUT);
  }
}

IR::~IR() 
{
  
}

void IR::tx(uint32_t *packet)
{
  
}

uint8_t IR::rx(uint32_t *packet)
{
  return this->rx(packet, 0);
}

uint8_t IR::rx(uint32_t *packet, uint32_t timeout)
{
  uint32_t startTimeMillis = millis();
  uint32_t packetMask = (1UL << this->irConfig.packetBits) - 1;
  *packet = 0;
  
  while(!this->checksum(packet) && (timeout == 0 || (millis() - startTimeMillis < timeout))) {
    uint32_t pulse = pulseIn(this->rxPin, HIGH, 2 * this->irConfig.highPulseDuration);
    
    if (this->irConfig.startPulseDuration > 0 
      && pulse > this->irConfig.startPulseDuration - this->irConfig.pulseTolerance
      && pulse < this->irConfig.startPulseDuration + this->irConfig.pulseTolerance) {
      *packet = 0;
    } else if (pulse > this->irConfig.lowPulseDuration - this->irConfig.pulseTolerance 
      && pulse < this->irConfig.lowPulseDuration + this->irConfig.pulseTolerance) {
      *packet = (*packet << 1) & packetMask;
    } else if (pulse > this->irConfig.highPulseDuration - this->irConfig.pulseTolerance
      && pulse < this->irConfig.highPulseDuration + this->irConfig.pulseTolerance) {
      *packet = ((*packet << 1) | 1) & packetMask;
    }
  }
  
  if (this->checksum(packet)) {
    return 1;
  }

  return 0;
}
