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

static pod_manager_async_fire_callback_t p_callback = NULL;

static void pod_read_done(void *ctx, eeproma_result_t res);

void relay_pwm_fire_callback(relay_pwm_evt_t *p_evt)
{
    pod_manager_async_evt_t evt;
    switch (*p_evt)
    {
    case RELAY_PWM_EVT_FIRE:
        evt = POD_MANAGER_ASYNC_EVT_FIRE;
        p_callback(&evt);
        break;

    case RELAY_PWM_EVT_STOP:
        evt = POD_MANAGER_ASYNC_EVT_STOP;
        p_callback(&evt);
        break;
    default:
        break;
    }
}

void pod_manager_async_init(pod_manager_async_t *pm, i2c_async_t *bus, pod_manager_async_fire_callback_t p_cb)
{
    memset(pm, 0, sizeof(*pm));
    pm->bus = bus;
    p_callback = p_cb;
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
        eeproma_read_block_async(&p->eeprom, 0x00, p->buf, POD_EEPROM_BLOCK_SIZE,
                                 pod_read_done, p);
    }
}

static void pod_read_done(void *ctx, eeproma_result_t res)
{
    poda_t *p = (poda_t *)ctx;
    if (res == EEPROMA_OK)
    {
        const uint8_t *b = p->buf;
        p->active = false;
        p->data.serial_number = u32_from_buf_le(&b[0]);
        p->data.scent_id = u16_from_buf_le(&b[4]);
        p->data.pwm_setting = (b[6] > POD_MAX_PWM_SETTING) ? POD_DEFAULT_PWM_SETTING : b[6]; // Prevent invalid data range
        p->data.time_period = u16_from_buf_le(&b[7]);
        p->data.burst_capacity_ms = u32_from_buf_le(&b[9]);
        p->data.version = b[13];
        p->data.reserved[0] = b[14];
        p->data.reserved[1] = b[15];
        p->bursts.bursts_active = u32_from_buf_le(&b[16]);

        // Check validity of pod
        if (p->data.serial_number != 0xFFFFFFFF)
        {
            p->active = true;
        }
    }
    else
    {
        p->active = false;
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
