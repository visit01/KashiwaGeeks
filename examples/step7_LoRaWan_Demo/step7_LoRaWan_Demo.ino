
#include <KashiwaGeeks.h>

ADB922S LoRa;

//================================
//          Initialize Device Function
//================================
#define BPS_9600       9600
#define BPS_19200     19200
#define BPS_57600     57600
#define BPS_115200   115200

void start()
{
    /*  Setup console */
    ConsoleBegin(BPS_57600);
    //DisableConsole();
    //DisableDebug();

    /*
     * Enable Interrupt 0 & 1  Uncomment the following two  lines.
     * For ADB922S only , CUT the pin2 and 3 of the Sheild.
     */
    //pinMode(2, INPUT_PULLUP);
    //pinMode(3, INPUT_PULLUP);

    ConsolePrint(F("**** Step7 LoRaWan_Demo*****\n"));

    /*  setup Power save Devices */
    //power_adc_disable();          // ADC converter
    //power_spi_disable();           // SPI
    //power_timer1_disable();    // Timer1
    //power_timer2_disable();    // Timer2, tone()
    //power_twi_disable();           // I2C

    /*  setup ADB922S  */
    if ( LoRa.begin(BPS_9600) == false )
    {
        while(true)
        {
            LedOn();
            delay(300);
            LedOff();
            delay(300);
        }
    }

    /*  join LoRaWAN */
    LoRa.reconnect();

    /* set DR. therefor, a payload size is fixed. */
    // Comment out a next line, then you can see a E rror: Data is too long
        LoRa.setDr(dr3);  // dr0 to dr5


    /*  seetup WDT interval to 1, 2, 4 or 8 seconds  */
    //setWDT(8);    // set to 8 seconds
}

//================================
//          Power save functions
//================================
void sleep(void)
{
    LoRa.sleep();
    DebugPrint(F("LoRa sleep.\n"));

    //
    //  ToDo: Set Device to Power saving mode
     //
}

void wakeup(void)
{
    LoRa.wakeup();
    DebugPrint(F("LoRa wakeup.\n"));
   //
   //  ToDo: Set Device to Power On mode
   //
}

//================================
//          INT0, INT2 callbaks
//================================
void int0D2(void)
{
  ConsolePrint(F("\nINT0 割込み発生\n"));
}

void int1D3(void)
{
  ConsolePrint(F("\nINT1 割込み発生\n"));
}


//================================
//    DownLink Data handler
//================================
void downLinkHdl(int rc)
{
  if ( rc == LoRa_RC_SUCCESS )
  {
     uint8_t port = LoRa.getDownLinkPort();
     if ( port == 0 )
     {
        return;
     }
     else if ( port == 14 )
     {
        ConsolePrint("\nPayload='%s'\n", LoRa.getDownLinkData().c_str());
        LedOn();
     }
     else if ( port == 15 )
     {
        ConsolePrint("\nPayload='%s'\n", LoRa.getDownLinkData().c_str());
        LedOff();
     }
  }
  
}

//================================
//    Functions to be executed periodically
//================================

#define LoRa_fPort_TEMP  12        // port 12 = 温度湿度気圧等

float bme_temp = 0;
float bme_humi = 0;
float bme_press = 0;

short port = LoRa_fPort_TEMP;    // port 12 = Temp

int temp = bme_temp * 100;
unsigned int humi = bme_humi * 100;
unsigned long press = bme_press * 100;


/*-------------------------------------------------------------*/
void task1(void)
{   
    char s[16];
    ConsolePrint(F("Temperature:  %s degrees C\n"), dtostrf(bme_temp, 6, 2, s));
    ConsolePrint(F("%%RH: %2d%s%%\n"), bme_humi);
    ConsolePrint(F("Pressure: %2d Pa\n"), bme_press);
    
    downLinkHdl( LoRa.sendString(port, true, F("%04x%04x%06lx"), temp, humi, press));
}

/*-------------------------------------------------------------*/
void task2(void)
{
    ConsolePrint(F("\n  Task2 invoked\n\n"));
    downLinkHdl(LoRa.sendStringConfirm(port, true, F("%04x%04x%06lx"), temp, humi, press));
}

/*-------------------------------------------------------------*/
void task3(void)
{
    //
    //  ToDo: Insert Task code
    //
}

//===============================
//            Execution interval
//    TASK( function, interval by second )
//===============================

TASK_LIST = {
        TASK(task1, 0, 15),
        TASK(task2, 5, 15),
        //TASK(task3, 7, 15),
        END_OF_TASK_LIST
};


/*   End of Program  */

