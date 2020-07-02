/*-
 * Copyright (c) 2020 Alexander A. Mishin <mishin@mh.net.ru>
 * All rights reserved.
 *
 */

#define CLOCKPOINT_ALWAYS	0
#define CLOCKPOINT_ONCE		1
#define CLOCKPOINT_TWICE	2

struct tm1637_clock_t {
    int tm_min;
    int tm_hour;
    bool tm_colon;
};

#define TM1637IOC_CLEAR			_IO('T', 1)
#define TM1637IOC_OFF			_IO('T', 2)
#define TM1637IOC_ON			_IO('T', 3)
#define TM1637IOC_SET_BRIGHTNESS	_IOW('T', 11, uint8_t)
#define TM1637IOC_SET_CLOCKPOINT	_IOW('T', 12, uint8_t)
#define TM1637IOC_SET_RAWMODE		_IOW('T', 13, uint8_t)
#define TM1637IOC_SET_CLOCK		_IOW('T', 14, struct tm1637_clock_t)
#define TM1637IOC_GET_RAWMODE		_IOR('T', 23, uint8_t)

#define sizeof_field(type,field)  (sizeof(((type *)0)->field))
