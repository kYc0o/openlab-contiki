#include "iotlab_uid_num_hashtable.h"
#include <stdlib.h>

#define LOG_LEVEL LOG_LEVEL_DISABLED
//#define LOG_LEVEL LOG_LEVEL_DEBUG
#include <debug.h>



struct node node_from_uid(uint16_t uid)
{
    log_info("node_from_uid(0x%x)", uid);
    struct node found_node = {0, 0};
    size_t first, last, cur;
    const struct node_entry *current;

    first = 0;
    last = (sizeof(nodes_uid_dict) / sizeof(struct node_entry)) -1;
    // Dichotomic search
    while (first <= last)  {

        cur = (first + last) / 2;
        current = &nodes_uid_dict[cur];
        log_debug("%4u <= %4u <= %4u : 0x%x", first, cur, last, current->uid);
        if (current->uid < uid) {
            first = cur + 1;
        } else if (current->uid > uid) {
            last = cur - 1;
        } else { // Equals, found it
            found_node.type = 0xf & (current->node >> 24);
            found_node.num  = 0x0fff & current->node;
            log_info("Node %u-%u found", found_node.type, found_node.num);
            return found_node;
        }
    }
    log_warning("Node not found");
    return found_node;
}
