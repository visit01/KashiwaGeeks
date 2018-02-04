#include <KashiwaGeeks.h>
#include <Wire.h>
#include "KGPS.h"

#define ADB        //  comment out this line for RAK811

#ifdef ADB
ADB922S LoRa;
#define RAK_CONFIG
#else
RAK811 LoRa;
#define RAK_CONFIG "dev_eui:xxxx&app_eui:xxxx&app_key:xxxx"
#endif

KGPS gps;
uint8_t portGPS = 12;
uint8_t portTemp = 13;

//================================
//          Initialize Device Function
//================================
#define BPS_9600       9600
#define BPS_19200     19200
#define BPS_57600     57600
#define BPS_115200   115200

#define CONSOLE_Rx_PIN    4
#define CONSOLE_Tx_PIN    5

void start()
{
    /*  Setup console */
#ifdef ADB
    ConsoleBegin(BPS_57600);
#else
    ConsoleBegin(BPS_19200, CONSOLE_Rx_PIN, CONSOLE_Tx_PIN);
#endif

    //DisableDebug();

    /*
     * Enable Interrupt 0 & 1  Uncomment the following two lines.
     * For ADB922S, CUT the pin2 and 3 of the Sheild.
     */
    pinMode(2, INPUT_PULLUP);
    pinMode(3, INPUT_PULLUP);

    /*  setup Power save Devices */
    power_adc_disable();       // ADC converter
    power_spi_disable();       // SPI

    /*  setup the ADB922S  */
    if ( LoRa.begin(BPS_19200) == false )
    {
        while(true)
        {
            LedOn();
            delay(300);
            LedOff();
            delay(300);
        }
    }
    LoRa.setConfig(RAK_CONFIG);

    /* set DR. therefor, a payload size is fixed. */
    //LoRa.setDr(dr3);  // dr0 to dr5

    /* setup the GPS */
    gps.begin(9600, 8, 9);
    ConsolePrint(F("Initilizing GPS\n"));
    while( !gps.isReady() ){ };

    /* setup I2C */
    Wire.begin();

    /*  join LoRaWAN */
    LoRa.reconnect();

    /*  setup WDT interval to 1, 2, 4 or 8 seconds  */
   setWDT(2);    // set to 2 seconds
}


//========================================
// Functions to be executed periodically
//========================================
void taskTemp(void)
{
    sendTemp();
}
//========================================
//            Execution interval
//    TASK( function, interval by second )
//========================================
TASK_LIST = {
        TASK(taskTemp, 0, 10),
        END_OF_TASK_LIST
};


//================================
//          INT0 callbaks
//================================
void int0D2(void)
{
    DebugPrint(F("********* INT0 *******\n"));
    sendLocation();
}


//================================
//          Power save functions
//================================
void sleep(void)
{
    LoRa.sleep();
    gps.sleep();
    DebugPrint(F("Sleep.\n"));
}
void wakeup(void)
{
    LoRa.wakeup();
    DebugPrint(F("Wakeup.\n"));
}


//================================
// GPS Function
//================================

void sendLocation(void)
{
    gps.wakeup();
    while( !gps.isReady() ){ };
    Payload* pl = gps.getPayload();
    if (pl)
    {
        LoRa.sendPayload(portTemp, true, pl);
    }
}

//================================
// I2C ADT7410 Sensor Functions
//================================
void sendTemp(void)
{
  Wire.requestFrom(0x48,2);
  uint16_t val = Wire.read() << 8;
  val |= Wire.read();
  val >>= 3;    // convert to 13bit format
  int ival = (int)val;
  if ( val & (0x8000 >> 3) )
  {
    ival -= 8192;
  }
  float temp = (float)ival / 16.0;
  char buf[6];
  DebugPrint(F("Temp=%s [C]\n"), dtostrf(temp,3, 2, buf));

  Payload pl(LoRa.getMaxPayloadSize());
  pl.set_float(temp);
  LoRa.sendPayloadConfirm(portTemp, true, &pl);
}

/*   End of Program  */
