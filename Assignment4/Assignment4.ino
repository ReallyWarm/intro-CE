#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define BUZZER 10
#define BUTTON_L 12
#define BUTTON_R 11
#define OLED_RESET -1 // Reset pin 
#define SCREEN_ADDRESS 0x3C // Address 0x3C for 128x32

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 32 

int px, py;
int vx, vy;

int barlen = 16;
int bx, by;

bool play = false;
unsigned long last_tone = 0;
unsigned long delay_tone = 100;

Adafruit_SSD1306 OLED(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(9600);
  while (!OLED.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) 
  {
    Serial.println("SSD1306 allocation failed");
    delay(3000);
  }
  pinMode(BUTTON_L, INPUT);
  pinMode(BUTTON_R, INPUT);
  randomSeed(analogRead(A0));

  initPong();
  noTone(BUZZER);
  OLED.clearDisplay();
}

void loop() {
  bool buttonL = digitalRead(BUTTON_L);
  bool buttonR = digitalRead(BUTTON_R);
  if (buttonL || buttonR) play = true;

  // Draw boarder, pong, bar
  OLED.clearDisplay();
  OLED.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
  OLED.drawPixel(px, py, SSD1306_WHITE); 
  OLED.drawLine(bx, by, barlen+bx, by, SSD1306_WHITE);
  
  if (play) {
    // update pong velocity
    if ((px == 1) || (px == 126)) {
      vx *= -1; 
      tone(BUZZER, 349);
      last_tone = millis();
    }
    if (py == 1) {
      vy *= -1; 
      tone(BUZZER, 370);
      last_tone = millis();
    }
    if ((py == by - 1) && (px >= bx) && (px <= bx + barlen)) {
      vy *= -1; 
      tone(BUZZER, 330);
      last_tone = millis();
    }
    if (py == (SCREEN_HEIGHT - 1)) {
      play = false;
      tone(BUZZER, 494);
      last_tone = millis();
    }

    // update bar and pong position
    if (buttonL && (bx > 1)) {
      bx -= 2;
    }
    if (buttonR && (bx < SCREEN_WIDTH - 1)) {
      bx += 2;
    }
    px += vx;
    py += vy;

    if (!play) {
      initPong();
    }
  }
  
  if ((millis() - last_tone) > delay_tone) {
      noTone(BUZZER);
  }
  OLED.display();
  delay(50);
}

void initPong() {
  px = random(5, SCREEN_WIDTH-4);
  py = random(5, 26);
  vx = random(1) > 0 ? 1 : -1; 
  vy = random(1) > 0 ? 1 : -1;
  
  bx = (SCREEN_WIDTH - barlen) / 2;
  by = SCREEN_HEIGHT - 3;
}
