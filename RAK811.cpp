/*
 * RAK811.cpp
 *
 *   Created on: 2017/12/15
 *       Author: tomoaki@tomy-tech.com
 *
 */

#include <Application.h>
#include <string.h>
#include <stdarg.h>
#include <RAK811.h>
#include <Payload.h>

using namespace tomyApplication;
extern int getFreeMemory(void);
extern PortList_t thePortList[];

//
//
//     Class RAK811
//
//

RAK811::RAK811(void):
        _baudrate{9600}, _joinStatus{not_joined}, _txTimeoutValue{ LoRa_RECEIVE_DELAY2_RAK}, _stat{0}, _txFlg{false}
{
    _serialPort = &Serial;
    _maxPayloadSize = LoRa_DEFAULT_PAYLOAD_SIZE;
}

RAK811::~RAK811(void)
{

}


bool RAK811::begin(uint32_t baudrate)
{
    _baudrate = baudrate;
    uint32_t  br[] = { 9600, 19200, 57600, 115200};
    String param = (String)baudrate + F(",8,0,1,0");

    for ( uint8_t i = 0; i < 4; i++ )
    {
            LoRaDebug( F("change baudrate to %ld\n"), br[i]);

            _serialPort->begin(br[i]);
            getVersion();  // Clear buffer of RAK

            if ( sendCommand(ECHOFLAG, F("uart"), param, LoRa_SERIAL_WAIT_TIME_RAK) == LoRa_RC_SUCCESS )
            {
                if ( baudrate == br[i] )
                {
                    return true;
                }
                else
                {
                    LoRaDebug( F("Bauderare was changed. Reset & Restart !!!\n"));
                    return false;
                }
               // _serialPort->begin(baudrate);
               // return true;
            }
    }
     return false;
}

void RAK811::sleep(void)
{
    int rc = sendCommand( ECHOFLAG, F("sleep"),F(""));
    if ( rc )
    {
        LoRaDebug(F("Sleep Err = %d\n"), rc );
    }
}

void RAK811::wakeup(void)
{
    _serialPort->println(" ");
    recvEventResponse();
}

bool RAK811::connect(void)
{
    sendCommand(ECHOFLAG, F("join"), F("otaa"));
    int rc =  recvEventResponse(JOIN__WAIT_TIME_RAK);
    if ( rc == STATUS_JOINED_SUCCESS )
    {
        _joinStatus = joined;
        return true;
    }
    else
    {
        _joinStatus = not_joined;
        return false;
    }
}

bool RAK811::reconnect(void)
{
    return connect();
}

//
//
//  sendCommand( )
//
//
int RAK811::sendCommand(bool echo, String cmd, String param, uint32_t timeout)
{
    String command;
    if ( param == "" )
    {
        command = String(F("at+")) + cmd;
    }
    else
    {
        command = String(F("at+")) + cmd + String(F("=")) + param;
    }

    if ( echo )
    {
        ConsolePrint(F("\nSend  =>%s<=\n"), command.c_str());
    }

    _serialPort->print(command);
    _serialPort->print("\r\n");

    recvResponse((timeout == 0 ? _txTimeoutValue : timeout ));

    if (  _resp.startsWith(F("OK")) )
    {
        _resp.replace(F("OK"), F(""));
        return LoRa_RC_SUCCESS;
    }

    if (  _resp.startsWith(F("ERROR")) )
   {
        _resp.replace(F("ERROR"), F(""));
       return _resp.toInt();
   }
    return CODE_UNKNOWN_ERR;
}


int RAK811::sendCommand(bool echo, const __FlashStringHelper* cmd, const __FlashStringHelper* param, uint32_t timeout)
{
    String sparam = String(param);
    return sendCommand(echo, cmd, sparam, timeout);
}

int RAK811::sendCommand(bool echo, const __FlashStringHelper* cmd, String param, uint32_t timeout)
{
    String scmd = String(cmd);
    return sendCommand(echo, scmd, param, timeout);
}

void RAK811::recvResponse(uint32_t time)
{
    _resp = "";
    delay(100);
    uint32_t tim = millis() + time + 100;

    while (millis() < tim)
    {
        if (_serialPort->available() > 0)
        {
            char ch = _serialPort->read();
            _resp += String(ch);
            LoRaDebug(F("%02x "), ch);

            if ( _resp.indexOf(F("\r\n")) >= 0 )
            {
                  _resp.replace(F("\r\n"), F(""));
                  _resp.trim();
                 #ifdef FREE_MEMORY_CHECK
                 ConsolePrint(F("\nFree RAM is %d bytes\n"), getFreeMemory());
                 #endif
                  return;
            }
        }
    }
    _resp = "";
    return;
}

