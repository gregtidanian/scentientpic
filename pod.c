#include "pod.h"
#include <string.h>

static inline uint16_t u16_from_buf(uint8_t *b)
{
    return ((uint16_t)b[0] << 8) | b[1];
}

void pod_init(pod_t *p, i2c_t *bus, uint8_t bay, uint8_t address)
{
    memset(p, 0, sizeof(*p));
    p->bay = bay;
    eeprom_init(&p->eeprom, bus, address);
    p->init = false;
}

void pod_deinit(pod_t *p)
{
    eeprom_deinit(&p->eeprom);
    memset(p, 0, sizeof(*p));
}

// ------------------------------------------------------------
// Detect if a pod is present by checking for EEPROM ACK
// ------------------------------------------------------------
bool pod_detect(pod_t *p)
{
    if (!p || !p->eeprom.bus)
    {
        return false;
    }

    const i2c_t *bus = p->eeprom.bus;
    uint8_t addr = p->eeprom.address;
    bool ack = false;

    if (i2c_start(bus) == I2C_OK)
    {
        i2c_result_t res = i2c_write_byte(bus, (addr << 1) | 0);
        i2c_stop(bus);
        ack = (res == I2C_OK);
    }

    return ack;
}

// ------------------------------------------------------------
// Read all metadata from EEPROM into struct
// ------------------------------------------------------------
bool pod_load_metadata(pod_t *p)
{
    if (!p || !p->eeprom.bus)
    {
        return false;
    }
    uint8_t buf[22];
    if (eeprom_read_block(&p->eeprom, 0x00, buf, sizeof(buf)) != EEPROM_OK)
    {
        return false;
    }

    memcpy(p->uid, &buf[0], 16);
    p->scent = u16_from_buf(&buf[16]);
    p->remaining = u16_from_buf(&buf[18]);
    p->init = true;

    return true;
}
