#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "main.h"
#include "owire.h"
#include "piezos.h"
#include "PWM.h"

PiezoFIFOItem piezo_fifo_right[10], piezo_fifo_left[10];
int fifo_items_right = 0, fifo_items_left = 0;
 uint8_t piezo_interrupt = 0; // changed from int8_t

#define LOCAL_PIEZO
void activate_piezo(uint8_t piezo, uint8_t activate)
{
    switch (piezo)
    {
        case 0:
            PIEZO_R1 = activate;
            break;
        case 1:
            PIEZO_R2 = activate;
            break;
        case 2:
            PIEZO_R3 = activate;
            break;
        case 3:
            PIEZO_L1 = activate;
            break;
        case 4:
            PIEZO_L2 = activate;
            break;
        case 5:
            PIEZO_L3 = activate;
            break;
    }
}

int fire_piezo_R1(uint8_t intensity, uint16_t duration)
{
    CLEAR_LEDS;
    PodData pod = find_by_channel_num(0);
    if (!pod.pod_id)
    {
        RED = 1;
        return 1;
    }
    
    uint8_t to_use = (duration / 1000);
    if (!(pod.scent_level - to_use) || (pod.scent_level - to_use) > 100)
    {
        //BLUE = 1;
        return 2;
    }
    
    //GREEN = 1;
#ifdef LOCAL_PIEZO
    // CHANGED: PIEZO_V_EN not used -> commenting out
    // PIEZO_V_EN = 1;     // Toggle piezo volts enable pin high.
    PWM_PIEZO_R(50);    // PWM drive the oscillator for the right piezos 
    adjust_intensity(intensity);
    PIEZO_R1 = 1;       // Switch on relay for R1
    __delay_ms(duration);
    CLEAR_LEDS;
    PIEZO_R1 = 0;       // Switch off relay for R1
    PWM_PIEZO_R(0);     // PWM shut the oscillator for the right piezos off 
    // CHANGED: PIEZO_V_EN not used -> commenting out
    // PIEZO_V_EN = 0;     // Toggle piezo volts enable pin low.
#else
    messageStore.pod_active[0] = 1;
    messageStore.current_intensity[0] = intensity;
    messageStore.current_duration[0] = duration;
    __delay_ms(250);
    CLEAR_LEDS;
#endif 
    
    uint8_t level = (pod.scent_level - (duration / 1000));
    set_scent_level(pod, level);
    return 0;
}

int fire_piezo_R2(uint8_t intensity, uint16_t duration)
{
    CLEAR_LEDS;
    PodData pod = find_by_channel_num(1);
    if (!pod.pod_id)
    {
        //RED = 1;
        return 1;
    }
    
    uint8_t to_use = (duration / 1000);
    if (!(pod.scent_level - to_use) || (pod.scent_level - to_use) > 100)
    {
        //BLUE = 1;
        return 2;
    }
    
    //GREEN = 1;
#ifdef LOCAL_PIEZO
    // CHANGED: PIEZO_V_EN not used -> commenting out
    // PIEZO_V_EN = 1;     // Toggle piezo volts enable pin high.
    PWM_PIEZO_R(50);    // PWM drive the oscillator for the right piezos 
    adjust_intensity(intensity);
    PIEZO_R2 = 1;       // Switch on relay for R2
    __delay_ms(duration);
    PIEZO_R2 = 0;       // Switch off relay for R2
    PWM_PIEZO_R(0);     // PWM shut the oscillator for the right piezos off 
    // CHANGED: PIEZO_V_EN not used -> commenting out
    // PIEZO_V_EN = 0;     // Toggle piezo volts enable pin low.
#else
    messageStore.pod_active[1] = 1;
    messageStore.current_intensity[1] = intensity;
    messageStore.current_duration[1] = duration;
    __delay_ms(250);
    CLEAR_LEDS;
#endif 
    
    uint8_t level = (pod.scent_level - (duration / 1000));
    set_scent_level(pod, level);
    return 0;
}

