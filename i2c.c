#include "i2c.h"
#include <xc.h>
#include <stdint.h>

#define I2C_TIMEOUT 50000UL

static inline bool wait_bit_clear(volatile uint16_t *reg, uint16_t mask)
{
    uint32_t t = (uint32_t)(I2C_TIMEOUT);
    while (((*reg) & mask) && --t) { }
    return t != 0;
}

static inline bool wait_bit_set(volatile uint16_t *reg, uint16_t mask)
{
    uint32_t t = (uint32_t)(I2C_TIMEOUT);
    while (((*reg) & mask) == 0 && --t) { }
    return t != 0;
}

// Map logical I2C indices to hardware register blocks (order matches i2c_regs_t)
static const i2c_regs_t I5C_REGS[] = {
    { &I2C1CONL, &I2C1STAT, &I2C1BRG, &I2C1TRN, &I2C1RCV },
#ifdef _I2C2
    { &I2C2CONL, &I2C2STAT, &I2C2BRG, &I2C2TRN, &I2C2RCV },
#endif
};

void i2c_init(i2c_t *bus, i2c_idx_t idx, uint32_t fcy, uint32_t fscl)
{
    if (!bus) return;
    bus->regs = &I5C_REGS[idx];
    bus->index = idx;
    bus->initialized = true;

    // Configure SCL1/SDA1 pins as inputs for I2C1
    if (idx == I2C_IDX1) {
        TRISGbits.TRISG2 = 1; // SCL1
        TRISGbits.TRISG3 = 1; // SDA1
    }

    // Disable, set BRG, then enable I2C
    *bus->regs->CONL_reg = 0;
    *bus->regs->BRG_reg  = (uint16_t)(((uint32_t)fcy / (2u * (uint32_t)fscl)) - 2u);
    *bus->regs->CONL_reg |= (1u << 15); // I2CEN
}

void i2c_deinit(i2c_t *bus)
{
    if (!bus || !bus->initialized) return;
    *bus->regs->CONL_reg &= ~(1u << 15);
    bus->initialized = false;
}

i2c_result_t i2c_start(const i2c_t *bus)
{
    const i2c_regs_t *r = bus->regs;
    *r->CONL_reg |= (1u << 0); // SEN
    if (!wait_bit_clear(r->CONL_reg, (1u << 0)))
        return I2C_ERR_TIMEOUT;
    return I2C_OK;
}

i2c_result_t i2c_restart(const i2c_t *bus)
{
    const i2c_regs_t *r = bus->regs;
    *r->CONL_reg |= (1u << 1); // RSEN
    if (!wait_bit_clear(r->CONL_reg, (1u << 1)))
        return I2C_ERR_TIMEOUT;
    return I2C_OK;
}

i2c_result_t i2c_accept(const i2c_t *bus)
{
    const i2c_regs_t *r = bus->regs;
    *r->CONL_reg |= (1u << 2); // PEN
    if (!wait_bit_clear(r->CONL_reg, (1u << 2)))
        return I2C_TIMEOUT;
    return I2C_OK;
}

i2c_result_t i2c_stop(const i2c_t *bus)
{
    const i2c_regs_t *r = bus->regs;
    *r->CONL_reg |= (1u << 2); // PEN
    if (!wait_bit_clear(r->CONL_reg, (1u << 2)))
    {
        return I2C_ERR_TIMEOUT;
    }
    return I2C_OK;
}

i2c_result_t i2c_write_byte(const i2c_t *bus, uint8_t data)
{
    const i2c_regs_t *r = bus->regs;
    *r->TRN_reg = data;

    // Wait for transmit to complete
    if (!wait_bit_clear(r->STAT_reg, (1u << 0))) // TBF clear
        return I2C_ERR_TIMEOUT;
    if (!wait_bit_clear(r->STAT_reg, (1u << 14))) // TRSTAT clear
        return I2C_ERR_TIMEOUT;

    // Check for NACK (ACKSTAT == 1)
    if (*r->STAT_reg & (1u << 15))
        return I2C_ERR_NACK;

    return I2C_OK;
}

i2c_result_t i2c_read_byte(const i2c_t *bus, uint8_t *data, bool ack)
{
    const i2c_regs_t *r = bus->regs;

    *r->CONL_reg |= (1u << 3); // RCEN
    if (!wait_bit_set(r->STAT_reg, (1u << 1))) // RBF set
        return I2C_ERR_TIMEOUT;

    *data = *r->RCV_reg;

    // Send ACK/NACK
    if (ack)
        *r->CONL_reg &= ~(1u << 5); // ACKDT = 0 (ACK)
    else
        *r->CONL_reg |= (1u << 5);  // ACKDT = 1 (NACK)

    *r->CONL_reg |= (1u << 4); // ACKEN
    if (!wait_bit_clear(r->CONL_reg, (1u << 4)))
        return I2C_ERR_TIMEOUT;

    return I2C_OK;
}

