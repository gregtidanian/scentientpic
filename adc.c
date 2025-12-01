#include "main.h"
#include "adc.h"

float c_volts, c_volts_comp, t_volts, t_volts_comp, b_volts, b_volts_comp;

void setup_adc(void)
{
    // Setup V_B_PIC as an analog input
    TRISBbits.TRISB14 = 1;
    ANSELBbits.ANSB14 = 1;

    AD1CHSbits.CH0SA = 14;   // Initialise ADC on AN12 (V_C_PIC)
    AD1CON1bits.ADON = 1;    // Enable ADC
    AD1CON3bits.SAMC = 0x0F; // Auto-Sample time select bits (15)
    AD1CON3bits.ADCS = 0x0F; // A/D conversion clock bits (15)
}

uint16_t calc_adc(void)
{
    uint16_t b_millis = 0;

    AD1CHSbits.CH0SA = channel;
    __delay_ms(10);
    AD1CON1bits.SAMP = 1;
    __delay_ms(10);
    AD1CON1bits.SAMP = 0;
    while (!AD1CON1bits.DONE)
        ;
    b_volts = ADC1BUF0;
    b_volts = (b_volts / 1024) * 2.5;
    /*err = b_volts - b_volts_comp;
    if (err > .05 || err < -.05)
        b_volts_comp = t_volts;*/
    b_volts_comp = 1.83 * b_volts; // (((8k2 + 10k) + 1) / 10k) * b_volts

    b_millis = (b_volts_comp * 1000);
    return b_millis;
}
