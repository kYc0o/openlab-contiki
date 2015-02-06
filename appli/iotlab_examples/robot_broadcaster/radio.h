/*
 * Simple radio (from ../ping_pong)
 */

void radio_init(void (*receive_callback)(char *from, char* data, int rssi, int lqi));
void radio_send(char *data);
char * const radio_local_address();
