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
static uint8_t pwm_duty = 0;
static uint16_t pulse_duration_ms = 0;
static volatile uint16_t pulse_timer_ms = 0;
static volatile bool pulse_on = false;
static uint16_t on_time = 0;
static uint16_t off_time = 0;

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
    uint16_t duty_16 = (uint16_t)((FREQ_DEFAULT * intensity) / 100U);

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
}

static void pwm_stop(void)
{
    CCP2CON1Lbits.CCPON = 0;
    CCP3CON1Lbits.CCPON = 0;
}

// ------------------------------------------------------------
// Fire a pod?s PWM + relay with pulsing
// ------------------------------------------------------------
void relay_pwm_fire(uint8_t pod_index, uint16_t duration_ms, uint8_t pulse_duty, uint16_t pulse_period, uint8_t duty, uint8_t multiplier)
{
    if (pod_index >= 6)
    {
        return;
    }

    all_relays_off();
    relay_on(pod_index);

    active_pod = pod_index;
    uint32_t temp_period = (uint32_t)pulse_period * pulse_duty * multiplier / 255UL;
    on_time = (uint16_t)(temp_period / 100U);
    on_time = (on_time > pulse_period) ? pulse_period : on_time;
    on_time = (on_time > 0) ? on_time : 1;
    off_time = pulse_period - on_time;
    off_time = (off_time > 0) ? off_time : 1;
    pulse_duration_ms = duration_ms;
    pulse_timer_ms = 0;
    pulse_on = true;
    pwm_duty = duty;
    pwm_start(pod_index, pwm_duty);
    relay_pwm_evt_t evt = RELAY_PWM_EVT_FIRE;
    p_callback(&evt);
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

    if (pulse_on)
    {
        if (pulse_timer_ms >= on_time)
        {
            pwm_stop();
            pulse_on = false;
            pulse_timer_ms = 0;
        }
    }
    else
    {
        if (pulse_timer_ms >= off_time)
        {
            pwm_start(active_pod, pwm_duty); // re-enable PWM at nominal freq
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
