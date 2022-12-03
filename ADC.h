/*
 * File name            : ADC.h
 * Compiler             : MPLAB XC8/ MPLAB C18 compiler
 * IDE                  : Microchip  MPLAB X IDE v5.25
 * Processor            : PIC18F66K40
 * Author               : Bhoomi Jalasandharan
 * Created on           : July 15, 2020, 05:23 PM
 * Description          : ADC functions header file
 */

#ifndef ADC_H
#define	ADC_H

void selectChannel(unsigned char);
unsigned int getADCResult(void);

#endif
 /* ADC_H */