/*
 * RAK811.h
 *
 *   Created on: 2017/12/15
 *       Author: tomoaki@tomy-tech.com
 *
 */

#ifndef LIBRARIES_KASHIWAGEEKS_RAK811_H_
#define LIBRARIES_KASHIWAGEEKS_RAK811_H_

#include <Application.h>
#include <Payload.h>

namespace tomyApplication
{
extern tomyApplication::SerialLog* theLog;
//
//  LoRaWAN defines
//
#define SOFTCONSOLE
#define LoRa_DEFAULT_PAYLOAD_SIZE      11

// Error Code
#define CODE_ARG_ERR    -1
#define CODE_ARG_NOT_FIND    -2
#define CODE_JOIN_ABP_ERR    -3
#define CODE_JOIN_OTAA_ERR    -4
#define CODE_NOT_JOIN    -5
#define CODE_MAC_BUSY_ERR    -6
#define CODE_TX_ERR    -7
#define CODE_INTER_ERR    -8
#define CODE_WR_CFG_ERR    -11
#define CODE_RD_CFG_ERR    -12
#define CODE_TX_LEN_LIMITE_ERR -13
#define CODE_UNKNOWN_ERR -20

// Event code
#define STATUS_RECV_DATA 0
#define STATUS_TX_COMFIRMED 1
#define STATUS_TX_UNCOMFIRMED 2
#define STATUS_JOINED_SUCCESS 3
#define STATUS_JOINED_FAILED 4
#define STATUS_TX_TIMEOUT 5
#define STATUS_RX2_TIMEOUT 6
#define STATUS_DOWNLINK_REPEATED 7
#define STATUS_WAKE_UP 8
#define STATUS_P2PTX_COMPLETE 9
#define STATUS_UNKNOWN 100

#define LoRa_RC_SUCCESS            0
#define LoRa_RC_DATA_TOO_LONG     -1
#define LoRa_RC_NOT_JOINED        -2
#define LoRa_RC_ERROR             -3

// Timeout values
#define LoRa_INIT_WAIT_TIME_RAK      2000
#define LoRa_SERIAL_WAIT_TIME_RAK    3000
#define LoRa_RECEIVE_DELAY2_RAK      5000
#define JOIN__WAIT_TIME_RAK         30000

// Debug log
#ifndef LoRaDebug
#ifdef SHOW_LORA_TRANSACTION
#define LoRaDebug(...)  DebugPrint( __VA_ARGS__)
#define ECHOFLAG  true
#else
#define LoRaDebug(...)
#define ECHOFLAG  false
#endif
#endif

#ifndef PORT_LIST
#define PORT_LIST   PortList_t  thePortList[]
#define PORT(...)         {__VA_ARGS__}
#define END_OF_PORT_LIST  {0, 0}

typedef enum
{
    dr0, dr1, dr2, dr3, dr4, dr5
}LoRaDR;

typedef enum
{
    ch0, ch1, ch2, ch3, ch4, ch5, ch6, ch7, ch8, ch9, ch10, ch11, ch12, ch13, ch14, ch15, ch16
}CHID;

typedef  enum {
    joined, not_joined
}JoineStatus;

typedef struct PortList
{
    uint8_t port;
    void (*callback)(void);
} PortList_t;

#endif

//
//
//   RAK811
//
//
class RAK811
{
public:
    RAK811(void);
    ~RAK811(void);

    bool begin(uint32_t baudrate = 9600);
    bool connect(void);
    bool reconnect(void);

    int sendString(uint8_t port, bool echo, const __FlashStringHelper* format, ...);
    int sendStringConfirm(uint8_t port, bool echo, const __FlashStringHelper* format, ...);
    int sendPayload(uint8_t port, bool echo, Payload* payload);
    int sendPayloadConfirm(uint8_t port, bool echo, Payload* payload);

    uint8_t getDownLinkPort( void);
    Payload* getDownLinkPayload(void);
    String getDownLinkData(void);
    void checkDownLink(void);

    void sleep(void);
    void wakeup(void);

    String getVersion(void);
    String get_config(String key);
    int getLinkCount(uint16_t* upCnt, uint16_t* dwnCnt);
    uint8_t getMaxPayloadSize(void);
    int setDr(LoRaDR dr);
    int setConfig(String param);
    int setLinkCount(uint16_t upCnt, uint16_t dwnCnt);


private:
    bool isConnected(void);
    int transmitString(bool echo, uint8_t port, bool confirm, const __FlashStringHelper* format, va_list args);
    int transmitBinaryData(bool echo,  uint8_t port, bool confirm, uint8_t* data, uint8_t dataLen);
    int sendCommand(bool echo, String cmd, String param, uint32_t timeout = LoRa_INIT_WAIT_TIME_RAK);
    int sendCommand(bool echo, const __FlashStringHelper* cmd, const __FlashStringHelper* param,  uint32_t timeout = LoRa_INIT_WAIT_TIME_RAK);
    int sendCommand(bool echo, const __FlashStringHelper* cmd, String paramparam,  uint32_t timeout = LoRa_INIT_WAIT_TIME_RAK);
    void recvResponse(uint32_t time);
    int recvEventResponse(uint32_t time = LoRa_SERIAL_WAIT_TIME_RAK);
    uint8_t getDownLinkBinaryData(uint8_t* data);
    uint8_t ctoh(uint8_t ch);

    HardwareSerial*  _serialPort;
    uint32_t  _baudrate;
    JoineStatus  _joinStatus;
    uint8_t  _maxPayloadSize;
    uint32_t  _txTimeoutValue;
    String  _resp;
    Payload _payload;
    int  _stat;
    bool   _txFlg;
};

}



#endif /* LIBRARIES_KASHIWAGEEKS_RAK811_H_ */
