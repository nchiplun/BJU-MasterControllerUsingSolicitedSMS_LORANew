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
#ifdef DEBUG_MODE_ON_H
#include "serialMonitor.h"
#endif


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
    setBCDdigit(0x06,1);  // (6) BCD indication for sendCmdToLora action
    checkLoraConnection = true;
    LoraConnectionFailed = false;
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
    timer3Count = 10; // 10 second window
    T3CONbits.TMR3ON = ON; // Start timer thread to unlock system if GSM fails to respond within 5 min
    switch (Action) {
    case 0x00:
        transmitStringToLora("#ON01TIME");
        myMsDelay(10);
        if (fieldValve[FieldNo].onPeriod > 0 && fieldValve[FieldNo].onPeriod < 995) {
            lower8bits = fieldValve[FieldNo].onPeriod + 5; // 5 count extra to lora auto timeout // fieldValve[FieldNo].onPeriod  Need to calculate on period for interrupted valve
        }
        else {
            lower8bits = fieldValve[FieldNo].onPeriod;
        }
        //temporaryBytesArray[2] = (unsigned char) ((lower8bits / 10000) + 48);
        //lower8bits = lower8bits % 10000;
        //temporaryBytesArray[3] = (unsigned char) ((lower8bits / 1000) + 48);
        //lower8bits = lower8bits % 1000;
        // Only last three digits i.e. 001 to 999
        temporaryBytesArray[4] = (unsigned char) ((lower8bits / 100) + 48);
        lower8bits = lower8bits % 100;
        temporaryBytesArray[5] = (unsigned char) ((lower8bits / 10) + 48);
        lower8bits = lower8bits % 10;
        temporaryBytesArray[6] = (unsigned char) (lower8bits + 48);
        transmitNumberToLora(temporaryBytesArray+4,3);
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
    PIR5bits.TMR3IF = SET; //Stop timer thread
    checkLoraConnection = false;
    if (LoraConnectionFailed) {  // No response from lora slave
        loraAttempt++;
        #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("No Response from Lora\r\n");
        //********Debug log#end**************//
        #endif   
    }
    else if (isLoraResponseAck(Action,FieldNo)) {  // correct response from lora slave
        LoraConnectionFailed = false;
        loraAttempt = 99;
        #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("isLoraResponseAck(Action,FieldNo)== true\r\n");
        //********Debug log#end**************//
        #endif
    }
    else {  // error response from lora slave
        LoraConnectionFailed = true;
        loraAttempt++;
        #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("isLoraResponseAck(Action,FieldNo)== off\r\n");
        //********Debug log#end**************//
        #endif
    }
    deleteDecodedString(); // delete received lora response after processing
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
    unsigned char index = 6;   //default index for "#65535SLAVE01$"
    myMsDelay(100);
    switch (Action) {
    case 0x00:    // check for Valve ON ack response
        field = fetchFieldNo(10);
        if(strncmp(decodedString+1, on, 2) == 0 && strncmp(decodedString+12, ack, 3) == 0 && field == FieldNo) {  //#ON01SLAVE01ACK$
            return true;
        }
        break;
    case 0x01:    // check for Valve OFF ack response
        field = fetchFieldNo(11);
        if(strncmp(decodedString+1, off, 3) == 0 && strncmp(decodedString+13, ack, 3) == 0 && field == FieldNo) {  //#OFF01SLAVE01ACK$
            return true;
        }
        break;
    case 0x02:   // check for SENSOR measurement reading response
        moistureLevel = CLEAR;
        for ( msgIndex = 1; msgIndex < 6 ; msgIndex++) {
            //is number
            if (isNumber(decodedString[msgIndex])) {
                if (decodedString[msgIndex + 1] == 'S') { // to fetch position of S  and then read value in "#65535SLAVE01$" since freq. value can vary fro 1 to 5 digits
                    decodedString[msgIndex] = decodedString[msgIndex] - 48;
                    moistureLevel = moistureLevel + decodedString[msgIndex];
                    index = msgIndex + 1;
                    msgIndex = 99; // break
                } 
                else {
                    decodedString[msgIndex] = decodedString[msgIndex] - 48;
                    decodedString[msgIndex] = decodedString[msgIndex] * 10;
                    moistureLevel = moistureLevel * 10;
                    moistureLevel = moistureLevel + decodedString[msgIndex];
                }
            }
            else {
                break;
            }
        }
        field = fetchFieldNo(index+5);
        if(strncmp(decodedString+index, slave, 5) == 0 && field == FieldNo) {  //"#65535SLAVE01$"
            return true;
        }
        else if(strncmp(decodedString+1, sensor, 6) == 0 && strncmp(decodedString+7, error, 5) == 0 && strncmp(decodedString+12, slave, 5) == 0) {  //"#SENSORERRORSLAVE01$"
            moistureSensorFailed = true;
            return true;
        } 
    }
    if(strncmp(decodedString+1, master, 6) == 0 && strncmp(decodedString+7, error, 5) == 0) { //#MASTERERROR$
        return false;
    }
    else if(strncmp(decodedString+1, slave, 5) == 0 && strncmp(decodedString+6, error, 5) == 0) {  //#SLAVEERROR$
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
