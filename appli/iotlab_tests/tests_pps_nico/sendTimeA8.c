#include <termios.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>

#define MAGICCHAR1 0x16
#define MAGICCHAR2 0x3

int configure_tty(char *tty_path, speed_t speed) {
  /*
   * TTY configuration inspired by:
   *   - Contiki/tunslip code
   *   - http://www.unixwiz.net/techtips/termios-vmin-vtime.html
   *   - termios manpage
   *   - http://www.tldp.org/HOWTO/Serial-Programming-HOWTO/x115.html
   */
  int serial_fd;
  struct termios tty;
  memset(&tty, 0, sizeof(tty));
  
  serial_fd = open(tty_path, O_RDWR );
  if (serial_fd == -1) {
    printf("Could not open %s\n", tty_path);
    return -1;
  }

  if (tcflush(serial_fd, TCIOFLUSH) == -1) {
    printf("Error in tcflush: %s\n",
		strerror(errno));
    return -2;
  }

  if (tcgetattr(serial_fd, &tty)) {
    printf("Error in tcgetattr: %s\n",
		strerror(errno));
    return -3;
  }
  
  /*
   * Configure TTY
   */
  cfmakeraw(&tty);
  // blocking mode, should read at least 1 char and then can return
  tty.c_cc[VMIN]  = 1;
  tty.c_cc[VTIME] = 0;
  // Disable control characters and signals and all
  tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS (hardware) flow control
  tty.c_cflag &= ~HUPCL;   // No "hanging up" when closing
  tty.c_cflag |=  CLOCAL;  // ignore modem status line
  if (cfsetspeed(&tty, speed)) {
    printf("Error while setting terminal speed: %s\n",
		strerror(errno));
    return -4;
  }
  
  // Apply and discard characters that may have arrived
  if (tcsetattr(serial_fd, TCSAFLUSH, &tty) == -1) {
    printf("Error could not set attribute to tty: %s\n",
	   strerror(errno));
    return -5;
  }
  return serial_fd;
}

int main(int argc, char *argv[]) {
  int GPS_fd  = 0;
  int M3_fd  = 0;
  int line=0;
  int gotnewline=0;
  int fullline=0;
  int linelength=0;
  int gottime=0;
  int n_chars;
  int hours, minutes,seconds;
  uint32_t secondsSinceMidnight;
  unsigned char hours_string[2];
  unsigned char minutes_string[2];
  unsigned char seconds_string[2];
  uint8_t rx_buff[256];
  uint8_t tx_buff[256];
  char *M3_path = "/dev/ttyA8_M3";
  char *GPS_path= "/dev/ttyO1"; 
  char rxchar;
  int ret;
  

  if ((GPS_fd = configure_tty(GPS_path,B9600)) <= 0) {
    printf("Could not open and configure TTY %s\n", GPS_path);
    close(GPS_fd);
    return -1;
  }

  if ((M3_fd = configure_tty(M3_path,B500000)) <= 0) {
    printf("Could not open and configure TTY %s\n", M3_path);
    close(M3_fd);
    return -1;
  }

  
  //
  // First we wait for a fresh NMEA time from the GPS
  //

  //discard partial lines (NMEA frames begins by '$')
  while (!gotnewline){
    n_chars = read(GPS_fd, &rxchar, 1);
    if (n_chars=1 && rxchar=='$') {
      gotnewline=1;
    }
  }
  // read full new line (till next '$')
  // until we got a GPGGA one and its time
  while(!gottime){
    while(!fullline){
      n_chars = read(GPS_fd, &rxchar, 1);
      if (n_chars==1 && rxchar=='$') {
	fullline=1;
	rx_buff[linelength]=0;
      } else {
	if (n_chars==1) {
	  rx_buff[linelength]=rxchar;
	  linelength++;
	}
      }
    }
    linelength=0;
    fullline=0;
    //we look for a GPGGA NMEA frame which looks like : 
    //$GPGGA,125315.00,4513.12989,N,00548.40890,E,1,06,1.47,230.0,M,47.4,M,,*5B
    // 125315.00 being the time of the day : 12h53mn15s
    if (strncmp(rx_buff,"GPGGA",5)==0) {
      sscanf(rx_buff,"GPGGA,%2s%2s%2s",&hours_string, &minutes_string, &seconds_string);
      hours=atoi(hours_string);
      minutes=atoi(minutes_string);
      seconds=atoi(seconds_string);
      secondsSinceMidnight=hours*3600+minutes*60+seconds;
      printf("Got %02d:%02d:%02d from GPS, sending %d to the M3\n",hours,minutes,seconds,secondsSinceMidnight);
      gottime=1;
    }
  }
  //
  // Then we send the time to the M3
  //
  tx_buff[0]=MAGICCHAR1;
  tx_buff[1]=MAGICCHAR2;
  memcpy(tx_buff+2,&secondsSinceMidnight,4);
  if (write(M3_fd, tx_buff, 6)!=6) printf("Error writing to %s\n",GPS_path);
  printf("wrote %02x  %02x  %02x  %02x\n",tx_buff[2],tx_buff[3],tx_buff[4],tx_buff[5]);
  close(M3_fd);
  close(GPS_fd);

}

