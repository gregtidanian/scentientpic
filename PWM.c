#include "main.h"
#include "PWM.h"

uint16_t freq = FREQ_HI;
char freq_override = 0;
float global_multi = 1;

uint16_t calc_pwm_multi(uint8_t value)
{
    uint16_t pwm = BASE_PWM;
    float calc = 0;
    calc = (value * global_multi) * 0.7;
    
    if (calc <= 70)
        pwm = (uint16_t)calc;
    return pwm;
}

void PWM_FREQ(uint8_t percent)
{
    if (freq_override)
        return;
    
    if (percent > 10)
    {
        if (percent > 60)
            freq = FREQ_HI;
        else if (percent > 30)
            freq = FREQ_MED;
        else
            freq = FREQ_LOW;
    }
    else
    {
        if (percent > 6)
            freq = FREQ_HI;
        else if (percent > 3)
            freq = FREQ_MED;
        else
            freq = FREQ_LOW;
    }
}

void PWM_Setup(void)            //PPS allocating
{
    // CHANGED: CCP2 output old RP26 -> new RP6 (RB6) for PIEZO_R oscillator
    RPOR3bits.RP6R = 16;       // RP6  PWM PIEZO R (CCP2)
#ifdef BOARD_VER_A
    // CHANGED: CCP3 output old RP31 -> new RP11 (RD0) for PIEZO_L oscillator
    RPOR5bits.RP11R = 18;      // RP11 PWM PIEZO L (CCP3)
#else
    // CHANGED: CCP3 output old RP5 -> new RP11 (RD0) for PIEZO_L oscillator
    RPOR5bits.RP11R = 18;      // RP11 PWM PIEZO L (CCP3)
#endif
}

void PWM_PIEZO_R(unsigned int level)
{
    unsigned int level_16;
    
    level_16 = level;// * 64;
    CCP2CON1Lbits.CCSEL = 0;        // Setup as a PWM perhipheral
    CCP2CON1Lbits.CCPON = 1;        // Enable PWM perhipheral
    CCP2CON1Lbits.TMRSYNC = 1;      // Enable sync with timer
    CCP2CON1Lbits.CLKSEL = 0;       // Use system clock
    CCP2CON1Lbits.MOD = 4;          // Set to capture every 4th rising edge
    CCP2CON2Hbits.OCAEN = 1;        // Enable output and set steering for PWM
    CCP2CON2Hbits.OCBEN = 1;        // Enable output and set steering for PWM
    CCP2RA = 0x0000;                // Write 0 to buffer A
    CCP2RB = level_16;              // Write pulse value to buffer B
    CCP2PRL = freq;                 // Set pulse duration for roughly 110KHz
}

void PWM_PIEZO_L(unsigned int level)
{
    unsigned int level_16;
    
    level_16 = level;// * 64;
    CCP3CON1Lbits.CCSEL = 0;        // Setup as a PWM perhipheral
    CCP3CON1Lbits.CCPON = 1;        // Enable PWM perhipheral
    CCP3CON1Lbits.TMRSYNC = 1;      // Enable sync with timer
    CCP3CON1Lbits.CLKSEL = 0;       // Use system clock
    CCP3CON1Lbits.MOD = 4;          // Set to capture every 4th rising edge
    CCP3CON2Hbits.OCAEN = 1;        // Enable output and set steering for PWM
    CCP3CON2Hbits.OCBEN = 1;        // Enable output and set steering for PWM
    CCP3RA = 0x0000;                // Write 0 to buffer A
    CCP3RB = level_16;              // Write pulse value to buffer B
    CCP3PRL = freq;                 // Set pulse duration for roughly 110KHz
}