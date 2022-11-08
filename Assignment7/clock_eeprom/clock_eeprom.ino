#include <EEPROM.h>

//Location we want the data to be put.
#define EEPROM_ADDRESS 0

struct ClockTime {
  volatile unsigned long time;
  unsigned long alarm;
};

void setup() {
  Serial.begin(9600);
  ClockTime clock = 
  {
    0,
    0
  };
  EEPROM.begin();
  EEPROM.put(EEPROM_ADDRESS, clock);
  Serial.println("Written reset clock data!");
  ClockTime test;
  EEPROM.get(EEPROM_ADDRESS, test);
  Serial.println(test.time);
  Serial.println(test.alarm);
}

void loop() {
  // Nothing ...
}
