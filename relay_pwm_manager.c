#include "relay_pwm_manager.h"

#define FCY 16000000UL

#define FREQ_DEFAULT 93

#define UART1RXPIN 26 // RG7 (RP26)
#define UART1TXPIN 21 // RG6 (RP21)

#define MCCP2A_FUNC_CODE (16)
#define MCCP3A_FUNC_CODE (18)

typedef struct
{
    volatile uint16_t *tris;
    volatile uint16_t *lat;
    uint16_t mask;
} relay_pin_t;

// Relay mapping (positive logic)
static const relay_pin_t RELAYS[6] = {
    {&TRISB, &LATB, (1u << 7)}, // 1R
    {&TRISB, &LATB, (1u << 8)}, // 2R
    {&TRISB, &LATB, (1u << 9)}, // 3R
    {&TRISE, &LATE, (1u << 1)}, // 1L
    {&TRISE, &LATE, (1u << 0)}, // 2L
    {&TRISF, &LATF, (1u << 1)}  // 3L
};

static uint8_t active_pod = 0xFF;
static uint8_t pulse_intensity = 100;
static uint16_t pulse_duration_ms = 0;
static uint16_t pulse_timer_ms = 0;
static bool pulse_on = false;

static relay_pwm_fire_callback_t p_callback = NULL;

// ------------------------------------------------------------
// Relay control
// ------------------------------------------------------------
static inline void relay_on(uint8_t pod)
{
    *RELAYS[pod].lat |= RELAYS[pod].mask;
}

static inline void relay_off(uint8_t pod)
{
    *RELAYS[pod].lat &= ~RELAYS[pod].mask;
}

static inline void all_relays_off(void)
{
    for (uint8_t i = 0; i < 6; i++)
    {
        relay_off(i);
    }
}

// ------------------------------------------------------------
// Initialize relays and CCP PWM modules
// ------------------------------------------------------------
void relay_pwm_init(relay_pwm_fire_callback_t p_cb)
{
    uint8_t i;

    p_callback = p_cb;

    // --- Relays ---
    for (i = 0; i < 6; i++)
    {
        *RELAYS[i].tris &= ~RELAYS[i].mask; // Output
        relay_off(i);
    }

/*     __builtin_write_OSCCONL(OSCCON & 0xBF);
    RPINR18bits.U1RXR = UART1RXPIN;    // U1RX <- RP26 (RG7)
    RPOR10bits.RP21R = 3;              // RP21 -> U1TX (RG6)
    RPOR4bits.RP8R = MCCP2A_FUNC_CODE; // RP6 <- MCCP2 Output A  (check func code!)
    RPOR5bits.RP11R = MCCP3A_FUNC_CODE;
    __builtin_write_OSCCONL(OSCCON | 0x40); */

    // --- Timer2 for PWM timebase ---
    //T2CON = 0;
    //TMR2 = 0;
    //PR2 = 0xFFFF;
    //T2CONbits.TCKPS = 0; // 1:1 prescale
    //T2CONbits.TON = 0;

    // --- CCP1 (Right) ---
    ANSELBbits.ANSB6 = 0; // digital
    TRISBbits.TRISB6 = 0; // output (so the PWM can drive the pin)

    CCP2CON1Lbits.CCPON = 0;      // disable MCCP2 to start
    CCP2CON1Lbits.CCSEL = 0;      // internal time base
    CCP2CON1Lbits.MOD = 0b0101;   // dual-edge buffered PWM
    CCP2CON1Lbits.CLKSEL = 0b000; // Tcy (Fosc/2)
    CCP2CON1Lbits.TMRPS = 0b00;   // 1:1 prescale

    CCP2PRL = FREQ_DEFAULT; // period
    CCP2RA = 0;             // edge A
    CCP2RB = 0;             // Set edge B for compare output

    CCP2CON3Hbits.OUTM = 0b000; // single-ended on OCxA
    CCP2CON2Hbits.OCAEN = 1;    // enable A output

    // --- CCP2 (Left) ---
    TRISDbits.TRISD0 = 0; // output (so the PWM can drive the pin)

    CCP3CON1Lbits.CCPON = 0;      // disable MCCP3 to start
    CCP3CON1Lbits.CCSEL = 0;      // internal time base
    CCP3CON1Lbits.MOD = 0b0101;   // dual-edge buffered PWM
    CCP3CON1Lbits.CLKSEL = 0b000; // Tcy (Fosc/2)
    CCP3CON1Lbits.TMRPS = 0b00;   // 1:1 prescale

    CCP3PRL = FREQ_DEFAULT; // period (match your R side)
    CCP3RA = 0;             // edge A
    CCP3RB = 0;             // Set edge B for output

    CCP3CON3Hbits.OUTM = 0b000; // single-ended on OCxA
    CCP3CON2Hbits.OCAEN = 1;    // enable A output

    // --- Timer3 for 1ms tick (moved from Timer4 to avoid conflict with main) ---
    T3CON = 0;
    TMR3 = 0;
    PR3 = (uint16_t)(FCY / 1000UL / 256UL);
    T3CONbits.TCKPS = 0b11; // 1:256 prescale
    IEC0bits.T3IE = 1;
    IFS0bits.T3IF = 0;
    T3CONbits.TON = 1;
}

