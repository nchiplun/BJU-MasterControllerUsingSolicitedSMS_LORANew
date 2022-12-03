/*
 * File name            : serialMonitor.h
 * Compiler             : MPLAB XC8/ MPLAB C18 compiler
 * IDE                  : Microchip  MPLAB X IDE v5.25
 * Processor            : PIC18F66K40
 * Author               : Bhoomi Jalasandharan
 * Created on           : July 15, 2020, 05:23 PM
 * Description          : UART functions header file
 */

#ifdef DEBUG_MODE_ON_H

#ifndef UART2_H
#define	UART2_H

/***************************** Serial communication functions declarations#start ****/
void txByteDebug(unsigned char); // To transmit a byte to Debug
void transmitStringToDebug(const char *); // To transmit string of bytes to debug
void transmitNumberToDebug(unsigned char*, unsigned char); // To transmit array of bytes to debug
#endif
/***************************** Serial communication functions declarations#end ******/

#endif
/* UART2_H */