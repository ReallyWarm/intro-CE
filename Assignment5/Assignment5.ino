#define BTC 12
#define LED_CR 8
#define LED_CG 9

#define BTW 11
#define LED_WR 5
#define LED_WY 6
#define LED_WG 7

#define BTS 10
#define LED_SR 2
#define LED_SY 3
#define LED_SG 4

#define goS 0
#define chS 1

#define goW 2
#define chW 3

#define goC 4
#define cfC 5
#define cgC 6

// max button state = 2 power by nButton = 8

struct State {
  unsigned long ledOut;
  unsigned long delay;
  unsigned long next[8];
};

typedef const struct State SType;
SType FSM[7] = {
  //RGRYGRYG [C-W-S] 000 001 010 011 100 101 110 111
  {B10100001, 1500, {goS,goS,chS,chS,chS,chS,chS,chS}}, // South Go
  {B10100010,  400, {goW,goW,goW,goW,goC,goC,goC,goW}}, // South Change
  {B10001100, 1500, {goW,chW,goW,chW,chW,chW,chW,chW}}, // West Go
  {B10010100,  400, {goS,goS,goS,goS,goC,goC,goC,goC}}, // West Change
  {B01100100, 1500, {goC,cfC,cfC,cfC,goC,cfC,cfC,cfC}}, // Cross Go
  {B00100100,  300, {cgC,cgC,cgC,cgC,cgC,cgC,cgC,cgC}}, // Cross ChangeF
  {B01100100,  300, {cfC,goS,goW,goS,goC,goS,goW,goS}}  // Cross ChangeG
};

int input, inputC, inputW, inputS;
unsigned s = 0, blink = 0;

void setup()
{
  Serial.begin(9600);
  
  pinMode(BTC, INPUT);
  pinMode(LED_CR, OUTPUT);
  pinMode(LED_CG, OUTPUT);
  
  pinMode(BTW, INPUT);
  pinMode(LED_WR, OUTPUT);
  pinMode(LED_WY, OUTPUT);
  pinMode(LED_WG, OUTPUT);
  
  pinMode(BTS, INPUT);
  pinMode(LED_SR, OUTPUT);
  pinMode(LED_SY, OUTPUT);
  pinMode(LED_SG, OUTPUT);
  
}

void loop()
{
  digitalWrite(LED_CR, FSM[s].ledOut & B10000000);
  digitalWrite(LED_CG, FSM[s].ledOut & B01000000);
  
  digitalWrite(LED_WR, FSM[s].ledOut & B00100000);
  digitalWrite(LED_WY, FSM[s].ledOut & B00010000);
  digitalWrite(LED_WG, FSM[s].ledOut & B00001000);
  
  digitalWrite(LED_SR, FSM[s].ledOut & B00000100);
  digitalWrite(LED_SY, FSM[s].ledOut & B00000010);
  digitalWrite(LED_SG, FSM[s].ledOut & B00000001);
  
  delay(FSM[s].delay);
  
  inputC = digitalRead(BTC);
  inputW = digitalRead(BTW);
  inputS = digitalRead(BTS);
  
  input = inputC*4 + inputW*2 + inputS;
  
  if (s == 6) {
    if (blink < 2) {
      blink++;
      input = 0;
    }
    else if (blink >= 2) {
      blink = 0;
    }
  }
  
  s = FSM[s].next[input];
}
