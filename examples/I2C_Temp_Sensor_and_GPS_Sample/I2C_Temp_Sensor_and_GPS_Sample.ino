
#include <KashiwaGeeks.h>
#include <TinyGPS++.h>
#include <Wire.h>

#define ADB        //  comment out this line when you use RAK811

#ifdef ADB
ADB922S LoRa;
#define RAK_CONFIG
#else
RAK811 LoRa;
#define RAK_CONFIG "dev_eui:xxxx&app_eui:xxxx&app_key:xxxx"
#endif

TinyGPSPlus gps;
SoftwareSerial gpsSerial(8, 9); // RX, TX
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

    /*  setup Power save Devices */
    power_adc_disable();       // ADC converter
    power_spi_disable();       // SPI

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
    LoRa.setConfig(RAK_CONFIG);

    /* set DR. therefor, a payload size is fixed. */
    //LoRa.setDr(dr3);  // dr0 to dr5

    /* setup the GPS */
    pinMode(8, INPUT);
    gpsSerial.begin(BPS_9600);
    ConsolePrint(F("Initializing GPS\n"));
    GpsWakeup();
    while( !isGpsReady() ){ };

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
    GpsWakeup();
    while( !isGpsReady() ){ };
    GpsSend();
}


//================================
//          Power save functions
//================================
void sleep(void)
{
    LoRa.sleep();
    GpsSleep();
    DebugPrint(F("Sleep.\n"));
}
void wakeup(void)
{
    LoRa.wakeup();
    DebugPrint(F("Wakeup.\n"));
}


//================================
// GPS Functions
//================================
void GpsWakeup(void)
{
    /* Provide Vcc */
    digitalWrite(10, HIGH);
}

void GpsSleep(void)
{
    digitalWrite(10, LOW);
}

bool isGpsReady(void)
{
    gpsSerial.listen();
    uint32_t  tim = millis() + 3000;

    while (millis() < tim)
    {
        if (gpsSerial.available() > 0)
        {
            char c = gpsSerial.read();
            gps.encode(c);

            if (gps.location.isUpdated())
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
}

void GpsSend(void)
{
  if ( isGpsReady() )
  {
        uint8_t hdopGps = gps.hdop.value()/10;
        uint32_t Latitude = ((gps.location.lat() + 90) / 180.0) * 16777215;
        uint32_t Longitude = ((gps.location.lng() + 180) / 360.0) * 16777215;
        uint16_t alt = gps.altitude.meters();

        Payload pl(LoRa.getMaxPayloadSize());
        pl.set_uint24(Latitude);
        pl.set_uint24(Longitude);
        pl.set_uint16(alt);
        pl.set_uint8(hdopGps);

        /*  Debug purpose */
        uint32_t la = pl.get_uint24();
        uint32_t lo = pl.get_uint24();
        uint16_t al = pl.get_uint16();
        uint8_t hd = pl.get_uint8();
        DebugPrint(F("Lat=%"PRIu32", Lon=%"PRIu32", Alt=%"PRIu16 ", %d\n"), Latitude, Longitude, alt, hdopGps);
        DebugPrint(F("Lat=%"PRIu32", Lon=%"PRIu32", Alt=%"PRIu16 ", %d\n"), la, lo, al, hd);

        LoRa.sendPayloadConfirm(portGPS, true, &pl);
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
