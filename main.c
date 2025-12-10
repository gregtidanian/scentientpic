/*
 * File:   main.c
 * Author: ALN
 * Description: Main source file for Scentient Collar Firmware
 * Comments: Copyright 2025 PD Studio 29 LTD
 * Created on 13 January 2025, 10:00
 */
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "main.h"
// #include "PWM.h"
#include "bluetooth.h"
// #include "owire.h"
#include "adc.h"
// #include "piezos.h"

#include "i2c_async.h"
#include "relay_pwm_manager.h"
#include "pod_manager_async.h"

#define FREQ_DEFAULT 93 // remove after testing

static void pod_manager_async_fire_callback(pod_manager_async_evt_t *p_evt);

uint8_t intensity_multi = 10;

static i2c_async_t i2c1_async;
static pod_manager_async_t podman;

/* State machine flags */
static volatile bool pod_fire_active = false;
static volatile unsigned int pod_fire_bay = 0;
static volatile bool pod_is_firing = false;
static volatile unsigned int battery_poll_counter = 0;

// #pragma config FWDTEN = ON//was on
#pragma config FWDTEN = OFF
#pragma config FNOSC = FRCPLL
#pragma config JTAGEN = OFF
#pragma config WDTPS = PS2048

data_store store;
message_store messageStore;
char led;
uint16_t charge_mV, battery_mV, trans_mV;
uint8_t button_state = 0, down_pressed = 0, up_pressed = 0;
int global_intensity = 0;
volatile uint8_t int3_state = 0; // 0: idle, 1: wait 500ms, 2: wait 1000ms

// Shared Timer2 debounce for SW2/SW3
typedef enum
{
    DEBOUNCE_NONE = 0,
    DEBOUNCE_SW2_DECR,
    DEBOUNCE_SW3_INCR
} debounce_source_t;
static volatile debounce_source_t debounce_source = DEBOUNCE_NONE;

#define UART1RXPIN 26 // RG7 (RP26)
#define UART1TXPIN 21 // RG6 (RP21)

// Track which bay is currently firing (for LED indication)
static volatile uint8_t current_led_bay = 0xFF;

/* NOTE: Verify this PPS function code in the datasheet's PPS table for GL306.
   16 is a COMMON value for MCCP2 Output A, but confirm on your DFP/datasheet. */
#define MCCP2A_FUNC_CODE (16)
#define MCCP3A_FUNC_CODE (18)

static inline void pps_unlock(void) { __builtin_write_OSCCONL(OSCCON & 0xBF); }
static inline void pps_lock(void) { __builtin_write_OSCCONL(OSCCON | 0x40); }

static void osccon_init(void)
{
    // ADDED: Unlock PPS to allow RPINR/RPOR writes (clear IOLOCK)
    // Datasheet: OSCCON.IOLOCK must be cleared via unlock sequence
    __builtin_write_OSCCONL(OSCCON & 0xBF);

    // CHANGED: U1RXR old RP20 -> new RP21 (RG6)
    RPINR18bits.U1RXR = UART1RXPIN; // U1RX <- RP26 (RG7)
    // CHANGED: U1TX old RP25R -> new RP26R (RG7)
    // RPOR13bits.RP26R = 3;   // RG7 as UART1 TX
    RPOR10bits.RP21R = 3;

    RPINR18bits.U1RXR = UART1RXPIN;    // U1RX <- RP26 (RG7)
    RPOR10bits.RP21R = 3;              // RP21 -> U1TX (RG6)
    RPOR3bits.RP6R = MCCP2A_FUNC_CODE; // RP6 <- MCCP2 Output A  (check func code!)
    RPOR5bits.RP11R = MCCP3A_FUNC_CODE;

    // ADDED: Lock PPS after mapping (set IOLOCK)
    __builtin_write_OSCCONL(OSCCON | 0x40);
}

