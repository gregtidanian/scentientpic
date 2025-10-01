/*
 * File:   main.h
 * Author: ALN
 * Description: Main header file for Scentient Collar Firmware
 * Comments: Copyright 2025 PD Studio 29 LTD
 * Created on 13 January 2025, 10:00
 */
#include <xc.h>   

#include <p24FJ128GL306.h>
#define FCY 16000000UL
#include <libpic30.h>

//#define BOARD_VER_A
#ifndef BOARD_VER_A
// CHANGED: PWR_LATCH old LATA0 -> new LATD9
#define PWR_LATCH   LATDbits.LATD9
// CHANGED: ENBSTPIC old LATD3 -> new LATF2
#define ENBSTPIC    LATFbits.LATF2
// CHANGED: PIEZO_V_EN not used -> commented out
//#define PIEZO_V_EN  LATEbits.LATE5

#define RED         LATBbits.LATB8
#define GREEN       LATBbits.LATB9
#define BLUE        LATBbits.LATB10
#else
#define ENBSTPIC    LATDbits.LATD3
#define PIEZO_V_EN  LATEbits.LATE5

#define RED         LATEbits.LATE0
#define GREEN       LATEbits.LATE1
#define BLUE        LATFbits.LATF1
#endif

// CHANGED: POT pins not used -> commented out
//#define POT_DATA    LATBbits.LATB3
//#define POT_CS      LATBbits.LATB2

// CHANGED: STAT old RD2 -> new RF6
#define STAT        PORTFbits.RF6

#define CLEAR_LEDS do {} //(RED = BLUE = GREEN = 0)
#define IDLE_PURPLE do {} //(RED = BLUE = 1)

// Firmware information
#define VER_NUM         (double)0.01
#define VER_BUILD_DAY   13
#define VER_BUILD_MON   1
#define VER_BUILD_YEAR  2025
#define VER_DESCRIPTION "Scentient Collar Firmware"

typedef struct _DATA_STORE {
    uint16_t pod_ids[6];
    uint16_t scent_ids[6];
    uint8_t scent_levels[6];
    
    uint16_t batt_mV;
} data_store;

typedef struct _MESSAGE_STORE {
    uint8_t pod_active[6];
    uint8_t current_intensity[6];
    uint16_t current_duration[6];
} message_store;

extern data_store store;
extern message_store messageStore;
extern int global_intensity;

extern int fire_piezo_R1(uint8_t intensity, uint16_t duration);
extern int fire_piezo_R2(uint8_t intensity, uint16_t duration);
extern int fire_piezo_R3(uint8_t intensity, uint16_t duration);
extern int fire_piezo_L1(uint8_t intensity, uint16_t duration);
extern int fire_piezo_L2(uint8_t intensity, uint16_t duration);
extern int fire_piezo_L3(uint8_t intensity, uint16_t duration);

// -----------------------------------------------------------------------------
// DEV MODE FLAG: Enable this to run firmware without 1-Wire hardware attached.
// When enabled, pod discovery and EEPROM writes are mocked so you can trigger
// piezos over Bluetooth/UART without DS28xx devices present.
// Set to 0 (or comment out) for normal hardware behavior.
// -----------------------------------------------------------------------------
#ifndef DEV_NO_OWIRE
#define DEV_NO_OWIRE 1
#endif