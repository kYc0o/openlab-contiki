#include <math.h>
#include "printf.h"
/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Drivers */
#include "soft_timer.h"
#include "soft_timer_delay.h"
#include "ina226.h"
#include "fiteco_lib_gwt.h"
#include "platform.h"
#include "timer.h"

/* Objects */
#include "../constants.h"
#include "../global.h"
#include "../control_node.h"
#include "phy.h"
#include "sensorCommand.h"

#include "event.h"
#include "leds_cmds.h"

/* Local Variables */
static uint16_t xVoltage, xCurrent, xPower;
phy_packet_t pkt;

static xPowerFrame_t *xPowerFrame, xPowerFrameTab[2];
static xRadioFrame_t *xRadioFrame, xRadioFrameTab[2];
static xAckConfigChanged_t xAckResetTime, xAckPowerPoll, xAckRadioConfig;
static uint32_t xRadioCountMax;
static uint32_t xPowerCountMax;
static uint32_t rad_period;
static uint32_t prec_time_rad_rx_start;

/* Function Protypes */
/*
 * Append time on the radio frame.
 * \param *frame is the pointer to the radio frame
 * \data is a pointer on the data to append
 */
static void vAppendTimeRadio(xRadioFrame_t *frame, uint32_t data);

/*
 * Append data to the power frame
 * \*frame is the pointer to the frame
 * \*data is the pointer to the data to add
 * \data_type is the data type to add, float for measure, uint32_t for time
 * \pos indicates the position in the frame where to add the data
 */
static void vAppendData(xPowerFrame_t *frame, void *data,
        uint16_t data_type, uint16_t pos);

/*
 * complete power frame data and call vSendFrame() to send it
 * \param *frame is the power frame to send
 * bunch is the unitary content of a set of measurement
 * ex: for a power frame dependinf of its PVC configuration, bunch is
 * time + power(opt) measure + voltage(opt) measure + current(opt) measure
 */
static void vSendPowerFrame(xPowerFrame_t *frame);

/*
 * complete radio frame data and call vSendFrame() to send it
 * \param *frame is the radio frame to send
 */
static void vSendRadioFrame(xRadioFrame_t *frame);

/*
 * Initiliaze the power frame
 */
inline void init_pw_frame();

/*
 * toggle the round robin buffer of 2 frames for each power and radio frame
 * \param type is type of the frame to toggle to the other buffer
 */
void* toggle_frame(enum responseType type);

/*
 * ina226 interrupt handler
 * \param arg not used to be compatible with the lib
 * \param v the voltage measure
 * \param c the current measure
 * \param p the power measure
 *  \param measure_time, the time of the measurement at the interrupt
 */
static void pw_measure_handler(handler_arg_t arg,
        float v, float c, float p, uint32_t measure_time);

/*
 * Initialize the radio frame
 */
static void init_radio_frame();

/*
 * radio reception interrupt handler
 * append the time and the measurement in the radio measure frame
 * send the frame once ready and be ready for new measurement if expected
 * return phy_status_t which value is expected to be PHY_SUCCESS
 */
static void radio_poll_handler(phy_status_t status);

/*
 *
 */
static void manage_radio_packet(phy_status_t status);

/*
 * Set a transition event marking the flush completion in the queue of previous
 * similar event with previous setting.
 * \return error_t indicating if there was an issue or not
 */
static enum error_t ack_transition(xAckConfigChanged_t *frame);

/*
 * send the current power frame used to save the power measurement
 * \param limit_count is the maximum number of measurements in the frame
 */
void send_and_toggle_pw_frame(uint32_t limit_count);

/*
 * send the current radio frame used to save the radio measurement
 * \param limit_count is the maximum number of measurements in the frame
 */
void send_and_toggle_radio_frame(uint8_t limit_count);

/*
 * There could be several error frames at the same time.
 * xErrorFrameMutex[count] is the table of available error frames
 * \return the number of the error frame available or ERR if there is none
 */
static uint8_t selectErrorFrame();

/*
 * Manage the frame, init, append, send
 */
