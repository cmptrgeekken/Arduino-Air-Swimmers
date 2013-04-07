/**
 * Augmented Air Swimmer
 *
 * This sketch provides the ability to control an Air Swimmer
 * blimp (http://www.airswimmers.com/) with the controller 
 * from the Propel Gyropter remote-controlled helicopter
 * (http://amzn.com/B00481GIDA/).
 *
 * This project serves as partial fulfillment of my Master's 
 * of Science in Computer Science at Rochester Institute of Technology. 
 *
 * The Circuit:
 * - Infrared Transmitter TX pin connected to Pin 3
 *   NOTE: The TX pin is hard-coded on Pin 3, and cannot be configured.
 * - Infrared Receiver RX pin connected to Pin 5
 *   NOTE: The rxPin variable can be modified to change the RX pin.
 *
 * Created: 2013-03-24
 * Author: Ken Beck (http://geekken.net/)
 *
 * An in-depth analysis of this project can be found at:
 * http://blog.geekken.net/2013/03/23/masters-project-final-report/
 */
// NOTE: Need to include libraries here so Arduino appends the appropriate library paths
// when compiling the project: the IR and TimerOne libraries are not referenced directly
// in this sketch, but referenced in the GyropterIR and AirSwimmerIR libraries.
#include <IR.h>
#include <GyropterIR.h>
#include <AirSwimmerIR.h>

#include <avr/io.h>
#include <inttypes.h>

// Configure the pin to use for receiving IR packets
int rxPin = 5;

// Buffers for storing incoming IR packets and the associated
// Gyropter command structure
uint32_t inputPacketBuffer;
GyropterIRCommand gyroCommand;

// References to the GyropterIR and AirSwimmerIR library classes
GyropterIR *gyropter;
AirSwimmerIR *airswimmer;

/**
 * Initialize the core libraries for this sketch
 */ 
void setup() 
{
  gyropter = new GyropterIR(rxPin);
  airswimmer = new AirSwimmerIR();
//   Serial.begin(9600);
}

/**
 * Handles the main logic for the program. Steps:
 * - Read an incoming IR packet from the Gyropter remote
 * - Convert packet to a Gyropter command packet
 * - Listen for sync command (zero throttle with light button depressed)
 *   - If sync command found, configure Air Swimmer library to send sync packet
 *   - Otherwise, configure Air Swimmer library with current command configuration
 */
void loop() 
{
  // Read an incoming Gyropter IR packet. This command times out after 200ms, if no 
  // commands are received.
  if (gyropter->rx(&inputPacketBuffer, 200)) {
    // Convert the Gyropter IR packet to a command packet. This is simpler to work with,
    // as it abstracts away the specific packet structure into one specific for use with
    // the Air Swimmers library.
    gyropter->getCommandPacket(&inputPacketBuffer, &gyroCommand);
       
    // Detect the sync command, which corresponds to a throttle set to 0 and 
    // the light button depressed
    if (gyroCommand.throttlePercent == 0 && gyroCommand.lightToggle == 1) {
      // Send sync command
      airswimmer->prepareSync(1);
      
      // If we're currently pairing with a blimp, we do not want to perform any other 
      // commands, so we drop out of the main loop.
      return;
    }
    
    // Disable sync packet
    airswimmer->prepareSync(0);
   
    // Configure the flap direction for the Air Swimmer library
    if (gyroCommand.leftPercent > 0) {
      // Sets the library to send a 'flap left' command
      airswimmer->prepareFlap(-1);
    } else if (gyroCommand.rightPercent > 0) {
       // Sets the library to send a 'flap right' command
       airswimmer->prepareFlap(1); 
    } else {
       // Sets the library to toggle between 'flap left' and 'flap right'
       airswimmer->prepareFlap(0); 
    }
    
    // Configure the 'dive' command for the Air Swimmer library
    if (gyroCommand.upPercent > 0) {
      // Sets the library to send the 'climb' command
      airswimmer->prepareDive(-1);
      Serial.println("Climbing??");
    } else if (gyroCommand.downPercent > 0) {
      // Sets the library to send the 'dive' command
      airswimmer->prepareDive(1);
      Serial.println("Diving??");
    } else {
      // Disables diving
      airswimmer->prepareDive(0); 
    }
    
    // Set the throttle percent for the Air Swimmer library
    airswimmer->setSpeed(gyroCommand.throttlePercent);
  }
}

