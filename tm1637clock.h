/*-
 * Copyright (c) 2020 Alexander A. Mishin <mishin@mh.net.ru>
 * All rights reserved.
 *
 */

#define CLOCKPOINT_ALWAYS	0
#define CLOCKPOINT_ONCE		1
#define CLOCKPOINT_TWICE	2

#define TM1637_IOCTL_CLEAR	_IO('T', 1)
#define TM1637_IOCTL_OFF	_IO('T', 2)
#define TM1637_IOCTL_ON		_IO('T', 3)
#define TM1637_IOCTL_BRIGHTNESS	_IOW('T', 11, uint8_t)
#define TM1637_IOCTL_CLOCKPOINT	_IOW('T', 12, bool)

#define sizeof_field(type,field)  (sizeof(((type *)0)->field))
