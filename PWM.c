#include "main.h"
#include "PWM.h"

uint16_t freq = FREQ_MED;
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
    __builtin_write_OSCCONL(OSCCON & 0xBF);
    // CHANGED: CCP2 output old RP26 -> new RP6 (RB6) for PIEZO_R oscillator
    //RPOR3bits.RP6R = 16;       // RP6  PWM PIEZO R (CCP2)
    RPOR4bits.RP8R = 16;       // RP6  PWM PIEZO R (CCP2)
#ifdef BOARD_VER_A
    // CHANGED: CCP3 output old RP31 -> new RP11 (RD0) for PIEZO_L oscillator
    RPOR5bits.RP11R = 18;      // RP11 PWM PIEZO L (CCP3)
#else
    // CHANGED: CCP3 output old RP5 -> new RP11 (RD0) for PIEZO_L oscillator
    RPOR5bits.RP11R = 18;      // RP11 PWM PIEZO L (CCP3)
#endif
    __builtin_write_OSCCONL(OSCCON | 0x40);
}

void PWM_PIEZO_R(unsigned int level)
{
    unsigned int l16L = 93u * level / 144u;
    ANSELBbits.ANSB6 = 0;                 // digital
    TRISBbits.TRISB6 = 0;                 // output (so the PWM can drive the pin)

    CCP2CON1Lbits.CCSEL  = 0;             // internal time base
    CCP2CON1Lbits.MOD    = 0b0101;        // dual-edge buffered PWM
    CCP2CON1Lbits.CLKSEL = 0b000;         // Tcy (Fosc/2)
    CCP2CON1Lbits.TMRPS  = 0b00;          // 1:1 prescale

    CCP2PRL = freq;                         // period
    CCP2RA  = 0;                          // edge A
    //CCP2RB  = (CCP2PRL + 1u) / 2u;        // ~50% duty
    CCP2RB = l16L;

    CCP2CON3Hbits.OUTM  = 0b000;          // single-ended on OCxA
    CCP2CON2Hbits.OCAEN = 1;              // enable A output
    CCP2CON1Lbits.CCPON = 1;              // enable MCCP2
}


void PWM_PIEZO_L(unsigned int level)
{
    unsigned int l16L = 93u * level / 144u;
    TRISDbits.TRISD0 = 0;                 // output (so the PWM can drive the pin)

    CCP3CON1Lbits.CCSEL  = 0;             // internal time base
    CCP3CON1Lbits.MOD    = 0b0101;        // dual-edge buffered PWM
    CCP3CON1Lbits.CLKSEL = 0b000;         // Tcy (Fosc/2)
    CCP3CON1Lbits.TMRPS  = 0b00;          // 1:1 prescale

    CCP3PRL = freq;                         // period (match your R side)
    CCP3RA  = 0;                          // edge A
    //CCP3RB  = 6u * (CCP3PRL + 1u) / 12u;        // ~50% duty (ignores 'level' just like R)
    CCP3RB = l16L;

    CCP3CON3Hbits.OUTM  = 0b000;          // single-ended on OCxA
    if (level == 0)
    {
        CCP3CON2Hbits.OCAEN = 0;              // enable A output
        CCP3CON1Lbits.CCPON = 0;              // enable MCCP3  
    }
    else
    {
        CCP3CON2Hbits.OCAEN = 1;              // enable A output
        CCP3CON1Lbits.CCPON = 1;              // enable MCCP3
    }
}