static void uart1_init(void)
{
    IEC0bits.U1TXIE = 0;
    IEC0bits.U1RXIE = 0;
    U1MODE = (0x8008 & ~(1 << 15));
    U1STA = 0;

    TRISGbits.TRISG6 = 0; // U1TX
    TRISGbits.TRISG7 = 1; // U1RX

    // ANSG = 0x0300;
    ANSGbits.ANSELG6 = 0;
    ANSGbits.ANSELG7 = 0;

    U1MODEbits.URXINV = 0;
    U1MODEbits.BRGH = 1;
    U1BRG = 34; // ~115200 @ FCY=16MHz
    U1MODEbits.UARTEN = 1;
    U1STAbits.UTXEN = 1;

    IPC16bits.U1ERIP = 1;
    IPC3bits.U1TXIP = 1;

    IFS0bits.U1RXIF = 0;
    IPC2bits.U1RXIP = 1;
    IEC0bits.U1RXIE = 1;
}

void pps_init(void)
{
    pps_unlock();
    RPINR18bits.U1RXR = UART1RXPIN;    // U1RX <- RP26 (RG7)
    RPOR10bits.RP21R = 3;              // RP21 -> U1TX (RG6)
    RPOR3bits.RP6R = MCCP2A_FUNC_CODE; // RP6 <- MCCP2 Output A  (check func code!)
    RPOR5bits.RP11R = MCCP3A_FUNC_CODE;
    pps_lock();
}