int RAK811::recvEventResponse( uint32_t time)
{
    recvResponse(time);
    int pos0 = _resp.indexOf("=");
    if ( pos0 > 0 )
    {
        int pos1 = _resp.indexOf(",", pos0);
        if ( pos1 > 0 )
        {
           return  _resp.substring(pos0 + 1, pos1).toInt();
         }
    }
    return STATUS_UNKNOWN;
}

Payload* RAK811::getDownLinkPayload(void)
{
    _payload.create(_maxPayloadSize);
    getDownLinkBinaryData(_payload.getRowData());
    return &_payload;
}

uint8_t RAK811::getDownLinkPort( void)
{
    int pos0 = _resp.indexOf(",");
    if ( pos0 >= 0 )
    {
        int pos1 = _resp.indexOf(",", pos0);
        return _resp.substring(pos0, pos1 - 1).toInt();
    }
    return 0;
}

//
//
//  sendXX( )
//  Return value: LoRa_RV_SUCCESS, LoRa_RV_DATA_TOO_LONG, LoRa_RV_NOT_JOINED, LoRa_RV_ERROR
//
//

int RAK811::sendString(uint8_t port, bool echo, const __FlashStringHelper* format,  ...)
{
    va_list args;
    va_start(args, format);
    int rc = transmitString(echo, port, false, format, args);
    va_end(args);

    return rc;
}


int RAK811::sendStringConfirm(uint8_t port, bool echo, const __FlashStringHelper* format,  ...)
{
    va_list args;
    va_start(args, format);
    int rc = transmitString(echo, port, true, format, args);
    va_end(args);

    return rc;
}


int RAK811::sendPayload(uint8_t port, bool echo, Payload* payload)
{
    return transmitBinaryData(echo, port, false, payload->getRowData(), payload->getLen());
}


int RAK811::sendPayloadConfirm(uint8_t port, bool echo, Payload* payload)
{
    return transmitBinaryData(echo, port, true, payload->getRowData(), payload->getLen());
}

int RAK811::transmitString(bool echo, uint8_t port, bool confirm, const __FlashStringHelper* format, va_list args)
{
    char param[(6 + _maxPayloadSize+2 )  ];     // <type>,<port>,   1+3+2=6
    memset(param, 0, (6 + _maxPayloadSize+1 )  );

    char* pos = 0;
    int len = 0;

    if (confirm)
    {
        param[0] = 0x31;
    }
    else
    {
        param[0] = 0x30;
    }
    pos = param  + 1;
    sprintf(pos, ",%d,", port);
    len = strlen(param);
    pos = param + len;

    vsnprintf_P(pos, sizeof(param) - len, (const char*)format, args);

    _stat = sendCommand(echo, F("send"), param,  _txTimeoutValue );
    if ( _stat == LoRa_RC_SUCCESS )
    {
        _stat = recvEventResponse();
        if ( _stat ==  STATUS_RECV_DATA )
        {
            return recvEventResponse( );
        }
    }
    return _stat;
}



int RAK811::transmitBinaryData(bool echo, uint8_t port, bool confirm, uint8_t* binaryData, uint8_t dataLen)
{
    uint8_t*  bd = binaryData;
    size_t  len = 0;
    char* pos = 0;
    char param[(6 + _maxPayloadSize*2+1)];
    memset(param, 0, (6+ _maxPayloadSize*2+1));


    if ( dataLen > _maxPayloadSize )
    {
        LoRaDebug(F("Error: Data %d is too long.\n"), dataLen);
        _stat = CODE_TX_LEN_LIMITE_ERR;
        goto exit;
    }

    if (confirm)
    {
        param[0] = 0x31;
    }
    else
    {
        param[0] = 0x30;
    }
    pos = param  + 1;
    sprintf(pos, ",%d,", port);
    len = strlen(param);
    pos = param + len;

    for ( uint8_t i = 0; i < dataLen; i++, bd++ )
    {
        sprintf(pos, "%02x", *bd);
        pos = param + strlen(param);
    }

    _stat = sendCommand(echo, F("send"), param,  _txTimeoutValue );
    if ( _stat == LoRa_RC_SUCCESS )
    {
        _stat = recvEventResponse();
        if ( _stat ==  STATUS_RECV_DATA )
        {
            return recvEventResponse( );
        }
    }
exit:
    _txFlg = false;
     return _stat;
 }


