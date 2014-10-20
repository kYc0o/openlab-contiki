/*
 * test_nico.c
 *
 *  Created on: Jul 24, 2013
 *      Author: Nicolas Turro <nicolas.turro.at.inria.fr>
 */

#include "platform.h"
#include "iotlab-a8-m3/iotlab-a8-m3.h"
#include "printf.h"
#include "gpio.h"
#include "afio.h"
#include "nvic_.h"
#include "phy.h"
#include "unique_id.h"
#include "debug.h"
#include "soft_timer.h"

// our STM32F103 runs at 72Mhz, 72000065 is an average measurement from a external GPS PPS source.
#define CYCLESPERSEC 72000065.0
#define CYCLES2NS 1000000000.0/CYCLESPERSEC

#define RADIO_CHANNEL 14
#define TX_PERIOD soft_timer_ms_to_ticks(2000)

/** Software Timer to periodically send a packet */
static soft_timer_t tx_timer;
/** Packet for sending */
static phy_packet_t tx_packet;
/** Packet for receiving */
static phy_packet_t rx_packet;

/** Counter incremented at each send packet */
static uint32_t tx_count = 0;

#define START 0
#define MAGIC1 1
#define MAGIC2 2
#define TIMEOK 3
#define MAGICCHAR1 0x16
#define MAGICCHAR2 0x3

//  DWT_CYCNT counter is used instead of SysTick because it's a 32bits counter which overflows once a minute whereas
// SysTick overflows every 234 ms
volatile unsigned int *DWT_CYCCNT     = (volatile unsigned int *)0xE0001004; //address of the register
volatile unsigned int *DWT_CONTROL    = (volatile unsigned int *)0xE0001000; //address of the register
volatile unsigned int *SCB_DEMCR         = (volatile unsigned int *)0xE000EDFC; //address of the register

volatile uint32_t seconds=0;
volatile int A8comState=START;
volatile int tx_done=1;

typedef struct {
  uint32_t sec;
  uint32_t nsec;
} time_t;


// return the time elapsed since the beginning of the application
void getTime(time_t * curtime){
  // the seconds are updated by the PPShandler, nothing to do
  (*curtime).sec=seconds;
  // the nsec are computed from the number of cycles elapsed since the last second
  (*curtime).nsec=*DWT_CYCCNT*(double)CYCLES2NS;
}

void PPShandler(handler_arg_t arg) {
  (void)arg;
  // we recieve this interrupt each second from the GPS PPS
  // we recieve this interrupt each second from the GPS PPS
  // so we increment the number of seconds elapsed from the beginning
  log_debug("Got PPS\n");
  seconds++;
  // and we reset the cycle counter
  *DWT_CYCCNT = 0;
}

static void rx_done_isr(phy_status_t status) {
  switch (status) {
    case PHY_SUCCESS:
      //event_post_from_isr(EVENT_QUEUE_APPLI, process_rx_packet, NULL);d
      log_debug("Got packet\n");
      break;
    default:
      // Bad packet
      //event_post_from_isr(EVENT_QUEUE_APPLI, enter_rx, NULL);
      log_debug("Bad packet\n");
      break;
    }
}

/** Enter RX state */
static void enter_rx(handler_arg_t arg) {
  // Set PHY IDLE, then enter RX
  phy_idle(platform_phy);
  phy_set_channel(platform_phy, RADIO_CHANNEL);
  // Prepare packet and start RX
  phy_prepare_packet(&rx_packet);
  phy_rx_now(platform_phy, &rx_packet, rx_done_isr);
}

