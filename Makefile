CFLAGS=-c -Wall -O2 -I/usr/local/include
LIBS=-lc -L/usr/local/lib
PREFIX=/usr/local
SUDO_CMD=sudo

all: tm1637clock

tm1637clock.o: tm1637clock.c
	$(CC) $(CFLAGS) -o $@ $<

tm1637clock: tm1637clock.o
	$(CC) $(LIBS) -lutil -lrt -o $@ tm1637clock.o

clean:
	rm *.o tm1637clock

install: tm1637clock
	${SUDO_CMD} install -s -m 755 -o root -g wheel tm1637clock ${PREFIX}/sbin
	${SUDO_CMD} install -m 755 -o root -g wheel ./rc.d/tm1637clock ${PREFIX}/etc/rc.d


uninstall:
	${SUDO_CMD} rm ${PREFIX}/sbin/tm1637clock
