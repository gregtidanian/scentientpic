/**
 * @file eeprom_async.h
 * @author Walt
 * @brief eeprom async driver
 * @version 0.1
 * @date 2025-10-22
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef __EEPROM_ASYNC_H__
#define __EEPROM_ASYNC_H__

#include <stdint.h>
#include <stdbool.h>
#include <xc.h>
#include <libpic30.h>
#include "i2c_async.h"

// Little-endian pack/unpack helpers for EEPROM data
static inline void u16_to_le(uint8_t* p, uint16_t v) { p[0] = (uint8_t)(v & 0xFF); p[1] = (uint8_t)(v >> 8); }
static inline void u32_to_le(uint8_t* p, uint32_t v) { p[0] = (uint8_t)(v & 0xFF); p[1] = (uint8_t)((v >> 8) & 0xFF); p[2] = (uint8_t)((v >> 16) & 0xFF); p[3] = (uint8_t)((v >> 24) & 0xFF); }
static inline uint16_t le_to_u16(const uint8_t* p) { return (uint16_t)p[0] | ((uint16_t)p[1] << 8); }
static inline uint32_t le_to_u32(const uint8_t* p) { return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24); }

// Convenience wrappers with explicit endianness in the name
static inline uint16_t u16_from_buf_le(const uint8_t *p) { return le_to_u16(p); }
static inline uint32_t u32_from_buf_le(const uint8_t *p) { return le_to_u32(p); }

typedef enum
{
    EEPROMA_OK = 0,
    EEPROMA_ERR_NACK,
    EEPROMA_ERR_TIMEOUT
} eeproma_result_t;

typedef void (*eeproma_callback_t)(void *ctx, eeproma_result_t res);

typedef struct
{
    bool init;
    uint8_t address;
    i2c_async_t *i2c;
} eeproma_t;

void eeproma_init(eeproma_t *e, i2c_async_t *bus, uint8_t addr);
bool eeproma_read_block_async(eeproma_t *e, uint8_t start_addr, uint8_t *buf, uint8_t len,
                              eeproma_callback_t cb, void *ctx);

#endif
