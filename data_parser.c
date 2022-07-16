/*
   data_parser.c
	- parse and print multimeter data

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
#include <string.h>
#include <time.h>
#include "data_parser.h"

#define UT_BUF_SIZE 11
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

/* */

#define UT_MSG_TYPE_DC_VOLTS 0x1
#define UT_MSG_TYPE_AC_VOLTS 0x2
#define UT_MSG_TYPE_MVOLTS   0x3
#define UT_MSG_TYPE_OHMS     0x4
#define UT_MSG_TYPE_CAP      0x5
#define UT_MSG_TYPE_TEMP_C   0x6
#define UT_MSG_TYPE_U_AMPS   0x7
#define UT_MSG_TYPE_M_AMPS   0x8
#define UT_MSG_TYPE_AMPS     0x9
#define UT_MSG_TYPE_BEEP     0xA
#define UT_MSG_TYPE_DIODE    0xB
#define UT_MSG_TYPE_FREQ     0xC
#define UT_MSG_TYPE_TEMP_F   0xD

#define N_TO_VAL(N) (N - 0x30)

/* */

static int use_unix_time = 0;	/* Don't use unixtime by default */
static int use_csv_format = 0;	/* Use tab delimeter by default */
static int print_units = 1;		/* Display measurements units by default */
static char date_format[42] = "%Y/%m/%d %H:%M:%S"; /* Default timestamp format */

/* */

/* Static configuration */
void parser_set_unix_time()
{
	use_unix_time = 1;
}

void parser_set_csv_format()
{
	use_csv_format = 1;
}

void parser_set_no_units()
{
	print_units = 0;
}

/* Set custom data format instead of default "%Y/%m/%d %H:%M:%S" */
void parser_set_time_format(const char *format)
{
	if (!format) {
		return;
	}

	memset(date_format, '\0', sizeof(date_format));
	memcpy(date_format, format, MIN(sizeof(date_format), strlen(format)));
}

/* Return required buffer size for the uni-t */
size_t get_data_buf_size()
{
	return UT_BUF_SIZE;
}

/* Print value and unit in configured format */
void data_printer(char *value, char *unit)
{
	time_t time_now;
	struct tm* current_tm;
	char date_buf[42];
	char *delimeter;

	time(&time_now);

	if (use_csv_format) {
		delimeter = ",";
	} else {
		delimeter = "\t";
	}

	if (use_unix_time) {
		printf("%ld", time_now);
	} else {
		current_tm = localtime(&time_now);
		strftime(date_buf, sizeof(date_buf), date_format, current_tm);
		printf("%s", date_buf);
	}

	printf("%s%s", delimeter, value);

	if (print_units) {
		printf("%s%s", delimeter, unit);
	}

	printf("\n");
}

/* */

/* Parse and print Volts measurements, both AC and DC */
void handle_volts(uint8_t buf[], size_t buf_size, int dc)
{
	char sbuf[15];
	char *fmt = "";
	char *prefix = dc ? (N_TO_VAL(buf[8]) == 5 ? "-" : "") : "";

	switch (N_TO_VAL(buf[5])) {
		case 1:
			fmt = "%s%X.%X%X%X%X";
			break;

		case 2:
			fmt = "%s%X%X.%X%X%X";
			break;

		case 3:
			fmt = "%s%X%X%X.%X%X";
			break;

		case 4:
			fmt = "%s%X%X%X%X.%X";
			break;
	}

	snprintf(sbuf, sizeof(sbuf), fmt, prefix
			, N_TO_VAL(buf[0]), N_TO_VAL(buf[1]), N_TO_VAL(buf[2]), N_TO_VAL(buf[3]), N_TO_VAL(buf[4]));

	data_printer(sbuf, "V");
}

/* Parse and print mili Volts measurements */
void handle_mvolts(uint8_t buf[], size_t buf_size)
{
	char sbuf[15];
	char *prefix = N_TO_VAL(buf[8]) == 4 ? "-" : "";

	snprintf(sbuf, sizeof(sbuf), "%s%X%X%X.%X%X", prefix
		, N_TO_VAL(buf[0]), N_TO_VAL(buf[1]), N_TO_VAL(buf[2]), N_TO_VAL(buf[3]), N_TO_VAL(buf[4]));

	data_printer(sbuf, "mV");
}

