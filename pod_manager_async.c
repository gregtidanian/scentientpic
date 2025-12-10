#include "pod_manager_async.h"
#include <string.h>

#define POD_MAX_PWM_SETTING 100
#define POD_DEFAULT_PWM_SETTING 50

/**
 * Note that the i2c addess and relay pin mapping goes
 * Pod:         1R      2R      3R      1L      2L      3L
 * Pod number:  0       1       2       3       4       5
 * E2,E1,E0:    001     011     111     110     101     100
 * i2c Address: 0x51    0x53    0x57    0x56    0x55    0x54
 * Pin:         RB7     RB8     RB9     RE1     RE0     RF1
 */
static const uint8_t POD_ADDRS[POD_BAY_COUNT] = {0x51, 0x53, 0x57, 0x56, 0x55, 0x54};

static pod_manager_async_fire_callback_t p_fire_callback = NULL;
static pod_manager_async_read_callback_t p_read_callback = NULL;
static pod_manager_async_t *g_pm = NULL;

static void pod_read_done(void *ctx, eeproma_result_t res);
static void pod_burst_write_done(void *ctx, eeproma_result_t res);
static void pod_try_write_bursts(poda_t *p);

void relay_pwm_fire_callback(const relay_pwm_evt_info_t *p_evt)
{
    pod_manager_async_fire_evt_t evt;
    switch (p_evt->type)
    {
    case RELAY_PWM_EVT_FIRE:
        evt = POD_MANAGER_ASYNC_EVT_FIRE_START;
        if (p_fire_callback)
        {
            p_fire_callback(&evt);
        }
        break;

    case RELAY_PWM_EVT_BURST:
        if (g_pm && p_evt->pod_index < POD_BAY_COUNT)
        {
            poda_t *p = &g_pm->pods[p_evt->pod_index];
            p->bursts.bursts_active += 1;
            u32_to_le(&p->buf[16], p->bursts.bursts_active);
            p->burst_write_needed = true;
        }
        break;

    case RELAY_PWM_EVT_STOP:
        evt = POD_MANAGER_ASYNC_EVT_FIRE_STOP;
        if (p_fire_callback)
        {
            p_fire_callback(&evt);
        }
        break;
    default:
        break;
    }
}

void pod_manager_async_init(pod_manager_async_t *pm, i2c_async_t *bus, pod_manager_async_fire_callback_t p_fire_cb, pod_manager_async_read_callback_t p_read_cb)
{
    memset(pm, 0, sizeof(*pm));
    pm->bus = bus;
    p_fire_callback = p_fire_cb;
    p_read_callback = p_read_cb;
    g_pm = pm;
    relay_pwm_init(relay_pwm_fire_callback);
    for (uint8_t i = 0; i < POD_BAY_COUNT; i++)
    {
        pm->pods[i].bay = i;
        eeproma_init(&pm->pods[i].eeprom, bus, POD_ADDRS[i]);
    }
}

void pod_manager_async_poll(pod_manager_async_t *pm)
{
    for (uint8_t i = 0; i < POD_BAY_COUNT; i++)
    {
        poda_t *p = &pm->pods[i];
        if (p->burst_write_needed)
        {
            if (!p->eeprom.pending)
            {
                pod_try_write_bursts(p);
            }
            continue; // finish pending write before issuing another transaction
        }
        eeproma_read_block_async(&p->eeprom, 0x00, p->buf, POD_EEPROM_BLOCK_SIZE,
                                 pod_read_done, p);
    }
}

static void pod_read_done(void *ctx, eeproma_result_t res)
{
    poda_t *p = (poda_t *)ctx;
    pod_manager_async_read_evt_t read_evt = POD_MANAGER_ASYNC_EVT_READ_FAIL;
    if (res == EEPROMA_OK)
    {
        const uint8_t *b = p->buf;
        p->data.serial_number = u32_from_buf_le(&b[0]);
        p->data.scent_id = u16_from_buf_le(&b[4]);
        p->data.pwm_setting = (b[6] > POD_MAX_PWM_SETTING) ? POD_DEFAULT_PWM_SETTING : b[6]; // Prevent invalid data range
        p->data.time_period = u16_from_buf_le(&b[7]);
        p->data.burst_capacity_ms = u32_from_buf_le(&b[9]);
        p->data.version = b[13];
        p->data.reserved[0] = b[14];
        p->data.reserved[1] = b[15];
        p->bursts.bursts_active = u32_from_buf_le(&b[16]);

        // Default is active
        p->active = true;

        // Check validity of pod
        if ((p->data.serial_number == 0xFFFFFFFF) &&
            (p->data.time_period == 0xFFFF))
        {
            p->active = false;
        }

        read_evt = POD_MANAGER_ASYNC_EVT_READ_SUCCESS;
    }
    else
    {
        p->active = false;
    }

    if (p_read_callback)
    {
        p_read_callback(ctx, &read_evt);
    }
}

static void pod_burst_write_done(void *ctx, eeproma_result_t res)
{
    poda_t *p = (poda_t *)ctx;
    if (!p)
    {
        return;
    }

    if (res != EEPROMA_OK)
    {
        // Retry on next poll
        p->burst_write_needed = true;
    }
}

static void pod_try_write_bursts(poda_t *p)
{
    uint8_t buf[4];
    u32_to_le(buf, p->bursts.bursts_active);
    if (eeproma_write_block_async(&p->eeprom, 16, buf, sizeof(buf), pod_burst_write_done, p))
    {
        p->burst_write_needed = false;
    }
}

void pod_manager_fire(pod_manager_async_t *pm, uint8_t bay, uint16_t duration_ms, uint8_t pulse_duty, uint8_t multiplier)
{
    if (bay >= POD_BAY_COUNT)
    {
        return;
    }
    poda_t *p = &pm->pods[bay];
    if (!p->active)
    {
        return;
    }
    relay_pwm_fire(bay, duration_ms, pulse_duty, p->data.time_period, p->data.pwm_setting, multiplier);
}
