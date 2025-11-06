#include "i2c.h"
#include <xc.h>

// Timeout helper
#define I2C_TIMEOUT 50000UL

static inline bool wait_clear(volatile uint16_t *reg, uint16_t mask)
{
    uint32_t t = I2C_TIMEOUT;
    while ((*reg & mask) && --t)
        ;
    return t != 0;
}

static inline bool wait_set(volatile uint16_t *reg, uint16_t mask)
{
    uint32_t t = I2C_TIMEOUT;
    while (((*reg & mask) == 0) && --t)
        ;
    return t != 0;
}

// Static table of register maps
static const i2c_regs_t I2C_REGMAPS[] = {
    {&I2C1CONL, &I2C1STAT, &I2C1BRG, &I2C1TRN, &I2C1RCV},
#ifdef _I2C2
    {&I2C2CONL, &I2C2STAT, &I2C2BRG, &I2C2TRN, &I2C2RCV},
#endif
};

// ------------------------------------------------------------
// Initialization / Deinit
// ------------------------------------------------------------

void i2c_init(i2c_t *bus, i2c_idx_t idx, uint32_t fcy, uint32_t fscl)
{
    const i2c_regs_t *r = &I2C_REGMAPS[idx];
    bus->index = idx;
    bus->regs = r;
    bus->initialized = true;

    // Configure TRIS (SCL1=RG2, SDA1=RG3)
    if (idx == I2C_IDX1)
    {
        TRISGbits.TRISG2 = 1;
        TRISGbits.TRISG3 = 1;
    }

    *r->CONL = 0;
    *r->BRG = (uint16_t)(((fcy / fscl) / 2) - 2);
    *r->CONL |= (1u << 15); // I2CEN

    __delay_ms(10); // Hitting ack wasn't returning -- REVISIT!
}

void i2c_deinit(i2c_t *bus)
{
    if (!bus || !bus->initialized)
    {
        return;
    }
    *bus->regs->CONL &= ~(1u << 15);
    bus->initialized = false;
}

// ------------------------------------------------------------
// Core primitives
// ------------------------------------------------------------

i2c_result_t i2c_start(const i2c_t *bus)
{
    const i2c_regs_t *r = bus->regs;
    *r->CONL |= (1u << 0); // SEN
    if (!wait_clear(r->CONL, (1u << 0)))
    {
        return I2C_ERR_TIMEOUT;
    }
    return I2C_OK;
}

i2c_result_t i2c_restart(const i2c_t *bus)
{
    const i2c_regs_t *r = bus->regs;
    *r->CONL |= (1u << 1); // RSEN
    if (!wait_clear(r->CONL, (1u << 1)))
    {
        return I2C_ERR_TIMEOUT;
    }
    return I2C_OK;
}

i2c_result_t i2c_stop(const i2c_t *bus)
{
    const i2c_regs_t *r = bus->regs;
    *r->CONL |= (1u << 2); // PEN
    if (!wait_clear(r->CONL, (1u << 2)))
    {
        return I2C_ERR_TIMEOUT;
    }
    return I2C_OK;
}

i2c_result_t i2c_write_byte(const i2c_t *bus, uint8_t data)
{
    const i2c_regs_t *r = bus->regs;
    *r->TRN = data;

    if (!wait_clear(r->STAT, (1u << 0)))
    {
        return I2C_ERR_TIMEOUT; // wait TBF=0
    }
    if (!wait_clear(r->STAT, (1u << 14)))
    {
        return I2C_ERR_TIMEOUT; // wait TRSTAT=0
    }
    if (*r->STAT & (1u << 15))
    {
        return I2C_ERR_NACK; // ACKSTAT=1 ? NACK
    }

    return I2C_OK;
}

i2c_result_t i2c_read_byte(const i2c_t *bus, uint8_t *data, bool ack)
{
    const i2c_regs_t *r = bus->regs;

    *r->CONL |= (1u << 3); // RCEN
    if (!wait_set(r->STAT, (1u << 1)))
    {
        return I2C_ERR_TIMEOUT; // RBF
    }
    *data = *r->RCV;

    // ACK/NACK
    if (ack)
    {
        *r->CONL &= ~(1u << 5); // ACKDT=0 ? ACK
    }
    else
    {
        *r->CONL |= (1u << 5); // ACKDT=1 ? NACK
    }
    *r->CONL |= (1u << 4); // ACKEN
    if (!wait_clear(r->CONL, (1u << 4)))
    {
        return I2C_ERR_TIMEOUT;
    }

    return I2C_OK;
}
