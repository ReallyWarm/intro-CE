#include <Wire.h>
#include <TimerOne.h>
#include <EEPROM.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define BUZZER 5
#define MODE_BUTTON 6
#define ADJS_BUTTON 7
#define INCR_BUTTON 8
#define LDR A0

#define OLED_RESET -1 // Aeset pin 
#define SCREEN_ADDRESS 0x3C // Address 0x3C for 128x32
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 32

#define EEPROM_ADDRESS 0

#define SHOW_TIME  0
#define SET_TIME   1
#define SET_ALARM  2
#define STOP_WATCH 3

#define SET_HOUR  0
#define SET_MIN   1

// Clock time in milliseconds
struct ClockTime {
  volatile unsigned long time;
  unsigned long alarm;
};

// For button
unsigned long timeNow;
unsigned long lastModeTime = 0;
unsigned long lastAdjsTime = 0;
unsigned long lastIncrTime = 0;
unsigned long timeDebounce = 200;
int lastMode = LOW;
int lastAdjs = LOW;
int lastIncr = LOW;

// For mode
int currentMode = SHOW_TIME;
int setMode = SET_HOUR;
bool alarmOn = true;
bool stopwatchStart = false;

// For timer
ClockTime clock;
volatile unsigned long timeCounter;
volatile unsigned long stopwatchTime;
int hourLost = -1149; // Estimate milliseconds (+) lost in an hour or (-) exceeded in an hour
unsigned long currentTime;
unsigned long timeCounterCopy;
unsigned long stopwatchCopy;

const unsigned long msInDay = 24ul * 60 * 60 * 1000;
const unsigned long msInHour = 60ul * 60 * 1000;

