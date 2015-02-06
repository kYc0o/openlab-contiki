#include <platform.h>
#include <stdint.h>
#include <stdlib.h>
#include <printf.h>
#include <string.h>

#include "phy.h"
#include "soft_timer.h"
#include "event.h"
#include "computing.h"
#include "radio_network.h"
#include "iotlab_uid.h"
#include "config.h"





// UART callback function
static void char_rx(handler_arg_t arg, uint8_t c);
static void handle_cmd(handler_arg_t arg);


static void print_values()
{
    int i;
    MSG("Values;%u", compute_number);
    for (i = 0; i < NUM_VALUES; i++)
        printf(";%f", my_values.v[i]);
    printf("\n");
}

static void print_final_value()
{
    uint32_t final_value = compute_final_value();
    MSG("FinalValue;%u;%u\n", compute_number, final_value);
}


static void compute_all_values()
{
    int i;
    compute_number++;
    num_neighbours = 0;
    for (i = 0; i < MAX_NUM_NEIGHBOURS; i++) {
        if (neighbours_values[i].valid)
            num_neighbours++;
    }
    for (i = 0; i < NUM_VALUES; i++) {
        my_values.v[i] = compute_value_from_neighbours(
                my_values.v[i], num_neighbours, neighbours_values, i);
    }

    // Reset values for next run
    memset(neighbours_values, 0, sizeof(neighbours_values));
}


void init_values()
{
    memset(neighbours_values, 0, sizeof(neighbours_values));
    compute_number = 0;

    network_reset();

    memset(&my_values, 0, sizeof(my_values));
    int i;
    for (i = 0; i < NUM_VALUES; i++)
        my_values.v[i] = init_value();
}

static void print_usage()
{
    printf("\n\nIoT-LAB Simple Demo program\n");
    printf("Type command\n");
    printf("\th:\tprint this help\n");
    printf("\n");
    printf("\tt:\tSet low tx power\n");
    printf("\tT:\tSet high tx power\n");
    printf("\n");
    printf("\ti:\tReset neighbours and init sensor value\n");
    printf("\n");
    printf("\tg:\tcreate connection graph for this node\n");
    printf("\tG:\tValidate Graph with neighbours\n");
    printf("\tp:\tprint neighbours table\n");
    printf("\n");
    printf("\ts:\tsend values to neighbours\n");
    printf("\tS:\tsend values to neighbours and make them compute a new value\n");
    printf("\n");
    printf("\tc:\tcompute values received from all neighbours\n");
    printf("\n");
    printf("\tv:\tprint current node values\n");
    printf("\tV:\tprint a calculated final int value\n");
}

static void handle_cmd(handler_arg_t arg)
{
    switch ((char) (uint32_t) arg) {
        case 't':
            network_set_low_tx_power();
            break;
        case 'T':
            network_set_high_tx_power();
            break;

        case 'g':
            network_neighbours_discover();
            break;
        case 'G':
            network_neighbours_acknowledge();
            break;
        case 'p':
            network_neighbours_print();
            break;

        case 'i':
            init_values();
            break;
        case 'c':
            compute_all_values();
            break;

        case 's':
            network_send_values(0, &my_values);
            break;
        case 'S':
            network_send_values(1, &my_values);
            break;
        case 'v':
            print_values();
            break;
        case 'V':
            print_final_value();
            break;
        case '\n':
            break;
        case 'h':
        default:
            print_usage();
            break;
    }
}

int main()
{
    platform_init();
    event_init();
    soft_timer_init();

    // Uart initialisation
    uart_set_rx_handler(uart_print, char_rx, NULL);
    // Radio communication init
    network_init(CHANNEL, GRAPH_RADIO_POWER, RADIO_POWER);
    // init values at start
    init_values();

    platform_run();
    return 0;
}


/* Reception of a char on UART and store it in 'cmd' */
static void char_rx(handler_arg_t arg, uint8_t c) {
    // disable help message after receiving char
    event_post_from_isr(EVENT_QUEUE_APPLI, handle_cmd,
            (handler_arg_t)(uint32_t) c);
}

