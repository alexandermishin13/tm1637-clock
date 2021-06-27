# $FreeBSD$

PREFIX= /usr/local
MK_DEBUG_FILES= no

PROG= tm1637clock
BINDIR= ${PREFIX}/sbin

SCRIPTS= ${PROG}.sh
SCRIPTSNAME_${PROG}.sh= ${PROG}
SCRIPTSDIR_${PROG}.sh= ${PREFIX}/etc/rc.d

MAN= ${PROG}.8
MANDIR= ${PREFIX}/share/man/man
MANSUBDIR= /arm

MANFULLDIR= ${MANDIR}8${MANSUBDIR}

LDADD= -lutil -lrt

beforeinstall:
.if !exists(${MANFULLDIR})
	@mkdir -p ${MANFULLDIR}
.endif

uninstall:
	rm ${BINDIR}/${PROG}
	rm ${PREFIX}/etc/rc.d/${PROG}
	rm ${MANFULLDIR}/${MAN}.gz

.include <bsd.prog.mk>
