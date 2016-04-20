// Sound sensor come fome:
// From Henry's Bench
// http://henrysbench.capnfatz.com/henrys-bench/arduino-sensors-and-input/arduino-sound-detection-sensor-tutorial-and-user-manual/
// Arduino Sound Detection Sensor Module

// Door sensor come from:
// MySensors website
// http://www.mysensors.org/build/binary

#include <MySensor.h>
#include <SPI.h>
#include <Bounce2.h>
#define ID 100
#define CHILD_ID 101
#define OPEN 1
#define CLOSE 0
#define BUTTON_PIN  3  // Arduino Digital I/O pin for button/reed switch
#define SOUND_PIN 5  // Arduino Digital I/O pin for sound sensor


MySensor gw;
Bounce debouncer = Bounce(); 
int oldValue=-1;
MyMessage msgring(ID, V_TRIPPED);
MyMessage msgdoor(CHILD_ID,V_TRIPPED);

int soundDetectedVal = HIGH; // This is where we record our Sound Measurement
boolean bAlarm = false;
unsigned long lastSoundDetectTime; // Record the time that we measured a sound
int soundAlarmTime = 500; // Number of milli seconds to keep the sound alarm high

void setup ()
{
  Serial.begin(115200);
  gw.begin();
  pinMode(SOUND_PIN,INPUT) ; // input from the Sound Detection Module
  pinMode(BUTTON_PIN,INPUT_PULLUP);
  debouncer.attach(BUTTON_PIN);
  debouncer.interval(5);
  gw.present(ID, S_DOOR);
  gw.present(CHILD_ID, S_DOOR);
}
void loop ()
{
  soundDetectedVal = digitalRead (SOUND_PIN) ; // read the sound alarm time
  debouncer.update();
  int value = debouncer.read();
  if (value != oldValue) {
     Serial.print("Door value: ");
     Serial.println(value);
     gw.send(msgdoor.set(value==HIGH ? 1 : 0));
     oldValue = value;
  }
  if (soundDetectedVal == LOW) // If we hear a sound
  {
    lastSoundDetectTime = millis(); // record the time of the sound alarm
    // The following is so you don't scroll on the output screen
    if (!bAlarm){
      Serial.println("LOUD, LOUD");
      gw.send(msgring.set(OPEN)); 
      bAlarm = true;
    }
  }
  else
  {
    if( (millis()-lastSoundDetectTime) > soundAlarmTime  &&  bAlarm){
      Serial.println("quiet");
      gw.send(msgring.set(CLOSE)); 
      bAlarm = false;
    }
  }
}
