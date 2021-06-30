#!/bin/sh
#
# $FreeBSD: 2018-08-26 18:15:00Z mishin $
#
# PROVIDE: tm1637clock
# REQUIRE: DAEMON
# KEYWORD: nojail shutdown
#
# Add the following lines to /etc/rc.conf to enable tm1637clock:
#
# tm1637clock_enable (bool): Set to "NO"  by default.
#                            Set to "YES" to enable tm1637clock.

. /etc/rc.subr

name=tm1637clock
rcvar=tm1637clock_enable

load_rc_config $name

: ${tm1637clock_enable:="NO"}
: ${tm1637clock_device:="/dev/tm1637"}
: ${tm1637clock_point:="1"}
: ${tm1637clock_flags:="-b"}

pidfile=${tm1637clock_pidfile-"/var/run/$name.pid"}

command=/usr/local/sbin/$name
command_args="--device ${tm1637clock_device} --point ${tm1637clock_point} ${tm1637clock_flags}"

run_rc_command "$@"
