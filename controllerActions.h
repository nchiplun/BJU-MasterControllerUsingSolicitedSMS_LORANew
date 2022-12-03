/*
 * File name            : controllerActions.h
 * Compiler             : MPLAB XC8/ MPLAB C18 compiler
 * IDE                  : Microchip  MPLAB X IDE v5.25
 * Processor            : PIC18F66K40
 * Author               : Bhoomi Jalasandharan
 * Created on           : July 15, 2020, 05:23 PM
 * Description          : Controller general functions definition header file
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef ACTIONS_H
#define	ACTIONS_H

// include processor files - each processor file is guarded.

//***********general purpose function prototype**************/

void myMsDelay(unsigned int);                   // System delay in milliseconds
void configureController(void);                 // System configuration
void deepSleep(void);                           // Sleep Activity
_Bool isSystemReady(void);                      // To check system is ready
_Bool isNumber(unsigned char);                  // To check if variable is number
_Bool isBase64String(char *);                   // To check if string is base 64 encoded                
void extractReceivedSms(void);                  // To extract data received in GSM
unsigned int days(unsigned char, unsigned char); // To calculate no. of days between 2 dates of calender
void getDateFromGSM(void);                      // To fetch current time stamp from GSM 
void getDueDate(unsigned char);                 // To calculate due date
void scanValveScheduleAndGetSleepCount(void);                       // To calculate sleep value to become idle
void activateValve(unsigned char);              // Trigger valve
void deActivateValve(unsigned char);            // Stop valve
void powerOnMotor(void);						// Start motor
void powerOffMotor(void);						// Stop motor
_Bool isFieldMoistureSensorWet(unsigned char);	// Measure moisture level for field 1 
_Bool isMotorInNoLoad(void);                    // Measure motor ct current
void calibrateMotorCurrent(unsigned char, unsigned char);      // Calibrate Motor rated current
void doDryRunAction(void);                      // Take action after detecting Dry Run
void doLowPhaseAction(void);                    // Take action after detecting Low Phase current
void doPhaseFailureAction(void);                // Take action after detecting PhaseFailure
_Bool isRTCBatteryDrained(void);                // Measure RTC Battery Voltage
unsigned char fetchFieldNo(unsigned char);      //Fetch field no from sms
_Bool phaseFailure(void);                       // Check all 3 phase presence
void scanIrrigationValveForAction(void);        // Navigate through each irrigation valve and check for any actions
void setBCDdigit(unsigned char, _Bool);                     // SET BCD digit to number
void actionsOnSystemReset(void);                 // Perform actions on system reset
void actionsOnSleepCountFinish(void);           // Perform actions on completing sleep count
void actionsOnDueValve(unsigned char);          // Perform actions on due valve checked
char *strcpyCustom(char *restrict dest, const char *restrict src); //custom string copy till \n character
void deleteUserData(void);                      // Perform User reset action
void deleteValveData(void);                      // Perform Valve reset action
void randomPasswordGeneration(void);            // Function to randomly generates password of length 6
void deleteGsmResponse(void);                   // This function is called to delete gsm response string
void deleteStringToDecode(void);                // This function is called to delete StringToDecode string
void deleteDecodedString(void);                 // This function is called to delete DecodedString string

#endif
/* ACTIONS_H */