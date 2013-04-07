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
 *
 * PWM logic taken with modifications from:
 * https://github.com/shirriff/Arduino-IRremote/blob/master/IRremoteInt.h
 */
#include "IR.h"
#include <avr/interrupt.h>

#define SYSCLOCK 16000000  // main Arduino clock

#define TIMER_PWM_PIN 3

// Instance reference to the IR class. This is used solely for the TIMER2 ISR
IR *instance;

/**
 * Interrupt Service Routine configured to run on TIMER2 overflows. This needs to be
 * defined outside of the IR class, which leads to the requirement to have an
 * instance variable for the ISR to reference.
 */
ISR(TIMER2_OVF_vect)
{
  if (instance) {
    instance->handleTx();
  }
}

/**
 * Construct a new instance of the IR class. Sets the RX pin based
 * on the passed-in argument, and configure TX functionality if necessary
 *
 * @param rxPin Pin hooked up to the IR receiver's data line
 * @param enableTx Boolean flag indicating whether to enable TX
 */
IR::IR(uint8_t rxPin, uint8_t enableTx) 
  : initialized(0),
    txEnabled(0),
    currentPacketBit(0),
    inPulseGap(0),
    startPulseSent(0),
    currentPulseDelay(0),
    lastBitTime(0),
    lastPacketTime(0)
{
  this->rxPin = rxPin;
  
  // Set the pin mode for the RX pin
  if (this->rxPin > 0) {
    pinMode(this->rxPin, INPUT); 
  }
  
  if (enableTx) {
    // Set the instance variable for the ISR
    instance = this;
    
    // Let the timer know that the the class has been initialized,
    // and TX packets can be sent
    this->initialized = 1;
  }
}

/**
 * Method called by the TIMER2 Interrupt Service Routine. Transmits each
 * bit of the current TX packet.
 */
void IR::handleTx()
{
  // If the IR configuration's gap duration has expired since the last packet was sent,
  // call the 'sendPacket()' method, which initializes the IR packet for transmission
  if (this->lastPacketTime < millis() - 50/*(this->irConfig.gapDuration / 1000)*/) {
    this->lastPacketTime = millis();
    this->sendPacket();
  }
    
  // If TX is not enabled (for instance, if no commands have been received from the
  // IR controller), turn the IR transmitter off
  if (!this->txEnabled) {
    this->irOff();
    return;
  }
  
  // If we've reached the end of the current packet, disable TX and turn the IR
  // transmitter off
  if (this->currentPacketBit == 255) {
    this->txEnabled = 0;
    this->irOff();
    return;
  }
  
  
  // If we have not sent the start pulse for the current packet, and the IR
  // configuration has defined a start pulse duration, transmit the start pulse.
  // NOTE: as the Air Swimmer packet does not utilize a start bit, this
  // functionality is untested and incomplete.
  /*if (this->irConfig.startPulseDuration > 0 && !this->startPulseSent) {
    this->startPulseSent = 1;
    this->currentPulseDelay = this->irConfig.startPulseDuration;
    this->irOn();
    
    return;
  }*/
  
  // Current pulse delay hasn't expired; stop processing
  if (this->lastBitTime > micros() - this->currentPulseDelay) {
    return;
  }
  
  // If we haven't yet sent a gap pulse, configure the transmitter
  // to send the pulse gap
  if (!this->inPulseGap) {
    this->inPulseGap = 1;
    this->currentPulseDelay = this->irConfig.pulseGapDuration;
    this->markGap();
  
    return;
  }
  
  // We are no longer in a pulse gap
  this->inPulseGap = 0;
  
  // Configure the pulse delay based on the current packet bit.
  if (bitRead(this->packetBuffer, this->currentPacketBit - 1)) {
    this->currentPulseDelay = this->irConfig.longPulseDuration;
  } else {
    this->currentPulseDelay = this->irConfig.shortPulseDuration;
  }

  // Enable IR for sending the next bit
  this->markPulse();
  
  // Decrement the current packet position
  --this->currentPacketBit;
}

/**
 * Called by subclasses to configure a new packet to be transmitted.
 * Sets the internal packet buffer to the new packet value
 * and initializes the variables required by the handleTx() method.
 */
void IR::tx(uint32_t *packet)
{
  if (!this->txEnabled) {
    this->txEnabled = 1;
    this->packetBuffer = *packet;
    this->currentPacketBit = this->irConfig.packetBits;
    this->startPulseSent = 0;
    this->inPulseGap = 0;
  }
}

/**
 * Enable IR LED
 */
void IR::irOn() { 
  this->lastBitTime = micros();
  
  TCCR2A |= _BV(COM2B1);
}

/**
 * Disable IR LED
 */
void IR::irOff() {
  this->lastBitTime = micros(); 

  TCCR2A &= ~_BV(COM2B1);
}

/**
 * Activate the IR signal. This will vary based on the pulseIn type.
 */
void IR::markPulse() {
  if (this->irConfig.pulseInType == LOW) {
    this->irOn();
  } else {
    this->irOff();
  }
}

/**
 * Activate the IR signal. This will vary based on the pulseIn type.
 */
void IR::markGap() {
  if (this->irConfig.pulseInType == LOW) {
    this->irOff();
  } else {
    this->irOn();
  }
}

