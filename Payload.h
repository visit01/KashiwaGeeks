/*
 * Payload.h
 *
 *   Created on: 2017/12/09
 *       Author: tomoaki@tomy-tech.com
 *
 */

#ifndef LIBRARIES_KASHIWAGEEKS_PAYLOAD_H_
#define LIBRARIES_KASHIWAGEEKS_PAYLOAD_H_

#include <Application.h>

namespace tomyApplication
{
#define MSGPACK_FALSE    0xc2
#define MSGPACK_TRUE     0xc3
#define MSGPACK_POSINT   0x80
#define MSGPACK_NEGINT   0xe0
#define MSGPACK_UINT8    0xcc
#define MSGPACK_UINT16   0xcd
#define MSGPACK_UINT32   0xce
#define MSGPACK_INT8     0xd0
#define MSGPACK_INT16    0xd1
#define MSGPACK_INT32    0xd2
#define MSGPACK_FLOAT32  0xca
#define MSGPACK_FIXSTR   0xa0
#define MSGPACK_STR8     0xd9
#define MSGPACK_STR16    0xda
#define MSGPACK_ARRAY15  0x90
#define MSGPACK_ARRAY16  0xdc
#define MSGPACK_MAX_ELEMENTS   50   // Less than 256

/*=====================================
        Class Payload
  =====================================*/
class Payload{
public:
    Payload();
    Payload(uint16_t len);
    ~Payload();

/*---------------------------------------------
  getLen() and getRowData() are
  minimum required functions of Payload class.
----------------------------------------------*/
    uint16_t getLen();       // get data length
    uint8_t* getRowData();   // get data pointer

/*--- Functions for MessagePack ---*/
    void init(void);
    int8_t set_bool(bool val);
    int8_t set_uint32(uint32_t val);
    int8_t set_int32(int32_t val);
    int8_t set_float(float val);
    int8_t set_str(char* val);
    int8_t set_str(const char* val);
    int8_t set_array(uint8_t val);

    bool    get_bool(uint8_t index);
    uint8_t getArray(uint8_t index);
    uint32_t get_uint32(uint8_t index);
    int32_t  get_int32(uint8_t index);
    float    get_float(uint8_t index);
    const char* get_str(uint8_t index, uint16_t* len);

    void     getPayload(uint8_t* payload, uint16_t payloadLen);
    uint16_t getAvailableLength();
private:
    uint8_t* getBufferPos(uint8_t index);
    uint8_t* _buff;
    uint16_t _len;
    uint8_t  _elmCnt;
    uint8_t* _pos;
    uint8_t  _memDlt;
};


};
#endif /* LIBRARIES_KASHIWAGEEKS_PAYLOAD_H_ */
