/*
 * Simple radio (from ../ping_pong)
 */

void radio_init(void (*receive_callback)(char *from, char* data));
void radio_send(char *data);
char * const radio_local_address();