void char_rx(handler_arg_t arg, uint8_t c)  {
  static union {
          uint8_t c[4];
          uint32_t i;
  } A8buf;
  static int A8comCount;

  // asm volatile ("cpsid i");

  switch (A8comState)  {
  case START :
    if (c==MAGICCHAR1) A8comState=MAGIC1;
    break;
  case MAGIC1 :
    if (c==MAGICCHAR2) {
      A8comState=MAGIC2;
      A8comCount=0;
    }
    else A8comState=START;
    break;
  case MAGIC2 :
    A8buf.c[A8comCount]=c;
    A8comCount++;
    if (A8comCount==4) {
      seconds = A8buf.i;
      A8comState=TIMEOK;
    }
    break;
  case TIMEOK :
    if (c==MAGICCHAR1) A8comState=MAGIC1;
    break;
  }
  //asm volatile ("cpsie i");
}

static void tx_done_isr(phy_status_t status)
{
    // TX is done, start RX again
  if (status != PHY_SUCCESS) log_debug("Error while sending %x", status);
  // else log_debug("Frame sent at %u, length: %u", tx_packet.timestamp, tx_packet.length);
}

static void send_packet()
{
  time_t curtime;
  phy_status_t phy_status;

  tx_done=0;
  // Set IDLE
  phy_idle(platform_phy);

  // Prepare the packet and place
  phy_prepare_packet(&tx_packet);
  getTime(&curtime);
  // Create a payload
  tx_packet.length = snprintf((char*) tx_packet.data, PHY_MAX_TX_LENGTH,
			      "%02x:%02x:%02x,%u,%09u.%09u", uid->uid8[9], uid->uid8[10],
			      uid->uid8[11], tx_count, curtime.sec,curtime.nsec);

  // Perform a CCA
  int32_t cca;
  phy_cca(platform_phy, &cca);
  //  uint32_t t = soft_timer_time() + soft_timer_ms_to_ticks(10);
  if (cca)
    {
      // Channel is clear
      // Send
      phy_status = phy_tx_now(platform_phy, &tx_packet, tx_done_isr);
      if (PHY_SUCCESS == phy_status)
              printf("packet %d sent at %d.%09d\n", tx_count, curtime.sec,
                              curtime.nsec);
      else
              printf("Error %x while sending packet %d sent at %d.%09d\n",
                              phy_status, tx_count, curtime.sec, curtime.nsec);
      // Increment counter
      tx_count++;
    }
  else
    {
      log_debug("TX aborted, channel is busy\n");
      enter_rx(NULL);
    }
}

int main() {

  platform_init();
  soft_timer_init();

  printf("nico test 1\n===================\n");
  printf("Waiting time message from A8\n");
  uart_set_rx_handler(uart_print, char_rx,NULL);

  while(A8comState!=TIMEOK){
    asm volatile ("nop");
  }
  printf("Got time from A8 : %us\n",seconds);
  // Init cycle counter
  *SCB_DEMCR = *SCB_DEMCR | 0x01000000;
  *DWT_CYCCNT = 0; // reset the counter
  *DWT_CONTROL = *DWT_CONTROL | 1 ; // enable the counter

  // GPS PPS output is wired to GPIO PA3
  gpio_set_input(gpioA, GPIO_PIN_3);
  gpio_enable(gpioA);

  // link GPIO PA3 to IRQ_LINE_EXTI3
  afio_select_exti_pin(EXTI_LINE_Px3, AFIO_PORT_A);
  nvic_enable_interrupt_line(NVIC_IRQ_LINE_EXTI3);

  // set IRQ handler and configure call to RISING signal on PA3
  exti_set_handler(EXTI_LINE_Px3, PPShandler, NULL);
  exti_enable_interrupt_line(EXTI_LINE_Px3, EXTI_TRIGGER_RISING);


  phy_reset(platform_phy);
  phy_set_channel(platform_phy, RADIO_CHANNEL);

  // Create a periodic packet sending
  soft_timer_set_handler(&tx_timer, send_packet, NULL);
  soft_timer_start(&tx_timer, TX_PERIOD, 1);

  /*
  while(1) {
    for(i=0; i<100000; i++) {
      asm volatile("nop");
    }
    getTime(&curtime);
    printf("time is %d.%09d\n",curtime.sec,curtime.nsec);
  }
  */
  platform_run();

  return 0;
}
