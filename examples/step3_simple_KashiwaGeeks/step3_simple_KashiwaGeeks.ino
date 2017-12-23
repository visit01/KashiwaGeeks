#include <KashiwaGeeks.h>

void setup()
{
    ConsoleBegin(9600);
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


