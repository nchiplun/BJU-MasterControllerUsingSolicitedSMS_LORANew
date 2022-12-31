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
#include "controllerActions.h"
#include "eeprom.h"
#include "gsm.h"
#include "lora.h"
#ifdef DEBUG_MODE_ON_H
#include "serialMonitor.h"
#endif
/***************************** Header file declarations#end **************************/

//**************interrupt service routine handler#start***********//

/*************************************************************************************************************************

This function is called when an interrupt has occurred at RX pin of ?c which is connected to TX pin of GSM.
Interrupt occurs at 1st cycle of each Data byte.
The purpose of this interrupt handler is to store the data received from GSM into Array called gsmResponse[]
Each response from GSM starts with '+' symbol, e.g. +CMTI: "SM", <index>
The End of SMS is detected by OK command.

 **************************************************************************************************************************/

void __interrupt(high_priority)rxANDiocInterrupt_handler(void) {
    // Interrupt on RX bit
    if (PIR4bits.RC3IF) {
        Run_led = GLOW; // Led Indication for system in Operational Mode
        rxCharacter = rxByte(); // Read byte received at Reception Register
        // Check if any overrun occur due to continuous reception
        if (RC3STAbits.OERR) {
            RC3STAbits.CREN = 0;
            Nop();
            RC3STAbits.CREN = 1;
        }
        // If interrupt occurred in sleep mode due to command from GSM
        if (inSleepMode) {
            SIM_led = GLOW;  // Led Indication for GSM interrupt in sleep mode 
            //sleepCount = 2; // Set minimum sleep to 2 for recursive reading from GSM
            //sleepCountChangedDueToInterrupt = true; // Sleep count is altered and hence needs to be read from memory
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
                    cmtiCmd= true; // Set to indicate cmti command received	
                }
            } 
            //To extract sim location for SMS storage
            else if (cmtiCmd && msgIndex == 12) {
                cmtiCmd= false; // reset for next cmti command reception	
                temporaryBytesArray[0] = rxCharacter; // To store sim memory location of received message
                msgIndex = CLEAR;
                newSMSRcvd = true; // Set to indicate New SMS is Received
                //sleepCount = 1;
                //sleepCountChangedDueToInterrupt = true; // Sleep count is altered and hence needs to be calculated again
            }
        } 
        // check if GSM response to µc command is not completed
        else if (!controllerCommandExecuted) {
            SIM_led = GLOW;  // Led Indication for GSM interrupt in operational mode
            // Start storing response if received data is '+' at index zero
            if (rxCharacter == '+' && msgIndex == 0) {
                gsmResponse[msgIndex] = rxCharacter; // Load received byte into storage buffer
                msgIndex++; // point to next location for storing next received byte
            }
            // Cascade received data to stored response after receiving first character '+'
            else if (msgIndex > 0 && msgIndex <=220) {
                gsmResponse[msgIndex] = rxCharacter; // Load received byte into storage buffer
                // Cascade till 'OK'  is found
                
                if (gsmResponse[msgIndex - 1] == 'O' && gsmResponse[msgIndex] == 'K') {
                    controllerCommandExecuted = true; // GSM response to µc command is completed
                    msgIndex = CLEAR; // Reset message storage index to first character to start reading for next received byte of cmd
                } 
                // Read bytes till 500 characters
                else if (msgIndex <= 220) {
                    msgIndex++;
                }
            }
        }
        SIM_led = DARK;  // Led Indication for GSM interrupt is done 
        PIR4bits.RC3IF= CLEAR; // Reset the ISR flag.
    } // end RX interrupt
    else if (PIR3bits.RC1IF) {
        Run_led = GLOW; // Led Indication for system in Operational Mode
        rxCharacter = rxByteLora(); // Read byte received at Reception Register
        // Check if any overrun occur due to continuous reception
        if (RC1STAbits.OERR) {
            RC1STAbits.CREN = 0;
            Nop();
            RC1STAbits.CREN = 1;
        }
        if (rxCharacter == '#') {
            msgIndex = CLEAR; // Reset message storage index to first character to start reading from '+'
            decodedString[msgIndex] = rxCharacter; // Load Received byte into storage buffer
            msgIndex++; // point to next location for storing next received byte
        }
        else if (msgIndex < 50) {
            decodedString[msgIndex] = rxCharacter; // Load received byte into storage buffer
            msgIndex++; // point to next location for storing next received byte
            // check if storage index is reached to last character of CMTI command
            if (rxCharacter == '$') {
                msgIndex = CLEAR;
                controllerCommandExecuted = true; // Set to indicate command received from lora	
            }
        }
        SIM_led = DARK;  // Led Indication for LORA interrupt is done 
        PIR3bits.RC1IF= CLEAR; // Reset the ISR flag.
    } // end RX interrupt
    //Interrupt-on-change pins
    else if (PIR0bits.IOCIF) {
        Run_led = GLOW; // Led Indication for system in Operational Mode
        // Rising Edge -- All phase present
        if ((IOCEF5 == 1 || IOCEF6 == 1 || IOCEF7 == 1)) {
            myMsDelay(5000);
            if (phaseB == 0 && phaseY == 0 &&  phaseR == 0) {
                //phase is on
                IOCEF &= (IOCEF ^ 0xFF); //Clearing Interrupt Flags
                phaseFailureDetected = false;
                setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
                myMsDelay(5000);
                RESET();
            }
            else {
                // phase is out
                IOCEF &= (IOCEF ^ 0xFF); //Clearing Interrupt Flags
                phaseFailureDetected = true; //true
                phaseFailureActionTaken = false;
            }
        }
        PIR0bits.IOCIF = CLEAR; // Reset the ISR flag.
    }          
}


