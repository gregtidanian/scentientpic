/**
 * @file pod.h
 * @author Walt
 * @brief pod driver
 * @version 0.1
 * @date 2025-10-22
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef __POD_H__
#define __POD_H__

#include <stdint.h>
#include <stdbool.h>
#include <xc.h>
#include <libpic30.h>
#include "eeprom.h"

/**
 * Example EEPROM layout per pod:
 *  0x00?0x0F : 128-bit Unique ID (16 bytes)
 *  0x10?0x11 : Scent ID (uint16)
 *  0x12?0x13 : Remaining volume (uint16, 0xffff = full)
 */
typedef struct
{
    bool init;
    uint8_t bay;        // bay number 0?5
    uint8_t uid[16];    // unique identifier
    uint16_t scent;     // scent ID
    uint16_t remaining; // remaining volume
    eeprom_t eeprom;    // EEPROM device (address + bus)
} pod_t;

void pod_init(pod_t *p, i2c_t *bus, uint8_t bay, uint8_t address);
void pod_deinit(pod_t *p);
bool pod_detect(pod_t *p);
bool pod_load_metadata(pod_t *p);

#endif /* __POD_H__ */
