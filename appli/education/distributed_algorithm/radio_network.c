#include <string.h>
#include <stddef.h>
#include "soft_timer.h"
#include "radio_network.h"
#include "mac_csma.h"


uint16_t neighbours[MAX_NUM_NEIGHBOURS] = {0};
uint32_t num_neighbours = 0;

static struct {
    uint32_t channel;
    uint32_t discovery_tx_power;
    uint32_t communication_tx_power;
} rn_config;

enum packet_type {
    PKT_GRAPH = 0,
    PKT_NEIGH = 1,
    PKT_VALUES = 2,
};


#define ADDR_BROADCAST 0xFFFF

void network_init(uint32_t channel, uint32_t discovery_tx_power,
        uint32_t communication_tx_power)
{
    network_reset();

    rn_config.channel = channel;
    rn_config.discovery_tx_power = discovery_tx_power;
    rn_config.communication_tx_power = communication_tx_power;

    mac_csma_init(rn_config.channel, rn_config.communication_tx_power);
}

void network_reset()
{
    memset(neighbours, 0, sizeof(neighbours));
    num_neighbours = 0;
    mac_csma_init(rn_config.channel, rn_config.communication_tx_power);
}

struct msg_send
{
    int try;
    uint16_t addr;
    uint8_t pkt[MAC_PKT_LEN];
    size_t length;
};


static void do_send(handler_arg_t arg)
{
    struct msg_send *send_cfg = (struct msg_send *)arg;
    int ret;

    ret = mac_csma_data_send(send_cfg->addr, send_cfg->pkt, send_cfg->length);
    if (ret != 0) {
        DEBUG("Sent to %04x try %u\n", send_cfg->addr, send_cfg->try);
    } else {
        ERROR("Send to %04x failed, try %u. Retrying\n",
                send_cfg->addr, send_cfg->try);
        if (send_cfg->try++ < 5)
            event_post(EVENT_QUEUE_APPLI, do_send, arg);
        //soft_timer_ms_to_ticks(20);
    }

}

static void send(uint16_t addr, void *packet, size_t length)
{
    static struct msg_send send_cfg;
    send_cfg.try = 0;
    send_cfg.addr = addr;
    send_cfg.length = length;
    memcpy(&send_cfg.pkt, packet, length);

    event_post(EVENT_QUEUE_APPLI, do_send, &send_cfg);
}

/*
 * Generic neighbours functions
 */
void network_neighbours_print()
{
    int i;
    MSG("Neighbours;%u", num_neighbours);
    for (i = 0; i < MAX_NUM_NEIGHBOURS; i++) {
        if (neighbours[i])
            printf(";%04x", neighbours[i]);
        else
            break; // no more neighbours
    }
    printf("\n");
}

static int network_neighbour_id(uint16_t src_addr)
{
    int i;
    for (i = 0; i < MAX_NUM_NEIGHBOURS; i++) {
        uint16_t neighbour_addr = neighbours[i];
        if (neighbour_addr == src_addr)
            return i;
        else if (neighbour_addr == 0)
            return -1;
    }
    return -1;
}


void network_set_low_tx_power()
{
    mac_csma_init(rn_config.channel, rn_config.discovery_tx_power);
}
void network_set_high_tx_power()
{
    mac_csma_init(rn_config.channel, rn_config.communication_tx_power);
}

/*
 * Neighbours discovery
 */
void network_neighbours_discover()
{
    uint8_t pkt = PKT_GRAPH;
    send(ADDR_BROADCAST, &pkt, 1);
}

static void network_neighbours_add(uint16_t src_addr, int8_t rssi)
{
    if (rssi < MIN_RSSI) {
        DEBUG("DROP neighbour %04x. rssi: %d\n", src_addr, rssi);
        return;
    } else {
        DEBUG("ADD  neighbour %04x. rssi: %d\n", src_addr, rssi);
    }

    int i;
    for (i = 0; i < MAX_NUM_NEIGHBOURS; i++) {
        uint16_t neighbour_addr = neighbours[i];
        if (neighbour_addr == 0) {
            neighbours[i] = src_addr;
            num_neighbours++;
            break;
        } else if (neighbour_addr == src_addr) {
            break;
        } else {
            continue;
        }

    }
}


