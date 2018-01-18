#include <KashiwaGeeks.h>

void start(void)
{
    // Baudrate is 57600, not 9600 any more

    ConsoleBegin(57600);
  //DisableConsole();
  //DisableDebug();

  /*  seetup WDT interval to 1, 2, 4 or 8 seconds
   *  Default interval is 1 second.  */
  //setWDT(8);    // set to 8 seconds
}

int cnt = 0;

void func1()
{
  ConsolePrint("Count=%d\n", cnt++);
  DebugPrint(F("Debug Count=%d\n"), cnt++);
  delay(1000);
}


//===============================
//            Execution interval
//    TASK( function, interval by second )
//===============================
TASK_LIST = {
TASK(func1, 0, 5),
  //TASK(func2, 2, 4),
  //TASK(func3, 3, 6),
  END_OF_TASK_LIST
};


/*****************
 * void loop()
 * {
 *   ConsolePrint("Count=%d\n", cnt++);
 *   delay(1000);
 * }
 ****************/


// uncomment the line8, then check ConsolePrint is disabled.
// uncomment the line9, then nothing shown on the screen.

// Check the interruption
// 1) connect D2 pin to the GND.
// 2) connect D3 pin to the GND.