/*************************************************************************************************************************

This function is called when an interrupt is occurred after 16 bit timer is overflow
The purpose of this interrupt handler is to count no. of overflows that the timer did.

 **************************************************************************************************************************/

void __interrupt(low_priority) timerInterrupt_handler(void) {
    /*To follow filtration  cycle sequence*/
    if (PIR0bits.TMR0IF) {
        Run_led = GLOW; // Led Indication for system in Operational Mode
        PIR0bits.TMR0IF = CLEAR;
        TMR0H = 0xE3; // Load Timer0 Register Higher Byte 
        TMR0L = 0xB0; // Load Timer0 Register Lower Byte
        Timer0Overflow++;
        // Control sleep count decrement for each one minute interrupt when motor is on 
        if (sleepCount > 0 && MotorControl == ON) {
            sleepCount--;
            if (dryRunCheckCount == 0 || dryRunCheckCount < 3) {
                dryRunCheckCount++;
            }
        } 
        //*To follow filtration  cycle sequence*/
        if (filtrationCycleSequence == 1 && Timer0Overflow == filtrationDelay1 ) { // 10 minute off
            Timer0Overflow = 0;
            filtration1ValveControl = ON;
            filtrationCycleSequence = 2;
        }
        else if (filtrationCycleSequence == 2 && Timer0Overflow == filtrationOnTime ) {  // 1 minute on
            Timer0Overflow = 0;
            filtration1ValveControl = OFF;
            filtrationCycleSequence = 3;
        }
        else if (filtrationCycleSequence == 3 && Timer0Overflow == filtrationDelay2 ) { // 1 minute off
            Timer0Overflow = 0;
            filtration2ValveControl = ON;
            filtrationCycleSequence = 4;
        }
        else if (filtrationCycleSequence == 4 && Timer0Overflow == filtrationOnTime ) { // 1 minute on
            Timer0Overflow = 0;
            filtration2ValveControl = OFF;
            filtrationCycleSequence = 5;
        }
        else if (filtrationCycleSequence == 5 && Timer0Overflow == filtrationDelay2 ) { // 1 minute off 
            Timer0Overflow = 0;
            filtration3ValveControl = ON;
            filtrationCycleSequence = 6;
        }
        else if (filtrationCycleSequence == 6 && Timer0Overflow == filtrationOnTime ) { // 1 minute on
            Timer0Overflow = 0;
            filtration3ValveControl = OFF;
            filtrationCycleSequence = 7;
        }
        else if (filtrationCycleSequence == 7 && Timer0Overflow == filtrationSeperationTime ) { // 30 minutes all off
            Timer0Overflow = 0;
            filtrationCycleSequence = 1;
        }
        else if (filtrationCycleSequence == 99) {
            Timer0Overflow = 0;
        }
    }
/*To measure pulse width of moisture sensor output*/
/*
    if (PIR5bits.TMR1IF) {
        Run_led = GLOW; // Led Indication for system in Operational Mode
        Timer1Overflow++;
        PIR5bits.TMR1IF = CLEAR;
    }
*/
    if (PIR5bits.TMR3IF) {
        Run_led = GLOW; // Led Indication for system in Operational Mode
        PIR5bits.TMR3IF = CLEAR;
        TMR3H = 0xF0; // Load Timer3 Register Higher Byte 
        TMR3L = 0xDC; // Load Timer3 Register lower Byte 
        Timer3Overflow++;
        
        if (Timer3Overflow > timer3Count  && !controllerCommandExecuted) {
            controllerCommandExecuted = true; // Unlock key
            Timer3Overflow = 0;
            T3CONbits.TMR3ON = OFF; // Stop timer
            if(checkLoraConnection) {
                LoraConnectionFailed = true;
            }
        } 
        else if (controllerCommandExecuted) {
            Timer3Overflow = 0;
            T3CONbits.TMR3ON= OFF; // Stop timer
        }       
    }
}
//**************interrupt service routine handler#end***********//



