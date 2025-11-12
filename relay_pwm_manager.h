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

void relay_pwm_init(void);
void relay_pwm_fire(uint8_t pod_index, uint16_t duration_ms, uint8_t intensity);
void relay_pwm_stop(void);

#endif
