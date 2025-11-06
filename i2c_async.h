/**
 * @file i2c_async.h
 * @author Walt
 * @brief i2c async driver
 * @version 0.1
 * @date 2025-10-22
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef __I2C_ASYNC_H__
#define __I2C_ASYNC_H__

#include <stdint.h>
#include <stdbool.h>
#include <xc.h>
#include <libpic30.h>
#include "i2c.h"

#define I2C_MAX_QUEUE 16

typedef enum
{
    I2C_EVENT_COMPLETE,
    I2C_EVENT_NACK,
    I2C_EVENT_TIMEOUT
} i2c_event_t;

typedef void (*i2c_callback_t)(void *context, i2c_event_t event);

typedef struct
{
    uint8_t address;
    const uint8_t *tx_buf;
    uint8_t tx_len;
    uint8_t *rx_buf;
    uint8_t rx_len;
    i2c_callback_t cb;
    void *context;
} i2c_transaction_t;

typedef enum
{
    I2C_STATE_IDLE,
    I2C_STATE_START,
    I2C_STATE_ADDR,
    I2C_STATE_TX,
    I2C_STATE_RESTART,
    I2C_STATE_RX,
    I2C_STATE_STOP,
    I2C_STATE_DONE
} i2c_state_t;

typedef struct
{
    const i2c_regs_t *regs;
    i2c_transaction_t queue[I2C_MAX_QUEUE];
    uint8_t head, tail;
    bool busy;
    i2c_transaction_t current;
    i2c_state_t state;
    uint8_t tx_index, rx_index;
} i2c_async_t;

void i2c_async_init(i2c_async_t *bus, const i2c_regs_t *regs, uint16_t brg);
bool i2c_async_submit(i2c_async_t *bus, const i2c_transaction_t *t);

void __attribute__((interrupt, no_auto_psv)) _MI2C1Interrupt(void);

#endif
