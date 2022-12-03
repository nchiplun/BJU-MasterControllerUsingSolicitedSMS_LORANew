/*
 * File name            : gsm.c
 * Compiler             : MPLAB XC8/ MPLAB C18 compiler
 * IDE                  : Microchip  MPLAB X IDE v5.25
 * Processor            : PIC18F66K40
 * Author               : Bhoomi Jalasandharan
 * Created on           : July 15, 2020, 05:23 PM
 * Description          : encoding-decoding functions definitions source file
 */

#include "variableDefinitions.h"
#ifdef Encryption_ON_H
#include "congfigBits.h"
#include "dataEncryption.h"
#include "controllerActions.h"


//*****************Message Encryption and Decryption function_Start****************//

/*************************************************************************************************************************

This function is called to Encode messages to be sent to GSM
The purpose of this function is to return encoded string of messages

 **************************************************************************************************************************/
/* 
void base64Encoder() { 
    // Character set of base64 encoding scheme 
    unsigned char char_set[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";   
    unsigned int index, no_of_bits = 0, padding = 0, count = 0, temp, stringLength; 
    unsigned int i, j, k = 0;
    unsigned long val = 0;
    setBCDdigit(0x0D,1); // (ç) BCD indication for Encryption Action
    stringLength = strlen((const char *)stringToEncode);
    // Loop takes 3 characters at a time from  
    // stringToEncode and stores it in val 
    for (i = 0; i < stringLength; i += 3) { 
        val = 0, count = 0, no_of_bits = 0;
        for (j = i; j < stringLength && j <= i + 2; j++) { 
            // binary data of input_str is stored in val 
            val = val << 8;
            // (A + 0 = A) stores character in val 
            val = val | stringToEncode[j];
            // calculates how many time loop  
            // ran if "MEN" -> 3 otherwise "ON" -> 2 
            count++;  
            NOP();
        }  
        no_of_bits = count * 8;   
        // calculates how many "=" to append after encodedString. 
        padding = no_of_bits % 3;  
        // extracts all bits from val (6 at a time)  
        // and find the value of each block 
        while (no_of_bits != 0) { 
            // retrieve the value of each block 
            if (no_of_bits >= 6) 
            { 
                temp = no_of_bits - 6;   
                // binary of 63 is (111111) f 
                index = (val >> temp) & 63;  
                no_of_bits -= 6;          
            } 
            else
            { 
                temp = 6 - no_of_bits;     
                // append zeros to right if bits are less than 6 
                index = (val << temp) & 63;  
                no_of_bits = 0; 
            } 
            encodedString[k++] = char_set[index];
            NOP();
        }
    }   
    // padding is done here 
    for (i = 1; i <= padding; i++) { 
        encodedString[k++] = '='; 
    }   
    encodedString[k] = '\0'; 
    setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
}
*/


/*************************************************************************************************************************

This function is called to Decode messages to came from GSM
The purpose of this function is to return decoded string of messages
 
**************************************************************************************************************************/

void base64Decoder() { 
    unsigned int i, j, k = 0; 
    // stores the bitstream. 
    unsigned long bitstream = 0; 
    // count_bits stores current 
    // number of bits in bitstream. 
    unsigned int count_bits = 0,stringLength; 
    setBCDdigit(0x0D,0); // (ç.) BCD indication for Decryption Action
    stringLength = strlen((const char *)stringToDecode);
    // selects 4 characters from 
    // encoded string at a time. 
    // find the position of each stringToDecode 
    // character in char_set and stores in bitstream. 
    for (i = 0; i < stringLength; i += 4) { 
        bitstream = 0, count_bits = 0; 
        for (j = 0; j < 4; j++) { 
            // make space for 6 bits. 
            if (stringToDecode[i + j] != '=') { 
                bitstream = bitstream << 6; 
                count_bits += 6; 
            }
            /* Finding the position of each stringToDecode  
            character in char_set  
            and storing in "bitstream", use OR  
            '|' operator to store bits.*/
  
            // stringToDecode[i + j] = 'E', 'E' - 'A' = 5 
            // 'E' has 5th position in char_set. 
            if (stringToDecode[i + j] >= 'A' && stringToDecode[i + j] <= 'Z') 
                bitstream = bitstream | (stringToDecode[i + j] - 'A');
            // stringToDecode[i + j] = 'e', 'e' - 'a' = 5, 
            // 5 + 26 = 31, 'e' has 31st position in char_set. 
            else if (stringToDecode[i + j] >= 'a' && stringToDecode[i + j] <= 'z') 
                bitstream = bitstream | (stringToDecode[i + j] - 'a' + 26);
            // stringToDecode[i + j] = '8', '8' - '0' = 8 
            // 8 + 52 = 60, '8' has 60th position in char_set. 
            else if (stringToDecode[i + j] >= '0' && stringToDecode[i + j] <= '9') 
                bitstream = bitstream | (stringToDecode[i + j] - '0' + 52);
            // '+' occurs in 62nd position in char_set. 
            else if (stringToDecode[i + j] == '+') 
                bitstream = bitstream | 62;
            // '/' occurs in 63rd position in char_set. 
            else if (stringToDecode[i + j] == '/') 
                bitstream = bitstream | 63;
            // ( str[i + j] == '=' ) remove 2 bits 
            // to delete appended bits during encoding. 
            else { 
                bitstream = bitstream >> 2; 
                count_bits -= 2; 
            } 
        }
        while (count_bits != 0) { 
            count_bits -= 8;
            // 255 in binary is 11111111 
            decodedString[k++] = (bitstream >> count_bits) & 255; 
        } 
    }
    // place NULL character to mark end of string. 
    decodedString[k] = '\0';  
    setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
}
#endif