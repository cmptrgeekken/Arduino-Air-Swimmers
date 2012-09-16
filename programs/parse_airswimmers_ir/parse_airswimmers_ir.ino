// NOTE: Need to include IR.h so Arduino appends the appropriate library paths
#include <IR.h>
#include <AirSwimmersIR.h>

#include <avr/io.h>
#include <inttypes.h>

int rxPin = 3;
int txPin = 5;
uint32_t packetBuffer;

AirSwimmersIR *ir;

void setup() 
{ 
  Serial.begin(9600);
  ir = new AirSwimmersIR(
    rxPin,
    txPin
  );
}

void loop() 
{ 
  if (ir->rx(&packetBuffer, 1000)) {
    AirSwimmersIRPacket *packet = ir->getPacket(&packetBuffer);
    if (packet->commandLeft) {
      Serial.print("Left ");
    } else if (packet->commandRight) {
      Serial.print("Right ");
    }
    
    if (packet->commandUp) {
      Serial.print("Up");
    } else if (packet->commandDown) {
      Serial.print("Down");
    }
    
    if (packet->commands) {
      Serial.println(); 
    }
  }
}