void init_frame()
{
    xAckResetTime.sync = SYNC_BYTE;
    //no config to send
    xAckResetTime.len = 2;
    xAckResetTime.type = ACK_FRAME;
    xAckResetTime.config_typ = RESET_TIME;
    xAckPowerPoll.sync = SYNC_BYTE;
    //1 byte for on/off measurement period and average
    xAckPowerPoll.len = 3;
    xAckPowerPoll.type = ACK_FRAME;
    xAckPowerPoll.config_typ = CONFIG_POWER_POLL;
    xAckRadioConfig.sync = SYNC_BYTE;
    //1 byte for power strength tx, 1 byte for channel
    xAckRadioConfig.len = 4;
    xAckRadioConfig.type = ACK_FRAME;
    xAckRadioConfig.config_typ = CONFIG_RADIO;

    xPowerFrame = (xPowerFrame_t*) toggle_frame(PW_POLL_FRAME);
    init_pw_frame();
    xPowerFrame = (xPowerFrame_t*) toggle_frame(PW_POLL_FRAME);
    init_pw_frame();
    xRadioFrame = (xRadioFrame_t*) toggle_frame(RADIO_POLL_FRAME);
    init_radio_frame();
    xRadioFrame = (xRadioFrame_t*) toggle_frame(RADIO_POLL_FRAME);
    init_radio_frame();

    uint8_t count;
    for(count=0;count < NBR_ERROR_FRAME;count++)
    {
        xErrorFrame[count].used = NO;
        xErrorFrame[count].sync = SYNC_BYTE;
        xErrorFrame[count].len = 2;
        xErrorFrame[count].type = ERROR_FRAME;
        xErrorFrame[count].error = 0;
    }
}

static void vAppendTimeRadio(xRadioFrame_t *frame, uint32_t data)
{
    uint32_t2byte_t data_32;
    data_32.nbr = data;

    frame->stat[xRadioFrame->count].time.byte[0] = data_32.byte[0];
    frame->stat[xRadioFrame->count].time.byte[1] = data_32.byte[1];
    frame->stat[xRadioFrame->count].time.byte[2] = data_32.byte[2];
    frame->stat[xRadioFrame->count].time.byte[3] = data_32.byte[3];
}

static void vAppendData(xPowerFrame_t *frame, void *data,
        uint16_t data_type, uint16_t pos)
{
    float2byte_t measure;
    uint32_t2byte_t data_32;
    float *temp;
    uint32_t *temp_32, nbr_meas;
    nbr_meas = (xVoltage + xCurrent + xPower + 1) * 4;

    switch (data_type)
    {
        case float_typ:
           temp = data;
           measure.nbr = *temp;
           frame->measures[frame->count * nbr_meas + pos * 4] = measure.byte[0];
           frame->measures[frame->count * nbr_meas + pos * 4 + 1] = measure.byte[1];
           frame->measures[frame->count * nbr_meas + pos * 4 + 2] = measure.byte[2];
           frame->measures[frame->count * nbr_meas + pos * 4 + 3] = measure.byte[3];
           break;
        case uint32_t_typ:
            temp_32 = data;
            data_32.nbr = *temp_32;
            frame->measures[frame->count * nbr_meas] = data_32.byte[0];
            frame->measures[frame->count * nbr_meas + 1] = data_32.byte[1];
            frame->measures[frame->count * nbr_meas + 2] = data_32.byte[2];
            frame->measures[frame->count * nbr_meas + 3] = data_32.byte[3];
            break;
        default:
            errManager(DEFENSIVE);
           break;
   }
}

static void vSendPowerFrame(xPowerFrame_t *frame)
{
    //power frame ? then pw_size_bunch = (xVoltage + xCurrent + xPower + 1) * 4
    //len + 2 because of type, count
    frame->len = 2 + frame->count * ((xVoltage + xCurrent + xPower + 1) * 4);

    //+2 for the synchro and len bytes
    vSendFrame(frame->data, (uint16_t) (frame->len + 2));

    // clear the count fields
    frame->count = 0;
}

static void vSendRadioFrame(xRadioFrame_t *frame)
{
    //time 4B + rssi 1B + lqi 1B = 6
    //2 for type count
    frame->len = 2 + frame->count * 6;

    //+ 2 for synchro and len bytes
    vSendFrame(frame->data, (uint16_t) (frame->len + 2));

    // clear the count of measure in a frame for next frame to send
    frame->count = 0;
}

/*
 * Power measurement functions
 */

inline void init_pw_frame()
{
    xPowerFrame->sync = SYNC_BYTE;
    xPowerFrame->len = 0;
    xPowerFrame->type = 0xFF;
    xPowerFrame->count = 0;
}

