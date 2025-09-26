/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

/* 
 * File:                bluetooth.h
 * Author:              ALN
 * Comments:            Copyright 2025 Studio 29
 * Revision history:    First release
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef XC_BLUETOOTH_H
#define	XC_BLUETOOTH_H

#include <stdbool.h>
#include <xc.h> // include processor files - each processor file is guarded.  

/* Characteristics and Aliases */
#define BTNALIAS    "BTN"
#define BTNUUID     "04C3"

#define PODALIAS    "PID"
#define POD1UUID    "527bac61-60dc-4a73-aa99-d37ac6242931"
#define POD2UUID    "88a45d22-9a11-48de-8789-ae339f714132"
#define POD3UUID    "3f83c8bc-e98b-4959-aaa9-526297a1a5be"
#define POD4UUID    "0c4bd73e-7111-4a03-a900-aabad7c3e34c"
#define POD5UUID    "8eb288f4-ba80-4554-a828-434ee5246f55"
#define POD6UUID    "c5ea8885-17db-4f55-afe0-c028a31f3a59"

#define BATTALIAS   "BATT"
#define BATTUUID    "00002A19?0000?1000?8000?00805f9b34fb"

#define SCENTALIAS  "SLVL"
#define SCENT1UUID  "61f4a89a-3e66-433a-a868-9ce62c102021"
#define SCENT2UUID  "24c01483-6bdd-4e34-a87a-13c4b2223e70"
#define SCENT3UUID  "aa68337e-35c0-4cc6-b7f3-3c84a617614d"
#define SCENT4UUID  "5456f2b8-66ce-4b3b-b980-0dd2f99e2407"
#define SCENT5UUID  "0408bfbb-9742-48de-8b50-48b8c303ae83"
#define SCENT6UUID  "6a62bff7-a9c2-4e18-91e4-4fa57fdc2940 "

#define IDALIAS     "SID"
#define ID1UUID     "4eafb3f1-9fa0-494e-a379-24b08b411d21"
#define ID2UUID     "13633c84-7693-4131-88f5-1d695a407c88"
#define ID3UUID     "bb506175-17f7-429e-8d45-ff6f3411a7c3"
#define ID4UUID     "2e392451-49fe-4313-b6a3-2df45af67ca0"
#define ID5UUID     "653652e0-ada3-40ba-b9e8-8571694afa74"
#define ID6UUID     "c93db299-bfc9-4038-8006-b0ad02ca4517"

#define MSGALIAS    "SMSG"
#define MSGUUID     "45335526-67BA-4D9D-8CFB-C3D97E8D8208"

#define CLRALIAS    "CMSG"
#define USE_CHARS
#define USE_ALIASES

extern void setupBT(void);
extern void enableSerial(bool enable);
extern void BTTX(uint8_t *cmd, int length);
extern void BTRX(void);
extern void sendLEName(uint8_t *name);
extern void sendName(uint8_t *name);
extern void sendTest(void);
extern void sendReboot(void);
extern void sendDSCA(void);
extern void sendA2dpDisc(void);
extern void sendRestore(void);
extern void enableSSP(uint8_t en);
extern void profileSelect(uint16_t profiles);
extern void sendData(uint8_t *data, int dlen);
extern void sendButtonState(uint8_t state);
extern void sendBattLevel(uint16_t level);
extern void sendPod(uint8_t pod, uint16_t value);
extern void sendScentLevel(uint8_t num, uint16_t level);
extern void sendScentIDs(uint8_t num, uint16_t id);
extern void sendScentMsg(uint32_t message);
extern void sendPodIDsOLD(void);
extern void sendScentIDsOLD(void);
extern void sendScentLevelsOLD(void);
extern void sendPowerStatsOLD(uint16_t charge_mV, uint16_t trans_mV, uint16_t battery_mV);
extern void dump_OK(void);

#endif	/* XC_BLUETOOTH_H */

