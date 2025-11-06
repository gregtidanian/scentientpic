#include "pod_manager.h"
#include <string.h>

static pod_manager_t *g_pm = NULL;

// E2:E1:E0 mapping table for six bays
static const uint8_t pod_addr_map[POD_BAY_COUNT] = {
    0x51, 0x52, 0x53, 0x54, 0x55, 0x56};

// ------------------------------------------------------------
// Callbacks
// ------------------------------------------------------------

typedef struct
{
    pod_manager_t *pm;
    uint8_t bay;
} pod_cb_ctx_t;

#ifdef OLD_MANAGER

static void eeprom_read_complete(void *ctx, i2c_event_t event)
{
    pod_cb_ctx_t *c = (pod_cb_ctx_t *)ctx;
    pod_t *p = &c->pm->pods[c->bay];

    if (event == I2C_EVENT_COMPLETE)
    {
        // EEPROM layout (example)
        // [0..15]  128-bit unique ID
        // [16..17] Scent ID
        // [18..19] Remaining volume
        // [20..21] Frequency (uint16_t)
        memcpy(p->uid, p->eeprom_buf, 16);
        p->scent = (p->eeprom_buf[16] << 8) | p->eeprom_buf[17];
        p->remaining = (p->eeprom_buf[18] << 8) | p->eeprom_buf[19];
        p->frequency = (p->eeprom_buf[20] << 8) | p->eeprom_buf[21];
        p->init = true;
    }
    else
    {
        // NACK or timeout ? consider pod removed
        p->init = false;
    }
}

// ------------------------------------------------------------
// Pod discovery and polling
// ------------------------------------------------------------

void pod_manager_init(pod_manager_t *pm, i2c_async_t *bus)
{
    memset(pm, 0, sizeof(*pm));
    pm->i2c = bus;

    for (uint8_t i = 0; i < POD_BAY_COUNT; i++)
    {
        pm->pods[i].position = i;
        pm->pods[i].init = false;
        pm->pods[i].eeprom.bus = bus;
        pm->pods[i].eeprom.address = pod_addr_map[i];
    }
    g_pm = pm;
}

// Probe a single address to see if it ACKs
static void probe_address_async(pod_manager_t *pm, uint8_t bay)
{
    uint8_t addr = pod_addr_map[bay];

    i2c_transaction_t probe = {
        .address = addr,
        .tx_buf = NULL,
        .tx_len = 0,
        .rx_buf = NULL,
        .rx_len = 0,
        .cb = NULL,
        .context = NULL};

    bool ok = i2c_async_submit(pm->i2c, &probe);

    if (!ok)
    {
        return;
    }

    // Simple check: if bus accepted, assume device might be there.
    // For full detection, try to read a known location (e.g., byte 0).
    uint8_t buf[22];
    pod_cb_ctx_t *ctx = malloc(sizeof(pod_cb_ctx_t));
    ctx->pm = pm;
    ctx->bay = bay;

    i2c_transaction_t t = {
        .address = addr,
        .tx_buf = (uint8_t[]){0x00},
        .tx_len = 1,
        .rx_buf = buf,
        .rx_len = 22, // enough to read metadata
        .cb = eeprom_read_complete,
        .context = ctx};
    pm->pods[bay].eeprom_buf = buf;
    i2c_async_submit(pm->i2c, &t);
}

void pod_manager_poll(pod_manager_t *pm)
{
    for (uint8_t i = 0; i < POD_BAY_COUNT; i++)
    {
        probe_address_async(pm, i);
    }
}

// ------------------------------------------------------------
// Timer integration
// ------------------------------------------------------------

void pod_manager_tick_100ms(void)
{
    if (g_pm)
    {
        pod_manager_poll(g_pm);
    }
}
#endif