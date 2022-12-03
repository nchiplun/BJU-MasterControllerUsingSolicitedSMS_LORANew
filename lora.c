/*
 * File name            : lora.c
 * Compiler             : MPLAB XC8/ MPLAB C18 compiler
 * IDE                  : Microchip  MPLAB X IDE v5.25
 * Processor            : PIC18F66K40
 * Author               : Bhoomi Jalasandharan
 * Created on           : July 15, 2020, 05:23 PM
 * Description          : uart functions definitions source file
 */
#include "variableDefinitions.h"
#include "congfigBits.h"
#include "controllerActions.h"
#include "lora.h"


//*****************Serial communication function_Start****************//

/*************************************************************************************************************************

This function is called to transmit Byte data to Lora
The purpose of this function is to transmit Data loaded into Transmit buffer (TXREG) until Transmit flag (TXIF) is pulled down

 **************************************************************************************************************************/
// Transmit data through TX pin
void txByteLora(unsigned char serialData) {
    TX1REG = serialData; // Load Transmit Register
    while (PIR3bits.TX1IF == CLEAR); // Wait until TXIF gets low
    // ADD indication if infinite
}

/*************************************************************************************************************************

This function is called to transmit data to Lora in string format
The purpose of this function is to call transmit Byte data (txByte) Method until the string register reaches null.

 **************************************************************************************************************************/
void transmitStringToLora(const char *string) {
    // Until it reaches null
    while (*string) {
        txByteLora(*string++); // Transmit Byte Data
        myMsDelay(10);
    }
}

/*************************************************************************************************************************

This function is called to transmit data to Lora in Number format
The purpose of this function is to call transmit Byte data (txByte) Method until mentioned index.

 **************************************************************************************************************************/
void transmitNumberToLora(unsigned char *number, unsigned char index) {
    unsigned char j = CLEAR;
    // Until it reaches index no.
    while (j < index) {
        txByteLora(*number++); // Transmit Byte Data
        j++;
        myMsDelay(10);
    }
}
//*****************Serial communication functions_End****************//
