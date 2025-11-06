#include "eeprom.h"
#include <xc.h>
#include <libpic30.h>

#define EEPROM_WRITE_CYCLE_MS 10

// --- Internal helper: poll device for ACK after write cycle ---
static bool eeprom_wait_ready(eeprom_t *e)
{
    const i2c_t *bus = e->bus;
    for (uint8_t i = 0; i < 20; ++i)
    {
        if (i2c_start(bus) != I2C_OK)
        {
            continue;
        }

        i2c_result_t res = i2c_write_byte(bus, (e->address << 1) | 0);
        i2c_stop(bus);

        if (res == I2C_OK)
        {
            return true; // device ACKed ? ready
        }

        __delay_ms(1);
    }
    return false;
}

// --- Public API ---
void eeprom_init(eeprom_t *e, i2c_t *bus, uint8_t address)
{
    e->bus = bus;
    e->address = address & 0x7F;
    e->init = true;
}

void eeprom_deinit(eeprom_t *e)
{
    e->init = false;
    e->bus = NULL;
}

// ------------------------------------------------------------
// Write one byte to EEPROM
// ------------------------------------------------------------
eeprom_result_t eeprom_write_byte(eeprom_t *e, uint8_t mem_addr, uint8_t data)
{
    if (!e || !e->init)
        return EEPROM_ERR_I2C;

    const i2c_t *bus = e->bus;
    if (i2c_start(bus) != I2C_OK)
    {
        return EEPROM_ERR_TIMEOUT;
    }
    if (i2c_write_byte(bus, (e->address << 1) | 0) != I2C_OK)
    {
        goto fail;
    }
    if (i2c_write_byte(bus, mem_addr) != I2C_OK)
    {
        goto fail;
    }
    if (i2c_write_byte(bus, data) != I2C_OK)
    {
        goto fail;
    }
    i2c_stop(bus);

    __delay_ms(EEPROM_WRITE_CYCLE_MS);
    if (!eeprom_wait_ready(e))
    {
        return EEPROM_ERR_TIMEOUT;
    }
    return EEPROM_OK;

fail:
    i2c_stop(bus);
    return EEPROM_ERR_I2C;
}

// ------------------------------------------------------------
// Read one byte from EEPROM
// ------------------------------------------------------------
eeprom_result_t eeprom_read_byte(eeprom_t *e, uint8_t mem_addr, uint8_t *data)
{
    if (!e || !e->init || !data)
        return EEPROM_ERR_I2C;

    const i2c_t *bus = e->bus;

    if (i2c_start(bus) != I2C_OK)
    {
        return EEPROM_ERR_TIMEOUT;
    }
    if (i2c_write_byte(bus, (e->address << 1) | 0) != I2C_OK)
    {
        goto fail;
    }
    if (i2c_write_byte(bus, mem_addr) != I2C_OK)
    {
        goto fail;
    }
    if (i2c_restart(bus) != I2C_OK)
    {
        goto fail;
    }
    if (i2c_write_byte(bus, (e->address << 1) | 1) != I2C_OK)
    {
        goto fail;
    }
    if (i2c_read_byte(bus, data, false) != I2C_OK)
    {
        goto fail;
    }
    i2c_stop(bus);
    return EEPROM_OK;

fail:
    i2c_stop(bus);
    return EEPROM_ERR_I2C;
}

// ------------------------------------------------------------
// Block read
// ------------------------------------------------------------
eeprom_result_t eeprom_read_block(eeprom_t *e, uint8_t start_addr, uint8_t *buf, uint8_t len)
{
    if (!e || !buf || !len)
        return EEPROM_ERR_I2C;
    const i2c_t *bus = e->bus;

    if (i2c_start(bus) != I2C_OK)
    {
        return EEPROM_ERR_TIMEOUT;
    }
    if (i2c_write_byte(bus, (e->address << 1) | 0) != I2C_OK)
    {
        goto fail;
    }
    if (i2c_write_byte(bus, start_addr) != I2C_OK)
    {
        goto fail;
    }
    if (i2c_restart(bus) != I2C_OK)
    {
        goto fail;
    }
    if (i2c_write_byte(bus, (e->address << 1) | 1) != I2C_OK)
    {
        goto fail;
    }

    for (uint8_t i = 0; i < len; i++)
    {
        bool ack = (i < len - 1);
        if (i2c_read_byte(bus, &buf[i], ack) != I2C_OK)
        {
            goto fail;
        }
    }
    i2c_stop(bus);
    return EEPROM_OK;

fail:
    i2c_stop(bus);
    return EEPROM_ERR_I2C;
}

// ------------------------------------------------------------
// Block write (page writes up to 8 bytes for M24C02)
// ------------------------------------------------------------
eeprom_result_t eeprom_write_block(eeprom_t *e, uint8_t start_addr, const uint8_t *buf, uint8_t len)
{
    if (!e || !buf || !len)
    {
        return EEPROM_ERR_I2C;
    }
    const i2c_t *bus = e->bus;

    uint8_t remaining = len;
    uint8_t addr = start_addr;
    const uint8_t *p = buf;

    while (remaining > 0)
    {
        uint8_t page_offset = addr & 0x07;
        uint8_t bytes_in_page = 8 - page_offset;
        if (bytes_in_page > remaining)
        {
            bytes_in_page = remaining;
        }

        if (i2c_start(bus) != I2C_OK)
        {
            return EEPROM_ERR_TIMEOUT;
        }
        if (i2c_write_byte(bus, (e->address << 1) | 0) != I2C_OK)
        {
            goto fail;
        }
        if (i2c_write_byte(bus, addr) != I2C_OK)
        {
            goto fail;
        }

        for (uint8_t i = 0; i < bytes_in_page; i++)
        {
            if (i2c_write_byte(bus, *p++) != I2C_OK)
            {
                goto fail;
            }
        }
        i2c_stop(bus);
        __delay_ms(EEPROM_WRITE_CYCLE_MS);
        if (!eeprom_wait_ready(e))
        {
            return EEPROM_ERR_TIMEOUT;
        }

        addr += bytes_in_page;
        remaining -= bytes_in_page;
    }

    return EEPROM_OK;

fail:
    i2c_stop(bus);
    return EEPROM_ERR_I2C;
}
