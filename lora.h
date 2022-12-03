/*
 * File name            : lora.h
 * Compiler             : MPLAB XC8/ MPLAB C18 compiler
 * IDE                  : Microchip  MPLAB X IDE v5.25
 * Processor            : PIC18F66K40
 * Author               : Bhoomi Jalasandharan
 * Created on           : July 15, 2020, 05:23 PM
 * Description          : UART functions header file
 */

#ifndef UART1_H
#define	UART1_H

/***************************** Lora Serial communication functions declarations#start ****/
void txByteLora(unsigned char); // To transmit a byte to Lora
void transmitStringToLora(const char *); // To transmit string of bytes to Lora
void transmitNumberToLora(unsigned char*, unsigned char); // To transmit array of bytes to Lora
/***************************** Lora Serial communication functions declarations#end ******/

#endif
/* UART1_H */