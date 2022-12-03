/*
 * File name            : gsm.h
 * Compiler             : MPLAB XC8/ MPLAB C18 compiler
 * IDE                  : Microchip  MPLAB X IDE v5.25
 * Processor            : PIC18F66K40
 * Author               : Bhoomi Jalasandharan
 * Created on           : July 15, 2020, 05:23 PM
 * Description          : encoding-decoding functions header file
 */

#ifdef Encryption_ON_H

#ifndef Encoding_H
#define	Encoding_H

/****************Message Encryption and Decryption functions declarations#start ****/

void base64Encoder(void); // To encode message to GSM
void base64Decoder(void); // To decode message from GSM

/****************Message Encryption and Decryption functions declarations#end ******/

#endif
#endif
/* Encoding_H */