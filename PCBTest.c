/*
 * File name            : Main.c
 * Compiler             : MPLAB XC8/ MPLAB C18 compiler
 * IDE                  : Microchip  MPLAB X IDE v5.25
 * Processor            : PIC18F66K40
 * Author               : Bhoomi Jalasandharan
 * Created on           : July 15, 2020, 05:23 PM
 * Description          : Main source file
 */

/***************************** Header file declarations#start ************************/

#include "congfigBits.h"
#include "variableDefinitions.h"
#include "ADC.h"
#include "eeprom.h"
#include "gsm.h"
#include "controllerActions.h"
#include "RTC_DS1307.h"
/***************************** Header file declarations#end **************************/

//**************interrupt service routine handler#start***********//

/*************************************************************************************************************************

This function is called when an interrupt has occurred at RX pin of ?c which is connected to TX pin of GSM.
Interrupt occurs at 1st cycle of each Data byte.
The purpose of this interrupt handler is to store the data received from GSM into Array called gsmResponse[]
Each response from GSM starts with '+' symbol, e.g. +CMTI: "SM", <index>
The End of SMS is detected by OK command.

 **************************************************************************************************************************/

void __interrupt(high_priority)rxInterrupt_handler(void) {
    //Interrupt-on-change pins
    
    if (PIR0bits.IOCIF) {
        // falling Edge -- All phase present
        if((IOCEF5 == 1 || IOCEF6 == 1 || IOCEF7 == 1) && (phaseB == 1)) { //phaseB == 1 && phaseY == 1 &&  
            //phase is on
            IOCEF &= (IOCEF ^ 0xFF); //Clearing Interrupt Flags
            allPhaseDetected = true; 
            //Run_led = GLOW;
        }
        // Rising Edge -- Any one Phase lost
        else if((IOCEF5 == 1 || IOCEF6 == 1 || IOCEF7 == 1) && (phaseB == 0)) { //phaseB == 0 || phaseY == 0 ||  
            // phase is out
            IOCEF &= (IOCEF ^ 0xFF); //Clearing Interrupt Flags
            allPhaseDetected = false;
            //Run_led = DARK;
        }
        PIR0bits.IOCIF = CLEAR; // Reset the ISR flag.
    }
   
    // Interrupt on RX bit
    if (PIR4bits.RC3IF) {
        rxCharacter = rxByte(); // Read byte received at Reception Register
        // Check if any overrun occur due to continuous reception
        
        if (RC3STAbits.OERR) {
            RC3STAbits.CREN = 0;
            Nop();
            Nop();
            RC3STAbits.CREN = 1;
        }
        // If interrupt occurred in sleep mode due to command from GSM
        if (inSleepMode == true) {
            sleepCount = 2; // Set minimum sleep to 2 for recursive reading from GSM
            sleepCountChangedDueToInterrupt = true; // Sleep count is altered and hence needs to be calculated again
            // check if GSM initiated communication with '+'
            if (rxCharacter == '+') {
                msgIndex = CLEAR; // Reset message storage index to first character to start reading from '+'
                gsmResponse[msgIndex] = rxCharacter; // Load Received byte into storage buffer
                msgIndex++; // point to next location for storing next received byte
            }
            // Check if Sms type cmd is initiated and received byte is cmti command
            else if (msgIndex < 12 && cmti[msgIndex] == rxCharacter) {
                gsmResponse[msgIndex] = rxCharacter; // Load received byte into storage buffer
                msgIndex++; // point to next location for storing next received byte
                // check if storage index is reached to last character of CMTI command
                if (msgIndex == 12) {
                    newSMSRcvd = true; // Set to indicate New SMS is Received	
                }
            } 
            //To extract sim location for SMS storage
            else if (newSMSRcvd == true && msgIndex == 12) {
                temporaryBytesArray[0] = rxCharacter; // To store sim memory location of received message
                msgIndex = CLEAR;
                sleepCount = 1;
                sleepCountChangedDueToInterrupt = true; // Sleep count is altered and hence needs to be calculated again
            }
        } 
        // check if GSM response to µc command is not completed
        
        else if (gsmRespondedToControllerCommand == false) {
            // Start storing response if received data is '+' at index zero
            if (rxCharacter == '+' && msgIndex == 0) {
                gsmResponse[msgIndex] = rxCharacter; // Load received byte into storage buffer
                msgIndex++; // point to next location for storing next received byte
            }
            // Cascade received data to stored response after receiving first character '+'
            else if (msgIndex > 0) {
                gsmResponse[msgIndex] = rxCharacter; // Load received byte into storage buffer
                // Cascade till 'OK'  is found
                
                if (gsmResponse[msgIndex - 1] == 'O' && gsmResponse[msgIndex] == 'K') {
                    gsmRespondedToControllerCommand = true; // GSM response to µc command is completed
                    msgIndex = CLEAR; // Reset message storage index to first character to start reading for next received byte of cmd
                } 
                // Read bytes till 500 characters
                else if (msgIndex < 500) {
                    msgIndex++;
                }
            }
        }
        PIR4bits.RC3IF= CLEAR; // Reset the ISR flag.
    } // end interrupt        
}


