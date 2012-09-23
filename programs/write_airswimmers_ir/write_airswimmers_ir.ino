// NOTE: Need to include IR.h so Arduino appends the appropriate library paths
#include <IR.h>
#include <AirSwimmersIR.h>

#include <avr/io.h>
#include <inttypes.h>

int txPin = 5;
uint32_t packetBuffer;

AirSwimmersIR *ir;

void setup() 
{ 
  Serial.begin(9600);
  ir = new AirSwimmersIR(
    0,
    txPin
  );
}

void loop() 
{ 
  AirSwimmersIRPacket *packet = ir->getPacket(&packetBuffer);
  uint8_t inByte;
  uint8_t dir = 0;
  
  if (Serial.available()) {
    inByte = Serial.read();
    
    switch(inByte) {
      case 'u':
        dir = 8;
        break;
      case 'd':
        dir = 4;
        break;
      case 'l':
        dir = 2;
        break;
      case 'r':
        dir = 1;
        break;
    }
  }
  
  if (dir > 0) {
    packet->commands = dir;
    ir->sendPacket(packet);
  }
}
