#define A 11
#define B 10
#define C 9
#define D 8
#define E 7
#define F 6
#define G 5
#define DP 12
#define BA 3
#define BB 4

long timePush = 0;
int num = 0, bitSet, ranNum;
int segPin[8] = {A,B,C,D,E,F,G,DP};
int numPin = 8;

// binary by pgfedcba
byte Nums[12] {
            B00000110,//1
            B01011011,//2
            B01001111,//3
            B01100110,//4
            B01101101,//5
            B01111101,//6
            B00000111,//7
            B01111111,//8
            B01101111,//9
            B00111111,//0
            B00111101,//greater
            B00111000 //lower
            };

void handle_guess_button();
void handle_start_button();
void display(int num);

void setup() 
{
  Serial.begin(9600);
  
  randomSeed(analogRead(A0));
  ranNum = random(1, 10);

  for(int i = 0; i < numPin; i++)
  {
  pinMode(segPin[i],OUTPUT);
  }
  pinMode(BA,INPUT);
  pinMode(BB,INPUT);
}

void loop()
{
  handle_start_button();
  handle_guess_button();
  display(num);
  
  delay(100);
}

void handle_start_button()
{
  if (digitalRead(BA) == LOW && millis() - timePush > 500)
  {
    if (num >= 8) 
      num = 0;
    else
      num += 1;
    timePush = millis();
  }
}

void handle_guess_button()
{
  if (digitalRead(BB) == LOW && millis() - timePush > 500)
  {
    if (num + 1 == ranNum)
    {
      display(9);
      ranNum = random(1, 10);
    }
    else if (num + 1 > ranNum)
    {
      display(10);
    }
    else
    {
      display(11);
    }
    delay(1000);
  }
}

void display(int num)
{
  for(int i = 0; i < numPin; i++)
  {
    bitSet = bitRead(Nums[num], i);
    digitalWrite(segPin[i], bitSet);
  }
}
