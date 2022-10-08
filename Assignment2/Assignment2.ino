#define LEFT_B 12
#define MID_B 11
#define RIGHT_B 10
#define RED_LED 4
#define YELLOW_LED 3
#define GREEN_LED 2

long last_LB_push = 0;
long last_MB_push = 0;
long last_RB_push = 0;
long time_delay = 200;

int red_work = false;
int yellow_work = false;
int green_work = false;

void setup()
{
  Serial.begin(9600);
  pinMode(LEFT_B, INPUT);
  pinMode(MID_B, INPUT);
  pinMode(RIGHT_B, INPUT_PULLUP);
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
}

void loop()
{
  int LB_state = digitalRead(LEFT_B);
  int MB_state = digitalRead(MID_B);
  int RB_state = digitalRead(RIGHT_B);
  
  // check if left button is pressed -> PULL DOWN
  if (LB_state == HIGH) 
  {
    // delay prevents switching on and off constantly
    if (millis() - last_LB_push > time_delay) 
    {
      red_work = !red_work;
      last_LB_push = millis();
    }
  }
  
  // check if right button is pressed -> PULL UP internal resistor
  // works if left button is not pressed
  if (RB_state == LOW && red_work == false) 
  {
    // delay prevents switching on and off constantly
    if (millis() - last_RB_push > time_delay) 
    {
      green_work = !green_work;
      last_RB_push = millis();
    }
  }
  
  // check if middle button is pressed -> PULL UP
  // works if left and right button is not pressed
  if (MB_state == LOW && red_work == false && green_work == false) 
  { 
    if (millis() - last_MB_push > time_delay)
    {
      yellow_work = true;
      last_MB_push = millis();
    }
  }
  
  // if left button is pressed
  if (red_work == true && millis() - last_LB_push <= 3000)
  {
    digitalWrite(RED_LED, HIGH);
  }
  else
  {
    red_work = false;
    digitalWrite(RED_LED, LOW);
  }
  
  // if right button is pressed
  if (green_work == true && millis() - last_RB_push <= 3000)
  {
    digitalWrite(GREEN_LED, HIGH);
  }
  else
  {
    green_work = false;
    digitalWrite(GREEN_LED, LOW);
  }
  
  // if middle button is pressed
  if (yellow_work == true && millis() - last_MB_push <= 2000)
  {
    if (((millis() - last_MB_push) / 500) % 2 == 0)
    {
      digitalWrite(YELLOW_LED, HIGH);
    }
    else
    {
      digitalWrite(YELLOW_LED, LOW);
    }
  }  
  else
  {
    yellow_work = false;
    digitalWrite(YELLOW_LED, LOW);
  }
  
  delay(100);
}
