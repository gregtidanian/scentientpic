/**
 * @file relay_pwm_manager.h
 * @author Walt
 * @brief Manager for the relays and PWM output
 * @version 0.1
 * @date 2025-10-22
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef __RELAY_PWM_MANAGER_H__
#define __RELAY_PWM_MANAGER_H__

#include <stdint.h>
#include <stdbool.h>
#include <xc.h>
#include <libpic30.h>

typedef enum
{
    RELAY_PWM_EVT_FIRE,
    RELAY_PWM_EVT_BURST,
    RELAY_PWM_EVT_STOP,
} relay_pwm_evt_t;

typedef struct
{
    relay_pwm_evt_t type;
    uint8_t pod_index;
    uint32_t bursts; // completed on_time intervals during the last fire
} relay_pwm_evt_info_t;

typedef void (*relay_pwm_fire_callback_t)(const relay_pwm_evt_info_t *p_evt);

void relay_pwm_init(relay_pwm_fire_callback_t p_cb);
void relay_pwm_stop(void);
void relay_pwm_fire(uint8_t pod_index, uint16_t duration_ms, uint8_t pulse_duty, uint16_t pulse_period, uint8_t pwm_duty, uint8_t multiplier);
uint32_t relay_pwm_get_burst_count(void);
uint8_t relay_pwm_get_last_pod(void);

#endif
