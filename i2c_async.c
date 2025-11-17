#include "i2c_async.h"
#include <string.h>
#include <libpic30.h>
static i2c_async_t *active_bus = NULL;

// ---------- Internal helpers ----------
static inline bool queue_empty(i2c_async_t *bus)
{
    return (bus->head == bus->tail);
}

static inline bool queue_full(i2c_async_t *bus)
{
    return ((bus->head + 1) % I2C_MAX_QUEUE) == bus->tail;
}

static void start_next_transaction(i2c_async_t *bus);

// ---------- Public API ----------
void i2c_async_init(i2c_async_t *bus, const i2c_regs_t *regs, uint16_t brg)
{
    memset(bus, 0, sizeof(*bus));
    bus->regs = regs;
    *regs->CONL_reg = 0;
    *regs->BRG_reg = brg;
    *regs->CONL_reg |= (1u << 15); // I2CEN
    IEC1bits.MI2C1IE = 1;
    IFS1bits.MI2C1IF = 0;
    active_bus = bus;
}

bool i2c_async_submit(i2c_async_t *bus, const i2c_transaction_t *t)
{
    if (queue_full(bus))
    {
        return false;
    }

    bus->queue[bus->head] = *t;
    bus->head = (bus->head + 1) % I2C_MAX_QUEUE;

    if (!bus->busy)
    {
        bus->busy = true;
        start_next_transaction(bus);
    }
    return true;
}

// ---------- Core state machine ----------
static void start_next_transaction(i2c_async_t *bus)
{
    if (queue_empty(bus))
    {
        bus->busy = false;
        return;
    }

    bus->current = bus->queue[bus->tail];
    bus->tail = (bus->tail + 1) % I2C_MAX_QUEUE;
    bus->tx_index = 0;
    bus->rx_index = 0;
    bus->state = I2C_STATE_START;

    const i2c_regs_t *r = bus->regs;
    *r->CONL_reg |= (1u << 0); // SEN
}
static bool ACKBIT = false;
static bool FROM_ADDR = false;
// ---------- ISR ----------
void __attribute__((interrupt, no_auto_psv)) _MI2C1Interrupt(void)
{
    IFS1bits.MI2C1IF = 0;
    if (!active_bus)
    {
        return;
    }

    i2c_async_t *bus = active_bus;
    const i2c_regs_t *r = bus->regs;

    switch (bus->state)
    {

    case I2C_STATE_START:
    {
        const i2c_regs_t *r = bus->regs;
        // Choose WRITE if any TX bytes remain; else READ
        bool read_phase = (bus->tx_index >= bus->current.tx_len) && (bus->current.rx_len > 0);
        *r->TRN_reg = (uint8_t)((bus->current.address << 1) | (read_phase ? 1 : 0));
        bus->state = I2C_STATE_ADDR;
        FROM_ADDR = false;
        break;
    }

    case I2C_STATE_ADDR:
    {
        const i2c_regs_t *r = bus->regs;
        FROM_ADDR = true;

        // NACK on address?
        if (*r->STAT_reg & (1u << 15))
        {
            *r->CONL_reg |= (1u << 2); // PEN
            ACKBIT = false;
            bus->state = I2C_STATE_STOP;
            if (bus->current.cb)
                bus->current.cb(bus->current.context, I2C_EVENT_NACK);
            break;
        }
        ACKBIT = true;

        // If TX bytes remain, go send next TX byte
        if (bus->tx_index < bus->current.tx_len)
        {
            *r->TRN_reg = bus->current.tx_buf[bus->tx_index++];
            bus->state = I2C_STATE_TX;
            break;
        }

        // No TX left: if RX requested, begin read
        if (bus->current.rx_len > 0)
        {
            *r->CONL_reg |= (1u << 3); // RCEN
            bus->state = I2C_STATE_RX;
        }
        else
        {
            *r->CONL_reg |= (1u << 2); // PEN
            bus->state = I2C_STATE_STOP;
        }
        break;
    }

    case I2C_STATE_TX:
    {
        const i2c_regs_t *r = bus->regs;

        // NACK during TX?
        if (*r->STAT_reg & (1u << 15))
        {
            *r->CONL_reg |= (1u << 2); // PEN
            bus->state = I2C_STATE_STOP;
            if (bus->current.cb)
            { /* optional: report NACK here */
            }
            break;
        }

        if (bus->tx_index < bus->current.tx_len)
        {
            *r->TRN_reg = bus->current.tx_buf[bus->tx_index++];
        }
        else if (bus->current.rx_len > 0)
        {
            // TX complete and RX requested: issue repeated START
            *r->CONL_reg |= (1u << 1); // RSEN
            bus->state = I2C_STATE_RESTART;
        }
        else
        {
            *r->CONL_reg |= (1u << 2); // PEN
            bus->state = I2C_STATE_STOP;
        }
        break;
    }

    case I2C_STATE_RESTART:
    {
        const i2c_regs_t *r = bus->regs;
        // RSEN still in progress?
        if (*r->CONL_reg & (1u << 1))
            break;

        // Now send READ address
        *r->TRN_reg = (uint8_t)((bus->current.address << 1) | 1);
        bus->state = I2C_STATE_ADDR;
        break;
    }

    case I2C_STATE_RX:
        if (!(*r->STAT_reg & (1u << 1)))
            break;
        bus->current.rx_buf[bus->rx_index++] = *r->RCV_reg;

        if (bus->rx_index < bus->current.rx_len)
        {
            *r->CONL_reg &= ~(1u << 5);
            *r->CONL_reg |= (1u << 4); // ACKEN
            bus->state = I2C_STATE_RX_WAIT_ACK;
            break;
        }

        *r->CONL_reg |= (1u << 5); // NACK
        *r->CONL_reg |= (1u << 4); // ACKEN
        bus->state = I2C_STATE_RX_LAST_WAIT_ACK;
        break;

    case I2C_STATE_RX_WAIT_ACK:
        if (*r->CONL_reg & (1u << 4))
            break;                 // ACKEN still busy
        *r->CONL_reg |= (1u << 3); // now safe to assert RCEN
        bus->state = I2C_STATE_RX;
        break;

    case I2C_STATE_RX_LAST_WAIT_ACK:
        if (*r->CONL_reg & (1u << 4))
            break;
        *r->CONL_reg |= (1u << 2); // now issue STOP
        bus->state = I2C_STATE_STOP;
        break;

    case I2C_STATE_STOP:
    {
        const i2c_regs_t *r = bus->regs;
        if (*r->CONL_reg & (1u << 2))
            break; // still stopping
        bus->state = I2C_STATE_DONE;

        if (bus->current.cb)
        {
            // signal COMPLETE when we successfully progressed past address
            bus->current.cb(bus->current.context, I2C_EVENT_COMPLETE);
        }

        start_next_transaction(bus);
        break;
    }

    case I2C_STATE_DONE:
        break;

    default:
        break;
    }
}