uint8_t RAK811::getDownLinkBinaryData(uint8_t* data)
{
    _resp = "";
   int pos1 = 0;
   int pos0= _resp.indexOf(",");
   pos0 = _resp.indexOf(",", pos0);
   if ( pos0 > 0 )
   {
       pos1 = _resp.lastIndexOf(",", pos0);
   }
   if ( pos1 > 0 )
   {
       int len = _resp.substring(pos0 + 1).toInt();
       if ( len > 0 )
       {
           uint8_t i = 0;
           for (++pos1; pos1 <= (uint8_t)_resp.length(); pos1 += 2 )
           {
               data[i] = ctoh(_resp[pos1]);
               data[i] = data[i] << 4;
               data[i++]  |= ctoh(_resp[pos1 + 1]);
           }
       return i;
       }
   }
   return 0;
}

String RAK811::getDownLinkData(void)
{
   int pos0 = 0;
   int pos1 = _resp.lastIndexOf(",");
   if ( pos1 >= 0 )
   {
       pos0 = _resp.lastIndexOf(",", pos1);
   }
   if ( pos0 >= 0 )
   {
       int len = _resp.substring(pos0, pos1 - 1).toInt();
       if ( len > 0 )
       {
           return _resp.substring(pos0, pos1 - 1);
       }
   }
   return String(F(""));
}

uint8_t RAK811::ctoh(uint8_t ch)
{
    if ( ch >= 0x30 && ch <=0x39 )
    {
        return ch - 0x30;
    }
    else if ( ch >= 0x41 && ch <= 0x46 )
    {
        return ch - 0x37;
    }
    return 0;
}


void RAK811::checkDownLink(void)
{
    if ( _stat == LoRa_RC_SUCCESS && _txFlg == true )
    {
        uint8_t port = getDownLinkPort();
        if ( port )
        {
            for ( uint8_t i = 0; thePortList[i].port != 0;  i++ )
            {
                if ( port == thePortList[i].port  )
                {
                    thePortList[i].callback();
                    break;
                }
            }
        }
    }
    _txFlg = false;
}


//
//
//    Getters
//
//

String RAK811::getVersion(void)
{
    sendCommand(ECHOFLAG, F("version"), F(""));
    return _resp;
}

int RAK811::getLinkCount(uint16_t* upCnt, uint16_t* dwnCnt)
{
    int rc = sendCommand(ECHOFLAG, F("link_cnt"), F(""));
    if ( rc == LoRa_RC_SUCCESS )
    {
        int pos0 = _resp.indexOf("=");
        if ( pos0 > 0 )
        {
            int pos1 = _resp.indexOf(",", pos0);
            *upCnt = (uint16_t)(_resp.substring(pos0, pos1 - 1).toInt());
            *dwnCnt =  (uint16_t)(_resp.substring(pos1).toInt());
            return LoRa_RC_SUCCESS;
        }
    }
    return LoRa_RC_ERROR;
}

String RAK811:: get_config(String key)
{
    sendCommand(ECHOFLAG, F("get_config"), key);
    return _resp;
}

uint8_t RAK811::getMaxPayloadSize(void)
{
    return _maxPayloadSize;
}

//
//
//    Setters
//
//
int RAK811::setADR(bool flg)
{
    String param = F("adr:");
    if ( flg )
    {
        param += String(F("on"));
    }
    else
    {
        param += String("off");
    }
    return setConfig(param);
}

int RAK811::setDr(LoRaDR dr)
{
    if ( sendCommand(ECHOFLAG, F("dr"), String(dr), LoRa_INIT_WAIT_TIME_RAK) == LoRa_RC_SUCCESS )
    {
        uint8_t pll[] = { 0, 0, 11, 53, 125, 242 };
        _maxPayloadSize = pll[dr];
        return _maxPayloadSize;
    }
    return LoRa_RC_ERROR;
}

int RAK811::setLinkCount(uint16_t upCnt, uint16_t dwnCnt)
{
    String param = String(upCnt) + "," + String(dwnCnt);
    return sendCommand(ECHOFLAG, F("link_cnt"), param);
}

int RAK811::setConfig(String param)
{
    return sendCommand(ECHOFLAG, F("set_config"), param, LoRa_INIT_WAIT_TIME_RAK);
}