int fire_piezo_R3(uint8_t intensity, uint16_t duration)
{
    CLEAR_LEDS;
    PodData pod = find_by_channel_num(2);
    if (!pod.pod_id)
    {
        //RED = 1;
        return 1;
    }
    
    uint8_t to_use = (duration / 1000);
    if (!(pod.scent_level - to_use) || (pod.scent_level - to_use) > 100)
    {
        //BLUE = 1;
        return 2;
    }
    
    //GREEN = 1;
#ifdef LOCAL_PIEZO
    // CHANGED: PIEZO_V_EN not used -> commenting out
    // PIEZO_V_EN = 1;     // Toggle piezo volts enable pin high.
    PWM_PIEZO_R(50);    // PWM drive the oscillator for the right piezos 
    adjust_intensity(intensity);
    PIEZO_R3 = 1;       // Switch on relay for R3
    __delay_ms(duration);
    PIEZO_R3 = 0;       // Switch off relay for R3
    PWM_PIEZO_R(0);     // PWM shut the oscillator for the right piezos off 
    // CHANGED: PIEZO_V_EN not used -> commenting out
    // PIEZO_V_EN = 0;     // Toggle piezo volts enable pin low.
#else
    messageStore.pod_active[2] = 1;
    messageStore.current_intensity[2] = intensity;
    messageStore.current_duration[2] = duration;
    __delay_ms(250);
    CLEAR_LEDS;
#endif 
    
    uint8_t level = (pod.scent_level - to_use);
    set_scent_level(pod, level);
    return 0;
}

int fire_piezo_L1(uint8_t intensity, uint16_t duration)
{
    CLEAR_LEDS;
    PodData pod = find_by_channel_num(3);
    if (!pod.pod_id)
    {
        //RED = 1;
        return 1;
    }
    
    uint8_t to_use = (duration / 1000);
    if (!(pod.scent_level - to_use) || (pod.scent_level - to_use) > 100)
    {
        //BLUE = 1;
        return 2;
    }
    
    //GREEN = 1;
#ifdef LOCAL_PIEZO
    // CHANGED: PIEZO_V_EN not used -> commenting out
    // PIEZO_V_EN = 1;     // Toggle piezo volts enable pin high.
    PWM_PIEZO_L(50);    // PWM drive the oscillator for the left piezos 
    adjust_intensity(intensity);
    PIEZO_L1 = 1;       // Switch on relay for L1
    __delay_ms(duration);
    PIEZO_L1 = 0;       // Switch off relay for L1
    PWM_PIEZO_L(0);     // PWM shut the oscillator for the left piezos off
    // CHANGED: PIEZO_V_EN not used -> commenting out
    // PIEZO_V_EN = 0;     // Toggle piezo volts enable pin low.
#else
    messageStore.pod_active[3] = 1;
    messageStore.current_intensity[3] = intensity;
    messageStore.current_duration[3] = duration;
    __delay_ms(250);
    CLEAR_LEDS;
#endif 
    
    uint8_t level = (pod.scent_level - (duration / 1000));
    set_scent_level(pod, level);
    return 0;
}

int fire_piezo_L2(uint8_t intensity, uint16_t duration)
{
    CLEAR_LEDS;
    PodData pod = find_by_channel_num(4);
    if (!pod.pod_id)
    {
        //RED = 1;
        return 1;
    }
    
    uint8_t to_use = (duration / 1000);
    if (!(pod.scent_level - to_use) || (pod.scent_level - to_use) > 100)
    {
        //BLUE = 1;
        return 2;
    }
    
    //GREEN = 1;
#ifdef LOCAL_PIEZO
    // CHANGED: PIEZO_V_EN not used -> commenting out
    // PIEZO_V_EN = 1;     // Toggle piezo volts enable pin high.
    PWM_PIEZO_L(50);    // PWM drive the oscillator for the left piezos 
    adjust_intensity(intensity);
    PIEZO_L2 = 1;       // Switch on relay for L2
    __delay_ms(duration);
    PIEZO_L2 = 0;       // Switch off relay for L2
    PWM_PIEZO_L(0);     // PWM shut the oscillator for the left piezos off
    // CHANGED: PIEZO_V_EN not used -> commenting out
    // PIEZO_V_EN = 0;     // Toggle piezo volts enable pin low.
#else
    messageStore.pod_active[4] = 1;
    messageStore.current_intensity[4] = intensity;
    messageStore.current_duration[4] = duration;
    __delay_ms(250);
    CLEAR_LEDS;
#endif 
    
    uint8_t level = (pod.scent_level - (duration / 1000));
    set_scent_level(pod, level);
    return 0;
}

