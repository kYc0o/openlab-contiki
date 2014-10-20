/*
 * In the build directory just under the openlab directory:
 * cmake .. -DRELEASE=2 -DPLATFORM=iotlab-m3
 * cmake .. -DRELEASE=2 -DPLATFORM=iotlab-a8-m3
 * -DRELEASE=2 to avoid any log_printf
 */
#include <stdint.h>
#include <string.h>
#include "printf.h"
#include "scanf.h"

#include "platform.h"

/* Drivers */
#include "unique_id.h"
#include "l3g4200d.h"
#include "lsm303dlhc.h"
#ifdef IOTLAB_M3
#include "lps331ap.h"
#include "isl29020.h"
#include "n25xxx.h"
#endif

#include "iotlab_gpio.h"
#include "iotlab_autotest_on.h"

#define COMMAND_LEN 256



#define ON_ERROR(msg, label)  do { \
    ret = msg; \
    goto label; \
} while (0)

/* Local variables */
static xQueueHandle radio_queue;

#define RADIO_TX_STR  "TX_PKT_HELLO_WORLD"

// L3G4200D ST Sensitivity specification page 10/42
#define GYR_SENS_8_75    (8.75e-3)  // for ±250dps   scale in dps/digit
// LSM303DLH ST Sensitivity specification page 11/47
#define ACC_SENS_2G      (1e-3)     // for ±2g       scale in g/digit
#define MAG_SENS_1_3_XY  (1/1055.)  // for ±1.3gauss scale in gauss/LSB
#define MAG_SENS_1_3_Z   (1/950.)   // for ±1.3gauss scale in gauss/LSB
#define ONE_SECOND  portTICK_RATE_MS * 1000




static unsigned int parse_power(char *power_str)
{
    /* valid PHY_POWER_ values for rf231 */

    if (strcmp("-17dBm", power_str) == 0)
        return PHY_POWER_m17dBm;
    if (strcmp("-12dBm", power_str) == 0)
        return PHY_POWER_m12dBm;

    if (strcmp("-9dBm", power_str) == 0)
        return PHY_POWER_m9dBm;
    if (strcmp("-7dBm", power_str) == 0)
        return PHY_POWER_m7dBm;
    if (strcmp("-5dBm", power_str) == 0)
        return PHY_POWER_m5dBm;
    if (strcmp("-4dBm", power_str) == 0)
        return PHY_POWER_m4dBm;
    if (strcmp("-3dBm", power_str) == 0)
        return PHY_POWER_m3dBm;
    if (strcmp("-2dBm", power_str) == 0)
        return PHY_POWER_m2dBm;
    if (strcmp("-1dBm", power_str) == 0)
        return PHY_POWER_m1dBm;

    if (strcmp("0dBm", power_str) == 0)
        return PHY_POWER_0dBm;
    if (strcmp("0.7dBm", power_str) == 0)
        return PHY_POWER_0_7dBm;

    if (strcmp("1.3dBm", power_str) == 0)
        return PHY_POWER_1_3dBm;
    if (strcmp("1.8dBm", power_str) == 0)
        return PHY_POWER_1_8dBm;

    if (strcmp("2.3dBm", power_str) == 0)
        return PHY_POWER_2_3dBm;
    if (strcmp("2.8dBm", power_str) == 0)
        return PHY_POWER_2_8dBm;

    if (strcmp("3dBm", power_str) == 0)
        return PHY_POWER_3dBm;

    return 255;
};


static void radio_rx_tx_done(phy_status_t status)
{
    int success = (PHY_SUCCESS == status);
    xQueueSend(radio_queue, &success, 0);
}


static char *radio_pkt(uint8_t channel, uint8_t tx_power)
{
    static phy_packet_t tx_pkt = {
        .data = tx_pkt.raw_data,
        .length = 125,
    };
    uint16_t i;
    for (i = 0; i < 125; i++) {
        tx_pkt.data[i] = i;
    }

    int success;
    char *ret = NULL;
    xQueueReceive(radio_queue, &success, 0);  // cleanup queue

    phy_idle(platform_phy);

    /*
     * config radio
     */
    if (phy_set_channel(platform_phy, channel))
        ON_ERROR("err_set_channel", radio_cleanup);
    if (phy_set_power(platform_phy, tx_power))
        ON_ERROR("err_set_power", radio_cleanup);

    /*
     * Send packet
     *
     * No interlocking because
     *     Current queue is EVENT_QUEUE_APPLI
     *     phy_ handlers are executed by EVENT_QUEUE_NETWORK
     */
    if (phy_tx_now(platform_phy, &tx_pkt, radio_rx_tx_done))
        ON_ERROR("err_tx", radio_cleanup);
    if (pdTRUE != xQueueReceive(radio_queue, &success, ONE_SECOND) || !success)
        ON_ERROR("tx_failed", radio_cleanup);

    // SUCCESS

radio_cleanup:
    phy_reset(platform_phy);
    return ret;
}