/*************************************************************************************************************************

This function is called when an interrupt is occurred after 16 bit timer is overflow
The purpose of this interrupt handler is to count no. of overflows that the timer did.

 **************************************************************************************************************************/

void __interrupt(low_priority) timerInterrupt_handler(void) {
    /*Wait till gsm responds or else break after timer limit*/
    if (PIR0bits.TMR0IF == 1) {
        //Run_led = ~Run_led;
        PIR0bits.TMR0IF = CLEAR;
        Timer0Overflow++;
        // GSM failed to respond within 5 min
        
        if (Timer0Overflow > 255 && gsmRespondedToControllerCommand == false) {
            gsmRespondedToControllerCommand = true; // Unlock key
            Timer0Overflow = 0;
            T0CON0bits.T0EN = 0; // Stop timer
            //Run_led = GLOW;
        } 
        else if (gsmRespondedToControllerCommand == true) {
            Timer0Overflow = 0;
            T0CON0bits.T0EN= 0; // Stop timer
            //Run_led = GLOW;
        }
    }
/*To measure pulse width of moisture sensor output*/
    if (PIR5bits.TMR1IF == 1) {
        Timer1Overflow++;
        PIR5bits.TMR1IF = CLEAR;
    }
/*To follow filtration  cycle sequence*/
    if (PIR5bits.TMR3IF == 1) {
        Timer3Overflow++;
        PIR5bits.TMR3IF = CLEAR;
        
        if (filtrationCycleSequence == 1 && Timer3Overflow > 10 ) {
            Timer3Overflow = 0;
            relay16 = ON;
            filtrationCycleSequence = 2;
            //T3CONbits.TMR3ON = OFF;
        }
        else if (filtrationCycleSequence == 2 && Timer3Overflow > 1 ) {
            Timer3Overflow = 0;
            relay16 = OFF;
            filtrationCycleSequence = 3;
            //T3CONbits.TMR3ON = OFF;
        }
        else if (filtrationCycleSequence == 3 && Timer3Overflow > 1 ) {
            Timer3Overflow = 0;
            relay17 = ON;
            filtrationCycleSequence = 4;
            //T3CONbits.TMR3ON = OFF;
        }
        else if (filtrationCycleSequence == 4 && Timer3Overflow > 1 ) {
            Timer3Overflow = 0;
            relay17 = OFF;
            filtrationCycleSequence = 5;
            //T3CONbits.TMR3ON = OFF;
        }
        else if (filtrationCycleSequence == 5 && Timer3Overflow > 1 ) {
            Timer3Overflow = 0;
            relay18 = ON;
            filtrationCycleSequence = 6;
            //T3CONbits.TMR3ON = OFF;
        }
        else if (filtrationCycleSequence == 6 && Timer3Overflow > 1 ) {
            Timer3Overflow = 0;
            relay18 = OFF;
            filtrationCycleSequence = 7;
            //T3CONbits.TMR3ON = OFF;
        }
        else if (filtrationCycleSequence == 7 && Timer3Overflow > 30 ) {
            Timer3Overflow = 0;
            filtrationCycleSequence = 1;
            //T3CONbits.TMR3ON = OFF;
        }
    }
}
//**************interrupt service routine handler#end***********//



