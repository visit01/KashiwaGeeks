
#include <KashiwaGeeks.h>

ADB922S LoRa;

//================================
//          Initialize Device Function
//================================
#define BPS_9600           9600
#define BPS_19200       19200
#define BPS_57600       57600
#define BPS_115200   115200

void start()
{
    /*  Setup console */
    Serial.begin(BPS_57600);
    //DisableConsole();
    //DisableDebug();

    ConsolePrint(F("**** Start*****\n"));

    /*  setup Power save Devices */
    //power_adc_disable();       // ADC converter
    //power_spi_disable();       // SPI
    //power_timer1_disable();    // Timer1
    //power_timer2_disable();    // Timer2, tone()
    //power_twi_disable();       // I2C

    /*  setup ADB922S  */
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

    /* set DR. therefor, a payload size is fixed. */
    LoRa.setDr(dr3);  // dr0 to dr5
    
    /*  join LoRaWAN */
    LoRa.reconnect();


    /*  for BME280 initialize  */
     //bme.begin();

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
  ConsolePrint(F("\nINT0 !!!\n"));
}

void int1D3(void)
{
  ConsolePrint(F("\nINT1 !!!\n"));
}


//================================
//    DownLink Data handler
//================================
void port14(void)
{
  ConsolePrint("%s\n", LoRa.getDownLinkData().c_str());
  LedOn();
}

void port15(void)
{
  ConsolePrint("%s\n", LoRa.getDownLinkData().c_str());
  LedOff();
}

PORT_LIST = { 
  PORT(14, port14),  // port & callback
  PORT(15, port15),
  END_OF_PORT_LIST
};

//================================
//    Functions to be executed periodically
//================================

#define LoRa_fPort_TEMP  12   
float bme_temp = 10;
float bme_humi = 20;
float bme_press = 50;

short port = LoRa_fPort_TEMP;   

int16_t temp = bme_temp * 100;
uint16_t humi = bme_humi * 100;
uint32_t press = bme_press * 100;


/*-------------------------------------------------------------*/
void task1(void)
{   
    char s[16];
    ConsolePrint(F("Temperature:  %s degrees C\n"), dtostrf(bme_temp, 6, 2, s));
    ConsolePrint(F("%%RH: %2d%s%%\n"), bme_humi);
    ConsolePrint(F("Pressure: %2d Pa\n"), bme_press);
    
    disableInterrupt();     //  INT0 & INT1 are disabled
      
    Payload pl(LoRa.getMaxPayloadSize());
    pl.set_int16(temp);
    pl.set_uint16(humi);
    pl.set_uint32(press);
    
    LoRa.sendPayload(port, true, &pl);    
    LoRa.checkDownLink();
    
    enableInterrupt();     //  INT0 & INT1 are enabled
}

/*-------------------------------------------------------------*/
void task2(void)
{
    ConsolePrint(F("\n  Task2 invoked\n\n"));
    disableInterrupt();     //  INT0 & INT1 are disabled
    LoRa.sendStringConfirm(port, true, F("%04X%04X%08X"), temp, humi, press);
    LoRa.checkDownLink();
    enableInterrupt();     //  INT0 & INT1 are enabled
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
        TASK(task2, 8, 15),
        //TASK(task3),
        END_OF_TASK_LIST
};


/*   End of Program  */

