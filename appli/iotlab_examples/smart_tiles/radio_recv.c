#include <printf.h>
#include "radio.h"

static void receive_callback(char *from, char* data)
{
	printf("Radio recv: from=%s data=%s\n", from, data);
}

void radio_recv_init()
{
	radio_init(receive_callback);
}