void* toggle_frame(enum responseType type)
{
    //init frame in order to start with frame 0 with 1st frame (2 toggles)
    static uint16_t which_frame[2] = { 0, 0};

    switch (type)
    {
    case PW_POLL_FRAME:
        if (1 == which_frame[0])
        {
            which_frame[0] = 0;
            return &xPowerFrameTab[0];
        }
        else
        {
            which_frame[0] = 1;
            return &xPowerFrameTab[1];
        }
        break;
    case RADIO_POLL_FRAME:
        if (1 == which_frame[1])
        {
            which_frame[1] = 0;
            return &xRadioFrameTab[0];
        }
        else
        {
            which_frame[1] = 1;
            return &xRadioFrameTab[1];
        }
        break;
    default:
        // send error frame
        errManager(DEFENSIVE);
        return NULL;
        break;
    }
}

void send_and_toggle_pw_frame(uint32_t limit_count)
{
    //send the prepared frame if any
    if (xPowerFrame->count >= limit_count)
    {
        vSendPowerFrame(xPowerFrame);

        //toggle_frame to not collide the frame to send
        do
        {
            xPowerFrame = (xPowerFrame_t*) toggle_frame(PW_POLL_FRAME);
        }
        //loop in case of issue to avoid crash and potentially allow to
        //send error frame
        while (NULL == xPowerFrame);

        //init frame of the selected buffer
        init_pw_frame();
    }
}

enum error_t set_power_poll(uint8_t config1, uint8_t config2,
                                uint32_t measure_period)
{
    ina226_sampling_period_t convertion_time;
    ina226_averaging_factor_t average;
    enum error_t ret;
    uint32_t max_count_8bits;

    //Stop ina measurement
    fiteco_lib_gwt_current_monitor_select(FITECO_GWT_CURRENT_MONITOR__OFF,
            pw_measure_handler, (handler_arg_t) NULL);


    send_and_toggle_pw_frame((uint32_t) 1);

    //case bit enable=0 so disable measurement
    if (MEASURE_DISABLE == !!(config2 & 0x80))
    {
        return OK;
    }

    //Post in the queue the transition boundary event
    xAckPowerPoll.config = config1;
    ret = ack_transition(&xAckPowerPoll);
    if (OK != ret)
    {
        return ret;
    }

    //do the configuration and start new measurement
    //configure period and average power measurement
    convertion_time = (ina226_sampling_period_t) (config2 & 0x07);
    average = (ina226_averaging_factor_t) ((config2 & 0x70) >> 4);
    ina226_configure(convertion_time, average);

    //determine measure and conditions
    xVoltage = !!(config1 & (1 << VOLTAGE_BIT));
    xCurrent = !!(config1 & (1 << CURRENT_BIT));
    xPower = !!(config1 & (1 << POWER_BIT));

    //determine maximum measure in the frame
    // 50ms - ~6ms of jitter give 44ms
    //double calc = 44000. / (double) measure_period;
    double calc = MAX_MEASURE_PERIOD_BEF_SEND_US / (double) measure_period;

    if (1. > calc)
    {//case period of measurement is > 50ms
     // send each measurement
        xPowerCountMax = 1;
    }
    else
    {//concatenate measure in one frame
        //length is a byte, in a pw frame already 2 bytes for type and count
        //then there are count * bunch byte
        //a bunch is composed of time + optional(xVoltage + xCurrent + xPower)
        //so a bunch is 2 to 4 32bits words
        max_count_8bits = (uint32_t) floor(
                (256 - 2) / ((xVoltage + xCurrent + xPower + 1) * 4 ));
        xPowerCountMax = (uint32_t) floor(calc);
        //
        if (max_count_8bits < xPowerCountMax)
        {
            xPowerCountMax = max_count_8bits;
        }
    }

    //complete configuration and start new measure
    uint8_t mode = config1 & 0x70;
    switch (mode)
    {
    case (1 << MODE_3V_BIT):
        fiteco_lib_gwt_current_monitor_select(FITECO_GWT_CURRENT_MONITOR__OPEN_3V,
                pw_measure_handler, (handler_arg_t) NULL);
        break;
    case (1 << MODE_5V_BIT):
        fiteco_lib_gwt_current_monitor_select(FITECO_GWT_CURRENT_MONITOR__OPEN_5V,
                pw_measure_handler, (handler_arg_t) NULL);
        break;
    case (1 << MODE_BATTERY_BIT):
        fiteco_lib_gwt_current_monitor_select(FITECO_GWT_CURRENT_MONITOR__BATTERY,
                pw_measure_handler, (handler_arg_t) NULL);
        break;
    default:
        // send error frame
        errManager(DEFENSIVE);
        return DEFENSIVE;
        break;
    }
    return OK;
}

