/**
 * IR
 *
 * PWM logic taken with modifications from:
 * https://github.com/shirriff/Arduino-IRremote/blob/master/IRremoteInt.h
 */
#include "IR.h"


#define TIMER_RESET
#ifdef F_CPU
#define SYSCLOCK F_CPU     // main Arduino clock
#else
#define SYSCLOCK 16000000  // main Arduino clock
#endif
#define TIMER_PWM_PIN 3
#define TIMER_ENABLE_PWM     (TCCR2A |= _BV(COM2B1))
#define TIMER_DISABLE_PWM    (TCCR2A &= ~(_BV(COM2B1)))
#define TIMER_ENABLE_INTR    (TIMSK2 = _BV(OCIE2A))
#define TIMER_DISABLE_INTR   (TIMSK2 = 0)
#define TIMER_INTR_NAME      TIMER2_COMPA_vect
#define TIMER_CONFIG_KHZ(val) ({ \
  const uint8_t pwmval = SYSCLOCK / 2000 / (val); \
  TCCR2A = _BV(WGM20); \
  TCCR2B = _BV(WGM22) | _BV(CS20); \
  OCR2A = pwmval; \
  OCR2B = pwmval / 3; \
})

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
  uint8_t i;
  
  this->enableIROut(this->irConfig.txFrequency);
  
  if (this->irConfig.gapDuration > 0) {
    mark(this->irConfig.gapDuration);
  }
  
  this->sendPulse(this->irConfig.startPulseDuration);
  
  for(i=this->irConfig.packetBits;i>0;i--) {
    if (bitRead(*packet, i - 1)) {
      this->sendPulse(this->irConfig.longPulseDuration + this->irConfig.txHighOffset);
    } else {
      this->sendPulse(this->irConfig.shortPulseDuration + this->irConfig.txLowOffset);
    }
  }
  this->mark(0);
}

void IR::sendPulse(uint32_t duration)
{
  this->mark(duration);
  this->space(this->irConfig.pulseGapDuration);
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
    uint32_t pulse = pulseIn(this->rxPin, this->irConfig.pulseInType, 2 * this->irConfig.longPulseDuration);
    
    if (this->irConfig.startPulseDuration > 0 
      && pulse > this->irConfig.startPulseDuration - this->irConfig.pulseTolerance
      && pulse < this->irConfig.startPulseDuration + this->irConfig.pulseTolerance) {
      *packet = 0;
    } else if (pulse > this->irConfig.shortPulseDuration - this->irConfig.pulseTolerance 
      && pulse < this->irConfig.shortPulseDuration + this->irConfig.pulseTolerance) {
      *packet = (*packet << 1) & packetMask;
    } else if (pulse > this->irConfig.longPulseDuration - this->irConfig.pulseTolerance
      && pulse < this->irConfig.longPulseDuration + this->irConfig.pulseTolerance) {
      *packet = ((*packet << 1) | 1) & packetMask;
    }
  }
  
  if (this->checksum(packet)) {
    return 1;
  }

  return 0;
}


void IR::space(int time) { 
  // Enable PWM by setting Timer Control Register to
  // clear OC2B on Compare Match. PWM occurs on pin 3.
  TCCR2A |= _BV(COM2B1);
  delayMicroseconds(time);
}

/* Leave pin off for time (given in microseconds) */
void IR::mark(int time) {
  // Sends an IR space for the specified number of microseconds.
  // A space is no output, so the PWM output is disabled.
  TIMER_DISABLE_PWM; // Disable pin 3 PWM output
  delayMicroseconds(time);
}

void IR::enableIROut(int khz) {
  // Enables IR output.  The khz value controls the modulation frequency in kilohertz.
  // The IR output will be on pin 3 (OC2B).
  // This routine is designed for 36-40KHz; if you use it for other values, it's up to you
  // to make sure it gives reasonable results.  (Watch out for overflow / underflow / rounding.)
  // TIMER2 is used in phase-correct PWM mode, with OCR2A controlling the frequency and OCR2B
  // controlling the duty cycle.
  // There is no prescaling, so the output frequency is 16MHz / (2 * OCR2A)
  // To turn the output on and off, we leave the PWM running, but connect and disconnect the output pin.
  // A few hours staring at the ATmega documentation and this will all make sense.
  // See my Secrets of Arduino PWM at http://arcfn.com/2009/07/secrets-of-arduino-pwm.html for details.

  
  // Disable the Timer2 Interrupt (which is used for receiving IR)
  TIMER_DISABLE_INTR; //Timer2 Overflow Interrupt
  
  pinMode(TIMER_PWM_PIN, OUTPUT);
  digitalWrite(TIMER_PWM_PIN, HIGH); // When not sending PWM, we want it low
  
  // COM2A = 00: disconnect OC2A
  // COM2B = 00: disconnect OC2B; to send signal set to 10: OC2B non-inverted
  // WGM2 = 101: phase-correct PWM with OCRA as top
  // CS2 = 000: no prescaling
  // The top value for the timer.  The modulation frequency will be SYSCLOCK / 2 / OCR2A.
  TIMER_CONFIG_KHZ(khz);
}
