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
    *regs->CONL = 0;
    *regs->BRG = brg;
    *regs->CONL |= (1u << 15); // I2CEN
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
    *r->CONL |= (1u << 0); // SEN
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
        if (*r->CONL & (1u << 0))
        {
            break; // SEN still set
        }
        *r->TRN = (bus->current.address << 1) |
                  (bus->current.rx_len ? 1 : 0);
        bus->state = I2C_STATE_ADDR;
        FROM_ADDR = false;
        break;

    case I2C_STATE_ADDR:
        FROM_ADDR = true;
        if (*r->STAT & (1u << 15))
        {                          // ACKSTAT = 1 ? NACK
            *r->CONL |= (1u << 2); // Stop
            ACKBIT = false;
            bus->state = I2C_STATE_STOP;
            if (bus->current.cb)
            {
                bus->current.cb(bus->current.context, I2C_EVENT_NACK);
            }
            break;
        }
        ACKBIT = true;
        if (bus->current.tx_len)
        {
            *r->TRN = bus->current.tx_buf[bus->tx_index++];
            bus->state = I2C_STATE_TX;
        }
        else if (bus->current.rx_len)
        {
            *r->CONL |= (1u << 3); // RCEN
            bus->state = I2C_STATE_RX;
        }
        else
        {
            *r->CONL |= (1u << 2); // Stop
            bus->state = I2C_STATE_STOP;
        }
        break;

    case I2C_STATE_TX:
        FROM_ADDR = false;
        if (*r->STAT & (1u << 15))
        { // NACK
            *r->CONL |= (1u << 2);
            bus->state = I2C_STATE_STOP;
            if (bus->current.cb)
            {
                // bus->current.cb(bus->current.context, I2C_EVENT_NACK);
            }
            break;
        }
        if (bus->tx_index < bus->current.tx_len)
        {
            *r->TRN = bus->current.tx_buf[bus->tx_index++];
        }
        else if (bus->current.rx_len)
        {
            // Repeated start for read
            *r->CONL |= (1u << 1); // RSEN
            bus->state = I2C_STATE_RESTART;
        }
        else
        {
            *r->CONL |= (1u << 2); // Stop
            bus->state = I2C_STATE_STOP;
        }
        break;

    case I2C_STATE_RESTART:
        FROM_ADDR = false;
        if (*r->CONL & (1u << 1))
        {
            break;
        }
        *r->TRN = (bus->current.address << 1) | 1; // Read
        bus->state = I2C_STATE_ADDR;
        break;

    case I2C_STATE_RX:
        FROM_ADDR = false;
        if (!(*r->STAT & (1u << 1)))
        {
            break; // RBF not set
        }
        bus->current.rx_buf[bus->rx_index++] = *r->RCV;
        if (bus->rx_index < bus->current.rx_len)
        {
            *r->CONL &= ~(1u << 5); // ACKDT=0
            *r->CONL |= (1u << 4);  // ACKEN
            *r->CONL |= (1u << 3);  // RCEN again
        }
        else
        {
            *r->CONL |= (1u << 5); // NACK
            *r->CONL |= (1u << 4); // ACKEN
            *r->CONL |= (1u << 2); // Stop
            bus->state = I2C_STATE_STOP;
        }
        break;

    case I2C_STATE_STOP:
        if (*r->CONL & (1u << 2))
        {
            break; // PEN still set
        }
        bus->state = I2C_STATE_DONE;
        if ((FROM_ADDR == false) || (ACKBIT == true))
        {
            if (bus->current.cb)
            {
                bus->current.cb(bus->current.context, I2C_EVENT_COMPLETE);
            }
        }
        FROM_ADDR = false;
        start_next_transaction(bus);
        break;

    case I2C_STATE_DONE:
        break;

    default:
        break;
    }
}