static char *radio_ping_pong(uint8_t channel, uint8_t tx_power)
{
    char *ret = NULL;
    int success;
    static phy_packet_t tx_pkt = {
        .data = tx_pkt.raw_data,
        .length = sizeof(RADIO_TX_STR),
        .raw_data = RADIO_TX_STR,
    };
    static phy_packet_t rx_pkt = {.data = rx_pkt.raw_data};

    xQueueReceive(radio_queue, &success, 0);  // cleanup queue

    phy_idle(platform_phy);

    /* config radio */
    if (phy_set_channel(platform_phy, channel))
        ON_ERROR("err_set_channel", ping_pong_cleanup);
    if (phy_set_power(platform_phy, tx_power))
        ON_ERROR("err_set_power", ping_pong_cleanup);

    /*
     * Send packet
     *
     * No interlocking because
     *     Current queue is EVENT_QUEUE_APPLI
     *     phy_ handlers are executed by EVENT_QUEUE_NETWORK
     */
    if (phy_tx_now(platform_phy, &tx_pkt, radio_rx_tx_done))
        ON_ERROR("err_tx", ping_pong_cleanup);
    if (pdTRUE != xQueueReceive(radio_queue, &success, ONE_SECOND) || !success)
        ON_ERROR("tx_failed", ping_pong_cleanup);

    /*
     * Wait for answer
     */
    memset(rx_pkt.raw_data, 0, sizeof(rx_pkt.raw_data));
    phy_rx_now(platform_phy, &rx_pkt, radio_rx_tx_done);
    if (pdTRUE != xQueueReceive(radio_queue, &success, ONE_SECOND) || !success)
        ON_ERROR("rx_timeout", ping_pong_cleanup);

    // Packet reception
    if (strcmp("RX_PKT_HELLO_WORLD", (char *)rx_pkt.raw_data))
        ON_ERROR("wrong_packet_received", ping_pong_cleanup);

    // SUCCESS
    ret = NULL;

ping_pong_cleanup:
    phy_reset(platform_phy);
    return ret;
}

#ifdef IOTLAB_M3

////////
static int test_flash_nand()
{
    static uint8_t buf_EE[256] = {[0 ... 255] = 0xEE};
    static uint8_t buf[256]    = {[0 ... 255] = 0x00};

    // Write subsector 200 and re-read it
    n25xxx_write_enable(); n25xxx_erase_subsector(0x00c80000);
    n25xxx_write_enable(); n25xxx_write_page(0x00c80000, buf_EE);
    n25xxx_read(0x00c80000, buf, sizeof(buf));
    n25xxx_write_enable(); n25xxx_erase_subsector(0x00c80000);

    // check read values
    return memcmp(buf_EE, buf, sizeof(buf));
}
////////


static int cmd_get_light(char *command)
{
    if (strcmp(command, "get_light"))
        return 1;
    printf("ACK get_light %f lux\n", isl29020_read_sample());
    return 0;
}

static int cmd_get_pressure(char *command)
{
    if (strcmp(command, "get_pressure"))
        return 1;

    uint32_t pressure;
    lps331ap_read_pres(&pressure);
    printf("ACK get_pressure %f mbar\n", pressure / 4096.0);
    return 0;
}

static int cmd_test_flash_nand(char *command)
{
    if (strcmp(command, "test_flash"))
        return 1;

    if (test_flash_nand())
        printf("NACK test_flash read_different_write\n");
    else
        printf("ACK test_flash\n");
    return 0;
}

#endif // IOTLAB_M3


/* Leds Commands */
static int cmd_leds_on(char *command)
{
    uint8_t leds = 0;
    if (1 != sscanf(command, "leds_on %u", &leds))
        return 1;

    leds_on(leds);
    printf("ACK leds_on %x\n", leds);
    return 0;
}

static int cmd_leds_off(char *command)
{
    uint8_t leds = 0;
    if (1 != sscanf(command, "leds_off %u", &leds))
        return 1;

    leds_off(leds);
    printf("ACK leds_off %x\n", leds);
    return 0;
}


