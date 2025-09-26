#include "main.h"
#include "adc.h"

float c_volts, c_volts_comp, t_volts, t_volts_comp, b_volts, b_volts_comp;

void setup_adc(void)
{
    // Setup V_C_PIC as an analog input
    TRISBbits.TRISB12 = 1;
    ANSELBbits.ANSB12 = 1;
    
    // Setup V_T_PIC as an analog input
    TRISBbits.TRISB13 = 1;
    ANSELBbits.ANSB13 = 1;
    
    // Setup V_B_PIC as an analog input
    TRISBbits.TRISB14 = 1;
    ANSELBbits.ANSB14 = 1;
    
    AD1CHSbits.CH0SA = 12;          // Initialise ADC on AN12 (V_C_PIC)
    AD1CON1bits.ADON = 1;           // Enable ADC
    AD1CON3bits.SAMC = 0x0F;        // Auto-Sample time select bits (15)
    AD1CON3bits.ADCS = 0x0F;        // A/D conversion clock bits (15)
}

uint16_t calc_adc(uint8_t channel)
{
    AD1CHSbits.CH0SA = channel;
    __delay_ms(10);
    AD1CON1bits.SAMP = 1;
    __delay_ms(10);
    AD1CON1bits.SAMP = 0;
    while (!AD1CON1bits.DONE);
    if (12 == channel)
    {
        c_volts = ADC1BUF0;
        c_volts = (c_volts / 1024) * 2.5;
        /*err = c_volts - c_volts_comp;
        if (err > .05 || err < -.05)
            c_volts_comp = c_volts;*/
        c_volts_comp = 2.21 * c_volts;  // (((12k + 10k) + 1) / 10k) * c_volts
        
        uint16_t c_millis = (c_volts_comp * 1000);
        return c_millis;
    }
    else if (13 == channel)
    {
        t_volts = ADC1BUF0;
        t_volts = (t_volts / 1024) * 2.5;
        /*err = t_volts - t_volts_comp;
        if (err > .05 || err < -.05)
            t_volts_comp = t_volts;*/
        t_volts_comp = 3.9 * t_volts;  // (((11k3 + 3k9) + 1) / 3k9) * t_volts
        
        uint16_t t_millis = (t_volts_comp * 1000);
        return t_millis;
    }
    else if (14 == channel)
    {
        b_volts = ADC1BUF0;
        b_volts = (b_volts / 1024) * 2.5;
        /*err = b_volts - b_volts_comp;
        if (err > .05 || err < -.05)
            b_volts_comp = t_volts;*/
        b_volts_comp = 1.83 * b_volts;  // (((8k2 + 10k) + 1) / 10k) * b_volts
        
        uint16_t b_millis = (b_volts_comp * 1000);
        return b_millis;
    }
    return 0;
}
