#define MY_RADIO_NRF24

// EmonLibrary examples openenergymonitor.org, Licence GNU GPL V3  ***/
#include <SPI.h>
#include <MySensor.h>  
#include "EmonLib.h"             // Include Emon Library
EnergyMonitor emon1;             // Create an instance
#define MY_NODE_ID 99
// #define CHILD_ID 99
#define MY_LEDS_BLINKING_FEATURE
#define PIN_ANALOG_I A2
MySensor gw;
unsigned long lastSend;
unsigned long lastSend2;
unsigned long SEND_FREQUENCY = 20000; // Minimum time between send (in milliseconds). We don't wnat to spam the gateway.
unsigned long SEND_FREQUENCY2 = SEND_FREQUENCY / 25;
int index = 0;
double Irms=0;
double power;
boolean pcReceived = false;
boolean onyva=true;
boolean debug=false;
float nrj=0, old_nrj;
MyMessage IrmsMsg(CHILD_ID,V_WATT);
MyMessage kWhMsg(CHILD_ID,V_KWH);
MyMessage pcMsg(CHILD_ID,V_VAR1);


void incomingMessage(const MyMessage &message) 
{
  if (message.type==V_VAR1) 
  {  
    nrj = old_nrj = message.getFloat();
    Serial.print("Received last nrj count from gw:");
    Serial.println(nrj);
    pcReceived = true;
  }
}

void presentation()
{  
  begin(incomingMessage);
  sendSketchInfo("Energy Meter", "1.1");  // Send the sketch version information to the gateway and Controller
  present(CHILD_ID, S_POWER);   // Register this device as power sensor
  request(CHILD_ID, V_VAR1);
  emon1.current(PIN_ANALOG_I, 30.0);       // Current: input pin, calibration.
}

void loop()
{
  if (debug) Serial.println("Starting...");
  if (onyva) gw.process();
  onyva = false; 
  unsigned long now = millis();
  //unsigned long now2 = millis();
  bool sendTime2 = now - lastSend2 > SEND_FREQUENCY2;
  if (sendTime2) //calcul Irms moy
  {
    if (index==0) Irms=emon1.calcIrms(1480);
    else {
    index++;
    Irms = (index*Irms+emon1.calcIrms(1480))/(index+1);
    }
    lastSend2 = now;
  }
  bool sendTime = now - lastSend > SEND_FREQUENCY;
  if (debug) Serial.print("DEBUG: ");
  if (debug) Serial.println(Irms*232.0);
  if (sendTime && pcReceived) 
  { 
    power = Irms*232.0;
    if (debug) Serial.println("Sending data ...");
    send(IrmsMsg.set(power,1));
    Serial.println(Irms*232.0);
    nrj += (power*SEND_FREQUENCY/1000)/3.6E6;
    send(kWhMsg.set(nrj,5));
    send(pcMsg.set(nrj,5));
    lastSend = now;
    index = 0;
    old_nrj=nrj;
    onyva=true;
  }
 else if (sendTime && !pcReceived)
 {
  if (debug) Serial.println("DEBUG AGAIN ...");
  request(CHILD_ID, V_VAR1);
  lastSend=now;
  index=0;
  onyva=true;
 }
}