static int cmd_leds_blink(char *command)
{
    uint8_t leds = 0;
    uint32_t period = 0;
    static soft_timer_t led_alarm;
    if (2 != sscanf(command, "leds_blink %u %u", &leds, &period))
        return 1;

    if (period) {
        soft_timer_set_handler(&led_alarm, (handler_t)leds_toggle,
                (handler_arg_t)(uint32_t)leds);
        soft_timer_start(&led_alarm, soft_timer_ms_to_ticks(period), 1);
        printf("ACK leds_blink %u %u\n", leds, period);
    } else {
        soft_timer_stop(&led_alarm);
        printf("ACK leds_blink stop\n");
    }
    return 0;
}

/* Leds Commands */


/* ON<->CN Commands */
static int cmd_test_i2c(char *command)
{
    if (strcmp(command, "test_i2c"))
        return 1;

    char *i2c2_err_msg = on_test_i2c2();
    if (NULL == i2c2_err_msg)
        printf("ACK test_i2c\n");
    else
        printf("NACK test_i2c %s\n", i2c2_err_msg);
    return 0;
}

static int cmd_test_gpio(char *command)
{
    if (strcmp(command, "test_gpio"))
        return 1;

    if (on_test_gpio())
        printf("NACK test_gpio\n");
    else
        printf("ACK test_gpio\n");
    return 0;
}

/* /ON<->CN Commands */


/* Simple Commands */
static int cmd_get_time(char *command)
{
    if (strcmp(command, "get_time"))
        return 1;
    printf("ACK get_time %u ticks_32khz\n", soft_timer_time());
    return 0;
}

static int cmd_get_uid(char *command)
{
    if (strcmp(command, "get_uid"))
        return 1;
    printf("ACK get_uid %08x%08x%08x\n", uid->uid32[0],
            uid->uid32[1], uid->uid32[2]);
    return 0;
}

static int cmd_echo(char *command)
{
    if (1 != sscanf(command, "echo %[^\n]", command))
        return 1;
    printf("%s\n", command);
    return 0;
}


/* /Simple Commands */

/* Get Sensor */

static int _cmd_get_xyz(char *command, char *get_cmd, char *unit,
        unsigned int (*sensor)(int16_t*),
        float x_factor, float y_factor, float z_factor)
{
    if (strcmp(command, get_cmd))
        return 1;

    int16_t xyz[3];
    if (sensor(xyz))
        printf("NACK %s error\n", get_cmd);
    else
        printf("ACK %s %f %f %f %s\n", get_cmd,
                xyz[0] * x_factor, xyz[1] * y_factor, xyz[2] * z_factor,
                unit);
    return 0;
}


static int cmd_get_gyro(char *command)
{
    return _cmd_get_xyz(command, "get_gyro", "dps", l3g4200d_read_rot_speed,
            GYR_SENS_8_75, GYR_SENS_8_75, GYR_SENS_8_75);
}

static int cmd_get_accelero(char *command)
{
    return _cmd_get_xyz(command, "get_accelero", "g", lsm303dlhc_read_acc,
            ACC_SENS_2G, ACC_SENS_2G, ACC_SENS_2G);
}

static int cmd_get_magneto(char *command)
{
    return _cmd_get_xyz(command, "get_magneto", "gauss", lsm303dlhc_read_mag,
            MAG_SENS_1_3_XY, MAG_SENS_1_3_XY, MAG_SENS_1_3_Z);
}

/* /Get Sensor */

/* Radio */

static int cmd_radio_pkt(char *command)
{
    char power[8] = {'\0'};
    uint8_t channel, tx_power;

    if (2 != sscanf(command, "radio_pkt %u %8s", &channel, power))
        return 1;
    if (11 > channel || channel > 26)
        return 1;
    tx_power = parse_power(power);
    if (255 == tx_power)
        return 1;

    char *err_msg = radio_pkt(channel, tx_power);
    if (NULL == err_msg)
        printf("ACK radio_pkt %u %s\n", channel, power);
    else
        printf("NACK radio_pkt %s\n", err_msg);
    return 0;
}