void init_pins(void)
{
    // CHANGED: PIEZO_R1 old RG6 -> new RB7
    ANSELBbits.ANSELB7 = 0; // PIEZO_1_R - digital pin
    TRISBbits.TRISB7 = 0;   // Use as an output
    // PIEZO_R1 = 0;               // Start with piezo 1 disabled

    // CHANGED: PIEZO_R2 old RE7 -> new RB8
    ANSELBbits.ANSELB8 = 0;
    TRISBbits.TRISB8 = 0; // PIEZO_2_R - use as output
    // PIEZO_R2 = 0;               // Start with piezo 2 disabled

    // CHANGED: PIEZO_R3 old RE6 -> new RB9
    ANSELBbits.ANSELB9 = 0;
    TRISBbits.TRISB9 = 0; // PIEZO_3_R - use as output
    // PIEZO_R3 = 0;               // Start with piezo 3 disabled

    // CHANGED: PIEZO_L1 old RD10 -> new RE1
    TRISEbits.TRISE1 = 0; // PIEZO_1_L - use as output
    // PIEZO_L1 = 0;               // Start with piezo 1 disabled

    // CHANGED: PIEZO_L2 old RD9 -> new RE0
    TRISEbits.TRISE0 = 0; // PIEZO_2_L - use as output
    // PIEZO_L2 = 0;               // Start with piezo 2 disabled

    // CHANGED: PIEZO_L3 old RD8 -> new RF1
    TRISFbits.TRISF1 = 0; // PIEZO_3_L - use as output
    // PIEZO_L3 = 0;               // Start with piezo 3 disabled

    // CHANGED: PIEZO_OSC_R old RG7 -> new RB6
    ANSELBbits.ANSELB6 = 0; // PIEZO_OSC_R - Use as digital pin
    TRISBbits.TRISB6 = 0;   // Use as an output

#ifdef BOARD_VER_A
    TRISGbits.TRISG3 = 0; // PIEZO_OSC_L - Use as an output
#else
    // CHANGED: PIEZO_OSC_L old RF6 -> new RD0
    TRISDbits.TRISD0 = 0; // PIEZO_OSC_L - Use as an output
#endif

    // CHANGED: STAT old RD2 -> new RF6, keep internal pull-up
    TRISFbits.TRISF6 = 1; // STAT input
    IOCPUF = 1;           // Enable pull up on Port F

    // CHANGED: ENBST old RD3 -> new RF2
    TRISFbits.TRISF2 = 0; // ENBST (Enable Boost) - Use as an output
    LATFbits.LATF2 = 1;   // Set high, to enable piezos to be fired on batt

    // Configure new LED pins (Green per pod, White intensity)
    // Right-side green LEDs: RB15, RF4, RF5
    ANSELBbits.ANSELB15 = 0; // RB15 digital
    TRISBbits.TRISB15 = 0;   // Output
    LED_G_R0 = 0;            // Idle low
    TRISFbits.TRISF4 = 0;    // RF4 output
    LED_G_R1 = 0;
    TRISFbits.TRISF5 = 0; // RF5 output
    LED_G_R2 = 0;

    // Left-side green LEDs: RD4, RD5, RD6
    TRISDbits.TRISD4 = 0; // RD4 output
    LED_G_L0 = 0;
    TRISDbits.TRISD5 = 0; // RD5 output
    LED_G_L1 = 0;
    TRISDbits.TRISD6 = 0; // RD6 output
    LED_G_L2 = 0;

    // White LEDs for intensity: RD1, RD2, RD3
    TRISDbits.TRISD1 = 0; // RD1 output
    LED_W_L0 = 0;
    TRISDbits.TRISD2 = 0; // RD2 output
    LED_W_L1 = 0;
    TRISDbits.TRISD3 = 0; // RD3 output
    LED_W_L2 = 0;

    // (Moved experimental TRISB6 indicator to after startup flash in main())

    // CHANGED: Button inputs on RD11 (SW2), RD10 (SW3), RD8 (SW1)
    ANSELDbits.ANSD11 = 0;
    ANSELDbits.ANSD10 = 0;
    // ANSELDbits.ANSD08 = 0;
    TRISDbits.TRISD11 = 1; // SW2 (INT1)
    TRISDbits.TRISD10 = 1; // SW3 (INT2)
    TRISDbits.TRISD8 = 1;  // SW1 (INT3)
    // CHANGED: Remove IOC config (using external INT1/2/3 instead)
    IOCPDbits.IOCPD11 = 1; // enable rising-edge on RD11
    IOCPDbits.IOCPD10 = 1; // enable rising-edge on RD10
    IOCPDbits.IOCPD8 = 1;  // enable rising-edge on RD10
    IOCNDbits.IOCND11 = 0; // disable falling-edge
    IOCNDbits.IOCND10 = 0; // disable falling-edge
    IOCNDbits.IOCND8 = 0;  // disable falling-edge
    IOCFD = 0;             // clear Port D per-pin change flags
    IFS1bits.IOCIF = 0;    // clear global IOC IF
    PADCONbits.IOCON = 1;  // global IOC enable
    IEC1bits.IOCIE = 1;    // enable IOC interrupt

#ifndef BOARD_VER_A
    // CHANGED: INT mappings -> INT1=RP12(RD11), INT2=RP3(RD10), INT3=RP2(RD8)

    // CHANGED: PWR_LATCH old RA0 -> new RD9 (no ANSEL bit on RD9)
    // ANSELDbits.ANSD9 = 0;       // PWR_LATCH - use as digital pin
    TRISDbits.TRISD9 = 0; // Use as an output pin.
    PWR_LATCH = 0;        // Idle high (Keep pic powered)
    __delay_ms(250);
    PWR_LATCH = 1;
    // while (PORTDbits.RD8);     // Previous startup polling (kept for reference)
    __delay_ms(100);

    IFS3bits.INT3IF = 0; // Clear INT3 flag
    IEC3bits.INT3IE = 1; // Enable INT3
#endif

    // CHANGED: ENBSTPIC TRIS old RD3 -> new RF2
    // TRISFbits.TRISF2 = 0; // ENBSTPIC - use as output pin
    // ENBSTPIC = 0;         // Idle low

    // Clear interrupt flags and enable interrupts for the
    // Increment and Decrement buttons.
    IFS1bits.INT1IF = 0;
    IEC1bits.INT1IE = 1;
    IFS1bits.INT2IF = 0;
    IEC1bits.INT2IE = 1;
    IFS3bits.INT3IF = 0;
    IEC3bits.INT3IE = 1;

    uart1_init();
}

