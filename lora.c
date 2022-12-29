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


//***************** Lora Serial communication function_Start****************//

/*************************************************************************************************************************

This function is called to receive Byte data from GSM
The purpose of this function is to return Data loaded into Reception buffer (RCREG) until Receive flag (RCIF) is pulled down

 **************************************************************************************************************************/
unsigned char rxByteLora(void) {
    while (PIR3bits.RC1IF == CLEAR); // Wait until RCIF gets low
    // ADD indication if infinite
    return RC1REG; // Return data stored in the Reception register
}

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

/*************************************************************************************************************************

This function is called to send cmd to Lora as per received action to given field no.

 **************************************************************************************************************************/
void sendCmdToLora(unsigned char Action, unsigned char FieldNo) {
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("sendCmdToLora_IN\r\n");
    //********Debug log#end**************//
#endif
    checkLoraConnection = true;
    // for field no. 01 to 09
    if (FieldNo<9){
        temporaryBytesArray[0] = 48; // To store field no. of valve in action 
        temporaryBytesArray[1] = FieldNo + 49; // To store field no. of valve in action 
    }// for field no. 10 to 12
    else if (FieldNo > 8 && FieldNo < 12) {
        temporaryBytesArray[0] = 49; // To store field no. of valve in action 
        temporaryBytesArray[1] = FieldNo + 39; // To store field no. of valve in action 
    }
    myMsDelay(100);
    controllerCommandExecuted = false;
    timer3Count = 15; // 15 second window
    T3CONbits.TMR3ON = ON; // Start timer thread to unlock system if GSM fails to respond within 5 min
    switch (Action) {
    case 0x00:
        transmitStringToLora("#ON01TIME");
        temporaryBytesArray[2]=fieldValve[FieldNo].onPeriod%10;
        temporaryBytesArray[3]=fieldValve[FieldNo].onPeriod/10;
        temporaryBytesArray[3]=temporaryBytesArray[3]%10;
        temporaryBytesArray[4]=fieldValve[FieldNo].onPeriod/100;
        transmitNumberToLora(temporaryBytesArray+2,3);
        transmitStringToLora("SLAVE");
        transmitNumberToLora(temporaryBytesArray,2);
        transmitStringToLora("$");
        myMsDelay(100);
        break;
    case 0x01:
        transmitStringToLora("#OFF01SLAVE");
        transmitNumberToLora(temporaryBytesArray,2);
        transmitStringToLora("$");
        myMsDelay(100);
        break;
    case 0x02:
        transmitStringToLora("#GETSENSOR01SLAVE");
        transmitNumberToLora(temporaryBytesArray,2);
        transmitStringToLora("$");
        myMsDelay(100);
        break;
    }
    while (!controllerCommandExecuted); // wait until lora responds to send cmd action
    checkLoraConnection = false;
    if (LoraConnectionFailed) {
        loraAttempt++;
    }
    else if (isLoraResponseAck(Action,FieldNo)) {
        LoraConnectionFailed = false;
        loraAttempt = 99;
    }
    else if (isLoraResponseAck(Action,FieldNo)== off) {
        LoraConnectionFailed = true;
        loraAttempt++;
    }
    PIR5bits.TMR3IF = SET; //Stop timer thread
    setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
    myMsDelay(500);
    
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("sendCmdToLora_OUT\r\n");
    //********Debug log#end**************//
#endif
}

/*************************************************************************************************************************

This function is called to send cmd to Lora as per received action to given field no.

 **************************************************************************************************************************/
_Bool isLoraResponseAck(unsigned char Action, unsigned char FieldNo) {
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("isLoraResponseAck_IN\r\n");
    //********Debug log#end**************//
#endif
    unsigned char field = 99;
    myMsDelay(100);
    switch (Action) {
    case 0x00:
        field = fetchFieldNo(10);
        if(strncmp(decodedString+1, slaveOnOK, 9) == 0 && strncmp(decodedString+9, ack, 3) == 0 && field == FieldNo) {
            return true;
        }
        break;
    case 0x01:
        field = fetchFieldNo(11);
        if(strncmp(decodedString+1, slaveOffOK, 10) == 0 && strncmp(decodedString+10, ack, 3) == 0 && field == FieldNo) {
            return true;
        }
        break;
    case 0x02:
        field = fetchFieldNo(11);
        if(strncmp(decodedString+6, slave, 5) == 0 && field == FieldNo) {
            return true;
        }
    }
    if(strncmp(decodedString+1, masterError, 11) == 0) {
        return false;
    }
    else if(strncmp(decodedString+1, slaveError, 10) == 0) {
        return false;
    }
    return false;
    
#ifdef DEBUG_MODE_ON_H
//********Debug log#start************//
transmitStringToDebug("isLoraResponseAck_OUT\r\n");
//********Debug log#end**************//
#endif
}

//*****************Lora Serial communication functions_End****************//