//****************************MAIN FUNCTION#Start***************************************//
 void main(void) {
    NOP();
    NOP();
    NOP();
    unsigned char last_Field_No = CLEAR;
    //actionsOnSystemReset();
    configureController(); // set Microcontroller ports, ADC, Timer, I2C, UART, Interrupt Config
    myMsDelay(1000);
    loadDataFromEeprom(); // Read configured valve data saved in EEprom
    myMsDelay(1000);
    while (1) {
        loraAttempt = 0;
        do {
            sendCmdToLora(0,0); 
        } while(loraAttempt<5);
        if (!LoraConnectionFailed && loraAttempt == 99) {  // Successful Valve Activation
            setBCDdigit(0x01,1);
            myMsDelay(10000);
        }
        else {
            setBCDdigit(0x01,0);
            myMsDelay(10000);
        }
        loraAttempt = 0;
        do {
            sendCmdToLora(1,0); 
        } while(loraAttempt<5);
        if (!LoraConnectionFailed && loraAttempt == 99) {  // Successful Valve Activation
            setBCDdigit(0x02,1);
            myMsDelay(10000);
        }
        else {
            setBCDdigit(0x02,0);
            myMsDelay(10000);
        }
        /*
        loraAttempt = 0;
        do {
            sendCmdToLora(2,0); 
        } while(loraAttempt<5);
        if (!LoraConnectionFailed && loraAttempt == 99) {  // Successful Valve Activation
            setBCDdigit(0x03,1);
            myMsDelay(10000);
        }
        else {
            setBCDdigit(0x03,0);
            myMsDelay(10000);
        }
        */
    }
}
 
 /****************************************Check Lora Communication*************************************************/
            //transmitStringToLora("#STATUSMASTER$\r\n");
            //setBCDdigit(0x01,1);
            //transmitStringToDebug("#STATUSMASTER$\r\n");
            //myMsDelay(50000);
            //transmitStringToLora("#STATUSSLAVE01$\r\n");
            //setBCDdigit(0x02,1);
            //transmitStringToDebug("#STATUSSLAVE01$\r\n");
            //myMsDelay(50000);
            //transmitStringToLora("#GETSENSOR01SLAVE01$\r\n");
            //setBCDdigit(0x03,1);
            //transmitStringToDebug("#GETSENSOR01SLAVE01$");
            //myMsDelay(50000);
            //transmitStringToLora("#ON01SLAVE01$\r\n");
            //setBCDdigit(0x04,1);
            //transmitStringToDebug("#ON01SLAVE01$\r\n");
            //myMsDelay(50000);
            //transmitStringToLora("#GETVALVESTATUS01SLAVE01$\r\n");
            //setBCDdigit(0x05,1);
            //transmitStringToDebug("#GETVALVESTATUS01SLAVE01$\r\n");
            //myMsDelay(50000);
            //transmitStringToLora("#OFF01SLAVE01$\r\n");
            //setBCDdigit(0x06,1);
            //transmitStringToDebug("#OFF01SLAVE01$\r\n");
            //myMsDelay(50000);
            //transmitStringToLora("#GETVALVESTATUS01SLAVE01$\r\n");
            //setBCDdigit(0x07,1);
            //transmitStringToDebug("#GETVALVESTATUS01SLAVE01$\r\n");
            //myMsDelay(50000);
            //transmitStringToLora("#GARBAGE$\r\n");
            //setBCDdigit(0x08,1);
            //transmitStringToDebug("#GARBAGE$\r\n");
            //myMsDelay(50000);
            /****************************************Check Lora Communication*************************************************/
//****************************MAIN FUNCTION#End***************************************//
 /*Features Implemented*/
 // On hold fertigation and irrigation
 // Restart after phase/dry run detection -- check flow
 // Phase failure during sleep -- Done
 // filtration done but need to check flow -- Done
 // fertigation status eeprom read write -- Done
 // EEprom address need to be mapped -- Done
 // fertigation interrupt due to power and dry run -- Done
 // eeprom read write for fertigation values -- Done
 // save status in eeprom -- Done
 // Activate filtration and disabling message reception -- Done
 // Activate filtration and disabling  -- Done
 // eeprom read write for filtration values -- Done
 // Sleep count correction for next due valve. -- Done
 // Send Filtration status -- Done
 // actionsOnSystemReset
 // Sleep count correction for valve without filtration
 // Sleep count controlled by Timer0 during valve in action
 // phase failure at system start
 // Decision on valve interrupted due to dry run.
 
 
 
/*features remaining*/
 // RTC battery check -- unDone
 // RTC battery threshold not set
 // CT threshold not set
 // Separating fertigation cycle
 // Code Protection
 // Sensor calibration
 // cycles for field execution in dry run and phase failure
 /************High Priority**************/
 // due in hours
 // Onhold sms verify
 // random factory password
 // gsm get time
 
 
 /*Changes in New Board*/
 // RYB phase detection logic reversed i.e. 1 for no phase and 0 for phase
 // Rain Sensor 7 and 8 pin changed from RC0 and RC1 to RE4 and RD4 respectively
 // Added Priority and changed SMS format
 // decode of incoming sms required in extract receive function
 
             