void Setup_Timer1(void)
{
    T1CONbits.TON = 0;   // Stop timer while configuring
    T1CONbits.TCS = 0;   // Internal clock (FCY)
    T1CONbits.TCKPS = 2; // Prescaler 1:64
    TMR1 = 0;
    PR1 = 24999;       // 100 ms: (16 MHz / 64) * 0.1s = 25,000 ticks → PR1=24999
    IFS0bits.T1IF = 0; // Clear flag
    IEC0bits.T1IE = 1; // Enable interrupt
    T1CONbits.TON = 1; // Start
}

// Timer2: one-shot debounce for SW2/SW3
void Setup_Timer2(void)
{
    T2CONbits.TON = 0;
    T2CONbits.TCS = 0;   // Internal clock
    T2CONbits.TCKPS = 2; // 1:64 prescale -> 4us/tick @ 16MHz FCY
    TMR2 = 0;
    PR2 = 3750; // ~15ms debounce window
    IFS0bits.T2IF = 0;
    IEC0bits.T2IE = 0; // Enabled on demand when scheduling
}

// Timer4 is used for non-blocking power button timing
void Setup_Timer4(void)
{
    T4CONbits.TON = 0;   // Ensure timer is off
    T4CONbits.TCS = 0;   // Internal clock (Fosc/2)
    T4CONbits.TCKPS = 3; // 1:256 prescaler (16us per tick @ FCY=16MHz)
    TMR4 = 0;
    PR4 = 0xFFFF; // Placeholder; will be set per use
    IFS1bits.T4IF = 0;
    IEC1bits.T4IE = 0; // Enable only when scheduling
}

void startUp(int bor, int pwr)
{
    // Blink all LEDs (green pod indicators and white intensity LEDs) twice
    for (int i = 0; i < 2; i++)
    {
        LED_G_R0 = 1;
        LED_G_R1 = 1;
        LED_G_R2 = 1;
        LED_G_L0 = 1;
        LED_G_L1 = 1;
        LED_G_L2 = 1;
        LED_W_L0 = 1;
        LED_W_L1 = 1;
        LED_W_L2 = 1;
        __delay_ms(200);
        LED_G_R0 = 0;
        LED_G_R1 = 0;
        LED_G_R2 = 0;
        LED_G_L0 = 0;
        LED_G_L1 = 0;
        LED_G_L2 = 0;
        LED_W_L0 = 0;
        LED_W_L1 = 0;
        LED_W_L2 = 0;
        __delay_ms(200);
    }
}

static void clear_all_leds(void)
{
    // Turn off all pod green LEDs and white intensity LEDs
    LED_G_R0 = 0;
    LED_G_R1 = 0;
    LED_G_R2 = 0;
    LED_G_L0 = 0;
    LED_G_L1 = 0;
    LED_G_L2 = 0;
    LED_W_L0 = 0;
    LED_W_L1 = 0;
    LED_W_L2 = 0;
}

static void indicate_poweroff(void)
{
    // Briefly light all LEDs to indicate shutdown, then clear
    LED_G_R0 = 1;
    LED_G_R1 = 1;
    LED_G_R2 = 1;
    LED_G_L0 = 1;
    LED_G_L1 = 1;
    LED_G_L2 = 1;
    LED_W_L0 = 1;
    LED_W_L1 = 1;
    LED_W_L2 = 1;
    __delay_ms(150);
    clear_all_leds();
}

// Display current global intensity on the white LEDs (3-step bar).
static inline void update_intensity_leds(void)
{
    LED_W_L0 = 0;
    LED_W_L1 = 0;
    LED_W_L2 = 0;
    if (intensity_multi >= 5)
        LED_W_L0 = 1;
    if (intensity_multi >= 8)
        LED_W_L1 = 1;
    if (intensity_multi >= 11)
        LED_W_L2 = 1;
}

// Map bay index (0..5) to corresponding green LED and set it
static inline void set_bay_led(uint8_t bay, uint8_t on)
{
    switch (bay)
    {
    case 0:
        LED_G_R0 = on;
        break; // LED_G_1
    case 1:
        LED_G_R1 = on;
        break; // LED_G_2
    case 2:
        LED_G_R2 = on;
        break; // LED_G_3
    case 3:
        LED_G_L2 = on;
        break; // LED_G_6
    case 4:
        LED_G_L1 = on;
        break; // LED_G_5
    case 5:
        LED_G_L0 = on;
        break; // LED_G_4
    default:
        break;
    }
}