static int cmd_radio_ping_pong(char *command)
{
    char power[8];
    uint8_t channel, tx_power;

    if (2 != sscanf(command, "radio_ping_pong %u %8s", &channel, power))
        return 1;
    if (11 > channel || channel > 26)
        return 1;
    tx_power = parse_power(power);
    if (255 == tx_power)
        return 1;

    char *err_msg = radio_ping_pong(channel, tx_power);
    if (NULL == err_msg)
        printf("ACK radio_ping_pong %u %s\n", channel, tx_power);
    else
        printf("NACK radio_ping_pong %s\n", err_msg);
    return 0;
}

/* /Radio */


/* GPS */
#ifdef IOTLAB_A8_M3

static volatile uint32_t seconds = 0;
static void pps_handler_irq(handler_arg_t arg)
{
    (void)arg;
    seconds++;
}

static int cmd_test_pps_start(char *command)
{
    if (strcmp(command, "test_pps_start"))
        return 1;
    // third gpio line
    seconds = 0;
    gpio_enable_irq(&gpio_config[3], IRQ_RISING, pps_handler_irq, NULL);
    printf("ACK test_pps_start\n");
    return 0;
}

static int cmd_test_pps_stop(char *command)
{
    if (strcmp(command, "test_pps_stop"))
        return 1;
    // third gpio line
    seconds = 0;
    gpio_disable_irq(&gpio_config[3]);
    printf("ACK test_pps_stop\n");
    return 0;
}

static int cmd_test_pps_get(char *command)
{
    if (strcmp(command, "test_pps_get"))
        return 1;
    printf("ACK test_pps_get %d pps\n", seconds);
    return 0;
}

#endif
/* /GPS */


/* command_func_t
 * Return 0 if command has been handled
 * Return non zero if the command was not for us
 */
typedef int (*command_func_t)(char *);
static command_func_t commands_handlers[] = {

    cmd_get_time,
    cmd_get_uid,
    cmd_echo,

    cmd_leds_on,
    cmd_leds_off,
    cmd_leds_blink,

    cmd_get_accelero,
    cmd_get_magneto,
    cmd_get_gyro,

    cmd_test_i2c,
    cmd_test_gpio,

    cmd_radio_pkt,
    cmd_radio_ping_pong,

#ifdef IOTLAB_M3
    cmd_get_light,
    cmd_get_pressure,
    cmd_test_flash_nand,
#endif

#ifdef IOTLAB_A8_M3
    cmd_test_pps_start,
    cmd_test_pps_stop,
    cmd_test_pps_get,
#endif

    NULL
};


static void parse_command(handler_arg_t arg)
{
    char *command_buffer = arg;
    command_func_t *callback = NULL;

    for (callback = commands_handlers; *callback != NULL; callback++) {
        if (0 == (*callback)(command_buffer))
            return;  // command handled
    }
    printf("NACK invalid_command '%s'\n", command_buffer);
}


static void char_handler_irq(handler_arg_t arg, uint8_t c)
{
    static char command_buffer[COMMAND_LEN];
    static size_t index = 0;

    if (('\n' != c) && ('\r' != c))
        command_buffer[index++] = c;

    // line full or new line
    if (('\n' == c) || (COMMAND_LEN == index)) {
        command_buffer[index] = '\0';
        index = 0;
        event_post_from_isr(EVENT_QUEUE_APPLI, parse_command, command_buffer);
    }
}

int main(void)
{
    platform_init();
    soft_timer_init();
    event_init();

    uart_set_rx_handler(uart_print, char_handler_irq, NULL);

    radio_queue = xQueueCreate(1, sizeof(int));  // radio sync queue

    //init sensor
#ifdef IOTLAB_M3
    isl29020_prepare(ISL29020_LIGHT__AMBIENT, ISL29020_RESOLUTION__16bit,
            ISL29020_RANGE__1000lux);
    isl29020_sample_continuous();

    lps331ap_powerdown();
    lps331ap_set_datarate(LPS331AP_P_12_5HZ_T_12_5HZ);
#endif

    l3g4200d_powerdown();
    l3g4200d_gyr_config(L3G4200D_200HZ, L3G4200D_250DPS, true);

    lsm303dlhc_powerdown();
    lsm303dlhc_mag_config(
            LSM303DLHC_MAG_RATE_220HZ, LSM303DLHC_MAG_SCALE_2_5GAUSS,
            LSM303DLHC_MAG_MODE_CONTINUOUS, LSM303DLHC_TEMP_MODE_ON);
    lsm303dlhc_acc_config(
            LSM303DLHC_ACC_RATE_400HZ, LSM303DLHC_ACC_SCALE_2G,
            LSM303DLHC_ACC_UPDATE_ON_READ);

    platform_run();
    return 1;
}

