#include "platform.h"
#include "soft_timer.h"
#include "unique_id.h"
#include "printf.h"
#include "iotlab_uid.h"
#include "iotlab_uid_num_hashtable.h"

static void print_uids(void *args)
{
    {
        printf("%17s == %04x\n", "iotlab_uid()", iotlab_uid());
    }

    // Print platform_uid if it has been implemented
    if (platform_uid) {
        printf("%17s == %04x\n", "platform_uid()", platform_uid());
    }

    struct node my_node = node_from_uid(iotlab_uid());
    {
        printf("%17s == %01x:%03x\n", "node_from_uid(id)",
                my_node.type, my_node.num);
    }
    if (my_node.num != 0) {
        char *node_str;

        switch (my_node.type) {
        case M3:
            node_str = "m3";
            break;
        case A8:
            node_str = "a8";
            break;
        default:
            node_str = "invalid";
            break;
        }
        printf("%17s == %s-%u\n", "node_str", node_str, my_node.num);
    } else {
        printf("Unknown node\n");
    }

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
