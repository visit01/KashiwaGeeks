#include <KashiwaGeeks.h>
#include <TinyGPS++.h>

ADB922S LoRa;
TinyGPSPlus gps;
SoftwareSerial gpsSerial(8, 9); // RX, TX

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
    //DisableDebug();

    /*
     * Enable Interrupt 0 & 1  Uncomment the following two  lines.
     * For ADB922S only , CUT the pin2 and 3 of the Sheild.
     */
    //pinMode(2, INPUT_PULLUP);
    //pinMode(3, INPUT_PULLUP);

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

    /* set DR. therefor, a payload size is fixed. */
    //LoRa.setDr(dr3);  // dr0 to dr5

    /* setup the GPS */
    pinMode(8, INPUT);
    gpsSerial.begin(BPS_9600);
    GpsWakeup();
    while( !isGpsReady() ){ };

    /*  join LoRaWAN */
    LoRa.reconnect();

    /*  seetup WDT interval to 1, 2, 4 or 8 seconds  */
    //setWDT(8);    // set to 8 seconds
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
    GpsWakeup();
    while( !isGpsReady() ){ };
    LoRa.wakeup();
    DebugPrint(F("Wakeup.\n"));
}
//================================
//          INT0, INT2 callbaks
//================================
void int0D2(void)
{
    DebugPrint(F("********* INT0 *******\n"));
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

            DebugPrint("%c", c);
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
  uint8_t port = 12;

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

        LoRa.sendPayloadConfirm(port, true, &pl);
  }
}

//========================================
// Functions to be executed periodically
//========================================
void task1(void)
{
    GpsSend();
}
//========================================
//            Execution interval
//    TASK( function, interval by second )
//========================================
TASK_LIST = {
        TASK(task1, 0, 15), // GPS
        END_OF_TASK_LIST
};
/*   End of Program  */