//****************************MAIN FUNCTION#Start***************************************//
 void main(void) {
    unsigned int a= 0;
    unsigned char field_No = CLEAR;
    myMsDelay(1000); // Warmup 60 seconds
    configureController(); // set Microcontroller ports, ADC, Timer, I2C, UART, Interrupt Config
    myMsDelay(500);
    SIM_led = DARK;
    Run_led = DARK;
    myMsDelay(2000);//

    /*
    T0CON0bits.T0EN = 1; // Start timer thread to unlock system if GSM fails to respond within 5 min
    while (gsmRespondedToControllerCommand == false) {
        bcd = 0x01;
    }
    PIR5bits.TMR1IF = 1;
    bcd = 0x02;
    while(1)
    {   
        
        relay1 = ON;
        myMsDelay(2000);
        relay2 = ON;
        myMsDelay(2000);
        relay3 = ON;
        myMsDelay(2000);
        relay4 = ON;
        myMsDelay(2000);
        relay5 = ON;
        myMsDelay(2000);
        relay6 = ON;
        myMsDelay(2000);
        relay7 = ON;
        myMsDelay(2000);
        relay8 = ON;
        myMsDelay(2000);
        relay9 = ON;
        myMsDelay(2000);
        relay10 = ON;
        myMsDelay(2000);
        relay11 = ON;
        myMsDelay(2000);
        relay12 = ON;
        myMsDelay(2000);
        relay13 = ON;
        myMsDelay(2000);
        relay14 = ON;
        myMsDelay(2000);
        relay15 = ON;
        myMsDelay(2000);
        relay16 = ON;
        myMsDelay(2000);
        relay17 = ON;
        myMsDelay(2000);
        relay18 = ON;
        myMsDelay(2000);
        
    
        
        isMotorInNoLoad();
        bcd = 0x01;
        relay7 = ON;
        selectChannel(0);
        a = getADCResult();
        bcd = 0x02;
        selectChannel(1);
        a = getADCResult();
        bcd = 0x03;
        selectChannel(2);
        a = getADCResult();
        bcd = 0x04;
        selectChannel(3);
        a = getADCResult();
        bcd = 0x05;
        selectChannel(4);
        a = getADCResult();
    }
        Run_led = GLOW;
        WDTCON0bits.SWDTEN = ENABLED;
        Nop();
        bcd = 0x02;
        Sleep(); // CPU sleep. Wakeup when Watchdog overflows, each of 16 Seconds if value of WDTPS is 4096
        bcd = 0x01;
        Nop();
        WDTCON0bits.SWDTEN = DISABLED;
        Run_led = DARK;
        WDTCON0bits.SWDTEN = ENABLED;
        Nop();
        
        Sleep(); // CPU sleep. Wakeup when Watchdog overflows, each of 16 Seconds if value of WDTPS is 4096
        Nop();
        WDTCON0bits.SWDTEN = DISABLED;
    
    
    while(1)
    {  
        bcd = 0x01;
        transmitStringToGSM("AT\r\n"); // Echo ON command
        myMsDelay(1000);
     if (PIR4bits.RC3IF) {
        rxCharacter = rxByte();
     }
        bcd = 0x02;
        myMsDelay(1000);
    }
 */
    loadDataFromEeprom(); // Read configured valve data saved in EEprom
    myMsDelay(1000);
    checkGsmConnection(); // Check response of GSM to AT command from controller
    bcd = 0x02;
    myMsDelay(1000);
    //setGsmToLocalTime(); // Set GSM at local time standard across the globe
    configureGSM(); // Configure GSM in TEXT mode
    bcd = 0x03;
    myMsDelay(1000);
    deleteMsgFromSIMStorage(); // Clear GSM storage memory for new Messages
    bcd = 0x04;
    getDateFromGSM(); // Get today's date from Network
    bcd = 0x05;
    feedTimeInRTC1307(); // Feed fetched date from network into RTC
    bcd = 0x06;
    myMsDelay(10000);
    fetchTimefromRTC1307();
    // do board power-up here
    // check if if system power reset occurred
    if (PCON0bits.NOT_POR == CLEAR) {   
        // check if system is authenticated and valve action is due
        if (systemAuthenticated == true) {   
            // check if System is configured
            for (field_No = 0; field_No < fieldCount; field_No++) {
                // check if any field valve status was true after reset
                if (fieldValve[field_No].status == ON) {
                    myMsDelay(100);
                    fetchTimefromRTC1307(); // Get today's date
                    myMsDelay(100);

                    /*** System Restarted on another date ***/
                    // if year over passes ||  if month  over passes ||  if day over passes 
                    if ((currentYY > fieldValve[field_No].nextDueYY)||(currentMM > fieldValve[field_No].nextDueMM && currentYY == fieldValve[field_No].nextDueYY)||(currentDD >= fieldValve[field_No].nextDueDD && currentMM == fieldValve[field_No].nextDueMM && currentYY == fieldValve[field_No].nextDueYY)) {
                        valveDue = false; // Clear Value Due
                        fieldValve[field_No].status = OFF;
                        startFieldNo = field_No;  // start action form interrupted field irrigation valve
                        if(fieldValve[field_No].fertigationStatus == ON) {
                            fieldValve[field_No].fertigationStatus = OFF;
                            fieldValve[field_No].fertigationValveInterrupted = true;
                            remainingFertigationOnPeriod = readSleepCountFromEeprom();
                            saveRemainingFertigationOnPeriod();
                        }
                        myMsDelay(100);
                        //saveFieldIrrigationValveOnOffStatusIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
                        myMsDelay(100);
                        break;
                    }
                    else { // if system restarted on same day with due valve
                        valveDue = true; // Set valve ON status
                        startFieldNo = field_No;  // start action form interrupted field irrigation valve
                        break;
                    }   
                }
            }
            if (valveDue == true) {
                PCON0bits.NOT_POR = SET; // Reset power status
                myMsDelay(50);
                dueValveChecked = true;

                /***************************/
                // for field no. 00 to 09
                if(field_No<9){
                    temporaryBytesArray[0] = 48; // To store field no. of valve in action 
                    temporaryBytesArray[1] = field_No + 49; // To store field no. of valve in action 
                }// for field no. 10 to 12
                else if(field_No > 8 && field_No < 12) {
                    temporaryBytesArray[0] = 49; // To store field no. of valve in action 
                    temporaryBytesArray[1] = field_No + 39; // To store field no. of valve in action 
                }
                fieldNoRequired = true; // Acknowledge field valve no. to use
                /***************************/
                /***************************/
                sendSms("System Restarted with Due Valve for Field No. ", userMobileNo); // Acknowledge user about system restarted with Valve action
                sleepCount = 2; // Load sleep count for SMS transmission action
                sleepCountChangedDueToInterrupt = true; // Sleep count needs to be calculated 
                deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
                /***************************/

                iterator = field_No; // buffer storage
            }
            else { // check if no valve action is due
                PCON0bits.NOT_POR = SET; // Reset power status
                myMsDelay(50);

                /***************************/
                sendSms("System Restarted in SD", userMobileNo); // Acknowledge user about system restart
                sleepCount = 2; // Load sleep count for SMS transmission action
                sleepCountChangedDueToInterrupt = true; // Sleep count needs to be calculated 
                deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider 
                /***************************/
            }
        }
        // check if System not yet configured
        else {
            while (systemAuthenticated != true) {
                strncpy(pwd, factryPswrd, 6); // consider factory password
                sleepCount = 65000; // Set Default Sleep count until next sleep count is calculated
                deepSleep(); // Sleep with default sleep count until system is configured
                // check if Sleep count executed with interrupt occurred due to new SMS command reception
                if (newSMSRcvd == true) {
                    myMsDelay(500);
                    newSMSRcvd = false; // received cmd is processed										
                    extractReceivedSms(); // Read received SMS
                }
            }
        }
        // Check need of below field no.
        /*
        field_No = 200;
        saveFieldIrrigationValveNoIntoEeprom(field_No); 
        */  
    }
    while (1) {
        // check if no valve is in Action  //??why two time
        if (valveDue == false) {
            myMsDelay(50);
            scanDueValveWithSleepCount(); // get sleep count for valve action
            myMsDelay(50);
            dueValveChecked = true;
        }
        if (valveDue == true && dueValveChecked == true) {
            field_No = iterator;  // Copy field no navigated through iterator.
            dueValveChecked = false;
            // Check if Field is wet
            if (isFieldMoistureSensorWet(field_No)) {
                valveDue = false;
                myMsDelay(50);
                getDueDate(fieldValve[field_No].offPeriod); // calculate next due date of valve
                myMsDelay(50); // Today's date is not known for next due date
                fieldValve[field_No].nextDueDD = dueDD;
                fieldValve[field_No].nextDueMM = dueMM;
                fieldValve[field_No].nextDueYY = dueYY;
                myMsDelay(100);
                saveFieldIrrigationValveOnOffStatusIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
                myMsDelay(100);
                saveFieldIrrigationValveDueTimeIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
                myMsDelay(100);

                /***************************/
                // for field no. 00 to 09
                if(field_No<9){
                    temporaryBytesArray[0] = 48; // To store field no. of valve in action 
                    temporaryBytesArray[1] = field_No + 49; // To store field no. of valve in action 
                }// for field no. 10 to 12
                else if(field_No > 8 && field_No < 12) {
                    temporaryBytesArray[0] = 49; // To store field no. of valve in action 
                    temporaryBytesArray[1] = field_No + 39; // To store field no. of valve in action 
                }
                fieldNoRequired = true; // Acknowledge field valve no. to use
                /***************************/
                /***************************/
                sendSms("Wet Field Detected.\r\nValve not activated for Field No. ", userMobileNo); // Notify user about  wet field and deactivation of valve						
                sleepCount = 2; // Load sleep count for SMS transmission action
                sleepCountChangedDueToInterrupt = true; // sleep count needs to be calculated for next action
                deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
                /***************************/

                sleepCount = 0; // Skip next sleep
                myMsDelay(100);
                saveSleepCountIntoEeprom(); // Save current valve on time 
                myMsDelay(100);
                sleepCountChangedDueToInterrupt = false; // Sleep until message transmission acknowledge SMS is received from service provider				
            } // All phase present
            else if (!phaseFailure()){
                myMsDelay(100);
                activateValve(field_No); // Activate valve for field
                myMsDelay(100);
                
                /**/
                //Switch ON Fertigation valve interrupted due to power on same day
                if (fieldValve[field_No].fertigationStatus == ON) {
                    myMsDelay(1000);
                    relay13 = ON;
                }
                

                /******** Calculate and save Field Valve next Due date**********/
                getDueDate(fieldValve[field_No].offPeriod); // calculate next due date of valve
                fieldValve[field_No].nextDueDD = dueDD;
                fieldValve[field_No].nextDueMM = dueMM;
                fieldValve[field_No].nextDueYY = dueYY;
                myMsDelay(100);
                saveFieldIrrigationValveDueTimeIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
                myMsDelay(100);
                /***********************************************/
                // DeActivate previous executed field valve
                if (valveExecuted == true) {
                    field_No = readFieldIrrigationValveNoFromEeprom();
                    deActivateValve(field_No); // Successful Deactivate valve
                    valveExecuted = false;
                    field_No = 200;
                    saveFieldIrrigationValveNoIntoEeprom(field_No);
                } 
                // Switch on Motor for First Valve activation
                else {                  
                    powerOnMotor(); // Power On Motor
                }
            }
            // phase failure detected
            else if (phaseFailure()) {                
                /***************************/
                // for field no. 00 to 09
                if(field_No<9){
                    temporaryBytesArray[0] = 48; // To store field no. of valve in action 
                    temporaryBytesArray[1] = field_No + 49; // To store field no. of valve in action 
                }// for field no. 10 to 12
                else if(field_No > 8 && field_No < 12) {
                    temporaryBytesArray[0] = 49; // To store field no. of valve in action 
                    temporaryBytesArray[1] = field_No + 39; // To store field no. of valve in action 
                }
                fieldNoRequired = true; // Acknowledge field valve no. to use
                /***************************/
                /***************************/
                sendSms("Phase failure detected, Valve not started for field ", userMobileNo); // Acknowledge user about system restart
                sleepCount = 2; // Load sleep count for SMS transmission action
                sleepCountChangedDueToInterrupt = true; // Sleep count needs to be calculated 
                deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider 
                /***************************/
                
                // DeActivate Previous valve and switch off motor pump
                /***************************/
                if (valveExecuted == true) { 
                powerOffMotor();
                field_No = readFieldIrrigationValveNoFromEeprom();
                deActivateValve(field_No);      // Successful Deactivate valve
                valveExecuted = false;
                field_No = 200;
                saveFieldIrrigationValveNoIntoEeprom(field_No);
                /***************************/
                sendSms("Motor Switched OFF.", userMobileNo); // Acknowledge user about successful action
                sleepCount = 2; // Load sleep count for SMS transmission action
                sleepCountChangedDueToInterrupt = true; // Sleep count needs to be calculated
                deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
                /***************************/
                }
                sleepCount = 6500;
                sleepCountChangedDueToInterrupt = false;
            }
        }
        // DeActivate last valve and switch off motor pump
        else if (valveExecuted == true) { 
            powerOffMotor();
            field_No = readFieldIrrigationValveNoFromEeprom();
            deActivateValve(field_No);      // Successful Deactivate valve
            valveExecuted = false;
            field_No = 200;
            saveFieldIrrigationValveNoIntoEeprom(field_No);

            /***************************/
            sendSms("All valves executed.\r\nMotor Switched OFF.", userMobileNo); // Acknowledge user about successful action
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to be calculated
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            /***************************/

            sleepCount = 0;
            sleepCountChangedDueToInterrupt = false;
        }// ?? need to check low water interrupt
        /***************Action on Dry detection condition****************/
        // Measure CT current after 10 seconds of Motor On Period
        if (valveDue == true && isMotorInNoLoad() && relay14 == ON) {  
            powerOffMotor();
            myMsDelay(1000);
            deActivateValve(field_No);   // Deactivate Valve upon low water interrupt and reset valve to next due time
            valveDue = false;
            startFieldNo = field_No;
           
            /************Fertigation switch off due to dry run***********/
            // Dry run detected during enabled fertigation valve ON period
            if(fieldValve[field_No].isFertigationEnabled == true && fieldValve[field_No].fertigationStatus == ON) {
                myMsDelay(1000);
                relay13 = OFF; // switch off fertigation valve for given field after on period
                fieldValve[field_No].fertigationStatus = OFF; // reset valve status for next cycle
                fieldValve[field_No].fertigationValveInterrupted = true; // set fertigation valve halted due to dry run
                remainingFertigationOnPeriod = readSleepCountFromEeprom();
                saveRemainingFertigationOnPeriod();
                myMsDelay(100);
                saveFieldIrrigationValveOnOffStatusIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
                myMsDelay(100);
            } // Dry run detected during post fertigation valve ON period
            else if(fieldValve[field_No].fertigationExecuted == true){
                fieldValve[field_No].fertigationExecuted = false;
                myMsDelay(100);
                saveFieldIrrigationValveOnOffStatusIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
                myMsDelay(100);
                
            }
            /***************************/
            sendSms("Dry Run Detected, Motor Switched OFF", userMobileNo); // Acknowledge user about dry run detected again
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to be calculated
            deepSleep();
            /***************************/

            sleepCount = 350; // Sleep for every 12 hours to check water level again
            sleepCountChangedDueToInterrupt = false; // Sleep until message transmission acknowledge SMS is received from service provider
            /*
            myMsDelay(100);
            saveSleepCountIntoEeprom(); // Save current valve on time 
            myMsDelay(100);
            */
        }
        // system on hold
        if (onHold == true) {
            sleepCount = 0; // Skip Next sleep
            sleepCountChangedDueToInterrupt = false;
        }
        if (sleepCountChangedDueToInterrupt == true) {
            /*************Debug****************/
            //transmitStringToGSM("Read Sleep count\r\n");
            /*************Debug****************/
            myMsDelay(50);
            sleepCount = readSleepCountFromEeprom(); // read ON action time of valve saved in EEprom
            myMsDelay(50);
            sleepCountChangedDueToInterrupt = false; // sleep count is calculated
        }
        /*************Debug****************/
        //temporaryBytesArray[0] = sleepCount +48;
        //transmitNumberToGSM(temporaryBytesArray, 1);
        //transmitStringToGSM(" In Sleep\r\n");
        /*************Debug****************/
        /****************************/
        deepSleep(); // sleep for given sleep count (	default/calculated )
        /****************************/
        // check if Sleep count executed with interrupt occurred due to new SMS command reception
        if (newSMSRcvd == true) {
            myMsDelay(500);
            newSMSRcvd = false; // received command is processed										
            extractReceivedSms(); // Read received SMS
        } 
        //check if Sleep count executed without external interrupt
        else if (sleepCount == 0 && sleepCountChangedDueToInterrupt == false && motorInDryRun == false) {
            for (field_No = 0; field_No < fieldCount; field_No++) {
                // upon completing first delay start period sleep , switch on fertigation valve
                if(fieldValve[field_No].status == ON && fieldValve[field_No].isFertigationEnabled == true && fieldValve[field_No].fertigationStatus == OFF && fieldValve[field_No].fertigationExecuted == false && fieldValve[field_No].fertigationInstance != 0 ) {
                    myMsDelay(1000);
                    relay13 = ON; // switch on fertigation valve for given field after start period
                    fieldValve[field_No].fertigationStatus = ON;
                    myMsDelay(100);
                    saveFieldIrrigationValveOnOffStatusIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
                    myMsDelay(100);
                    if(fieldValve[field_No].fertigationValveInterrupted == true) {
                        //remainingFertigationOnPeriod = readremainingFertigationOnPeriod();
                        sleepCount = remainingFertigationOnPeriod;
                    }
                    else {
                        sleepCount = fieldValve[field_No].fertigationONperiod; // calculate sleep count for on period of Valve 
                        sleepCount = 9 * sleepCount;
                        sleepCount = sleepCount / 19;
                    }
                    sleepCountChangedDueToInterrupt = false; // sleep count is calculated for next action
                    myMsDelay(100);
                    saveSleepCountIntoEeprom(); // Save current valve on time 
                    myMsDelay(100);
                    /*Send sms*/
                    break;
                }
                // Upon completing fertigation on period sleep, switch off fertigation valve
                else if (fieldValve[field_No].status == ON && fieldValve[field_No].isFertigationEnabled == true && fieldValve[field_No].fertigationStatus == ON) {
                    myMsDelay(1000);
                    relay13 = OFF; // switch off fertigation valve for given field after on period
                    fieldValve[field_No].fertigationStatus = OFF;
                    fieldValve[field_No].fertigationInstance--;
                    fieldValve[field_No].fertigationExecuted = true;
                    myMsDelay(100);
                    saveFieldIrrigationValveOnOffStatusIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
                    myMsDelay(100);
                    sleepCount = fieldValve[field_No].onPeriod - (fieldValve[field_No].fertigationDelay + fieldValve[field_No].fertigationONperiod); // calculate sleep count for on period of Valve 
                    sleepCount = 9 * sleepCount;
                    sleepCount = sleepCount / 19;
                    sleepCountChangedDueToInterrupt = false; // sleep count is calculated for next action
                    myMsDelay(100);
                    saveSleepCountIntoEeprom(); // Save current valve on time 
                    myMsDelay(100);
                    /*Send sms*/    
                }
                // upon completing entire filed valve on period switch off field valve
                else if (fieldValve[field_No].status == ON) {
                    fieldValve[field_No].status = OFF;
                    fieldValve[field_No].fertigationExecuted = false;
                    valveDue = false;
                    valveExecuted = true;
                    myMsDelay(100);
                    saveFieldIrrigationValveNoIntoEeprom(field_No);
                    myMsDelay(100);
                    saveFieldIrrigationValveOnOffStatusIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
                    myMsDelay(100);
                    onHold = false;
                    break;
                }
            }
        }
    }
}
//****************************MAIN FUNCTION#End***************************************//
/*features remaining*/ //RTC, phase failure, CT, fertigation, filtration
 // RTC battery check
 // Restart after phase detection
 // Phase failure during sleep
 // filtration done but need to check flow -- Done
 // fertigation status eeprom read write
 // EEprom address need to be mapped -- Done
 // fertigation interrupt due to power and dry run
 // eeprom read write for fertigation values
 // save status in eeprom