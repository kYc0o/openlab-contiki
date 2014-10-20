/*
 * Ping Pong game using 2 agile-fox rackets
 *
 * Blinker widget (the ball)
 *
 */

#include <stdbool.h>


void blinker_start();
void blinker_stop();
bool blinker_is_blinking();

void blinker_set_period_ms(int period_ms);
void blinker_set_led(int led_id);
void blinker_toggle_blinking();