int fire_piezo_L3(uint8_t intensity, uint16_t duration)
{
    CLEAR_LEDS;
    PodData pod = find_by_channel_num(5);
    if (!pod.pod_id)
    {
        //RED = 1;
        return 1;
    }
    
    uint8_t to_use = (duration / 1000);
    if (!(pod.scent_level - to_use) || (pod.scent_level - to_use) > 100)
    {
        //BLUE = 1;
        return 2;
    }
    
    //GREEN = 1;
#ifdef LOCAL_PIEZO
    // CHANGED: PIEZO_V_EN not used -> commenting out
    // PIEZO_V_EN = 1;     // Toggle piezo volts enable pin high.
    PWM_PIEZO_L(50);    // PWM drive the oscillator for the left piezos 
    adjust_intensity(intensity);
    PIEZO_L3 = 1;       // Switch on relay for L3
    __delay_ms(duration);
    PIEZO_L3 = 0;       // Switch off relay for L3
    PWM_PIEZO_L(0);     // PWM shut the oscillator for the left piezos off
    // CHANGED: PIEZO_V_EN not used -> commenting out
    // PIEZO_V_EN = 0;     // Toggle piezo volts enable pin low.
#else
    messageStore.pod_active[5] = 1;
    messageStore.current_intensity[5] = intensity;
    messageStore.current_duration[5] = duration;
    __delay_ms(250);
    CLEAR_LEDS;
#endif 
    
    uint8_t level = (pod.scent_level - to_use);
    set_scent_level(pod, level);
    return 0;
}

void wait_4_piezo(uint16_t duration)
{
    uint16_t cycles = duration / 10;
    for (uint16_t x = 0; x < cycles; x++)
    {
        if (piezo_interrupt)
            return;
        __delay_ms(10);
    }
}

int fire_piezo(uint8_t chan, bool is_right, uint8_t intensity, uint16_t duration)
{
    CLEAR_LEDS;
    PodData pod = find_by_channel_num(chan-1);
    if (!pod.pod_id)
    {
        //RED = 1;
        return 1;
    }
    
    uint8_t to_use = (duration / 1000);
    if (!(pod.scent_level - to_use) || (pod.scent_level - to_use) > 100)
    {
        //BLUE = 1;
        return 2;
    }
    
    //GREEN = 1;
    //PWM_FREQ(intensity);
    piezo_interrupt = 0;
    uint16_t pwm = calc_pwm_multi(intensity);
    if (is_right)
        PWM_PIEZO_R(pwm);    // PWM drive the oscillator for the left piezos 
    else
        PWM_PIEZO_L(pwm);
    __delay_ms(100);
    // CHANGED: PIEZO_V_EN not used -> commenting out
    // PIEZO_V_EN = 1;     // Toggle piezo volts enable pin high.
    //adjust_intensity(!global_intensity ? intensity : global_intensity);
    activate_piezo(chan-1, 1);
    //__delay_ms(duration);
    wait_4_piezo(duration);
    activate_piezo(chan-1, 0);
    if (is_right)
        PWM_PIEZO_R(0);     // PWM shut the oscillator for the left piezos off
    else
        PWM_PIEZO_L(0);
    // CHANGED: PIEZO_V_EN not used -> commenting out
    // PIEZO_V_EN = 0;     // Toggle piezo volts enable pin low.
    
    uint8_t level = (pod.scent_level - to_use);
    set_scent_level(pod, level);
    return 0;
}