void check_poll(void)
{
    if (battery_poll_counter > 50)
    {
        battery_poll_counter = 0;
        // Grab current voltages on BATT line
        store.batt_mV = battery_mV = calc_adc();

        uint16_t percent = 0;
        if (battery_mV > 4000)
            percent = 100;
        else if (battery_mV > 3950)
            percent = 90;
        else if (battery_mV > 3900)
            percent = 80;
        else if (battery_mV > 3850)
            percent = 70;
        else if (battery_mV > 3800)
            percent = 60;
        else if (battery_mV > 3750)
            percent = 50;
        else if (battery_mV > 3700)
            percent = 40;
        else if (battery_mV > 3650)
            percent = 30;
        else if (battery_mV > 3600)
            percent = 20;
        else if (battery_mV > 3550)
            percent = 10;
        sendBattLevel(percent);

        if (battery_mV <= 3600)
        {
            // CLEAR_LEDS;
            PWR_LATCH = 0;
        }

        // Fire piezos
        // up_pressed = down_pressed = 0;
    }
}

void pod_fire_handler(void)
{
    if (pod_fire_active && !pod_is_firing)
    {
        pod_is_firing = true;
        current_led_bay = pod_fire_bay;
        pod_manager_fire(&podman, pod_fire_bay, podman.pods[pod_fire_bay].duration_ms, podman.pods[pod_fire_bay].intensity, intensity_multi);
        pod_fire_active = false;
    }
}

void pod_manager_async_read_callback(void *ctx, pod_manager_async_read_evt_t *p_evt)
{
    poda_t *p = (poda_t *)ctx;
    switch (*p_evt)
    {
    case POD_MANAGER_ASYNC_EVT_READ_SUCCESS:
    {
        // Figure out which pod this is
        int pod;
        for (pod = 0; pod < POD_BAY_COUNT; i++)
        {
            if (p == &podman.pods[pod])
            {
                break;
            }
        }
        // Send ScentID and level
        sendScentIDs(podman.pods[pod].bay, podman.pods[pod].data.scent_id);
        sendScentLevel(podman.pods[pod].bay, podman.pods[pod].bursts);
        break;
    }

    case POD_MANAGER_ASYNC_EVT_READ_FAIL: // Fall through for now

    default:
        break;
    }
}

void pod_manager_async_fire_callback(pod_manager_async_evt_t *p_evt)
{
    switch (*p_evt)
    {
    case POD_MANAGER_ASYNC_EVT_FIRE_START:
        set_bay_led(current_led_bay, 1);
        break;
    case POD_MANAGER_ASYNC_EVT_FIRE_STOP:
        set_bay_led(current_led_bay, 0);
        pod_is_firing = false;
        T1CONbits.TON = 1;
        break;
    default:
        break;
    }
}

void bluetooth_evt_callback(bluetooth_evt_data_t *p_evt_data)
{
    switch (p_evt_data->evt)
    {
    case BLUETOOTH_EVT_POD_FIRE:
        if (podman.pods[p_evt_data->pod].active)
        {
            if (pod_is_firing)
            {
                // Preempt current puff
                set_bay_led(current_led_bay, 0);
                relay_pwm_stop();
                pod_is_firing = false;
            }

            pod_fire_active = true;
            podman.pods[p_evt_data->pod].intensity = p_evt_data->intensity;
            podman.pods[p_evt_data->pod].duration_ms = p_evt_data->duration;
            pod_fire_bay = p_evt_data->pod;
        }
        break;
    }
}

