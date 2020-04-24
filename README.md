# digitalclock

![Raspberry PI 2 & TM1637](/raspberry-pi_tm1637.jpg?raw=true "Raspberry PI 2 & TM1637")

## About

A simple clock daemon on `tm1637-display` over `gpio` written for **FreeBSD**.

* It creates a `pidfile` and uses `daemon`(8) for detach from a terminal;
* It uses `timer_create`(2) for output a time and marking seconds and signal interception for stopping itself correctly;
* It also uses a `socket` for a connection with another running process of itself for controls purpose.

I wrote this program for my little-task server on **Raspberry Pi 2** (One of a task is a NTPD service for getting a time from GLONASS).
For this program, I had to adapt the [tm1637-display library for Arduino](https://github.com/AlexGyver/GyverLibs/tree/master/TM1637_Gyver)
(written by Fred.Chu and modified by AlexGyver) for FreeBSD.

## Dependencies

For the `digitalclock` program You need:
* ARM SoC, e.g. **Raspberry Pi 2** or **3** (I think a **Orange** or **Banana** also is good);
* **FreeBSD 11** or **12** (as I have) as operating system;
* C++ compatible compilator (You already have clang++, I'm sure);
* An installed [tm1637-display library for FreeBSD](https://gitlab.com/alexandermishin13/tm1637-display).

## Download

```
git clone https://gitlab.com/alexandermishin13/digitalclock.git
```

## Installation

For installation type:
```
make
sudo make install
```
The executable file will be installed as **/usr/local/sbin/digitalclock** and
the service file as **/usr/local/etc/rc.d/digitalclock**.

For deinstallation type:
```
sudo make uninstall
```

## Usage

The programm can be run either as a daemon and as a control utility for the daemon.
A bright level can be set by an `-l` key followed by an integer from `0` to `7` (from darkest to brightest).
A clock point change mode can be set by an `-p` key followed by integer.
Possible values are `0`-always on, `1`-blinks once a second and `2`-blinks twice per second.
Least efficient processor mode is `2`, although it was optimized.
Almost two times more efficient mode is `1` as it less often writes to a display.
And even more efficient mode is `0` as it writes to the display just once a minute.

On that moment implemented only brightness control by the control utility mode.

Running as a daemon
```
./digitalclock -b [-l <brightness>] [-p <clockpoint mode>]
```

Running as a control utility
```
./digitalclock [-l <brightness>]
```

## Manage the service

You can manage the service by one of a command:
```
service digitalclock onestart
service digitalclock onestop
service digitalclock onestatus
```

If You wish to start service automatically with a boot of operating system,
You can:
* add to a file */etc/rc.conf*
* create a new file */usr/local/etc/rc.conf.d/digitalclock* and add a followed string to it:
```
digitalclock_enable=YES
```

Then You can manage the service by:
```
service digitalclock start
service digitalclock stop
service digitalclock status
```

