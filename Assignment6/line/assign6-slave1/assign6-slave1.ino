// Slave 1

#include <Wire.h>
#define ADDR1 0x01
#define ADDR2 0x02
#define ADDR3 0x03
#define MAX_TOKEN 30

char token[MAX_TOKEN];
bool haveAdr = false;
const unsigned indexMsg = 10;
unsigned i = indexMsg;

void setup()
{
  Serial.begin(9600);
  Wire.begin(ADDR2);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
}

void loop()
{

}

void receiveEvent(int howMany)
{
  for (int c = 0; c < howMany; c++)
    token[c] = Wire.read();
}

void requestEvent()
{
  haveAdr = false;
  
  while (Serial.available()) {
    char inChar = (char) Serial.read();
  
    if (!haveAdr && inChar >= '1' && inChar <= '3') {
      token[6] = '1';
      token[8] = inChar;
      token[9] = '#';
      
      i = indexMsg;
      haveAdr = true;
    }
    
    else if (haveAdr && i < MAX_TOKEN - 1) {
      token[i] = inChar;
      i++;
    }
    
    if(!Serial.available())
      token[i] = '\0';
  }
  
  //Serial.println(token); // display token
  
  if (token[6] == '1') { 
    int addr = token[8] - '0';
    Serial.println(token); // display token
    
    if (addr == ADDR2) {
      for (i = indexMsg; token[i] != '\0'; i++)
      {
        Serial.print(token[i]);
      }
      Serial.println();
      token[6] = '0';
      token[8] = '\0';
      Serial.println(token);
    }
  }
  Wire.write(token, sizeof(token));
  token[6] = '0';
  token[8] = '\0';
  
}