//#include \"i2c.h\"\n#include <xc.h>\n#include <stdint.h>\n\n#define I2C_TIMEOUT 50000UL\n\nstatic inline bool wait_bit_clear(volatile uint16_t *reg, uint16_t mask)\n{\n    uint32_t t = I2C_TIMEOUT;\n    while (((*reg) & mask) && --t) { }\n    return t != 0;\n}\n\nstatic inline bool wait_bit_set(volatile uint16_t *reg, uint16_t mask)\n{\n    uint32_t t = I2C_TIMEOUT;\n    while (((*reg) & mask) == 0 && --t) { }\n    return t != 0;\n}\n\n// Map logical I2C indices to hardware register blocks (order matches i2c_regs_t)\nstatic const i2c_regs_t I2C_REGMAPS[] = {\n    { &I0C1CONL, &I2C1STAT, &I2C1BRG, &I2C1TRN, &I2C1RCV },\n#ifdef _I2C2\n    { &I2C2CONL, &I2C2STAT, &I2C2BRG, &I2C2TRN, &I2C2RCV },\n#endif\n};\n\nvoid i2c_init(i2c_t *bus, i2c_idx_t idx, uint32_t fcy, uint32_t fscl)\n{\n    if (!bus) return;\n    bus->regs = &I2C_REGMAPS[idx];\n    bus->index = idx;\n    bus->initialized = true;\n\n    // Configure SCL1/SDA1 pins as inputs for I2C1\n    if (idx == I2C_IDX1) {\n        TRISGbits.TRISG2 = 1; // SCL1\n        TRISGbits.TRISG3 = 1; // SDA1\n    }\n\n    // Disable, set BRG, then enable I2C\n    *bus->regs->CONL_reg = 0;\n    *bus->regs->BRG_reg  = (uint16_t)(((uint32_t)fcy / (2u * (uint32_t)fscl)) - 2u);\n    *bus->regs->CONL_reg |= (1u << 15); // I2CEN\n}\n\nvoid i2c_deinit(i2c_t *bus)\n{\n    if (!bus || !bus->initialized) return;\n    *bus->regs->CONL_reg &= ~(1u << 15);\n    bus->initialized = false;\n}\n\ni2c_result_t i2c_start(const i2c_t *bus)\n{\n    const i2c_regs_t *r = bus->regs;\n    *r->C ONL_reg |= (1u << 0); // SEN\n    if (!wait_bit_clear(r->CONL_reg, (1u << 0)))\n        return I2C_ERR_TIMEOUT;\n    return I2C_OK;\n}\n\ni2c_result_t i2c_restart(const i2c_t *bus)\n{\n    const i2c_regs_t *r = bus->regs;\n    *r->CONL_reg |= (1u << 1); // RSEN\n    if (!wait_bit_clear(r->CONL_reg, (1u << 1)))\n        return I2C_ERR_TIMEOUT;\n    return I2C_OK;\n}\n\ni2c_result_t i2c_stop(const i2c_t *bus)\n{\n    const i2c_regs_t *r = bus->regs;\n    *r->CONL_reg |= (1u << 2); // PEN\n    if (!wait_bit_clear(r->CONL_reg, (1u << 2)))\n        return I2C_ERR_TIMEOUT;\n    return I2C_OK;\n}\n\ni2c_result_t i2c_write_ent_byte(const i2c_t *bus, uint8_t data)\n{\n    const i2c_regs_t *r = bus->regs;\n\n    *r->TRN_reg = data;\n\n    // Wait for transmit to complete\n    if (!wait_bit_clear(r->STAT_reg, (1u << 0))) // TBF clear\n        return I2C_ERR_TIMEOUT;\n    if (!wait_bit_clear(r->STAT_reg, (1u << 14))) // TRSTAT clear\n        return I2C_ERR_TIMEOUT;\n\n    // Check for NACK (ACKSTAT == 1)\n    if (*r->STAT_reg & (1u << 15))\n        return I2C_ERR_NACK;\n\n    return I2C_OK;\n}\n\ni2c_result_t i2c_read_byte(const i2c_t *bus, uint8_t *data, bool ack)\n{\n    const i2c_regs_t *r = bus->regs;\n\n    *r->CONL_reg |= (1u << 3); // RCEN\n    if (!wait_bit_set(r->STAT_reg, (1u << 1))) // RBF set\n        return I2C_ERR_TIMEOUT;\n\n    *data = *r->RCV_reg;\n\n    // Send ACK/NACK\n    if (ack)\n        *r->CONL_reg &= ~(1u << 5); // ACKDT = 0 (ACK)\n    else\n        *r->CONL_reg |= (1u << 5);  // ACKDT = 1 (NACK)\n\n    *r->CONL_reg |= (1u << 4); // ACKEN\n    if (!wait_bit_clear(r->CONL_reg, (1u << 4)))\n        return I2C_ERR_TIMEOUT;\n\n    return I2C_OK;\n}\n*** End Patch" } ***!

