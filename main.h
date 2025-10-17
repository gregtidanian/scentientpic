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

#define RED         /* removed */
#define GREEN       /* removed */
#define BLUE        /* removed */
#else
#define ENBSTPIC    LATDbits.LATD3
#define PIEZO_V_EN  LATEbits.LATE5

#define RED         /* removed */
#define GREEN       /* removed */
#define BLUE        /* removed */
#endif

// CHANGED: POT pins not used -> commented out
//#define POT_DATA    LATBbits.LATB3
//#define POT_CS      LATBbits.LATB2

// CHANGED: STAT old RD2 -> new RF6
#define STAT        PORTFbits.RF6

// New LED mappings (Green per pod side; White for intensity level)
// Right-side green LEDs
#define LED_G_R0    LATBbits.LATB15   // Pin 30 RB15
#define LED_G_R1    LATFbits.LATF4    // Pin 31 RF4
#define LED_G_R2    LATFbits.LATF5    // Pin 32 RF5
// Left-side green LEDs
#define LED_G_L0    LATDbits.LATD4    // Pin 52 RD4
#define LED_G_L1    LATDbits.LATD5    // Pin 53 RD5
#define LED_G_L2    LATDbits.LATD6    // Pin 54 RD6
// White LEDs for intensity feedback
#define LED_W_L0    LATDbits.LATD1    // Pin 49 RD1
#define LED_W_L1    LATDbits.LATD2    // Pin 50 RD2
#define LED_W_L2    LATDbits.LATD3    // Pin 51 RD3

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