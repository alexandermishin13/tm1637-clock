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

/*
 * Displays digital time on TM1637
 * Writen by Mishin Alexander, 2020
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

timer_t timerID;
struct pidfh *pfh;
int dev;

bool clockPoint = true;
unsigned char digits[10] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f };
unsigned char TimeDisp[4] = {'-'}; // Fill with not numbers for not succesful first compare

/* Parameters */
bool backgroundRun = false;
uint8_t tpsPoint = 0; // Times per second switch a point sign

/* Signals handler. Prepare the programm for end */
static void
termination_handler(int signum)
{
  /* Destroy timer */
  timer_delete(timerID);

  /* Off the display and terminate connection */
  ioctl(dev, TM1637_IOCTL_OFF);

  /* Close the device */
  close(dev);

  /* Remove pidfile and exit */
  pidfile_remove(pfh);

  exit(EXIT_SUCCESS);
}


/* timer handler. Effectively redraw the display */
static
void
timer_handler(int sig, siginfo_t *si, void *uc)
{
  time_t rawtime;
  struct tm tm;
  unsigned char loMin;

  /* Get local time (with timezone) */
  time (&rawtime);
  localtime_r(&rawtime, &tm);

  /* Prepare new display digits array if a time change occurs
   * or use old one
   */
  loMin = digits[tm.tm_min % 10]; // Check if change occurs
  if (loMin != TimeDisp[3])
  {
    TimeDisp[0] = digits[tm.tm_hour / 10];
    TimeDisp[1] = digits[tm.tm_hour % 10];
    TimeDisp[2] = digits[tm.tm_min / 10];
    TimeDisp[3] = loMin; // Calculated already

    /* Display a just prepared time array */
    lseek(dev, 0, SEEK_SET);
    write(dev, TimeDisp, 4);
  }

  /* Toggle a clock point for each timer tick if not always on */
  if (tpsPoint > CLOCKPOINT_ALWAYS)
  {
    ioctl(dev, TM1637_IOCTL_CLOCKPOINT, &clockPoint);
    clockPoint = !clockPoint;
  }
}


/* Create and start a half-second timer */
static
void
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
  if (tps == 2) {
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
static
void
usage(char* program)
{
  printf("Usage:\n %s [-b] [-p <clock_point_mode>]\n", program);
  printf("\t a clock point: 0-always on, 1-once per second, 2-twice per second\n");
}

/* Get a clockpoint blink value from params */
static
uint8_t
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
void
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
        break;
    }
  }
}


/* Demonize wrapper */
void
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

  /* If run as a daemon oa as a control utility */
  if (backgroundRun)
    demonize();

  /* Clear a display and turn it on */
  ioctl(dev, TM1637_IOCTL_CLEAR);
  ioctl(dev, TM1637_IOCTL_ON);

  /* Intercept signals to our function */
  if (signal (SIGINT, termination_handler) == SIG_IGN)
    signal (SIGINT, SIG_IGN);
  if (signal (SIGTERM, termination_handler) == SIG_IGN)
    signal (SIGTERM, SIG_IGN);

  /* Run a half seconds redraw of the display */
  createTimer(tpsPoint);

  /* Main loop */
  while(true) {
    sleep(30);
  }

  exit(EXIT_SUCCESS);
}