static void pw_measure_handler(handler_arg_t arg,
        float v, float c, float p, uint32_t measure_time)
{
    uint8_t pos_in_bunch = 0;
    uint32_t time_tmp = (uint32_t) measure_time - time0;
    vAppendData(xPowerFrame, &time_tmp, uint32_t_typ, (uint16_t) pos_in_bunch);

    //order of data is PVC
    if (xPower)
    {
        vAppendData(xPowerFrame, &p, float_typ, (uint16_t) ++pos_in_bunch);
    }
    if (xVoltage)
    {
        vAppendData(xPowerFrame, &v, float_typ, (uint16_t) ++pos_in_bunch);
    }
    if (xCurrent)
    {
        vAppendData(xPowerFrame, &c, float_typ, (uint16_t) ++pos_in_bunch);
    }
    xPowerFrame->count++;
    send_and_toggle_pw_frame(xPowerCountMax);
}

/*
 * Radio measure functions
 */

void send_and_toggle_radio_frame(uint8_t limit_count)
{
    //send the prepared frame if any
    if (xRadioFrame->count >= limit_count)
    {
        vSendRadioFrame(xRadioFrame);

        //toggle_frame to not collide the frame to send
        //loop in case of issue to avoid crash and potentially allow to
        //send error frame
        do
        {
            xRadioFrame = (xRadioFrame_t*) toggle_frame(RADIO_POLL_FRAME);
        }
        while (NULL == xPowerFrame);

        //init frame of the selected buffer
        init_radio_frame();//only necessary at setup, len not needed
    }
}

phy_status_t radio_poll(uint8_t start_stop, uint16_t phy_rx_period)
{
    if (START == start_stop)
    {
        //stops any ongoing RX or TX and put the underlying radio chip in IDLE mode.
        phy_idle(platform_phy);

        //send the prepared frame if any
        send_and_toggle_radio_frame(1);

        //determine maximum measure in the frame
        // 50ms - ~6ms of jitter give 44ms
        //double calc = 44. / (double) phy_rx_period;
        double calc = MAX_MEASURE_PERIOD_BEF_SEND_MS / (double) phy_rx_period;

        if (phy_rx_period >= MAX_MEASURE_PERIOD_BEF_SEND_MS)
        {//case period of measurement is > 50ms
         // send each measurement
            xRadioCountMax = 1;
        }
        else
        {//calc > 1, concatenate measure in one frame
            xRadioCountMax = (uint16_t) floor(calc);
        }

        rad_period = soft_timer_ms_to_ticks((int32_t) phy_rx_period);

        uint32_t cur_time = soft_timer_time();
        prec_time_rad_rx_start = cur_time;
        phy_status_t phy_ret = phy_rx(platform_phy, cur_time + rad_period, cur_time + 2 * rad_period,
                            &pkt, (phy_handler_t) radio_poll_handler);
        //phy_rx(platform_phy, 0, phy_rx_period, &pkt, (phy_handler_t) radio_poll_handler);
        return phy_ret;
    }
    else if (STOP == start_stop)
    {
        //reset the underlying radio chip, and all states to SLEEP mode
        phy_reset(platform_phy);
        return PHY_SUCCESS;
    }
    //we should never comes here
    return ERR;
}

static void init_radio_frame()
{
    xRadioFrame->sync = SYNC_BYTE;
    xRadioFrame->len = 0;
    xRadioFrame->type = 0xFE;
    xRadioFrame->count = 0; //reset when frame sent
}

static void manage_radio_packet(phy_status_t status)
{
    xRadioFrame->stat[xRadioFrame->count].rssi = pkt.rssi;
    if (PHY_ERR_TOO_LATE == status) {
        //phy reception time out, no pkt received send rssi, lqi=0
        xRadioFrame->stat[xRadioFrame->count].lqi = 0;
    } else if (PHY_SUCCESS == status) {
        xRadioFrame->stat[xRadioFrame->count].lqi = pkt.lqi;
    }

    xRadioFrame->count++;

    send_and_toggle_radio_frame(xRadioCountMax);

    //start again phy rx
    uint32_t cur_time = soft_timer_time();

    //find next period of measurement in the future
    do
    {
        prec_time_rad_rx_start += rad_period;
    }
    while (cur_time > prec_time_rad_rx_start);
    phy_rx(platform_phy, prec_time_rad_rx_start, prec_time_rad_rx_start + rad_period,
                        &pkt, (phy_handler_t) radio_poll_handler);
}

