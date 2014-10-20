#ifndef _SENSORCOMMAND_H
#define _SENSORCOMMAND_H

enum powerBits {
    POWER_BIT = 0,
    VOLTAGE_BIT = 1,
    CURRENT_BIT = 2,
    MODE_3V_BIT = 4,
    MODE_5V_BIT = 5,
    MODE_BATTERY_BIT = 6,
};

enum { float_typ = 1, uint16_t_typ = 2, uint32_t_typ = 3, };

//xSemaphoreHandle xRadioSem;

/*
 * Enable / disable periodic send of a power frame.
 * If Enable, then configure the measurement.
 * \config1
 *        powerBits {
            POWER_BIT = 0,
            VOLTAGE_BIT = 1,
            CURRENT_BIT = 2,
            MODE_3V_BIT = 4,
            MODE_5V_BIT = 5,
            MODE_BATTERY_BIT = 6,
        };
        bit 3, bit 7 unused
 * \config2
 *         bits 0-2 ina226_sampling_period_t in seconds in [0x0, 0x7]
 *             { 140e-6, 204e-6, 332e-6, 588e-6, 1100e-6, 2116e-6, 4156e-6,
 *            8244e-6 }
 *         bit 3 unused
 *         bits 4-6 ina226_averaging_factor_t
 *          { 1., 4., 16., 64., 128., 256., 512., 1024. }
 *      bit 7 : 1/0 ENABLE/DISABLE PERIODIC POWER FRAME
 */
enum error_t set_power_poll(uint8_t config1, uint8_t config2,
        uint32_t measure_period);


/*
 * Initialise a radio reception, set the handler
 * \return phy_status_t which value is expected to be PHY_SUCCESS
 */
//phy_status_t radio_poll();
phy_status_t radio_poll(uint8_t start_stop, uint16_t phy_rx_period);

/*
 * Initialize all the frames, power, radio, acknowledge and error
 */
void init_frame();

/*
 * Set a time transition marking the flush completion in the queue of previous
 * time reference event.
 * Wrapper to avoid having xAckConfigResetTime a global variable
 * \return error_t indicating if there was an issue or not
 */
enum error_t time_ack_transition();

/*
 * Set a radio transition marking the flush completion in the queue of previous
 * radio setting event.
 * Wrapper to avoid having xAckRadioConfig a global variable
 * Save the configuration power strength and channel in the ack frame
 * \return error_t indicating if there was an issue or not
 */
enum error_t radioConfig_ack_transition(uint8_t conf1, uint8_t conf2);

/*
 * complete ack frame data and call vSendFrame() to send it
 * \param handler_arg_t arg is a pointer on the frame to send
 */
void send_ack_frame(handler_arg_t arg);

/*
 * Complete and send an error frame
 * \param enum error_t error is the type of the error
 */
void errManager(enum error_t error);

#endif
