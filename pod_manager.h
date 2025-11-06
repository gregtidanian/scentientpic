/**
 * @file pod_manager.h
 * @author Walt
 * @brief pod manager
 * @version 0.1
 * @date 2025-10-22
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef __POD_MANAGER_H__
#define __POD_MANAGER_H__

#include <stdint.h>
#include <stdbool.h>
#include <xc.h>
#include <libpic30.h>
#include "pod.h"
#include "i2c_async.h"
#include "eeprom.h"

#define POD_BAY_COUNT 6

typedef struct
{
    i2c_async_t *i2c;
    pod_t pods[POD_BAY_COUNT];
    uint32_t last_poll_ms;
} pod_manager_t;

void pod_manager_init(pod_manager_t *pm, i2c_async_t *bus);
void pod_manager_poll(pod_manager_t *pm); // called ~100 ms
void pod_manager_tick_100ms(void);        // Timer ISR entry point

#endif /* __POD_MANAGER_H__ */