static void radio_poll_handler(phy_status_t status)
{
    if ( (PHY_SUCCESS == status) || (PHY_RX_TIMEOUT_ERROR == status) )
    {
        //manage time before sending to queue to avoid a potential send frame before time update
        //if error the time will be overwritten in a next measure
        uint32_t time_tmp = soft_timer_time();
        time_tmp -= time0;
        vAppendTimeRadio(xRadioFrame, time_tmp);
        if (EVENT_FULL == event_post(EVENT_QUEUE_APPLI, (handler_t) manage_radio_packet, (handler_arg_t) status))
        {
            errManager(APPLICATION_QUEUE_OVERFLOW);
        }
    }
}

/*
 * Acknowledge frame functions
 */

enum error_t time_ack_transition()
{
    return (ack_transition(&xAckResetTime));
}

enum error_t radioConfig_ack_transition(uint8_t conf1, uint8_t conf2)
{
    xAckRadioConfig.config = conf1;
    xAckRadioConfig.config2 = conf2;
    return (ack_transition(&xAckRadioConfig));
}

static enum error_t ack_transition(xAckConfigChanged_t * frame)
{
    //Post in the queue the transition boundary event
    if (EVENT_FULL == event_post(EVENT_QUEUE_APPLI, send_ack_frame, frame))
    {
        errManager(APPLICATION_QUEUE_OVERFLOW);
        return APPLICATION_QUEUE_OVERFLOW;
    }
    return OK;
}

void send_ack_frame(handler_arg_t arg)
{
    xAckConfigChanged_t *frame = arg;

    switch (frame->config_typ)
    {
        case RESET_TIME:
            vSendFrame(xAckResetTime.data, (uint16_t) (2 + frame->len));
            break;
        case CONFIG_RADIO:
            vSendFrame(xAckRadioConfig.data, (uint16_t) (2 + frame->len));
            break;
        case CONFIG_POWER_POLL:
            vSendFrame(xAckPowerPoll.data, (uint16_t) (2 + frame->len));
            break;
        default:
            // send error frame
            errManager(DEFENSIVE);
            break;
    }
}

/*
 * Errors managements
 */

void errManager(enum error_t error)
{
    error_led();
    uint8_t errFrameNbr = selectErrorFrame();
    if (ERR != errFrameNbr)
    {
        xSemaphoreTake(xErrorFrameMutex[errFrameNbr], portMAX_DELAY);
        xErrorFrame[errFrameNbr].used = YES;
        switch (error)
        {
        case DEFENSIVE:
            xErrorFrame[errFrameNbr].error = DEFENSIVE;
            break;
        case NETWORK_QUEUE_OVERFLOW:
            xErrorFrame[errFrameNbr].error = NETWORK_QUEUE_OVERFLOW;
            break;
        case APPLICATION_QUEUE_OVERFLOW:
            xErrorFrame[errFrameNbr].error = APPLICATION_QUEUE_OVERFLOW;
            break;
        default:
            //to avoid possible infinite loop see the global error variable
            no_frame_error_available_defensive_issue = ERR;
            break;
        }
        xSemaphoreGive(xErrorFrameMutex[errFrameNbr]);
    }
    else
    {
        //issue set global variable signalling for debug this issue
        no_frame_error_available_defensive_issue = ERR;
    }
    vSendErrorFrame(errFrameNbr);
}

static uint8_t selectErrorFrame()
{
    uint8_t count;
    uint8_t found = 0;
    for(count=0;count < NBR_ERROR_FRAME;count++)
    {
        xSemaphoreTake(xErrorFrameMutex[count], portMAX_DELAY);
        if (NO == xErrorFrame[count].used)
        {
            xErrorFrame[count].used = YES;
            found = 1;
        }
        xSemaphoreGive(xErrorFrameMutex[count]);
        if (1 == found)
        {
            return count;
        }
    }
    return ERR;
}


