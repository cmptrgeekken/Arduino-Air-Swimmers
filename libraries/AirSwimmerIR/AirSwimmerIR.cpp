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
#include "AirSwimmerIR.h"

// Note: The IR Signature is unique to each controller. The sync routine
// is required to ensure that this program works properly with the
// Air Swimmers device being used.
#define AIRSWIMMER_IR_SIGNATURE 0b0110101010111101
#define AIRSWIMMER_IR_CHECKSUM_BASE 0b1010

/**
 * Initialize the Air Swimmer IR class. Configures the IR interface with the
 * necessary parameters as determined by reverse-engineering the protocol,
 * and enables IR Out at the appropriate frequency.
 *
 * @param rxPin Pin hooked up to the IR receiver's data line
 */
AirSwimmerIR::AirSwimmerIR()
: IR(0, 1)
{
	this->irConfig.startPulseDuration = 0;
	this->irConfig.gapDuration        = 50000l;
	this->irConfig.pulseGapDuration   = 340;
	this->irConfig.shortPulseDuration = 220;
	this->irConfig.longPulseDuration  = 720;
	this->irConfig.pulseTolerance     = 100;
	this->irConfig.packetBits         = 24;
	this->irConfig.txFrequency        = 38;
	this->irConfig.pulseInType        = HIGH;
	this->irConfig.hasChecksum        = 1;
  
  this->currentFlapTime = 0;
  
  this->enableIROut(this->irConfig.txFrequency);
}
 
/**
 * Configures the packet for transmission based on the current settings,
 * and sends a TX command (which is processed during the next appropriate
 * timer cycle)
 */
void AirSwimmerIR::sendPacket()
{
  // If Sync has been initialized, we configure the command packet
  // with the sync command (which is equivalent to all four commands
  // enabled at once)
  if (this->syncEnabled) {
    this->irPacket.commandLeft = 1;
    this->irPacket.commandRight = 1;
    this->irPacket.commandUp = 1;
    this->irPacket.commandDown = 1;
  } else {
    
    if (this->lastCommandTime < millis() - 1000) {
      // If no commands have been received for at least a second, stop all motion
      this->currentSpeed = 0;
    } else {
      // Initialize the dive command, as long as we're receiving commands
      this->irPacket.commandDown = 0;
      this->irPacket.commandUp = 0;
      
      if (this->diveDirection == 1) {
        this->irPacket.commandDown = 1;
      } else if (this->diveDirection == -1) {
        this->irPacket.commandUp = 1;
      }
    }
    
    if (this->currentSpeed > 0) {
      // The time between subsequent flap commands is adjusted based on the
      // current speed. The flap speed ranges from 250ms to 500ms. Increasing
      // the speed decreases this interval.
      uint32_t delayTime = 250000l + 2500l * (100l - (uint32_t)this->currentSpeed);
      
      // Check the time of the last flap event and determine if
      // at least 'delaytime' microseconds have elapsed. 
      if (this->currentFlapTime < micros() - delayTime) {
        this->irPacket.commandLeft = 0;
        this->irPacket.commandRight = 0;

        if (this->flapDirection == 0) { // Toggle flap direction (to fly straight)
          // Toggle between the two flap directions
          // (if last direction was left, flap right;
          // otherwise, flap left).
          if (this->lastFlapDirection == -1) {
            this->irPacket.commandRight = 1;
            this->lastFlapDirection = 1;
          } else {
            this->irPacket.commandLeft = 1;
            this->lastFlapDirection = -1;
          }
        } else if (this->lastFlapDirection != 0) { // Bring flapping to idle position
          this->lastFlapDirection = 0;
        } else { // Flap in the direction specified
          this->lastFlapDirection = this->flapDirection;
          
          if (this->flapDirection == -1) { // Flap left
            this->irPacket.commandLeft = 1;
          } else { // Flap right
            this->irPacket.commandRight = 1;
          }
        }
        
        // Set the current flap time to now
        this->currentFlapTime = micros();
      }
    } else {
      // Disable the flapping commands
      this->irPacket.commandLeft = 0;
      this->irPacket.commandRight = 0;
    }
  }
  
  // If we have not set any commands, do not send the 
  // TX packet
  if (this->irPacket.commands == 0) {
    return;
  }
  
  // Configure the packet signature and checksum prior to sending
  // the TX packet
  this->irPacket.signature = AIRSWIMMER_IR_SIGNATURE;
  this->irPacket.checksum  = this->irPacket.commands ^ AIRSWIMMER_IR_CHECKSUM_BASE;

  // Transmit the IR packet
  this->tx((uint32_t *)&this->irPacket);
}
 
 /**
  * Checksum for the Air Swimmer packet. Verifies that the signature of the packet
  * matches and ensures the command checksum is valid.
  *
  * @param packet IR packet to verify
  */
uint8_t AirSwimmerIR::checksum(uint32_t *packet)
{
	return this->irPacket.signature == AIRSWIMMER_IR_SIGNATURE
          && (this->irPacket.commands ^ this->irPacket.checksum) == AIRSWIMMER_IR_CHECKSUM_BASE;
}
 
/**
 * Set the current speed for sending commands
 *
 * @param speed Speed at which commands should be sent (Min: 0; Max: 100)
*/
void AirSwimmerIR::setSpeed(uint8_t speed)
{
	if (speed > 100) speed = 100;
	 
	this->currentSpeed = speed;
  
  this->lastCommandTime = millis();
}
 
/**
 * Prepares the specified packet for a flap
 *
 * @param direction Direction of travel. -1 for left, 1 for right, 0 for straight
 * @param irPacket Packet to configure for the flap
*/
void AirSwimmerIR::prepareFlap(int8_t direction)
{
  this->flapDirection = direction;
  
  this->lastCommandTime = millis();
}
 
/**
 * Prepares the specified packet for a dive
 *
 * @param direction Direction of travel. -1 for dive, 1 for climb
 * @param irPacket Packet to configure for the dive
*/
void AirSwimmerIR::prepareDive(int8_t direction)
{
  this->diveDirection = direction;
  
  this->lastCommandTime = millis();
}

/**
 * Sets the sync state for the packet
 *
 * @param syncOn Boolean value indicating whether sync should be enabled
 */
void AirSwimmerIR::prepareSync(uint8_t syncOn)
{
  this->syncEnabled = syncOn;
}
