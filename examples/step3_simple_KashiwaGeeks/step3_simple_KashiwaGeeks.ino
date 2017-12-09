#include <KashiwaGeeks.h>

void setup()
{
  Serial.begin(9600);
  pinMode(13, OUTPUT);
}

int cnt = 0;

void loop()
{
  ConsolePrint("Count=%d\n", cnt++);
  LedOn();
  delay(1000);                
  LedOff();
  delay(1000);
}


