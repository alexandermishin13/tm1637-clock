/*-
 * Copyright (c) 2020 Alexander A. Mishin <mishin@mh.net.ru>
 * All rights reserved.
 *
 */

#include <libutil.h>
#include <err.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <dev/tm1637/tm1637.h>

#define CLOCKPOINT_ALWAYS	0
#define CLOCKPOINT_ONCE		1
#define CLOCKPOINT_TWICE	2

timer_t timerID;
struct pidfh *pfh;
int dev;

struct tm1637_clock_t cl;

/* Parameters */
bool backgroundRun = false;
uint8_t tpsPoint = CLOCKPOINT_ALWAYS; // Times per second switch a point sign

/* Signals handler. Prepare the programm for end */
static void
termination_handler(int signum)
{
  /* Destroy timer */
  timer_delete(timerID);

  /* Off the display, restore old mode, and terminate connection */
  ioctl(dev, TM1637IOC_OFF);

  /* Close the device */
  close(dev);

  /* Remove pidfile and exit */
  pidfile_remove(pfh);

  exit(EXIT_SUCCESS);
}


/* timer handler. Effectively redraw the display */
static void
timer_handler(int sig, siginfo_t *si, void *uc)
{
  time_t rawtime;
  struct tm tm;

  /* Toggle a clock point for each timer tick if not always on */
  if (tpsPoint > CLOCKPOINT_ALWAYS)
    cl.tm_colon = !cl.tm_colon;

  /* No need to check time often than once of a second */
  if (cl.tm_colon || tpsPoint < CLOCKPOINT_TWICE)
  {
    /* Get local time (with timezone) */
    time (&rawtime);
    localtime_r(&rawtime, &tm);

    /* Prepare new display digits array if a time change occurs
     * or use old one
     */
    if (cl.tm_min != tm.tm_min) {
	cl.tm_min = tm.tm_min;
	cl.tm_hour = tm.tm_hour;
    }
  }

  ioctl(dev, TM1637IOC_SET_CLOCK, &cl);
}


/* Create and start a half-second timer */
static void
createTimer(uint8_t tps)
{
  struct sigaction sa;

  /* Set SIGRTMIN as first available realtime signal number
   * to avoid of uncompatibleties
   */
  sa.sa_flags     = SA_SIGINFO;
  sa.sa_sigaction = timer_handler;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGRTMIN, &sa, NULL);

  struct sigevent te;
  memset(&te,0,sizeof(struct sigevent));
  te.sigev_notify          = SIGEV_SIGNAL;
  te.sigev_signo           = SIGRTMIN;
  te.sigev_value.sival_ptr = &timerID;
  timer_create(CLOCK_REALTIME, &te, &timerID);

  /* Get current time for a TIMER_ABSTIME flag */
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);

  struct itimerspec its;
  its.it_value.tv_sec     = ts.tv_sec + 1; // Align a start to next second...
  its.it_value.tv_nsec    = 0;             // ...as exactly as we could

  /* Normally the timer handler runs once a second.
   * But if we want a clock point changes two times per second then so be it.
   * And if there is no clock point changes at all, all the same once per second
   */
  if (tps == CLOCKPOINT_TWICE) {
      its.it_interval.tv_sec  = 0;
      its.it_interval.tv_nsec = 500000000L;    // A half of a second
  }
  else
  {
      its.it_interval.tv_sec  = 1;    // One second
      its.it_interval.tv_nsec = 0;
  }

  timer_settime(timerID, TIMER_ABSTIME, &its, NULL);
}

/* Print usage after mistaken params */
static void
usage(char* program)
{
  printf("Usage:\n %s [-b] [-p <mode>]\n", program);
  printf("\t -b: Run the program in background;\n");
  printf("\t -p: Set a clockpoint mode, where <mode>:\n");
  printf("\t\t0-always on, 1-once per second, 2-twice per second\n");
}

/* Get a clockpoint blink value from params */
static uint8_t
get_tpsPoint(char* nptr)
{
  int number;
  const char *errstr;

  number = strtonum(nptr, CLOCKPOINT_ALWAYS, CLOCKPOINT_TWICE, &errstr);
  if (errstr != NULL)
    errx(EXIT_FAILURE, "The clock point change mode is %s: %s (must be from %d to %d)", errstr, nptr, CLOCKPOINT_ALWAYS, CLOCKPOINT_TWICE);

  return number;
}

/* Get and decode params */
static void
get_param(int argc, char **argv)
{
  int opt;

  while((opt = getopt(argc, argv, "hbp:")) != -1)
  {
    switch(opt)
    {
      case 'b': // go to background (demonize)
        backgroundRun = true;
        break;

      case 'p': // clock point change mode
        tpsPoint = get_tpsPoint(optarg);
        break;

      case 'h': // help request
      case '?': // unknown option...
      default:
        usage(argv[0]);
        exit (0);
    }
  }
}


/* Demonize wrapper */
static void
demonize(void)
{
  pid_t otherpid;

  /* Try to create a pidfile */
  pfh = pidfile_open(NULL, 0600, &otherpid);
  if (pfh == NULL)
  {
    if (errno == EEXIST)
      errx(EXIT_FAILURE, "Daemon already running, pid: %jd.", (intmax_t)otherpid);

    /* If we cannot create pidfile from other reasons, only warn. */
    warn("Cannot open or create pidfile");
    /*
     * Even though pfh is NULL we can continue, as the other pidfile_*
     * function can handle such situation by doing nothing except setting
     * errno to EDOOFUS.
     */
  }

  /* Try to demonize the process */
  if (daemon(0, 0) == -1)
  {
    pidfile_remove(pfh);
    errx(EXIT_FAILURE, "Cannot daemonize");
  }

  pidfile_write(pfh);
}


int
main(int argc, char **argv)
{
  /* Analize params and set
   * configureOnly, backgroundRun
   */
  get_param(argc, argv);

  dev = open("/dev/tm1637", O_WRONLY | O_DIRECT);
  if (dev < 0)
  {
    perror("opening tm1637 device");
    exit(EXIT_FAILURE);
  }

  /* Clear a display and turn it on */
  ioctl(dev, TM1637IOC_CLEAR);
  ioctl(dev, TM1637IOC_ON);

  /* If run as a daemon oa as a control utility */
  if (backgroundRun)
    demonize();

  /* Intercept signals to our function */
  if (signal (SIGINT, termination_handler) == SIG_IGN)
    signal (SIGINT, SIG_IGN);
  if (signal (SIGTERM, termination_handler) == SIG_IGN)
    signal (SIGTERM, SIG_IGN);

  /* Run a half seconds redraw of the display */
  cl.tm_colon = true;
  createTimer(tpsPoint);

  /* Main loop */
  while(true) {
    sleep(30);
  }

  exit(EXIT_SUCCESS);
}