int fire_piezo_two(uint8_t rchan, uint8_t lchan, uint8_t rintensity, uint16_t rduration, uint8_t lintensity, uint16_t lduration)
{
    uint16_t duration1 = 0, duration2 = 0;
    if (rduration > lduration)
    {
        duration1 = rduration;
        duration2 = rduration - lduration;
    }
    else
    {
        duration1 = lduration;
        duration2 = lduration - rduration;
    }
    
    CLEAR_LEDS;
    PodData rpod = find_by_channel_num(rchan-1);
    PodData lpod = find_by_channel_num(lchan-1);
    if (!rpod.pod_id || !lpod.pod_id)
    {
        //RED = 1;
        return 1;
    }
    
    uint8_t rto_use = (rduration / 1000);
    uint8_t lto_use = (lduration / 1000);
    if ((!(rpod.scent_level - rto_use) || (rpod.scent_level - rto_use) > 100) &&
            (!(lpod.scent_level - lto_use) || (lpod.scent_level - lto_use) > 100))
    {
        //BLUE = 1;
        return 2;
    }
    
    // GREEN = 1;
    // CHANGED: PIEZO_V_EN/adjust_voltage not used -> commenting out enable; keep PWM and relay control
    // PIEZO_V_EN = 1;     // Toggle piezo volts enable pin high.
    PWM_PIEZO_R(50);
    PWM_PIEZO_L(50);
    adjust_intensity(rintensity);
    activate_piezo(rchan-1, 1);
    activate_piezo(lchan-1, 1);
    __delay_ms(duration1);
    if (rduration > lduration)
        activate_piezo(lchan-1, 0);
    else
        activate_piezo(rchan-1, 0);
    __delay_ms(duration2);
    if (rduration > lduration)
        activate_piezo(rchan-1, 0);
    else
        activate_piezo(lchan-1, 0);
    PWM_PIEZO_R(0);
    PWM_PIEZO_L(0);
    // CHANGED: PIEZO_V_EN not used -> commenting out
    // PIEZO_V_EN = 0;     // Toggle piezo volts enable pin low.
    
    uint8_t rlevel = (rpod.scent_level - rto_use);
    uint8_t llevel = (lpod.scent_level - lto_use);
    set_scent_level(rpod, rlevel);
    __delay_ms(50);
    set_scent_level(lpod, llevel);
    return 0;
}

void piezo_handler(void)
{
#ifndef LOCAL_PIEZO
    for (int i = 0; i < 6; i++)
    {
        if (!messageStore.current_duration[i] && messageStore.pod_active[i])
        {
            messageStore.pod_active[i] = 0;
            activate_piezo(i, 0);
        }
    }
    
    uint8_t right_pod_active = 0, left_pod_active = 0, l_fired = 0, r_fired = 0;
    for (int i = 0; i < 3; i++)
        if (messageStore.pod_active[i])
            right_pod_active = 1;
    for (int i = 3; i < 6; i++)
        if (messageStore.pod_active[i])
            left_pod_active = 1;
    
    // CHANGED: PIEZO_V_EN not used -> commenting out
    // PIEZO_V_EN = right_pod_active || left_pod_active;     // Toggle piezo volts enable pin high.
    PWM_PIEZO_R(right_pod_active ? 50 : 0);
    PWM_PIEZO_L(left_pod_active ? 50 : 0);
    
    for (int i = 0; i < 3; i++)
    {
        if (messageStore.current_duration[i] && messageStore.pod_active[i] && !r_fired)
        {
            CLEAR_LEDS;
            IDLE_PURPLE;
            adjust_intensity(messageStore.current_intensity[i]);
            __delay_ms(100);
            activate_piezo(i, messageStore.pod_active[i]);
            if (messageStore.current_duration[i] > 1000)
                messageStore.current_duration[i] -= 1000;
            else
                messageStore.current_duration[i] = 0;
            r_fired = 1;
        }
    }
    for (int i = 3; i < 6; i++)
    {
        if (messageStore.current_duration[i] && messageStore.pod_active[i] && !l_fired)
        {
            CLEAR_LEDS;
            IDLE_PURPLE;
            adjust_intensity(messageStore.current_intensity[i]);
            __delay_ms(100);
            activate_piezo(i, messageStore.pod_active[i]);
            if (messageStore.current_duration[i] > 1000)
                messageStore.current_duration[i] -= 1000;
            else
                messageStore.current_duration[i] = 0;
            l_fired = 1;
        }
    }
    
    for (int i = 0; i < 6; i++)
    {
        if (messageStore.pod_active[i])
        {
            Nop();
            Nop();
            Nop();
            Nop();
        }
    }
#else
    PiezoFIFOItem right, left;
    /*if (piezo_fifo_right_items() && piezo_fifo_left_items())
    {
        piezo_fifo_right_get(&right);
        piezo_fifo_left_get(&left);
        fire_piezo_two(right.channel, left.channel, right.intensity, right.duration, left.intensity, left.duration);
    }
    else*/ if (piezo_fifo_right_items() || piezo_fifo_left_items())
    {
        if (piezo_fifo_right_items())
        {
            piezo_fifo_right_get(&right);
            fire_piezo(right.channel, true, right.intensity, right.duration);
        }
        else if (piezo_fifo_left_items())
        {
            piezo_fifo_left_get(&left);
            fire_piezo(left.channel, false, left.intensity, left.duration);
        }
    }
    else
        __delay_ms(1000);
#endif
}

