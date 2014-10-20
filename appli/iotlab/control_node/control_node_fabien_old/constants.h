/*
 * Copyright  2008-2009 INRIA/SensLab
 *
 * <dev-team@senslab.info>
 *
 * This software is a set of libraries designed to develop applications
 * for the WSN430 embedded hardware platform.
 *
 * This software is governed by the CeCILL license under French law and
 * abiding by the rules of distribution of free software.  You can  use,
 * modify and/ or redistribute the software under the terms of the CeCILL
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and  rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty  and the software's author,  the holder of the
 * economic rights,  and the successive licensors  have only  limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading,  using,  modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean  that it is complicated to manipulate,  and  that  also
 * therefore means  that it is reserved for developers  and  experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and,  more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL license and that you accept its terms.
 */

#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

//time to wait before send a frame of only 1 measure, in ms and in us
#define SPEC_MAX_MEASURE_PERIOD_BEF_SEND_MS 50.
#define POSSIBLE_DELAY_IN_THE_QUEUE_MS 6.
#define MAX_MEASURE_PERIOD_BEF_SEND_MS     (SPEC_MAX_MEASURE_PERIOD_BEF_SEND_MS - POSSIBLE_DELAY_IN_THE_QUEUE_MS)
#define MAX_MEASURE_PERIOD_BEF_SEND_US MAX_MEASURE_PERIOD_BEF_SEND_MS * 1000.

/* FRAME */
// first byte
#define SYNC_BYTE 0x80

// type byte
enum frameType {
    //code command control node
    OPENNODE_START = 0x15,
    OPENNODE_STARTBATTERY = 0x16,
    OPENNODE_STOP = 0x17,

    BATTERY_CHARGE = 0x18,
    N0_BATTERY_CHARGE = 0x19,

    //CONFIG_RADIO = 0x40,

    CONFIG_POWERPOLL = 0x42,
    CONFIG_RADIOPOLL = 0x44,

    CONFIG_RADIONOISE = 0x45,

    SET_TIME = 0x52,

    FAKE_SIMPLE_POLLING = 0x60,

    OPEN_NODE_START = 0x70,
    OPEN_NODE_STOP = 0x71,

    RESET_TIME = 0x72,
    GET_TIME = 0x73,

    CONFIG_RADIO = 0x74,
    CONFIG_RADIO_POLL = 0x75,
    CONFIG_RADIO_NOISE = 0x76,
    CONFIG_SNIFFER = 0x77,

    CONFIG_SENSOR = 0x78,

    CONFIG_POWER_POLL = 0x79,

    //LED management
    GREEN_LED_FIX = 0xB5,
    GREEN_LED_BLINK = 0xB6,

    //code command autotest
    TST_I2C2 = 0xB9,
    PINGPONG = 0xBB,
    GPIO = 0xBE,

    TEST_PPS     = 0xBC,
    TEST_GOT_PPS = 0xBD,
};

//phy driver is more generic than rf231
//tab_pw_str code the pw_strentgh on 4 bits as it is in the hardware
// and as it will be input from the gateway ; it does the direct conversion
enum powerStrength
{
    POWER_m17dBm = 0xf, POWER_m12dBm = 0xe, POWER_m9dBm = 0xd,
    POWER_m7dBm = 0xc, POWER_m5dBm = 0xb, POWER_m4dBm = 0xa,
    POWER_m3dBm = 9, POWER_m2dBm = 8, POWER_m1dBm = 7, POWER_0dBm = 6,
    POWER_0_7dBm = 5, POWER_1_3dBm = 4, POWER_1_8dBm = 3, POWER_2_3dBm = 2,
    POWER_2_8dBm = 1, POWER_3dBm = 0
};

/*uint16_t tab_pw_str[2][16] =
{
    {
    POWER_3dBm, POWER_2_8dBm, POWER_2_3dBm, POWER_1_8dBm,
    POWER_1_3dBm, POWER_0_7dBm, POWER_0dBm, POWER_m1dBm,
    POWER_m2dBm, POWER_m3dBm, POWER_m4dBm, POWER_m5dBm,
    POWER_m7dBm, POWER_m9dBm, POWER_m12dBm, POWER_m17dBm,
    },
    {
    PHY_POWER_3dBm, PHY_POWER_2_8dBm, PHY_POWER_2_3dBm, PHY_POWER_1_8dBm,
    PHY_POWER_1_3dBm, PHY_POWER_0_7dBm, PHY_POWER_0dBm, PHY_POWER_m1dBm,
    PHY_POWER_m2dBm, PHY_POWER_m3dBm, PHY_POWER_m4dBm, PHY_POWER_m5dBm,
    PHY_POWER_m7dBm, PHY_POWER_m9dBm, PHY_POWER_m12dBm, PHY_POWER_m17dBm,
    }
};*/

/*uint16_t tab_pw_str[16] =
{
    PHY_POWER_3dBm, PHY_POWER_2_8dBm, PHY_POWER_2_3dBm, PHY_POWER_1_8dBm,
    PHY_POWER_1_3dBm, PHY_POWER_0_7dBm, PHY_POWER_0dBm, PHY_POWER_m1dBm,
    PHY_POWER_m2dBm, PHY_POWER_m3dBm, PHY_POWER_m4dBm, PHY_POWER_m5dBm,
    PHY_POWER_m7dBm, PHY_POWER_m9dBm, PHY_POWER_m12dBm, PHY_POWER_m17dBm,
};*/

enum {
    MEASURE_DISABLE = 0,
    MEASURE_ENABLE  = 0x80,
};

enum responseType {
    ACK = 0xa, NACK = 0x2, PW_POLL_FRAME = 0xFF,
    RADIO_POLL_FRAME = 0xFE, ACK_FRAME = 0xFA,
    ERROR_FRAME = 0xEE
};

enum load_battery {
    NOCHARGE = 0x0, CHARGE = 0x1,
};

enum powerMode {
    BATTERY = 0x0, DC = 0x1,
};

enum error_t {
    OK = 0, NETWORK_QUEUE_OVERFLOW = -1, APPLICATION_QUEUE_OVERFLOW = -2,
    DEFENSIVE = -3
};

enum
{
    NO = 0, YES = 1, ERR = 0xFF
};

enum
{
    STOP = 0, START = 1
};

enum
{
    FRAME0 = 0,
    FRAME1 = 1
};

enum
{
    SUCCESS = 0,
    FAILED = 1,
};

enum
{
    //here it is one bit of the parameter byte to pass to lib functions
    RED_LED = 2,
    GREEN_LED = 1,
};

/*typedef enum
{
    INA226_PERIOD_140us = 0,
    INA226_PERIOD_204us = 1,
    INA226_PERIOD_332us = 2,
    INA226_PERIOD_588us = 3,
    INA226_PERIOD_1100us = 4,
    INA226_PERIOD_2116us = 5,
    INA226_PERIOD_4156us = 6,
    INA226_PERIOD_8244us = 7,
} ina226_sampling_period_t;


typedef enum
{
    INA226_AVERAGE_1 = 0,
    INA226_AVERAGE_4 = 1,
    INA226_AVERAGE_16 = 2,
    INA226_AVERAGE_64 = 3,
    INA226_AVERAGE_128 = 4,
    INA226_AVERAGE_256 = 5,
    INA226_AVERAGE_512 = 6,
    INA226_AVERAGE_1024 = 7,
} ina226_averaging_factor_t;*/

extern uint16_t tab_ina226_period[8];
extern uint16_t tab_ina226_average[8];

#endif