int main(void)
{
    // int pods = 0;
    int bor = RCONbits.BOR;
    int pwr = RCONbits.POR;
    led = 0;
    battery_poll_counter = 0;
    memset(&store, 0, sizeof(data_store));
    memset(&messageStore, 0, sizeof(message_store));

    init_pins(); // Set up relevant PIC pins
    osccon_init();
    pod_manager_async_init(&podman, &i2c1_async, pod_manager_async_fire_callback);
    bluetooth_init(bluetooth_evt_callback);

    static const i2c_regs_t i2c1_regs = {
        &I2C1CONL, &I2C1STAT, &I2C1BRG, &I2C1TRN, &I2C1RCV};
    i2c_async_init(&i2c1_async, &i2c1_regs, 0x4E);
    __delay_ms(1000);

#ifndef USE_CHARS
    sendName((uint8_t *)"ScentGATT"); // Change Bluetooth module's name
    __delay_ms(2000);
#endif

    // CHANGED: ADC disabled -> commenting out setup and reads
    setup_adc();
    battery_mV = calc_adc();

    startUp(bor, pwr); // Play start up LED sequence.
    // Show current global intensity immediately at boot
    update_intensity_leds();

    Setup_Timer1(); // Set up timer for pod manager async polling
    Setup_Timer2(); // One-shot debounce for SW2/SW3
    Setup_Timer4(); // Set up timer for power button timing
    __builtin_enable_interrupts();
    while (1) // Start infinite loop, keeping code alive
    {
        pod_fire_handler();
        check_poll();
    }
    return 0;
}

void toggleLed(char dir)
{
    // Depending on dir, increment/decrement LED state
    if ('I' == dir) // Increment?
    {
        led++;                   // Yes - increment LED state
        if (led > POD_BAY_COUNT) // LED state exceeds max?
            led = 0;             // Yes - Reset state to 0
    }
    else
    {
        led--;                   // No - decrement LED state
        if (led < 1)             // LED state lower than 0?
            led = POD_BAY_COUNT; // Yes - Set state to 3
    }
}

void change_global_intensity(char dir)
{
    // Depending on dir, increment/decrement LED state
    // if (mode != LOCATE_IDLE)
    //  return;

    if ('I' == dir) // Increment?
    {
        if ((intensity_multi + 1) > 12)
            return;
        intensity_multi += 1;
    }
    else
    {
        if ((intensity_multi - 1) < 5)
            return;
        intensity_multi -= 1;
    }

    // White LED feedback: represent ~0.5..1.2 in 3 steps
    LED_W_L0 = 0;
    LED_W_L1 = 0;
    LED_W_L2 = 0;
    if (intensity_multi >= 5)
        LED_W_L0 = 1;
    if (intensity_multi >= 8)
        LED_W_L1 = 1;
    if (intensity_multi >= 11)
        LED_W_L2 = 1;
}

// Schedule debounce via Timer2 (shared for SW2/SW3)
static inline void start_button_debounce(debounce_source_t source)
{
    if (debounce_source != DEBOUNCE_NONE)
    {
        return; // Debounce already in progress
    }

    debounce_source = source;

    // Prevent re-entry while debouncing
    switch (source)
    {
    case DEBOUNCE_SW2_DECR:
        IEC1bits.INT1IE = 0;
        IFS1bits.INT1IF = 0;
        break;
    case DEBOUNCE_SW3_INCR:
        IEC1bits.INT2IE = 0;
        IFS1bits.INT2IF = 0;
        break;
    default:
        break;
    }

    TMR2 = 0;
    IFS0bits.T2IF = 0;
    IEC0bits.T2IE = 1;
    T2CONbits.TON = 1; // One-shot; PR2 set in Setup_Timer2
}

void __attribute__((interrupt, no_auto_psv)) _IOCInterrupt(void)
{
    if (IOCFDbits.IOCFD11)
    {
        IOCFDbits.IOCFD11 = 0; // clear this pin's change flag

        start_button_debounce(DEBOUNCE_SW2_DECR);
    }
    if (IOCFDbits.IOCFD10)
    {
        IOCFDbits.IOCFD10 = 0; // clear this pin's change flag

        start_button_debounce(DEBOUNCE_SW3_INCR);
    }
    if (IOCFDbits.IOCFD8)
    {

        IOCFDbits.IOCFD8 = 0;
        IFS3bits.INT3IF = 0; // Clear interrupt flag
        IEC3bits.INT3IE = 0; // Disable interrupt while here

        // Non-blocking handling via Timer4
        int3_state = 1; // Schedule 500ms check
        T4CONbits.TON = 0;
        TMR4 = 0;
        PR4 = 31250; // 500ms @ 1:256 prescaler
        IFS1bits.T4IF = 0;
        IEC1bits.T4IE = 1;
        T4CONbits.TON = 1;
    }

    IFS1bits.IOCIF = 0; // clear global IOC flag
}