/* Parse and print resistance measurements */
void handle_ohms(uint8_t buf[], size_t buf_size)
{
	char sbuf[15];
	char *unit = "";
	char *fmt = "";

	if (N_TO_VAL(buf[0]) > 9) {
		data_printer("out of range", unit);
	} else {
		switch (N_TO_VAL(buf[5])) {
			case 1:
				fmt = "%X%X%X.%X%X";
				unit = "ohm";
				break;

			case 2:
				fmt = "%X.%X%X%X%X";
				unit = "kohm";
				break;

			case 3:
				fmt = "%X%X.%X%X%X";
				unit = "kohm";
				break;

			case 4:
				fmt = "%X%X%X.%X%X";
				unit = "kohm";
				break;

			case 5:
				fmt = "%X.%X%X%X%X";
				unit = "mohm";
				break;

			case 6:
				fmt = "%X%X.%X%X%X";
				unit = "mohm";
				break;

			default:
				data_printer("error", "");
				return;
		}

		snprintf(sbuf, sizeof(sbuf), fmt
				, N_TO_VAL(buf[0]), N_TO_VAL(buf[1]), N_TO_VAL(buf[2]), N_TO_VAL(buf[3]), N_TO_VAL(buf[4]));

		data_printer(sbuf, unit);
	}
}

/* Parse and print capacitance measurements */
void handle_cap(uint8_t buf[], size_t buf_size)
{
	char sbuf[15];
	char *unit = "";
	char *fmt = "";

	if (N_TO_VAL(buf[0]) > 9) {
		data_printer("out of range", unit);
	} else {
		switch (N_TO_VAL(buf[5])) {
			case 1:
				fmt = "%X%X.%X%X%X";
				unit = "nF";
				break;

			case 2:
				fmt = "%X%X%X.%X%X";
				unit = "nF";
				break;

			case 3:
				fmt = "%X.%X%X%X%X";
				unit = "uF";
				break;

			case 4:
				fmt = "%X%X.%X%X%X";
				unit = "uF";
				break;

			case 5:
				fmt = "%X%X%X.%X%X";
				unit = "uF";
				break;

			case 6:
				fmt = "%X.%X%X%X%X";
				unit = "mF";
				break;

			default:
				data_printer("error", "");
				return;
		}

		snprintf(sbuf, sizeof(sbuf), fmt
				, N_TO_VAL(buf[0]), N_TO_VAL(buf[1]), N_TO_VAL(buf[2]), N_TO_VAL(buf[3]), N_TO_VAL(buf[4]));

		data_printer(sbuf, unit);
	}
}

/* Parse and print temperature measurements, both C and F */
void handle_temperature(uint8_t buf[], size_t buf_size, int metric)
{
	char sbuf[15];
	char *unit = metric ? "C" : "F";

	if (N_TO_VAL(buf[0]) > 9) {
		data_printer("out of range", unit);
	} else {
		snprintf(sbuf, sizeof(sbuf), "%X%X%X%X.%X"
				, N_TO_VAL(buf[0]), N_TO_VAL(buf[1]), N_TO_VAL(buf[2]), N_TO_VAL(buf[3]), N_TO_VAL(buf[4]));
		data_printer(sbuf, unit);
	}
}

/* Parse and print micro Amp measurements */
void handle_u_amps(uint8_t buf[], size_t buf_size)
{
	char sbuf[15];
	char *prefix = N_TO_VAL(buf[8]) == 4 ? "-" : "";
	char *fmt = "";

	switch (N_TO_VAL(buf[5])) {
		case 0:
			fmt = "%s%X%X%X.%X%X";
			break;

		case 1:
			fmt = "%s%X%X%X%X.%X";
			break;

		default:
			data_printer("error", "");
			return;
	}

	snprintf(sbuf, sizeof(sbuf), fmt, prefix
		, N_TO_VAL(buf[0]), N_TO_VAL(buf[1]), N_TO_VAL(buf[2]), N_TO_VAL(buf[3]), N_TO_VAL(buf[4]));

	data_printer(sbuf, "uA");
}

/* Parse and print mili Amp measurements */
void handle_m_amps(uint8_t buf[], size_t buf_size)
{
	char sbuf[15];
	char *prefix = N_TO_VAL(buf[8]) == 4 ? "-" : "";
	char *fmt = "";

	switch (N_TO_VAL(buf[5])) {
		case 0:
			fmt = "%s%X%X.%X%X%X";
			break;

		case 1:
			fmt = "%s%X%X%X.%X%X";
			break;

		default:
			data_printer("error", "");
			return;
	}

	snprintf(sbuf, sizeof(sbuf), fmt, prefix
		, N_TO_VAL(buf[0]), N_TO_VAL(buf[1]), N_TO_VAL(buf[2]), N_TO_VAL(buf[3]), N_TO_VAL(buf[4]));

	data_printer(sbuf, "mA");
}

