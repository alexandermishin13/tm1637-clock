# tm1637-clock

![Raspberry PI 2 & TM1637](/raspberry-pi_tm1637.jpg?raw=true "Raspberry PI 2 & TM1637")

## About

A simple clock daemon which worked with
[tm1637-kmod](https://gitlab.com/alexandermishin13/tm1637-kmod) kernel driver
written for **FreeBSD**. It is a successor of my other project
[digitalclock](https://gitlab.com/alexandermishin13/digitalclock) based on a 
library [tm1637-display](https://gitlab.com/alexandermishin13/tm1637-display).
I cut out a socket functional from it with controlling a brightness level of
the display. This functionality is implemented by the kernel driver using
sysctl variables.

The daemon:
* creates a `pidfile` and uses `daemon`(8) if run with `-b` parameter for
detach from a terminal;
* uses `timer_create`(2) for output time and marking seconds by colon
(1, 2 times per second or always on);
* a signal interception for stopping itself correctly;
* set raw mode for the device but restore its value on exit;

I wrote this program for my little-task server on **Raspberry Pi 2** (One of
a task is a NTPD service for getting a time from GLONASS).

## Dependencies

For the `tm1637-clock` program You need:
* ARM SoC, e.g. **Raspberry Pi 2** or **3** (**Orange** or **Banana** also is
good);
* **FreeBSD 12** operating system;
* C compatible compilator (FreeBSD already has a native `clang`);
* An installed and loaded the [tm1637-kmod](https://gitlab.com/alexandermishin13/tm1637-kmod)
kernel driver.

## Download

```
git clone https://gitlab.com/alexandermishin13/tm1637-clock.git
```

## Installation

For installation type:
```
make
sudo make install
```
The executable file will be installed as **/usr/local/sbin/tm1637clock** and
the service file as **/usr/local/etc/rc.d/tm1637clock**. You also can a copy
a **/rc.conf.d/tm1637clock.example** to **/usr/local/etc/rc.conf.d/tm1637clock**
to control the service.

For deinstallation type:
```
sudo make uninstall
```

## Usage

The program can be run either as a daemon (with `-b` parameter) and as a 
standalone program.
A clock point change mode can be set by a `-p` key followed by integer. Possible
values are `0`-always on, `1`-blinks once a second and `2`-blinks twice per
second. Least processor efficient mode is `2`, although it was optimized. Almost
two times more efficient mode is `1` as it less often writes to the display.
And even more efficient mode is `0` as it writes to the display just once a
minute.

Typical it is run as a daemon by a service startup script:
```
./tm1637clock -b [-p <mode>]
```

but You can run it like a regular program by omitting the `-b` option.

You can turn on, off or set brightness of the display by interacting with the
tm1637 kernel driver using sysctl variables. No special privileges required
for it:
```
sysctl dev.tm1637.0.brightness=3
sysctl dev.tm1637.0.on=0
```

## Manage the service

If You wish to start service automatically with a boot of operating system,
You can:
* add to a file */etc/rc.conf*
* create a new file */usr/local/etc/rc.conf.d/tm1637clock* and add a followed
string to it (Or just copy its example from sources):
```
tm1637clock_enable="YES"
tm1637clock_flags="-b -p 2"
```

Then You can manage the service by one of a command:
```
service tm1637clock start
service tm1637clock stop
service tm1637clock status
```