void _ISR _T1Interrupt(void)
{
    IFS0bits.T1IF = 0;

    pod_manager_async_poll(&podman);

    battery_poll_counter++; // processes battery ADC every 50 counts > 5s
}

void _ISR _INT1Interrupt(void) // Decrement button interrupt (swapped)
{
    IFS1bits.INT1IF = 0; // Clear interrupt flag
    start_button_debounce(DEBOUNCE_SW2_DECR);
}

void _ISR _INT2Interrupt(void) // Increment button interrupt (swapped)
{
    IFS1bits.INT2IF = 0; // Clear interrupt flag
    start_button_debounce(DEBOUNCE_SW3_INCR);
}

void _ISR _T2Interrupt(void) // Debounce timer for SW2/SW3
{
    IFS0bits.T2IF = 0;
    T2CONbits.TON = 0; // One-shot complete

    debounce_source_t source = debounce_source;
    debounce_source = DEBOUNCE_NONE;

    switch (source)
    {
    case DEBOUNCE_SW2_DECR:
        if (PORTDbits.RD11)
        {
            change_global_intensity('D');
            down_pressed++;
        }
        IEC1bits.INT1IE = 1; // Re-enable SW2 interrupt
        break;
    case DEBOUNCE_SW3_INCR:
        if (PORTDbits.RD10)
        {
            change_global_intensity('I');
            up_pressed++;
        }
        IEC1bits.INT2IE = 1; // Re-enable SW3 interrupt
        break;
    default:
        break;
    }
}

void _ISR _INT3Interrupt(void) // Decrement button interrupt
{
    IFS3bits.INT3IF = 0; // Clear interrupt flag
    IEC3bits.INT3IE = 0; // Disable interrupt while here

    // Non-blocking handling via Timer4
    int3_state = 1; // Schedule 500ms check
    T4CONbits.TON = 0;
    TMR4 = 0;
    PR4 = 31250; // 500ms @ 1:256 prescaler
    IFS1bits.T4IF = 0;
    IEC1bits.T4IE = 1;
    T4CONbits.TON = 1;
}

void _ISR _T4Interrupt(void)
{
    IFS1bits.T4IF = 0;
    T4CONbits.TON = 0; // Stop timer (one-shot)

    switch (int3_state)
    {
    default:
    case 0:
        // Nothing scheduled
        break;
    case 1:
        // 500ms after INT3
        // CHANGED: Power button read old PORTBbits.RB15 -> new PORTDbits.RD8
        if (!PORTDbits.RD8)
        {
            button_state = !button_state;
            sendButtonState(button_state);
            int3_state = 0;
            IFS3bits.INT3IF = 0;
            IEC3bits.INT3IE = 1; // Re-enable INT3
        }
        else
        {
            // Schedule 1000ms follow-up
            int3_state = 2;
            TMR4 = 0;
            PR4 = 62500; // 1000ms @ 1:256 prescaler
            IFS1bits.T4IF = 0;
            T4CONbits.TON = 1;
        }
        break;
    case 2:
        // 1000ms after the previous check
        // CHANGED: Power button read old PORTBbits.RB15 -> new PORTDbits.RD8
        if (PORTDbits.RD8)
        {
            indicate_poweroff();
            // PWM_PIEZO_R(0);
            // PWM_PIEZO_L(0);
            PWR_LATCH = 0;
        }
        int3_state = 0;
        IFS3bits.INT3IF = 0;
        IEC3bits.INT3IE = 1; // Re-enable INT3
        break;
    }
}
