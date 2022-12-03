/*
 * File name            : delay.h
 * Compiler             : MPLAB XC8/ MPLAB C18 compiler
 * IDE                  : Microchip  MPLAB X IDE v5.25
 * Processor            : PIC18
 * Author               : Nandesh
 * Created on           : October 21, 2019, 05:23 PM
 * Description          : Delay functions header file
 */

#ifndef DELAY__H
#define	DELAY__H

/*____________________Milli seconds delay function header_________________________________*/
void delay_ms(unsigned char t);
/*
 * Include          :       delay.h
 * Description      :       generates milli seconds delay
 * Arguments        :       Decimal
 * Returns          :       None
 */



/* ______________________Micro seconds delay function header. ____________________________ */
void delay_us(unsigned char t);
/*
 * Include          :       delay.h
 * Description      :       generates micro seconds delay
 * Arguments        :       Decimal
 * Returns          :       None
 */


/* ___________________________Seconds delay function header______________________________ */
void Delay_s(unsigned char t);
/*
 * Include          :       delay.h
 * Description      :       generates seconds delay  
 * Arguments        :       Decimal
 * Returns          :       None
 */
void DelayFor18TCY( void );  
extern void DelayXLCD (void);
void DelayXLCD (void);

#endif	/* DELAY__H */

