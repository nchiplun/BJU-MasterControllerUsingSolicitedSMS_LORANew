/*
 * File name            : eeprom.h
 * Compiler             : MPLAB XC8/ MPLAB C18 compiler
 * IDE                  : Microchip  MPLAB X IDE v5.25
 * Processor            : PIC18F66K40
 * Author               : Bhoomi Jalasandharan
 * Created on           : July 15, 2020, 05:23 PM
 * Description          : EEPROM Read/ Write functions header file
 */


#ifndef EEPROM_H
#define	EEPROM_H

void eepromWrite(unsigned int, unsigned char); // To write 8 bit variable/8 bit data into EEPROM
unsigned char eepromRead(unsigned int); // To fetch 8 bit variable/8 bit data from EEPROM
void saveIrrigationValveValuesIntoEeprom(unsigned int, struct FIELDVALVE *); // To save all varibales of field valves EEPROM
void saveIrrigationValveDueTimeIntoEeprom(unsigned int, struct FIELDVALVE *); // To save field valve time variables into EEPROM
void saveIrrigationValveOnOffStatusIntoEeprom(unsigned int, struct FIELDVALVE *); // To save field valve On Off status into EEPROM
void saveIrrigationValveCycleStatusIntoEeprom(unsigned int, struct FIELDVALVE *); // To save field valve cycle status into EEPROM
void saveFertigationValveStatusIntoEeprom(unsigned int, struct FIELDVALVE *); // To save field valve On Off status into EEPROM
void saveFertigationValveValuesIntoEeprom(unsigned int, struct FIELDVALVE *); // To save all varibales of field valves EEPROM
void saveIrrigationValveConfigurationStatusIntoEeprom(unsigned int, struct FIELDVALVE *); // To save field valve Hold status into EEPROM
void readValveDataFromEeprom(unsigned int, struct FIELDVALVE *); // To read field valve values in to EEPROM
unsigned char readFieldIrrigationValveNoFromEeprom(void); // To read last saved field valve no. 
void loadDataFromEeprom(void); // To read all data from EEPROM
void loadDataIntoEeprom(void); // To save all data into EEPROM
void savePasswordIntoEeprom(void); // To save system password into EEPROM
void saveMobileNoIntoEeprom(void); // To save user identity into EEPROM
void saveActiveSleepCountIntoEeprom(void); // To save active sleep time into EEPROM
void saveRemainingFertigationOnPeriod(void); // To save fertigation remaining sleep time into EEPROM
void saveAuthenticationStatus(void); // To save system authentication status into EEPROM
void saveRTCBatteryStatus(void); // To save RTCBattery status into EEPROM
unsigned int readActiveSleepCountFromEeprom(void); // To read sleep time from EEPROM for active valve on period
unsigned int readRemainingFertigationOnPeriodFromEeprom(void);// To read Remaining Fertigation On Period From Eeprom
void saveIrrigationValveNoIntoEeprom(unsigned char); // To save field no. into EEPROM 
void saveFiltrationSequenceData(void); // To save filtration valve sequence data
void saveResetCountIntoEeprom(void); //To save reset occurred by MCLR reset
//void readResetCountFromEeprom(void); //To read reset occurred by MCLR reset
void saveDeviceProgramStatusIntoEeprom(void); // To save device programming status
//void readDeviceProgramStatusFromEeprom(void); // To read device programming status
void saveFactryPswrdIntoEeprom(void); // To save factory set password
void readFactryPswrdFromEeprom(void); // To read factory set password
void saveMotorLoadValuesIntoEeprom(void); // To save motor load conditions
void readMotorLoadValuesFromEeprom(void); // To load motor load conditions

#endif
/* EEPROM_H */