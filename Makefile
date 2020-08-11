# $FreeBSD$

MK_DEBUG_FILES= no

PROG= tm1637clock
PROG_TYPE= DAEMON
BINDIR= /usr/local/sbin

FILESGROUPS= RC
RCDIR= /usr/local/etc/rc.d
RCMODE= 0755
RC= rc.d/tm1637clock

MAN=

LDADD= -lutil -lrt

uninstall:
	rm ${BINDIR}/tm1637clock
	rm ${RCDIR}/tm1637clock

.include <bsd.prog.mk>
