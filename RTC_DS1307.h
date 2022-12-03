/*
 * File name            : RTC_DS1307.h
 * Compiler             : MPLAB XC8/ MPLAB C18 compiler
 * IDE                  : Microchip  MPLAB X IDE v5.25
 * Processor            : PIC18F66K40
 * Author               : Bhoomi Jalasandharan
 * Created on           : July 15, 2020, 05:23 PM
 * Description          : RTC_DS1307 functions header file
 */

#ifndef RTC_DS1307_H
#define	RTC_DS1307_H

/**********Function Prototypes**********/
void i2cStart(void);
void i2cRestart(void);
void i2cStop(void);
void i2cWait(void);
void i2cSend(unsigned char);
unsigned char i2cRead(_Bool);
void fetchTimefromRTC(void);
unsigned char decimal2BCD (unsigned char);
unsigned char bcd2Decimal (unsigned char bcd);
void feedTimeInRTC(void);
/************************************/
#endif
 /* RTC_DS1307_H */