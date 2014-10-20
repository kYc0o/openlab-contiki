#include <stdint.h>
#include <string.h>
#include "printf.h"
#include "scanf.h"
#include "platform.h"
#include "queue.h"


#define COMMAND_LEN (256)
enum mode_t { MODE_TX, MODE_RX };

#define CONFIG_RADIO "config_radio"
#define SEND_PKTS "send_packets"


/* Local variables */
// static version of phy_preprare_packet
static phy_packet_t rx_pkt = {.data = rx_pkt.raw_data };
static phy_packet_t tx_pkt = {.data = tx_pkt.raw_data };
static xQueueHandle cmd_queue;
static xQueueHandle radio_tx_queue;


/* Current configuration */
static struct {
    char power_str[8];
    uint8_t tx_power;
    uint8_t channel;

    char node_id[16];
    uint16_t num_pkts;
    uint16_t delay;

    enum mode_t radio_mode;

} conf;


/* Function Protypes */
static void char_handler_irq(handler_arg_t arg, uint8_t c);
static void parse_command_task(void *param);

static void radio_listen();

static void packet_received(handler_arg_t arg);

static int send_one_packet(char *node_id, char *power_str, int num);

/*
 * commands
 */
static void configure_radio(handler_arg_t arg)
{
    int ret = (int)arg;
    if (ret) {
        printf(CONFIG_RADIO " NACK %d\n", ret);
        return;
    }
    conf.radio_mode = MODE_RX;

    phy_idle(platform_phy);

    phy_set_channel(platform_phy, conf.channel);
    radio_listen();

    printf(CONFIG_RADIO " ACK\n");
}



static void send_packets(handler_arg_t arg)
{
    int ret = (int)arg;

    if (ret) {
        printf(SEND_PKTS " NACK %d\n", ret);
        return;
    }

    /* Enter TX mode */
    conf.radio_mode = MODE_TX;
    phy_idle(platform_phy);

    phy_set_power(platform_phy, conf.tx_power);

    /* Send num_pkts packets and count errors */
    int failure_count = 0;
    int i;
    for (i = 0; i < conf.num_pkts; i++) {
        if (send_one_packet(conf.node_id, conf.power_str, i))
            failure_count++;
        soft_timer_delay_ms(conf.delay);
    }

    /* restart RX */
    conf.radio_mode = MODE_RX;
    radio_listen();

    printf(SEND_PKTS " ACK %u\n", failure_count);
}

static void radio_tx_done(phy_status_t status);
static int send_one_packet(char *node_id, char *power_str, int num)
{
    int success = 0;
    int ret;

    // setup packet
    snprintf((char *)tx_pkt.data, 125, "%s %s %u", node_id, power_str, num);
    tx_pkt.length = strlen((char *)tx_pkt.data) + 1;

    // send packet
    phy_idle(platform_phy);
    phy_tx_now(platform_phy, &tx_pkt, radio_tx_done);
    // wait tx done
    ret  = (pdTRUE == xQueueReceive(radio_tx_queue, &success,
                1000 * portTICK_RATE_MS));
    ret &= success;

    return (ret ? 0 : 1);
}
static void radio_tx_done(phy_status_t status)
{
    int success = (PHY_SUCCESS == status);
    xQueueSend(radio_tx_queue, &success, 0);
}


/*
 * Radio RX
 */
static void packet_received_net_handler(phy_status_t status);
static void radio_listen()
{
    if (MODE_RX == conf.radio_mode) {
        /* keep receiving packets */
        phy_idle(platform_phy);
        phy_rx_now(platform_phy, &rx_pkt, packet_received_net_handler);
    }
}
static void packet_received_net_handler(phy_status_t status)
{
    /* Printf should be done from EVENT_QUEUE_APPLI */
    event_post(EVENT_QUEUE_APPLI, packet_received, (handler_arg_t)status);
}
static void packet_received(handler_arg_t arg)
{
    phy_status_t status = (phy_status_t) arg;
    if (PHY_SUCCESS == status) {
        printf("radio_rx %s %d %u sender power num rssi lqi\n",
                rx_pkt.data, rx_pkt.rssi, rx_pkt.lqi);
    } else {
        printf("radio_rx_error 0x%x\n", status);
    }
    radio_listen();
}


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



static void parse_command_task(void *param)
{
    static char command_buffer[COMMAND_LEN];
    char *cmd;
    char *args;

    int ret;
    int valid_command;

    while (1) {
        if (pdTRUE != xQueueReceive(cmd_queue, command_buffer, portMAX_DELAY))
            continue;

        cmd = strtok_r(command_buffer, " \n\r", &args);
        if (cmd == NULL)
            continue;


        if (strcmp(CONFIG_RADIO, cmd) == 0) {
            // "config_radio -c <channel>"
            ret = sscanf(args, "-c %u", &conf.channel);

            valid_command = ((1 == ret) &&
                    (conf.channel >= 11) && (conf.channel <= 26));

            event_post(EVENT_QUEUE_APPLI, configure_radio, (handler_arg_t)
                    (valid_command ? 0 : 1));

        } else if (strcmp(SEND_PKTS, cmd) == 0) {
            // "send_packet -i <current_node_id>
            //              -p <tx_power:XXdBm>
            //              -n <num_packets>
            //              -d <delay_ms>"
            ret = sscanf(args, "-i %s -p %s -n %u -d %u\n",
                    conf.node_id, conf.power_str,
                    &conf.num_pkts, &conf.delay);
            conf.tx_power =  parse_power(conf.power_str);

            valid_command = ((4 == ret) && (conf.tx_power != 255));

            event_post(EVENT_QUEUE_APPLI, send_packets, (handler_arg_t)
                    (valid_command ? 0 : 1));
        } else {
            printf("invalid_command\n");
            continue;
        }
    }
}


static void char_handler_irq(handler_arg_t arg, uint8_t c)
{
    static char command_buffer[COMMAND_LEN];
    static size_t index = 0;

    portBASE_TYPE yield;

    command_buffer[index++] = c;

    // line full or new line
    if (('\n' == c) || (COMMAND_LEN == index)) {
        command_buffer[index] = '\0';
        index = 0;

        xQueueSendFromISR(cmd_queue, command_buffer, &yield);

        if (yield)
            portYIELD();
    }
}

int main(void) {
    /* Setup the hardware. */
    platform_init();
    soft_timer_init();

    cmd_queue      = xQueueCreate(2, 256);  // command queue
    radio_tx_queue = xQueueCreate(8, sizeof(int));


    xTaskCreate(parse_command_task, (const signed char *const) "Parse command",
            4 * configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    uart_set_rx_handler(uart_print, char_handler_irq, NULL);

    platform_run();
    return 1;
}
