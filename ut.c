/*
   ut_reader.c

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
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include "port_utils.h"
#include "data_parser.h"
#include "version.h"

/* */

#define DEFAULT_BAUD_RATE 2400

/* */

void show_help(const char *pname)
{
	printf("%s - v%d.%d\n", pname, VERSION_MAJOR, VERSION_MINOR);
	printf("Oleg Kutkov <contact@olegkutkov.me>, 2022\n\n");
	printf("Usage:\n");
	printf("\t-d <device> - Set serial device\n");
	printf("\t-b <baud> - Set baud rate, default is %d\n", DEFAULT_BAUD_RATE);
	printf("\t-f <format> - Set custom date format, default is %%Y-%%m-%%d %%H:%%M:%%S\n");
	printf("\t-u - Use Unix timestamps instead of date/time\n");
	printf("\t-c - Print data in CVS format\n");
	printf("\t-v - Don't print data units\n");
	printf("\t-h - Display this help and exit\n");
}

/* Handle Ctrl-C */
void handle_signal(int sig)
{
	stop_data_polling();
}

int main(int argc, char *argv[])
{
	int c, fd;
	const char *serial_dev = NULL;
	uint32_t baud = DEFAULT_BAUD_RATE;

	/* Parse args */
	while ((c = getopt(argc, argv, "d:b:f:ucvh")) != -1) {
		switch (c) {
			case 'd':
				serial_dev = optarg;
				break;

			case 'b':
				baud = atoi(optarg);
				break;

			case 'f':
				parser_set_time_format(optarg);
				break;

			case 'u':
				parser_set_unix_time();
				break;

			case 'c':
				parser_set_csv_format();
				break;

			case 'v':
				parser_set_no_units();
				break;

			case 'h':
				show_help(argv[0]);
				return 0;
		}
	}

	if (!serial_dev) {
		fprintf(stderr, "Please set the serial device with -d\n");
		return 1;
	}

	/* Open serial device and set baud rate */
	fd = open_serial_dev(serial_dev, baud, 0);

	if (fd < 0) {
		fprintf(stderr, "Couldn't open device %s, error: %s\n", serial_dev, strerror(errno));
		return 1;
	}

	/* Set Ctrl-C signal handler */
	signal(SIGINT, handle_signal);

	setbuf(stdout, NULL);

	printf("# Starting capture, press Ctrl-C to stop\n");

	/* Start data poller with parser cb */
	int ret = start_data_polling(fd, get_data_buf_size(), data_parse_and_print);

	if (ret > 0 && ret != EINTR) {
		fprintf(stderr, "Data capture failed, error: %s\n", strerror(errno));
	}

	close_serial_dev(fd);

	printf("\n# Capture finished\n");

	return 0;
}

