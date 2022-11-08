#include <TimerOne.h>

volatile unsigned long Count = 0; // use volatile for shared variables

void setup(void)
{
  Timer1.initialize(1000);
  Timer1.attachInterrupt(counter); // blinkLED to run every 1 milliseconds
  Serial.begin(9600);
}

void counter(void)
{
  Count = Count + 1;
}

void loop(void)
{
  unsigned long Copy;
  noInterrupts();
  Copy = Count;
  interrupts();
  // to read a variable which the interrupt code writes, we
  // must temporarily disable interrupts, to be sure it will
  // not change while we are reading.  To minimize the time
  // with interrupts off, just quickly make a copy, and then
  // use the copy while allowing the interrupt to keep working.

  Serial.print("Count = ");
  Serial.println(Copy);
  delay(1000);
}