/*
 * Validate who are your neighbours
 */
struct neighbours_pkt {
    uint8_t type;
    uint16_t neighbours[MAX_NUM_NEIGHBOURS];
};

void network_neighbours_acknowledge()
{
    struct neighbours_pkt pkt;

    memset(&pkt, 0, sizeof(pkt));
    pkt.type = PKT_NEIGH;
    memcpy(&pkt.neighbours, neighbours, sizeof(neighbours));
    send(ADDR_BROADCAST, &pkt, sizeof(pkt));
}

static void network_neighbours_validate(uint16_t src_addr,
        const uint8_t *data, size_t length)
{
    if (sizeof(struct neighbours_pkt) != length)
        ERROR("Invalid neighbours pkt len\n");
    struct neighbours_pkt pkt;
    memcpy(&pkt, data, sizeof(struct neighbours_pkt));

    const uint16_t my_id = iotlab_uid();

    int i;
    // Add 'src_addr' has a neighbour if I'm his neighbourg
    for (i = 0; i < MAX_NUM_NEIGHBOURS; i++) {
        const uint16_t cur_id = pkt.neighbours[i];
        if (cur_id == my_id)
            network_neighbours_add(src_addr, INT8_MAX);
        else if (cur_id == 0)
            break; // no more neighbours in pkt
    }
}

/*
 * Values management
 */
struct values_pkt {
    uint8_t type;
    uint8_t should_compute;
    uint32_t num_neighbours;
    struct values values;
};

void network_send_values(uint8_t should_compute, struct values *values)
{
    struct values_pkt pkt;
    memset(&pkt, 0, sizeof(pkt));

    // Header
    pkt.type           = PKT_VALUES;
    pkt.should_compute = should_compute;
    pkt.num_neighbours = num_neighbours;
    // Values
    memcpy(&pkt.values, values, sizeof(struct values));

    send(ADDR_BROADCAST, &pkt, sizeof(struct values_pkt));
}

static void handle_value(uint16_t src_addr, const uint8_t *data, size_t length)
{
    if (sizeof(struct values_pkt) != length)
        ERROR("Invalid Values pkt len\n");

    int index = network_neighbour_id(src_addr);
    if (index == -1) {
        DEBUG("Values from %04x: not neighbour\n", src_addr);
        return;
    } else {
        DEBUG("Values from %04x\n", src_addr);
    }
    struct values_pkt pkt;
    memcpy(&pkt, data, sizeof(struct values_pkt));

    struct received_values *neigh_values = &neighbours_values[index];
    neigh_values->valid = 1;
    neigh_values->num_neighbours = pkt.num_neighbours;
    memcpy(&neigh_values->values, &pkt.values, sizeof(struct values));

    // Gossip mode, compute after each measures received
    if (pkt.should_compute)
        compute_all_values_from_gossip(neigh_values);
}


/*
 * Packet reception
 */
void mac_csma_data_received(uint16_t src_addr,
        const uint8_t *data, uint8_t length, int8_t rssi, uint8_t lqi)
{
    uint8_t pkt_type = data[0];
    DEBUG("pkt received from %04x\n", src_addr);

    switch (pkt_type) {
    case (PKT_GRAPH):
        network_neighbours_add(src_addr, rssi);
        break;
    case (PKT_NEIGH):
        network_neighbours_validate(src_addr, data, length);
        break;
    case (PKT_VALUES):
        handle_value(src_addr, data, length);
        break;
    default:
        INFO("Unknown pkt type %01x from %04x\n", pkt_type, src_addr);
        break;
    }
}
