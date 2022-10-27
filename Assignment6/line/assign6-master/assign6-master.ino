// Master

#include <Wire.h>
#define ADDR1 0x01
#define ADDR2 0x02
#define ADDR3 0x03
#define MAX_TOKEN 30

char token[MAX_TOKEN] = "Token#0#";
bool haveAdr = false;
const unsigned indexMsg = 10;
unsigned i = indexMsg;

void setup()
{
  Serial.begin(9600);
  Wire.begin(ADDR1);
  Wire.onReceive(receiveEvent);
  
  Wire.beginTransmission(ADDR2);
  Wire.write(token, sizeof(token));
  Wire.endTransmission();
  Wire.beginTransmission(ADDR3);
  Wire.write(token, sizeof(token));
  Wire.endTransmission();
}

void loop()
{
  haveAdr = false;
  
  Wire.requestFrom(ADDR2, sizeof(token));
  readToken();
  if (token[6] == '1') sendToken();
  
  Wire.requestFrom(ADDR3, sizeof(token));
  readToken();
  if (token[6] == '1') sendToken();
  
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
  if (token[6] == '1') sendToken();
  
  delay(1000);
}

void receiveEvent(int howMany)
{
  Serial.println(1);
  for (int i = 0; i < howMany; i++)
    token[i] = Wire.read();
  Serial.println(token);
}

int readToken()
{
  for (int i = 0; Wire.available() && i < MAX_TOKEN - 1; i++)
  {
    token[i] = Wire.read();
    
    if(!Wire.available())
      token[i] = '\0';
  }

  if (token[6] != '1') {
    char restart[] = "Token#0#";
    for (int i = 0; restart[i] != '\0'; i++)
      token[i] = restart[i];
    token[8] = '\0';
  }
}

void sendToken()
{
  Serial.println(token); // display token
  
  if (token[6] == '1') {
    
    int addr = token[8] - '0';
    
    if (addr != ADDR1) {
      Wire.beginTransmission(addr);
      Wire.write(token, sizeof(token));
      Wire.endTransmission();
    }
    
    else {
      for (i = indexMsg; token[i] != '\0'; i++)
      {
        Serial.print(token[i]);
      }
      Serial.println();
    }
    
    token[6] = '0';
    token[8] = '\0';
    Serial.println(token);
  }
}