Adafruit_SSD1306 OLED(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  pinMode(MODE_BUTTON, INPUT);
  pinMode(ADJS_BUTTON, INPUT);
  pinMode(INCR_BUTTON, INPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(LDR, INPUT);
  Serial.begin(9600);

  while (!OLED.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) 
  {
    Serial.println("SSD1306 allocation failed");
    delay(2000);
  }
  OLED.setTextSize(1);
  OLED.setTextColor(SSD1306_WHITE);
  OLED.clearDisplay();

  // Get eeprom saved time data
  EEPROM.begin();
  EEPROM.get(EEPROM_ADDRESS, clock);

  timeCounter = 0;
  stopwatchTime = 0;
  timeNow = millis();
  currentTime = clock.time;
  timeCounterCopy = timeCounter;
  stopwatchCopy = stopwatchTime;

  Timer1.initialize(1000);
  Timer1.attachInterrupt(clockTimer); // Run at 1 millisecond
}

void loop() {
  // Copy Timer1 interrupt data
  noInterrupts();
  currentTime = clock.time;
  timeCounterCopy = timeCounter;
  stopwatchCopy = stopwatchTime;
  interrupts();

  // Time for button debounce
  timeNow = millis();

  // Mode button 1 time press
  if (timeNow - lastModeTime > timeDebounce) {
    int ModeRead = digitalRead(MODE_BUTTON);
    if (ModeRead == HIGH && lastMode == LOW) {
      // Switch mode
      switch (currentMode) {
        case SHOW_TIME:
          currentMode = SET_TIME;
          break;
        case SET_TIME:
          currentMode = SET_ALARM;
          break;
        case SET_ALARM:
          currentMode = STOP_WATCH;
          break;
        case STOP_WATCH:
          currentMode = SHOW_TIME;
          break;
      }
      // Reset default
      noTone(BUZZER);
      stopwatchStart = false;
      setMode = SET_HOUR;

      lastModeTime = timeNow;
    }
    if (ModeRead != lastMode) {
      lastMode = ModeRead;
    }
  } 

  // Adjusting time
  if (currentMode != SET_TIME) {
    // New day
    if (currentTime > msInDay) {
      noInterrupts();
      clock.time -= msInDay;
      currentTime = clock.time;
      interrupts();
    }
    if (stopwatchCopy > msInDay) {
      stopwatchStart = false;
      noInterrupts();
      stopwatchTime = 0;
      interrupts();
      stopwatchCopy = 0;
    }
    // Adjust time by time lost or exceeded in an hour
    if (timeCounterCopy > msInHour) {
      noInterrupts();
      clock.time += hourLost;
      currentTime = clock.time;
      timeCounter = 0;
      interrupts(); 
      timeCounterCopy = 0;
    }
  }

  OLED.clearDisplay();
  OLED.setCursor(0,10);
  
  // Function and display in modes
  switch (currentMode) {
    case SHOW_TIME:
      setMain();
      showTime(currentTime);
      showAlarm(clock.alarm);
      break;

    case SET_TIME:
      setTime();
      showSetTime(currentTime);
      break;

    case SET_ALARM:
      setAlarm();
      showSetAlarm(clock.alarm);
      break;

    case STOP_WATCH:
      setStopwatch();
      showStopwatch(stopwatchCopy);
      break;
  }

  // Dim display if in dark room
  if (analogRead(LDR) < 940) {
    OLED.dim(false);
  }
  else {
    OLED.dim(true);
  }
  
  OLED.display();
  delay(100);
}

// Timer1 interrupt
void clockTimer()
{
  if (currentMode != SET_TIME) {
    clock.time = clock.time + 1;
    timeCounter = timeCounter + 1;
    if (currentMode == STOP_WATCH && stopwatchStart == true) {
      stopwatchTime = stopwatchTime + 1;
    }
  }
}

// Get HH:MM:SS from milliseconds by referencing HH:MM:SS variables
void getHMSTime(unsigned long time, unsigned long* hours, unsigned long* minutes, unsigned long* seconds) {
  *seconds = time / 1000;
  *minutes = *seconds / 60;
  *hours = *minutes / 60;
  *minutes = *minutes - (*hours * 60);
  *seconds = *seconds - (*minutes * 60) - (*hours * 60 * 60);
}

// Get milliseconds from HH:MM:SS by referencing milliseconds variables
void getMilliTime(unsigned long* time, unsigned long hours, unsigned long minutes, unsigned long seconds) {
  *time = (hours * 60 * 60 * 1000) + (minutes * 60 * 1000) + (seconds * 1000);
}

// Adjust button change mode to set hour or set minute 
void buttonSetAdjust() {
  // Adjust button 1 time press
  if (timeNow - lastAdjsTime > timeDebounce) {
    int AdjsRead = digitalRead(ADJS_BUTTON);
    if (AdjsRead == HIGH && lastAdjs == LOW) {
      // Switch to hour or minute 
      switch (setMode) {
        case SET_HOUR:
          setMode = SET_MIN;
          break;
        case SET_MIN:
          setMode = SET_HOUR;
          break;
      }
      lastAdjsTime = timeNow;
    }
    if (AdjsRead != lastAdjs) {
      lastAdjs = AdjsRead;
    }
  }
}

// Show time mode functions
void setMain() {
  // If hold push adjust then turn off alarm 
  if (timeNow - lastAdjsTime > timeDebounce) {
    if (digitalRead(ADJS_BUTTON) == HIGH) {
      alarmOn = false;

      // If hold push adjust and increment then save to eeprom
      if (timeNow - lastIncrTime > timeDebounce) {
        if (digitalRead(INCR_BUTTON) == HIGH) {
          noInterrupts();
          EEPROM.put(EEPROM_ADDRESS, clock);
          interrupts();
          // Serial.println("Saved Time");
          OLED.println("Saved Time and Alarm!");
          OLED.display();

          delay(1000);
          OLED.clearDisplay();
          OLED.setCursor(0,10);

          lastIncrTime = timeNow;
        }
      }
      lastAdjsTime = timeNow;
    }
  }

  // If past alarm time for less than 30 seconds
  if (currentTime - clock.alarm > 0 && currentTime - clock.alarm < 30000) {
    // If alarm is turned on then play sound
    if (alarmOn) {
      tone(BUZZER, 330);
    }
    else {
      noTone(BUZZER);
    }
  }
  // If not alarm time then turn on alarm
  else {
    alarmOn = true;
    noTone(BUZZER);
  }
}

// Set time mode functions
void setTime() {
  buttonSetAdjust();
  // Increment button hold push
  if (timeNow - lastIncrTime > timeDebounce) {
    if (digitalRead(INCR_BUTTON) == HIGH) {
      unsigned long currHours;
      unsigned long currMinutes;
      unsigned long currSeconds;
      // Get current HH:MM:SS of current time
      getHMSTime(currentTime, &currHours, &currMinutes, &currSeconds);
      // Add hour or minute by mode from adjust button 
      switch (setMode) {
        case SET_HOUR:
          if (currHours < 23) {
            currHours++;
          }
          else {
            currHours = 0;
          }
          break;
        case SET_MIN:
          if (currMinutes < 59) {
            currMinutes++;
          }
          else {
            currMinutes = 0;
          }
          break;
      }
      // Set current time by new HH:MM:SS and reset hour time counter
      getMilliTime(&currentTime, currHours, currMinutes, 0);
      noInterrupts();
      clock.time = currentTime;
      timeCounter = 0;
      interrupts();
      timeCounterCopy = 0;

      lastIncrTime = timeNow;
    }
  }
}

// Set alarm mode functions
void setAlarm() {
  buttonSetAdjust();
  // Increment button hold push
  if (timeNow - lastIncrTime > timeDebounce) {
    if (digitalRead(INCR_BUTTON) == HIGH) {
      unsigned long currHours;
      unsigned long currMinutes;
      unsigned long currSeconds;
      // Get current HH:MM:SS of alarm time
      getHMSTime(clock.alarm, &currHours, &currMinutes, &currSeconds);
      // Add hour or minute by mode from adjust button 
      switch (setMode) {
        case SET_HOUR:
          if (currHours < 23) {
            currHours++;
          }
          else {
            currHours = 0;
          }
          break;
        case SET_MIN:
          if (currMinutes < 59) {
            currMinutes++;
          }
          else {
            currMinutes = 0;
          }
          break;
      }
      // Set alarm time by new HH:MM:SS
      getMilliTime(&clock.alarm, currHours, currMinutes, 0);

      lastIncrTime = timeNow;
    }
  }
}

// Set stopwatch mode functions
void setStopwatch() {
  // Adjust button 1 time press
  if (timeNow - lastAdjsTime > timeDebounce) {
    int AdjsRead = digitalRead(ADJS_BUTTON);
    if (AdjsRead == HIGH && lastAdjs == LOW) {
      // Stop stopwatch and reset stopwatch time      
      stopwatchStart = false;
      noInterrupts();
      stopwatchTime = 0;
      interrupts();
      stopwatchCopy = 0;

      lastAdjsTime = timeNow;
    }
    if (AdjsRead != lastAdjs) {
      lastAdjs = AdjsRead;
    }
  }
  
  // Increment button 1 time press
  if (timeNow - lastIncrTime > timeDebounce) {
    int IncrRead = digitalRead(INCR_BUTTON);
    if (IncrRead == HIGH && lastIncr == LOW) {
      // Stopwatch switch between start and stop
      stopwatchStart = !stopwatchStart;

      lastIncrTime = timeNow;
    }
    if (IncrRead != lastIncr) {
      lastIncr = IncrRead;
    }
  }
}

char* formatString(char* numStr, unsigned long number, bool milli = false) {
  if (!milli) {
    if (number < 10) {
      numStr[0] = '0';
      numStr[1] = '0' + number;
    }
    else {
      numStr[0] = '0' + (number / 10);
      numStr[1] = '0' + (number % 10);
    }
    numStr[2] = '\0';
  }

  else {
    if (number < 10) {
      numStr[0] = '0';
      numStr[1] = '0';
      numStr[2] = '0' + number;
    }
    else if (number < 100) {
      numStr[0] = '0';
      numStr[1] = '0' + number / 10;
      numStr[2] = '0' + (number % 10);
    }
    else {
      numStr[0] = '0' + (number / 100);
      numStr[1] = '0' + ((number % 100) / 10);
      numStr[2] = '0' + (number % 10);
    }
    numStr[3] = '\0';
  }

  return numStr;
}

void showTime(unsigned long time) {
  char numStr[3];
  unsigned long currHours;
  unsigned long currMinutes;
  unsigned long currSeconds;
  getHMSTime(time, &currHours, &currMinutes, &currSeconds);
  OLED.print("Time : ");
  OLED.print(formatString(numStr, currHours));
  OLED.print(":");
  OLED.print(formatString(numStr, currMinutes));
  OLED.print(":");
  OLED.println(formatString(numStr, currSeconds));
}

void showAlarm(unsigned long time) {
  char numStr[3];
  unsigned long currHours;
  unsigned long currMinutes;
  unsigned long currSeconds;
  getHMSTime(time, &currHours, &currMinutes, &currSeconds);
  OLED.print("Alarm : ");
  OLED.print(formatString(numStr, currHours));
  OLED.print(":");
  OLED.print(formatString(numStr, currMinutes));
  OLED.print(":");
  OLED.println(formatString(numStr, currSeconds));
}

void showSetTime(unsigned long time) {
  char numStr[3];
  unsigned long currHours;
  unsigned long currMinutes;
  unsigned long currSeconds;
  getHMSTime(time, &currHours, &currMinutes, &currSeconds);
  OLED.println("Set Time");
  OLED.print("Time : ");
  OLED.print(formatString(numStr, currHours));
  OLED.print(":");
  OLED.print(formatString(numStr, currMinutes));
  OLED.print(":");
  OLED.println(formatString(numStr, currSeconds));
}

void showSetAlarm(unsigned long time) {
  char numStr[3];
  unsigned long currHours;
  unsigned long currMinutes;
  unsigned long currSeconds;
  getHMSTime(time, &currHours, &currMinutes, &currSeconds);
  OLED.println("Set Alarm");
  OLED.print("Time : ");
  OLED.print(formatString(numStr, currHours));
  OLED.print(":");
  OLED.print(formatString(numStr, currMinutes));
  OLED.print(":");
  OLED.println(formatString(numStr, currSeconds));
}

void showStopwatch(unsigned long time) {
  char numStr[3];
  unsigned long currHours;
  unsigned long currMinutes;
  unsigned long currSeconds;
  getHMSTime(time, &currHours, &currMinutes, &currSeconds);

  char milliStr[4];
  unsigned long currMillis;
  getMilliTime(&currMillis, currHours, currMinutes, currSeconds);
  currMillis = time - currMillis;
  OLED.println("Stopwatch");
  OLED.print("Time : ");
  OLED.print(formatString(numStr, currHours));
  OLED.print(":");
  OLED.print(formatString(numStr, currMinutes));
  OLED.print(":");
  OLED.print(formatString(numStr, currSeconds));
  OLED.print(":");
  OLED.println(formatString(milliStr, currMillis, true));
}
