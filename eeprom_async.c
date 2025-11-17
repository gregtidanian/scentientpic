#include "eeprom_async.h"

static void eeproma_read_cb(void *context, i2c_event_t event);

bool eeproma_read_block_async(eeproma_t *e, uint8_t start_addr, uint8_t *buf, uint8_t len,
                              eeproma_callback_t cb, void *ctx)
{
    if (!e || !e->init || !len || !buf)
    {
        return false;
    }
    if (e->pending)
    {
        return false;
    }

    e->tx_buf[0] = start_addr;
    e->pending = true;
    e->pending_cb = cb;
    e->pending_ctx = ctx;

    i2c_transaction_t t = {
        .address = e->address,
        .tx_buf = e->tx_buf,
        .tx_len = 1,
        .rx_buf = buf,
        .rx_len = len,
        .cb = eeproma_read_cb,
        .context = e,
    };
    return i2c_async_submit(e->i2c, &t);
}

static void eeproma_read_cb(void *context, i2c_event_t event)
{
    eeproma_t *e = (eeproma_t *)context;
    if (!e)
    {
        return;
    }

    eeproma_result_t res = EEPROMA_ERR_TIMEOUT;
    if (event == I2C_EVENT_COMPLETE)
    {
        res = EEPROMA_OK;
    }
    else if (event == I2C_EVENT_NACK)
    {
        res = EEPROMA_ERR_NACK;
    }

    eeproma_callback_t cb = e->pending_cb;
    void *cb_ctx = e->pending_ctx;
    e->pending_cb = NULL;
    e->pending_ctx = NULL;
    e->pending = false;
    if (cb)
    {
        cb(cb_ctx, res);
    }
}

void eeproma_init(eeproma_t *e, i2c_async_t *bus, uint8_t addr)
{
    e->address = addr & 0x7F;
    e->i2c = bus;
    e->pending = false;
    e->tx_buf[0] = 0;
    e->pending_cb = NULL;
    e->pending_ctx = NULL;
    e->init = true;
}
