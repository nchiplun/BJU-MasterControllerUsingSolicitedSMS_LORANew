/*
 * File name            : ADC.h
 * Compiler             : MPLAB XC8/ MPLAB C18 compiler
 * IDE                  : Microchip  MPLAB X IDE v5.25
 * Processor            : PIC18F66K40
 * Author               : Bhoomi Jalasandharan
 * Created on           : July 15, 2020, 05:23 PM
 * Description          : ADC functions header file
 */


#include "congfigBits.h"
#include "variableDefinitions.h"
#include "ADC.h"
#include "controllerActions.h"
#ifdef DEBUG_MODE_ON_H
#include "serialMonitor.h"
#endif

void selectChannel(unsigned char channel) {
	switch(channel) {
    case 0 :
        ADPCH = CT;
        #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("CT Channel selected\r\n");
        //********Debug log#end**************//
        #endif
        break;
    case 1 : 
        ADPCH = WindSpeed;
        #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("Windspeed Channel selected\r\n");
        //********Debug log#end**************//
        #endif
        break;
    case 2 : 
        ADPCH = Temperature;
        #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("Temperature Channel selected\r\n");
        //********Debug log#end**************//
        #endif
        break;
    case 3 : 
        ADPCH = RTCBattery;
        #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("Analog Channel selected\r\n");
        //********Debug log#end**************//
        #endif
        break;
	}
}


//If you do not wish to use ADC conversion interrupt you can use this
//to do conversion manually. It assumes conversion format is right adjusted
unsigned int getADCResult(void) {
	unsigned int adcResult=0;
    #ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("GetADCResult_IN\r\n");
    //********Debug log#end**************//
    #endif
	ADCON0bits.GO = 1; //Start conversion
    setBCDdigit(0x05,1); // (5) BCD Indication for ADC Action
	while (ADCON0bits.GO)
        ; //Wait for conversion done
	//myMsDelay(500);
    setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
    adcResult = ADRESL;
	adcResult|=((unsigned int)ADRESH) << 8;
    #ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("GetADCResult_OUT\r\n");
    //********Debug log#end**************//
    #endif
	return adcResult;
} 