/**
  * Enables IR output.  The khz value controls the modulation frequency in kilohertz.
  * The IR output will be on pin 3 (OC2B).
  * This routine is designed for 36-40KHz; if you use it for other values, it's up to you
  * to make sure it gives reasonable results.  (Watch out for overflow / underflow / rounding.)
  * TIMER2 is used in phase-correct PWM mode, with OCR2A controlling the frequency and OCR2B
  * controlling the duty cycle.
  * There is no prescaling, so the output frequency is 16MHz / (2 * OCR2A)
  * To turn the output on and off, we leave the PWM running, but connect and disconnect the output pin.
  * A few hours staring at the ATmega documentation and this will all make sense.
  * See Ken Shirriff's Secrets of Arduino PWM at http://arcfn.com/2009/07/secrets-of-arduino-pwm.html for details.
  */
void IR::enableIROut(int khz) {  
  pinMode(TIMER_PWM_PIN, OUTPUT);
  digitalWrite(TIMER_PWM_PIN, LOW); // When not sending PWM, we want it high
  
  
  // COM2A = 00: disconnect OC2A
  // COM2B = 00: disconnect OC2B; to send signal set to 10: OC2B non-inverted
  // WGM2 = 101: phase-correct PWM with OCRA as top
  // CS2 = 000: no prescaling
  // The top value for the timer.  The modulation frequency will be SYSCLOCK / 2 / OCR2A.
  const uint8_t pwmval = SYSCLOCK / 2000 / khz;
  TCCR2A = _BV(WGM20);
  TCCR2B = _BV(WGM22) | _BV(CS20);
  OCR2A = pwmval;
  OCR2B = pwmval / 3;
  TIMSK2 = _BV(TOIE1);
  
  this->irOff();
}

/**
 * Reads in an IR packet from the configured RX pin
 *
 * @param packet Variable that will store the packet
 * @param timeout Maximum execution time of this routine
 * @return Boolean indicating whether a packet was received
 */
uint8_t IR::rx(uint32_t *packet, uint32_t timeout)
{
  uint32_t startTimeMillis = millis();
  uint32_t lastPulseMillis = millis();
  uint32_t packetMask = (1UL << this->irConfig.packetBits) - 1;
  uint8_t currentPacketBits = 0;
  *packet = 0;
  
  // Loop until the packet passes the IR configuration's checksum, we've
  // retrieved the correct amount of packet bits, and we have not
  // timed out.
  while(
    (!this->irConfig.hasChecksum || !this->checksum(packet))
      && (this->irConfig.packetBits == 0 || this->irConfig.packetBits != currentPacketBits) 
      && (timeout == 0 || millis() - startTimeMillis < timeout)) {
      
    // Read in the next pulse from the RX pin. This returns the number of microseconds that 
    // pass between two peaks or two valleys on the RX pin, depending on the configured
    // Pulse In type
    uint32_t pulse = readPulse(this->rxPin, this->irConfig.pulseInType, (this->irConfig.startPulseDuration > 0 ? this->irConfig.startPulseDuration : 2 * this->irConfig.longPulseDuration));

    // If a start pulse duration is defined, and the current pulse length falls within
    // the tolerance for the start pulse, we need to zero out the packet and restart
    // the read routine
    if (this->irConfig.startPulseDuration > 0 
      && pulse > this->irConfig.startPulseDuration - this->irConfig.pulseTolerance
      && pulse < this->irConfig.startPulseDuration + this->irConfig.pulseTolerance) {
      *packet = 0;
      currentPacketBits = 0;
    } 
    // If the current pulse length falls within the tolerance for the short pulse,
    // we append a '0' to the current packet.
    else if (pulse > this->irConfig.shortPulseDuration - this->irConfig.pulseTolerance 
      && pulse < this->irConfig.shortPulseDuration + this->irConfig.pulseTolerance) {
      *packet = (*packet << 1) & packetMask;
      currentPacketBits++;
    }
    // If the current pulse length falls within the tolerance for the long pulse,
    // we append a '1' to the current packet.
    else if (pulse > this->irConfig.longPulseDuration - this->irConfig.pulseTolerance
      && pulse < this->irConfig.longPulseDuration + this->irConfig.pulseTolerance) {
      *packet = ((*packet << 1) | 1) & packetMask;
      currentPacketBits++;
    } 
    // If we haven't received a pulse for the configured gap duration,
    // we need to zero out the packet and restart the read routine
    else if (lastPulseMillis < millis() - this->irConfig.gapDuration) {
      *packet = 0;
      currentPacketBits = 0;
    }
    lastPulseMillis = millis();
  }
  
  // If the packet passes the checksum (if defined) and the length of the packet matches
  // the defined packet length, return TRUE
  if ((!this->irConfig.hasChecksum || this->checksum(packet))
      && (!this->irConfig.packetBits || currentPacketBits == this->irConfig.packetBits)) {
    return 1;
  }

  return 0;
}

/**
 * The built-in pulseIn() routine does not return correct timings, perhaps because it relies
 * on clock cycles. This method utilizes timer readings instead, so is less prone to error
 * introduced by interrupts.
 * @author DaveMAA (http://arduino.cc/forum/index.php/topic,25816.msg193479.html#msg193479)
 */
uint32_t IR::readPulse(uint16_t pin, uint16_t signal, uint32_t timeout){
  uint32_t current, killTime, ptime;
  current = micros();
  killTime = current + timeout;
  
  while (digitalRead(pin) == signal) {
    delayMicroseconds(4); 
    current = micros();
    if(current >= killTime){
      return 0UL;
    }
  }
  
  while(digitalRead(pin) != signal){
    delayMicroseconds(4);
    current = micros(); 
    if(current >= killTime){
      return 0UL;
    }
  }
  
  ptime = micros();
  while (digitalRead(pin) == signal) {
    delayMicroseconds(4); 
  }
  return micros() - ptime;
}