uint8_t adjust_intensity(uint8_t percentage)
{
    uint8_t steps = 0;
    
    if (percentage > 100)
        return 0;
    
    steps = (64 * percentage / 100);
    if (steps > 58)
        steps = 58;
    //steps = 64 - steps;
    //steps = (32 * percentage / 100);
    /*for (int x = 0; x < 64; x++)
    {
        adjust_voltage(0, 1);
        __delay_ms(30);
    } */               
    // CHANGED: adjust_voltage not used -> commenting out
    // adjust_voltage(0, 64); // Adjust voltage output for Piezos to 5V
    // __delay_ms(10);
    // adjust_voltage(1, steps);
    return steps;
}

// CHANGED: adjust_voltage and POT pins not used -> commenting out function body
// void adjust_voltage(uint8_t state, uint8_t steps)
// {
//     unsigned char x = 0;
//     
//     POT_DATA = state;       // Drive POT_DATA as state (0 - increase, 1 - decrease)
//     __delay_us(20);
//     POT_CS = 0;             // Ensure POT_CS pin is low
//     __delay_us(20);
//     for (x = 0; x < steps; x++)    
//     {
//         POT_DATA = 1;
//         __delay_us(20);
//         POT_DATA = 0;
//         __delay_us(20);
//     }
//     POT_DATA = state;       // Drive POT_DATA as state (0 - increase, 1 - decrease)
//     __delay_us(20);
//     POT_CS = 1;             // Idle POT_CS high
//     __delay_us(20);    
// }

void piezo_fifo_init(void)
{
    memset(piezo_fifo_right, 0, (sizeof(PiezoFIFOItem) * 10));
    memset(piezo_fifo_left, 0, (sizeof(PiezoFIFOItem) * 10));
}

void piezo_fifo_right_get(PiezoFIFOItem *item)
{
    int i;
    if (!fifo_items_right)
        return;
    
    memcpy(item, &piezo_fifo_right[0], sizeof(PiezoFIFOItem));
    for (i = 1; i < fifo_items_right; i++)
        memcpy(&piezo_fifo_right[i-1], &piezo_fifo_right[i], sizeof(PiezoFIFOItem));
    memset(&piezo_fifo_right[i], 0, sizeof(PiezoFIFOItem));
    fifo_items_right--;
}

void piezo_fifo_right_put(PiezoFIFOItem *item)
{
    if (fifo_items_right >= 10)
        return;
    
    memcpy(&piezo_fifo_right[fifo_items_right++], item, sizeof(PiezoFIFOItem));
}

int piezo_fifo_right_items(void)
{
    return fifo_items_right;
}

void piezo_fifo_left_get(PiezoFIFOItem *item)
{
    int i;
    if (!fifo_items_left)
        return;
    
    memcpy(item, &piezo_fifo_left[0], sizeof(PiezoFIFOItem));
    for (i = 1; i < fifo_items_left; i++)
        memcpy(&piezo_fifo_left[i-1], &piezo_fifo_left[i], sizeof(PiezoFIFOItem));
    memset(&piezo_fifo_left[i], 0, sizeof(PiezoFIFOItem));
    fifo_items_left--;
}

void piezo_fifo_left_put(PiezoFIFOItem *item)
{
    if (fifo_items_left >= 10)
        return;
    
    memcpy(&piezo_fifo_left[fifo_items_left++], item, sizeof(PiezoFIFOItem));
}

int piezo_fifo_left_items(void)
{
    return fifo_items_left;
}