/*
   data_parser.h
	- parse and print multimeter data, header file

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

#ifndef DATA_PARSER_H
#define DATA_PARSER_H

void parser_set_unix_time();
void parser_set_csv_format();
void parser_set_no_units();
void parser_set_time_format(const char *format);

size_t get_data_buf_size();
void data_parse_and_print(uint8_t buf[], size_t buf_size);

#endif
