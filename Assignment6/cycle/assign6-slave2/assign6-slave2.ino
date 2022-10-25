// Slave 2

#include <Wire.h>
#define ADDR1 0x01
#define ADDR2 0x02
#define ADDR3 0x03
#define MAX_MSG 25

char mystr[MAX_MSG];
bool haveAdr = false;
bool haveMsg = false;
unsigned indexMsg = 10;

void setup()
{
  Serial.begin(9600);
  Wire.begin(ADDR3);
  Wire.onReceive(receiveEvent); 
}

void loop()
{
  indexMsg = 10;
  
  while (Serial.available())
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
      else {
        mystr[indexMsg] = '\0';
        indexMsg = 10;
      }
    }
  }
  //Serial.println(mystr);
  
  Wire.beginTransmission(ADDR1);
  Wire.write(mystr, sizeof(mystr));
  Wire.endTransmission();
  
  delay(1000);
}

void receiveEvent(int howMany)
{  
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
  
  if (haveAdr && mystr[10] != '\0') {
    haveMsg = true;
  }
  else {
    haveMsg = false;
  }
  
  if (haveMsg && mystr[8] == '0' + ADDR3) {
    for (int i = 10; mystr[i] != '\0'; i++)
    {
      Serial.print(mystr[i]);
    }
    Serial.println();
    mystr[6] = '0';
    mystr[8] = '\0';
  }
  
  Serial.println(mystr);
}
