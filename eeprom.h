/**
 * @file eeprom.h
 * @author Walt
 * @brief eeprom driver
 * @version 0.1
 * @date 2025-10-22
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef __EEPROM_H__
#define __EEPROM_H__

#define FCY 16000000UL
#include <stdint.h>
#include <stdbool.h>
#include <xc.h>
#include <libpic30.h>
#include "i2c.h"

typedef enum
{
    EEPROM_OK = 0,
    EEPROM_ERR_I2C,
    EEPROM_ERR_TIMEOUT,
    EEPROM_ERR_NACK
} eeprom_result_t;

typedef struct
{
    bool init;
    uint8_t address; // 7-bit I²C address (e.g., 0x50?0x57)
    i2c_t *bus;      // pointer to I²C bus object
} eeprom_t;

// --- Initialization / teardown ---
void eeprom_init(eeprom_t *e, i2c_t *bus, uint8_t address);
void eeprom_deinit(eeprom_t *e);

// --- Read / write operations ---
eeprom_result_t eeprom_write_byte(eeprom_t *e, uint8_t mem_addr, uint8_t data);
eeprom_result_t eeprom_read_byte(eeprom_t *e, uint8_t mem_addr, uint8_t *data);

// Optional: block read/write
eeprom_result_t eeprom_read_block(eeprom_t *e, uint8_t start_addr, uint8_t *buf, uint8_t len);
eeprom_result_t eeprom_write_block(eeprom_t *e, uint8_t start_addr, const uint8_t *buf, uint8_t len);

#endif /* __EEPROM_H__ */
