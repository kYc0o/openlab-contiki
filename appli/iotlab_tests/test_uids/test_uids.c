#include "platform.h"
#include "soft_timer.h"
#include "unique_id.h"
#include "printf.h"
#include "iotlab_uid.h"
#include "iotlab_uid_num_hashtable.h"

static void print_uids(void *args)
{
    uint16_t id;
    printf("Chip UIDs extraction:\n");
    printf("Working as little endian as platform arch\n");

    printf("FULL UID 32: %08x:%08x:%08x\n",
            uid->uid32[2], uid->uid32[1], uid->uid32[0]
            );
    printf("FULL UID 16: %04x%04x:%04x%04x:%04x%04x\n",
            uid->uid16[5], uid->uid16[4],
            uid->uid16[3], uid->uid16[2],
            uid->uid16[1], uid->uid16[0]
            );

    printf("FULL UID  8: %02x%02x%02x%02x:%02x%02x%02x%02x:%02x%02x%02x%02x\n",
            uid->uid8[11], uid->uid8[10], uid->uid8[9], uid->uid8[8],
            uid->uid8[7], uid->uid8[6], uid->uid8[5], uid->uid8[4],
            uid->uid8[3], uid->uid8[2], uid->uid8[1], uid->uid8[0]
            );

    printf("Processed UID used in platform:\n");
    id = iotlab_uid();
    printf("iotlab_uid() == %02x:%02x\n", id >> 8, id & 0xFF);


    printf("platform_uid: 0x%x\n", platform_uid);
    if (platform_uid) {
        id = platform_uid();
        printf("platform_uid() == %02x:%02x\n", id >> 8, id & 0xFF);
    }


    struct node my_node = node_from_uid(id);
    printf("node_from_uid(id) == %01x:%03x\n", my_node.type, my_node.num);
    char *node_str;

    switch (my_node.type) {
    case M3:
        node_str = "m3";
        break;
    case A8:
        node_str = "a8";
        break;
    default:
        node_str = "Uknown type";
        break;
    }
    printf("My node is: %x%03x: %s-%u\n", my_node.type, my_node.num,
            node_str, my_node.num);


    printf("\n");
}


int main(void)
{
    static soft_timer_t print_timer;

    platform_init();
    soft_timer_init();

    soft_timer_set_handler(&print_timer, print_uids, NULL);
    soft_timer_start(&print_timer, soft_timer_s_to_ticks(2), 1);

    platform_run();
    return 1;
}