// ------------------------------------------------------------
// PWM start/stop
// ------------------------------------------------------------
static void pwm_start(uint8_t pod, uint16_t intensity)
{
    uint16_t duty_16 = (uint16_t)((FREQ_DEFAULT * intensity) / FREQ_DEFAULT);
    
    // Avoid race with 1 ms ISR while touching CCP/T2
    uint16_t t3ie = IEC0bits.T3IE;
    IEC0bits.T3IE = 0;

    T2CONbits.TON = 0;

    if (pod < 3)
    {
        CCP2RB = duty_16;        // falling edge (50% duty)
        CCP2CON1Lbits.CCPON = 1; // enable MCCP2
    }
    else
    {
        CCP3RB = duty_16;
        CCP3CON1Lbits.CCPON = 1; // enable MCCP3
    }

    T2CONbits.TON = 1; // enable Timer2 for PWM timebase
    IEC0bits.T3IE = t3ie;
}

static void pwm_stop(void)
{
    // Avoid race with 1 ms ISR while touching CCP/T2
    uint16_t t3ie = IEC0bits.T3IE;
    IEC0bits.T3IE = 0;

    CCP2CON1Lbits.CCPON = 0;
    CCP3CON1Lbits.CCPON = 0;
    T2CONbits.TON = 0;
    IEC0bits.T3IE = t3ie;
}

// ------------------------------------------------------------
// Fire a pod?s PWM + relay with pulsing
// ------------------------------------------------------------
void relay_pwm_fire(uint8_t pod_index, uint16_t duration_ms, uint8_t intensity)
{
    if (pod_index >= 6)
    {
        return;
    }

    all_relays_off();
    relay_on(pod_index);

    pwm_start(pod_index, intensity);
    relay_pwm_evt_t evt = RELAY_PWM_EVT_FIRE;
    p_callback(&evt);
    active_pod = pod_index;
    pulse_intensity = intensity;
    pulse_duration_ms = duration_ms;
    pulse_timer_ms = 0;
    pulse_on = true;
}

void relay_pwm_stop(void)
{
    pwm_stop();
    all_relays_off();
    active_pod = 0xFF;
    pulse_duration_ms = 0;
    pulse_on = false;
}

// ------------------------------------------------------------
// Timer3 ISR: handles 1ms tick for pulsing and duration
// ------------------------------------------------------------
void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void)
{
    IFS0bits.T3IF = 0;

    if (active_pod == 0xFF)
    {
        return;
    }

    pulse_timer_ms++;

    uint16_t on_ms = (pulse_intensity * 1000UL) / 100;
    uint16_t off_ms = 1000UL - on_ms;

    if (pulse_on)
    {
        if (pulse_timer_ms >= on_ms)
        {
            pwm_stop();
            pulse_on = false;
            pulse_timer_ms = 0;
        }
    }
    else
    {
        if (pulse_timer_ms >= off_ms)
        {
            pwm_start(active_pod, 50); // re-enable PWM at nominal freq
            pulse_on = true;
            pulse_timer_ms = 0;
        }
    }

    // Duration countdown
    if (pulse_duration_ms)
    {
        if (pulse_duration_ms > 1)
        {
            pulse_duration_ms--;
        }
        else
        {
            relay_pwm_stop();
            relay_pwm_evt_t evt = RELAY_PWM_EVT_STOP;
            p_callback(&evt);
        }
    }
}
