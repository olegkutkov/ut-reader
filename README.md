# UT-READER - a PC tool for UNI-T multimeters

Read measurements from the multimeter in real time.</br>
Print the data in different formats.

This tool is primarily designed for the ut800 series, but it might work with other models.</br>
The code inspired by [https://github.com/tmatejuk/ut804_linux_logger](https://github.com/tmatejuk/ut804_linux_logger)

Supported platforms: Linux, MacOS

### Build and install
`make && sudo make install`

### Usage

`ut_reader -d /dev/ttyXXX <options>`

`-d` - Set the serial device name, it might be RS232 or USB converter</br>

Available options:

```
	-b <baud> - Set baud rate, default is 2400
	-f <format> - Set custom date format, default is %Y-%m-%d %H:%M:%S
	-u - Use Unix timestamps instead of date/time
	-c - Print data in CVS format
	-v - Don't print data units
```

##### CVS output example:

```
ut_reader -d /dev/ttyUSB2 -c
Serial device /dev/ttyUSB2 with baud rate = 2400
Starting capture, press Ctrl-C to stop
2022-07-17 01:43:01,231.38,V
2022-07-17 01:43:02,231.42,V
2022-07-17 01:43:02,231.43,V
2022-07-17 01:43:03,231.43,V
2022-07-17 01:43:03,231.44,V
2022-07-17 01:43:04,231.45,V
2022-07-17 01:43:05,231.43,V
2022-07-17 01:43:05,231.41,V
2022-07-17 01:43:06,231.42,V
```

##### Custom date/time format example:

```
ut_reader -d /dev/ttyUSB2 -c -f "%c"
Serial device /dev/ttyUSB2 with baud rate = 2400
Starting capture, press Ctrl-C to stop
Sun Jul 17 01:43:52 2022,232.19,V
Sun Jul 17 01:43:52 2022,232.20,V
Sun Jul 17 01:43:53 2022,232.19,V
Sun Jul 17 01:43:54 2022,232.23,V
Sun Jul 17 01:43:54 2022,232.25,V
Sun Jul 17 01:43:55 2022,232.27,V
Sun Jul 17 01:43:56 2022,232.32,V
Sun Jul 17 01:43:56 2022,232.36,V

ut_reader -d /dev/ttyUSB2 -c -f "%H:%M:%S"
Serial device /dev/ttyUSB2 with baud rate = 2400
Starting capture, press Ctrl-C to stop
01:44:45,231.39,V
01:44:46,231.36,V
01:44:46,231.37,V
01:44:47,231.30,V
01:44:48,231.34,V
01:44:48,231.31,V
01:44:49,231.32,V
01:44:50,231.33,V
01:44:50,231.30,V
01:44:51,231.27,V
```

##### Unix time:
```
ut_reader -d /dev/ttyUSB2 -c -u
Serial device /dev/ttyUSB2 with baud rate = 2400
Starting capture, press Ctrl-C to stop
1658011522,231.46,V
1658011523,231.48,V
1658011523,231.47,V
1658011524,231.47,V
1658011525,231.47,V
1658011525,231.51,V
1658011526,231.54,V
1658011527,231.52,V
1658011527,231.44,V
```

##### No units:
```
ut_reader -d /dev/ttyUSB2 -c -v
Serial device /dev/ttyUSB2 with baud rate = 2400
Starting capture, press Ctrl-C to stop
2022-07-17 01:46:54,231.44
2022-07-17 01:46:55,231.46
2022-07-17 01:46:56,231.44
2022-07-17 01:46:56,231.42
2022-07-17 01:46:57,231.40
2022-07-17 01:46:58,231.42
2022-07-17 01:46:58,231.45
2022-07-17 01:46:59,231.44
2022-07-17 01:47:00,231.40
2022-07-17 01:47:00,231.38
```
