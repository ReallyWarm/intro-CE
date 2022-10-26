// Slave 1

#include <Wire.h>
#define ADDR1 0x01
#define ADDR2 0x02
#define ADDR3 0x03
#define MAX_MSG 25

char mystr[MAX_MSG];
bool haveAdr = false;
bool haveMsg = false;
bool reEvent = false;
unsigned indexMsg = 10;

void setup()
{
  Serial.begin(9600);
  Wire.begin(ADDR2);
  Wire.onReceive(receiveEvent);
}

void loop()
{
  indexMsg = 10;
  
  while (Serial.available() && reEvent)
  {
    char inChar = (char) Serial.read();
    if (!haveAdr) {
      if (inChar >= '1' && inChar <= '3') {
        mystr[6] = '1';
        mystr[8] = inChar;
        mystr[9] = '#';
        mystr[10] = '\0';
      }
    }
    else {
      if (inChar != '\n' && indexMsg < MAX_MSG - 1) {
        mystr[indexMsg] = inChar;
        indexMsg++;
      }
      if (Serial.available() == 0) {
        mystr[indexMsg] = '\0';
      }
    }
  }
  
  //Serial.println(mystr);
  if (reEvent) {
    Wire.beginTransmission(ADDR3);
    Wire.write(mystr, sizeof(mystr));
    Wire.endTransmission();
  }
  reEvent = false;
  
}

void receiveEvent(int howMany)
{
  reEvent = true;
  
  for (int i = 0; i < howMany; i++)
  {
    mystr[i] = Wire.read();
  }
  
  if (mystr[8] == '\0') {
    haveAdr = false;
    haveMsg = false;
  }
  else {
    haveAdr = true;
  }
  
  if (!haveAdr || mystr[10] == '\0') {
    haveMsg = false;
  }
  else {
    haveMsg = true;
  }

  Serial.println(mystr);  
  
  if (haveMsg && mystr[8] == '0' + ADDR2) {
    for (int i = 10; mystr[i] != '\0'; i++)
    {
      Serial.print(mystr[i]);
    }
    Serial.println();
    mystr[6] = '0';
    mystr[8] = '\0';
  }
}
