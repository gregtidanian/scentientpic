#include "eeprom_async.h"

typedef struct
{
    eeproma_t *e;
    uint8_t *buf;
    uint8_t len;
    eeproma_callback_t cb;
    void *ctx;
} eeproma_ctx_t;

static void eeproma_read_cb(void *context, i2c_event_t event);

bool eeproma_read_block_async(eeproma_t *e, uint8_t start_addr, uint8_t *buf, uint8_t len,
                              eeproma_callback_t cb, void *ctx)
{
    if (!e || !e->init || !len)
    {
        return false;
    }

    static uint8_t tx[1];
    tx[0] = start_addr;

    static eeproma_ctx_t c;
    c.e = e;
    c.buf = buf;
    c.len = len;
    c.cb = cb;
    c.ctx = ctx;

    i2c_transaction_t t = {
        .address = e->address, .tx_buf = tx, .tx_len = 1, .rx_buf = buf, .rx_len = len, .cb = eeproma_read_cb, .context = &c};
    return i2c_async_submit(e->i2c, &t);
}

static void eeproma_read_cb(void *context, i2c_event_t event)
{
    eeproma_ctx_t *c = (eeproma_ctx_t *)context;
    if (c->cb)
    {
        if (event == I2C_EVENT_COMPLETE)
        {
            c->cb(c->ctx, EEPROMA_OK);
        }
        else if (event == I2C_EVENT_NACK)
        {
            c->cb(c->ctx, EEPROMA_ERR_NACK);
        }
        else
        {
            c->cb(c->ctx, EEPROMA_ERR_TIMEOUT);
        }
    }
}

void eeproma_init(eeproma_t *e, i2c_async_t *bus, uint8_t addr)
{
    e->address = addr & 0x7F;
    e->i2c = bus;
    e->init = true;
}
