/*
 * File name            : RTC_DS1307.c
 * Compiler             : MPLAB XC8/ MPLAB C18 compiler
 * IDE                  : Microchip  MPLAB X IDE v5.25
 * Processor            : PIC18F66K40
 * Author               : Bhoomi Jalasandharan
 * Created on           : July 15, 2020, 05:23 PM
 * Description          : Main source file
 */


#include "congfigBits.h"
#include "variableDefinitions.h"
#include "controllerActions.h"
#include "RTC_DS1307.h"
#ifdef DEBUG_MODE_ON_H
#include "serialMonitor.h"
#endif
/****************RTC FUNCTIONS*****************/

/*Feed Current timestamp from gsm in to RTC*/
void feedTimeInRTC(void) {
    unsigned char day = 0x01; // Storing dummy day 'Monday'
    /* Convert time stamp to BCD format*/
    setBCDdigit(0x0E,1); // (t) BCD indication for RTC Clock feed Action
    myMsDelay(500);
    #ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("feedTimeInRTC_IN\r\n");
    //********Debug log#end**************//
    #endif
    currentSeconds = decimal2BCD(currentSeconds); 
    currentMinutes = decimal2BCD(currentMinutes);
    currentHour = decimal2BCD(currentHour);
    currentDD = decimal2BCD(currentDD);
    currentMM = decimal2BCD(currentMM);
    currentYY = decimal2BCD(currentYY);
    i2cStart();          /*start condition */
    
    i2cSend(0xD0);     /* slave address with write mode */
    i2cSend(0x00);     /* address of seconds register written to the pointer */ 
    
    i2cSend(currentSeconds);  /*time register values */
    i2cSend(currentMinutes);
    i2cSend(currentHour);
    
    i2cSend(day);      /*date registers */
    i2cSend(currentDD);
    i2cSend(currentMM);
    i2cSend(currentYY);
    
    i2cStop();          /*i2c stop condition */
    setBCDdigit(0x0F,0); // Blank BCD Indication for Normal Condition
    #ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("feedTimeInRTC_OUT\r\n");
    //********Debug log#end**************//
    #endif
}

/* convert the decimal values to BCD using below function */
unsigned char decimal2BCD (unsigned char decimal)
{
    unsigned char temp;
    temp = (unsigned char)((decimal/10) << 4);
    temp = temp | (decimal % 10);
    return temp;
}

/* Convert BCD to decimal */
unsigned char bcd2Decimal (unsigned char bcd)
{
    unsigned char temp;
    temp = (bcd & 0x0F) + ((bcd & 0xF0)>>4)*10;
    return temp;
}

/* Fetch Current timestamp from RTC */
void fetchTimefromRTC(void) {
    unsigned char day = 0x01; // Storing dummy day 'Monday'
    setBCDdigit(0x0E,0);  // (t.) BCD indication for RTC Clock fetch action
    myMsDelay(500);
    #ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("fetchTimefromRTC_IN\r\n");
    //********Debug log#end**************//
    #endif
    i2cStart();
	i2cSend(0xD0);
	i2cSend(0x00);
	i2cRestart();
	i2cSend(0xD1);
	currentSeconds = i2cRead(1); /* Read the slave with ACK */
	currentMinutes = i2cRead(1);
	currentHour = i2cRead(1);
    day = i2cRead(1);
	currentDD = i2cRead(1);
	currentMM = i2cRead(1);
	currentYY = i2cRead(0);
    i2cStop();
    
    /* Convert Timestamp to decimal*/
    currentSeconds = bcd2Decimal(currentSeconds); 
    currentMinutes = bcd2Decimal(currentMinutes);
    currentHour = bcd2Decimal(currentHour);
    currentDD = bcd2Decimal(currentDD);
    currentMM = bcd2Decimal(currentMM);
    currentYY = bcd2Decimal(currentYY);
    setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
    #ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("fetchTimefromRTC_OUT\r\n");
    //********Debug log#end**************//
    #endif
}

/**********************************************/

/****************I2C-Library*******************/

void i2cStart(void) {
	SSP2CON2bits.SEN = 1;
	//SSPCON2 bit 0
	while (SSP2CON2bits.SEN == SET);
	//SEN =1 initiate the Start Condition on SDA and SCL Pins
	//Automatically Cleared by Hardware
	// 0 for Idle State
}

void i2cRestart(void) {
	SSP2CON2bits.RSEN = 1;
	//SSPCON2 bit 1
	while (SSP2CON2bits.RSEN == SET);
	//RSEN = 1 initiate the Restart Condition
	//Automatically Cleared by Hardware
}

void i2cStop(void) {
	SSP2CON2bits.PEN = 1;
	while (SSP2CON2bits.PEN == SET);
}

void i2cWait(void) {
    while ((SSP2CON2 & 0x1F) | (SSP2STATbits.R_NOT_W));
    // Wait condition until I2C bus is Idle.
}

void i2cSend(unsigned char dat) {
	SSP2BUF = dat;    /* Move data to SSPBUF */
    while (SSP2STATbits.BF);       /* wait till complete data is sent from buffer */
    i2cWait();       /* wait for any pending transfer */
}

unsigned char i2cRead(_Bool ACK) {
	unsigned char temp;
    SSP2CON2bits.RCEN = 1;
    /* Enable data reception */
    while (SSP2STATbits.BF == CLEAR);      /* wait for buffer full */
    temp = SSP2BUF;   /* Read serial buffer and store in temp register */
    i2cWait();       /* wait to check any pending transfer */
    if (ACK)
        SSP2CON2bits.ACKDT=0;               //send acknowledge
    else
        SSP2CON2bits.ACKDT=1;				//Do not  acknowledge
	SSP2CON2bits.ACKEN=1;
	while (SSP2CON2bits.ACKEN == SET)
        ;
    return temp;     /* Return the read data from bus */
}
/**********************************************/