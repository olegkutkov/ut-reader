/*
   port_utils.c
	- serial port utilities: open, configure, read

   Copyright 2022  Oleg Kutkov <contact@olegkutkov.me>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <poll.h>
#include "port_utils.h"

#define READ_POLL_TIEMOUT_MS 1000

static int run_poller = 1;

/* */

#if defined (__linux__)
/* Convert baud num to the Linux baud define */
static speed_t baud_rate_to_speed_t(uint32_t baud)
{
#define B(x) case x: return B##x
	switch (baud) {
		B(50);     B(75);     B(110);    B(134);    B(150);
		B(200);    B(300);    B(600);    B(1200);   B(1800);
		B(2400);   B(4800);   B(9600);   B(19200);  B(38400);
		B(57600);  B(115200); B(230400); B(460800); B(500000); 
		B(576000); B(921600); B(1000000);B(1152000);B(1500000); 
	default:
		return 0;

	}
#undef B
}
#elif defined (__APPLE__)
static speed_t baud_rate_to_speed_t(uint32_t baud)
{
	return baud;
}
#else
	#pragma error("Bad platform!")
#endif

/* Set specified baud rate
   Other params are constant:
    - NO PARITY
    - 1 STOP BIT
    - 8 DATA BITS
  */
void set_baud_rate(int fd, speed_t baud)
{
	struct termios settings;
	tcgetattr(fd, &settings);

	cfsetispeed(&settings, baud); /* input baud rate */
	cfsetospeed(&settings, baud); /* output baud rate */

	/* Set odd parity, 7 bit data */
	settings.c_cflag |= PARENB;
	settings.c_cflag |= PARODD;
	settings.c_cflag &= ~CSTOPB;
	settings.c_cflag &= ~CSIZE;
	settings.c_cflag |= CS7;

	settings.c_cflag &= ~CRTSCTS;

	/* Set canonical */
	settings.c_lflag |= ICANON;

	 /* Turn off s/w flow ctrl */
	settings.c_iflag &= ~(IXON | IXOFF | IXANY);

	/* Disable any special handling of received bytes */
	settings.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

	/* apply the settings */
	tcsetattr(fd, TCSANOW, &settings);
	tcflush(fd, TCOFLUSH);
}

/* Open serial device "dev", configure baud rate and apply some default params */
int open_serial_dev(const char* dev, uint32_t baud, int non_block)
{
	int serial_fd = 0;
	int fd_flags = 0;
	speed_t bs = baud_rate_to_speed_t(baud);

	if (!bs) {
		errno = EINVAL;
		return -errno;
	}

	printf("Serial device %s with baud rate = %d\n", dev, baud);

	serial_fd = open(dev, O_RDONLY | O_NOCTTY | O_NDELAY);

	if (serial_fd == -1) {
		return -errno;
	}

	set_baud_rate(serial_fd, bs);

	if (non_block) {
		fd_flags = fcntl(serial_fd, F_GETFL, 0);
		fcntl(serial_fd, F_SETFL, fd_flags | O_NONBLOCK);
	}

	return serial_fd;
}

/* Poll serial device fd */
int start_data_polling(int fd, size_t req_data_size, data_cb cb)
{
	struct pollfd fds;
	ssize_t ret;
	uint8_t *buf = (uint8_t*) malloc(req_data_size);

	if (!buf) {
		return errno;
	}

	fds.fd = fd;
	fds.events = POLLIN;

	while (run_poller) {
		ret = poll(&fds, 1, READ_POLL_TIEMOUT_MS);

		/* Polling error. Exit cycle */
		if (ret < 0) {
			free(buf);
			return errno;
		} else if (ret > 0) {
			/* We got something, check the event */
			if (fds.revents == POLLIN) {
				/* Try to read the data */
				ret = read(fd, buf, req_data_size);

				/* Check read data size */
				if (ret != req_data_size) {
					if (errno == EAGAIN) {
						/* Let's read one more time */
						continue;
					}

					/* Unexpected I/O failure */
					free(buf);
					return EIO;
				} else {
					/* Call the cb function */
					cb(buf, req_data_size);
				}
			}
		}
		/* Timeout or other event, just continue the loop */
	}

	free(buf);

	return 0;
}

void stop_data_polling()
{
	run_poller = 0;
}

/* */
int close_serial_dev(int fd)
{
	return close(fd);
}