/* Parse and print Amp measurements */
void handle_amps(uint8_t buf[], size_t buf_size)
{
	char sbuf[15];
	char *prefix = N_TO_VAL(buf[8]) == 4 ? "-" : "";

	snprintf(sbuf, sizeof(sbuf), "%s%X%X.%X%X%X", prefix
		, N_TO_VAL(buf[0]), N_TO_VAL(buf[1]), N_TO_VAL(buf[2]), N_TO_VAL(buf[3]), N_TO_VAL(buf[4]));

	data_printer(sbuf, "A");
}

/* Beep functin - display open circuit or some resistance */
void handle_beep(uint8_t buf[], size_t buf_size)
{
	char sbuf[15];

	if (N_TO_VAL(buf[0]) == 0xA) {
		data_printer("open", "");
	} else {
		if (N_TO_VAL(buf[5]) == 0) {
			snprintf(sbuf, sizeof(sbuf), "%X%X%X.%X%X"
				, N_TO_VAL(buf[0]), N_TO_VAL(buf[1]), N_TO_VAL(buf[2]), N_TO_VAL(buf[3]), N_TO_VAL(buf[4]));
			data_printer(sbuf, "ohm");
		} else {
			data_printer("high resistance", "");
		}
	}
}

/* Parse and print diode measurements */
void handle_diode(uint8_t buf[], size_t buf_size)
{
	char sbuf[15];

	if (N_TO_VAL(buf[0]) == 0xA) {
		data_printer("open", "");
	} else {
		snprintf(sbuf, sizeof(sbuf), "%X.%X%X%X%X"
		, N_TO_VAL(buf[0]), N_TO_VAL(buf[1]), N_TO_VAL(buf[2]), N_TO_VAL(buf[3]), N_TO_VAL(buf[4]));

		data_printer(sbuf, "mV");
	}
}

/* Parse and print frequency measurements */
void handle_freq(uint8_t buf[], size_t buf_size)
{	
	char sbuf[15];
	char *unit = "";
	char *fmt = "";

	switch (N_TO_VAL(buf[5])) {
		case 0:
			fmt = "%X%X.%X%X%X";
			unit = "Hz";
			break;

		case 1:
			fmt = "%X%X%X.%X%X";
			unit = "Hz";
			break;

		case 2:
			fmt = "%X.%X%X%X%X";
			unit = "kHz";
			break;

		case 3:
			fmt = "%X%X.%X%X%X";
			unit = "kHz";
			break;

		case 4:
			fmt = "%X%X%X.%X%X";
			unit = "kHz";
			break;

		case 5:
			fmt = "%X.%X%X%X%X";
			unit = "MHz";
			break;

		case 6:
			fmt = "%X%X.%X%X%X";
			unit = "MHz";
			break;

		case 7:
			fmt = "%X%X%X.%X%X";
			unit = "MHz";
			break;
	}

	snprintf(sbuf, sizeof(sbuf), fmt
			, N_TO_VAL(buf[0]), N_TO_VAL(buf[1]), N_TO_VAL(buf[2]), N_TO_VAL(buf[3]), N_TO_VAL(buf[4]));

	data_printer(sbuf, unit);
}

/* Select data handler depending on the data type */
void data_parse_and_print(uint8_t buf[], size_t buf_size)
{
	uint8_t msg_type = N_TO_VAL(buf[6]);

	switch (msg_type) {
		case UT_MSG_TYPE_DC_VOLTS:
			handle_volts(buf, buf_size, 1);
			break;

		case UT_MSG_TYPE_AC_VOLTS:
			handle_volts(buf, buf_size, 0);
			break;

		case UT_MSG_TYPE_MVOLTS:
			handle_mvolts(buf, buf_size);
			break;

		case UT_MSG_TYPE_OHMS:
			handle_ohms(buf, buf_size);
			break;

		case UT_MSG_TYPE_CAP:
			handle_cap(buf, buf_size);
			break;

		case UT_MSG_TYPE_TEMP_C:
			handle_temperature(buf, buf_size, 1);
			break;

		case UT_MSG_TYPE_TEMP_F:
			handle_temperature(buf, buf_size, 0);
			break;

		case UT_MSG_TYPE_U_AMPS:
			handle_u_amps(buf, buf_size);
			break;

		case UT_MSG_TYPE_M_AMPS:
			handle_m_amps(buf, buf_size);
			break;

		case UT_MSG_TYPE_AMPS:
			handle_amps(buf, buf_size);
			break;

		case UT_MSG_TYPE_BEEP:
			handle_beep(buf, buf_size);
			break;
			
		case UT_MSG_TYPE_DIODE:
			handle_diode(buf, buf_size);
			break;
			
		case UT_MSG_TYPE_FREQ:
			handle_freq(buf, buf_size);
			break;

		default:
			fprintf(stderr, "Unknown message type 0x%02x\n", msg_type);
	}
}

