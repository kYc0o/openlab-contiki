#include "soft_timer_delay.h"
#include "stm32f1xx.h"
#include "i2c.h"
#include "i2c_slave.h"

#include "iotlab_i2c_.h"
#include "iotlab_i2c_slave.h"

#include "debug.h"

/* http://c-faq.com/decl/recurfuncp.html */
typedef int (*fctptr)();  /* generic function pointer */
typedef fctptr (*i2c_state)(i2c_slave_event_t ev, uint8_t *data);


struct {
    struct iotlab_i2c_handler *msg_handler;
    struct iotlab_i2c_handler_arg msg;
    struct iotlab_i2c_handler handlers_list;  // sentinel
    void (*get_timestamp)(struct soft_timer_timeval *time);
} _state = {
    .msg_handler = NULL,
    .msg.len = 0,
    .handlers_list = {.next = NULL},
    .get_timestamp = NULL,
};


static i2c_state st_idle(i2c_slave_event_t ev, uint8_t *data);

/* Loop the state machine */
static void i2c_slave_handler(i2c_slave_event_t ev, uint8_t *data)
{
    static i2c_state fsm_state = (i2c_state) st_idle;
    fsm_state = (i2c_state)fsm_state(ev, data);
}


void iotlab_i2c_slave_register_handler(struct iotlab_i2c_handler *handler)
{
    handler->next = _state.handlers_list.next;
    _state.handlers_list.next = handler;
}

void iotlab_i2c_slave_register_timestamp_fct(
        void (*get_timestamp)(struct soft_timer_timeval *))
{
    _state.get_timestamp = get_timestamp;
}

void iotlab_i2c_slave_start()
{
    i2c_enable(I2C_2, I2C_CLOCK_MODE_FAST);
    i2c_slave_set_address(I2C_2, IOTLAB_I2C_CN_ADDR);
    i2c_slave_configure(I2C_2, i2c_slave_handler);
}

void iotlab_i2c_slave_stop()
{
    i2c_disable(I2C_2);
    _state.handlers_list.next = NULL;  // remove handlers
}

static struct iotlab_i2c_handler *decode(uint8_t msg_type)
{
    struct iotlab_i2c_handler *current;
    for (current = _state.handlers_list.next;
            current != NULL; current = current->next)
        if (current->header == msg_type)
            return current;

    return NULL;
}


/*
 * State machine Implementation
 */

//static i2c_state st_idle(i2c_slave_event_t ev, uint8_t *data);
static i2c_state error(i2c_slave_event_t ev, uint8_t *data);
static i2c_state st_get_header(i2c_slave_event_t ev, uint8_t *data);
// RX message
static i2c_state st_rx_payload(i2c_slave_event_t ev, uint8_t *data);
static i2c_state st_rx_wait_rx_stop(i2c_slave_event_t ev, uint8_t *data);
// TX message
static i2c_state st_tx_wait_tx_start(i2c_slave_event_t ev, uint8_t *data);
static i2c_state st_tx_payload(i2c_slave_event_t ev, uint8_t *data);
static i2c_state st_tx_wait_tx_stop(i2c_slave_event_t ev, uint8_t *data);


static i2c_state st_idle(i2c_slave_event_t ev, uint8_t *data)
{
    log_debug("%u", ev);
    if (ev != I2C_SLAVE_EV_RX_START)
        return error(ev, data);

    _state.get_timestamp(&_state.msg.timestamp);
    return (i2c_state)st_get_header;
}
static i2c_state error(i2c_slave_event_t ev, uint8_t *data)
{
    log_error("Got an error during i2c transfer");
    if (ev == I2C_SLAVE_EV_TX_BYTE)
        *data = IOTLAB_I2C_ERROR;
    return (i2c_state)st_idle;
}

static i2c_state st_get_header(i2c_slave_event_t ev, uint8_t *data)
{
    log_debug("%u", ev);
    if (ev != I2C_SLAVE_EV_RX_BYTE)
        return error(ev, data);

    _state.msg_handler = decode(*data);  // decode pkt type
    if (_state.msg_handler == NULL)
        return error(ev, data);

    switch (_state.msg_handler->type) {
        case (IOTLAB_I2C_SLAVE_RX):
            _state.msg.len = 0;  // reset rx buffer
            return (i2c_state)st_rx_payload;
        case (IOTLAB_I2C_SLAVE_TX):
            return (i2c_state)st_tx_wait_tx_start;
    }

    return error(ev, data);
}


/*
 * RX transaction
 */
static i2c_state st_rx_payload(i2c_slave_event_t ev, uint8_t *data)
{
    log_debug("%u", ev);
    if (ev != I2C_SLAVE_EV_RX_BYTE)
        return error(ev, data);

    _state.msg.payload[_state.msg.len++] = *data;
    if (_state.msg.len == _state.msg_handler->payload_len)  // end of rx
        return (i2c_state)st_rx_wait_rx_stop;

    return (i2c_state)st_rx_payload;
}

static i2c_state st_rx_wait_rx_stop(i2c_slave_event_t ev, uint8_t *data)
{
    log_debug("%u", ev);
    if (ev != I2C_SLAVE_EV_STOP)
        return error(ev, data);
    _state.msg_handler->handler(&_state.msg);
    return (i2c_state)st_idle;
}


/*
 * TX transaction
 */

static i2c_state st_tx_wait_tx_start(i2c_slave_event_t ev, uint8_t *data)
{
    log_debug("%u", ev);
    if (ev != I2C_SLAVE_EV_TX_START)
        return error(ev, data);

    _state.msg_handler->handler(&_state.msg);
    _state.msg.len = 0;  // reset tx buffer
    return (i2c_state)st_tx_payload;
}

static i2c_state st_tx_payload(i2c_slave_event_t ev, uint8_t *data)
{
    log_debug("%u", ev);
    if (ev != I2C_SLAVE_EV_TX_BYTE)
        return error(ev, data);

    *data = _state.msg.payload[_state.msg.len++];
    if (_state.msg.len == _state.msg_handler->payload_len)  // end of tx
        return (i2c_state)st_tx_wait_tx_stop;

    return (i2c_state)st_tx_payload;
}

static i2c_state st_tx_wait_tx_stop(i2c_slave_event_t ev, uint8_t *data)
{
    log_debug("%u", ev);
    if (ev != I2C_SLAVE_EV_STOP)
        return error(ev, data);
    return (i2c_state)st_idle;
}
