/*
 * KashiwaGeeks.h
 * 
 *   Created on: 2017/11/25
 *       Author: tomoaki@tomy-tech.com
 *
 */

#ifndef KASHIWAGEEKS_H_
#define KASHIWAGEEKS_H_

#include <ADB922S.h>
#include <Application.h>
#include <Payload.h>
using namespace tomyApplication;

extern Application *theApplication;

uint32_t getSysTime(void);
void DisableConsole(void);
void DisableDebug(void);
void DebugPrint(const char *fmt, ...);
void DebugPrint(const __FlashStringHelper *format, ...);
void ConsolePrint(const char *fmt, ...);
void ConsolePrint(const __FlashStringHelper *format, ...);
void setWDT(uint8_t sec);


#endif /* KASHIWAGEEKS_H_ */
