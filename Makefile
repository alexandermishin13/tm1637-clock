# $FreeBSD$

PREFIX= /usr/local
MK_DEBUG_FILES= no

PROG= tm1637clock
BINDIR= ${PREFIX}/sbin

FILESGROUPS= RC
RCDIR= ${PREFIX}/etc/rc.d
RCMODE= 0755
RC= rc.d/${PROG}

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
	rm ${RCDIR}/${PROG}
	rm ${MANFULLDIR}/${MAN}.gz

.include <bsd.prog.mk>
