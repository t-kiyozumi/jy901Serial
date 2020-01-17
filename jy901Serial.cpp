
// C library headers
#include <stdio.h>
#include <string.h>

// Linux headers
#include <fcntl.h>   // Contains file controls like O_RDWR
#include <errno.h>   // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>  // write(), read(), close()

int main()
{
  int serial_port = open("/dev/ttyUSB0", O_RDWR);

  // Create new termios struc, we call it 'tty' for convention
  struct termios tty;
  memset(&tty, 0, sizeof tty);

  // Read in existing settings, and handle any error
  if (tcgetattr(serial_port, &tty) != 0)
  {
    printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
  }

  tty.c_cflag &= ~PARENB;        // Clear parity bit, disabling parity (most common)
  tty.c_cflag &= ~CSTOPB;        // Clear stop field, only one stop bit used in communication (most common)
  tty.c_cflag |= CS8;            // 8 bits per byte (most common)
  tty.c_cflag &= ~CRTSCTS;       // Disable RTS/CTS hardware flow control (most common)
  tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

  tty.c_lflag &= ~ICANON;
  tty.c_lflag &= ~ECHO;                                                        // Disable echo
  tty.c_lflag &= ~ECHOE;                                                       // Disable erasure
  tty.c_lflag &= ~ECHONL;                                                      // Disable new-line echo
  tty.c_lflag &= ~ISIG;                                                        // Disable interpretation of INTR, QUIT and SUSP
  tty.c_iflag &= ~(IXON | IXOFF | IXANY);                                      // Turn off s/w flow ctrl
  tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable any special handling of received bytes

  tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
  tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
  // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
  // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

  tty.c_cc[VTIME] = 10; // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
  tty.c_cc[VMIN] = 0;

  // Set in/out baud rate to be 9600
  cfsetispeed(&tty, B115200);
  cfsetospeed(&tty, B115200);

  if (tcsetattr(serial_port, TCSANOW, &tty) != 0)
  {
    printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
  }

  unsigned char read_buf[1];
  unsigned char jy_buf[11];
  int i = 0;

  memset(&read_buf, '\0', sizeof(read_buf));
  memset(&jy_buf, '\0', sizeof(jy_buf));
  int num_bytes;
  unsigned char Roll, RollH, RollL;
  while (1)
  {
    num_bytes = read(serial_port, &read_buf, sizeof(read_buf));
    if (num_bytes < 0)
    {
      printf("Error reading: %s \n", strerror(errno));
    }
   // printf("%x ", read_buf[0]);
    if (read_buf[0] == 0x55)
    {
      printf("\n");
      i = 0;
      jy_buf[i] = read_buf[0];
    }
    jy_buf[i] = read_buf[0];

    if (i == 10)
    {
     // printf("|%x|%x|%x|%x|%x|%x|%x|%x|%x|%x|%x|\n", jy_buf[0], jy_buf[1], jy_buf[2], jy_buf[3], jy_buf[4], jy_buf[5], jy_buf[6], jy_buf[7], jy_buf[8], jy_buf[9], jy_buf[10]);
      // if (jy_buf[1] == 0x51)
      // {
      //   printf("Acceleration Z : %f \n", ((jy_buf[7] << 8) | jy_buf[6]) / 32768.0 * (16.0 * 9.8));
      // }
      if (jy_buf[1] == 0x53)
      {
        printf("pich : %f \n", ((read_buf[6] << 8) | read_buf[4]) / 182.54);
        printf("Roll : %f \n", ((read_buf[4] << 8) | read_buf[3]) / 182.54);
      }
    }
      i++;
  }
  close(serial_port);
}

// if (read_buf[i + 10] == read_buf[i] + read_buf[i + 1] + read_buf[i + 2] + read_buf[i + 3] + read_buf[i + 4] + read_buf[i + 5] + read_buf[i + 6] + read_buf[i + 7] + read_buf[i + 8] + read_buf[i + 9])
// {