# $FreeBSD$

PREFIX?= /usr/local
MK_DEBUG_FILES= no

PROG= tm1637clock
BINDIR= ${PREFIX}/sbin

SCRIPTS= ${PROG}.sh
SCRIPTSNAME_${PROG}.sh= ${PROG}
SCRIPTSDIR_${PROG}.sh= ${PREFIX}/etc/rc.d

MAN= ${PROG}.8
MANDIR= ${PREFIX}/man/man

LDADD= -lutil -lrt

uninstall:
	rm ${BINDIR}/${PROG}
	rm ${PREFIX}/etc/rc.d/${PROG}
	rm ${MANDIR}8/${MAN}.gz

.include <bsd.prog.mk>
