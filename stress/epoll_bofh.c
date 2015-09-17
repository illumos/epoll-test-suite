#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>
#include <pthread.h>
#include <stdarg.h>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <sys/time.h>

char *g_cmd = "epoll_bofh";

int *g_timers;
int g_ntimers = 10;
int g_timermax;

int *g_epfds;
int g_nepfds = 10;

int g_weight;

int g_threadcount;

pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;

#ifndef MILLISEC
#define MILLISEC	1000
#define	NANOSEC		1000000000
#endif

static void
fatal(char *fmt, ...)
{
	va_list ap;
	int error = errno;

	va_start(ap, fmt);

	(void) fprintf(stderr, "%s: ", g_cmd);
	(void) vfprintf(stderr, fmt, ap);

	if (fmt[strlen(fmt) - 1] != '\n')
		(void) fprintf(stderr, ": %s\n", strerror(error));

	exit(EXIT_FAILURE);
}

void
timers_create()
{
	int i, fd;
	struct itimerspec val;
	uint64_t ms = 1;

	if ((g_timers = malloc(g_ntimers * sizeof (int))) == NULL)
		fatal("couldn't allocate timers");

	bzero(&val, sizeof (val));

	for (i = 0; i < g_ntimers; i++) {
		if ((fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK)) == -1)
			fatal("timerfd_create %d", i);

		val.it_value.tv_sec = ms / MILLISEC;
		val.it_value.tv_nsec = (ms % MILLISEC) * (NANOSEC / MILLISEC);

		val.it_interval.tv_sec = val.it_value.tv_sec;
		val.it_interval.tv_nsec = val.it_value.tv_nsec;

		if (timerfd_settime(fd, 0, &val, NULL) == -1)
			fatal("timer_settime");
		g_timers[i] = fd;
		ms <<= 1;
	}

	g_timermax = fd;
}

int
timers_random()
{
	return (g_timers[rand() % g_ntimers]);
}

void
epfds_create()
{
	int i;

	if ((g_epfds = malloc(g_nepfds * sizeof (int))) == NULL)
		fatal("couldn't allocate epoll file descriptors");
 
	for (i = 0; i < g_nepfds; i++)
		g_epfds[i] = -1;
}

int
epfds_random()
{
	return (g_epfds[rand() % g_nepfds]);
}

void
work_timer_add()
{
	int fd = timers_random();
	int epfd = epfds_random();
	struct epoll_event evt;

	bzero(&evt, sizeof (evt));
	evt.data.fd = fd;
	evt.events = EPOLLIN;

	(void) epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &evt);
}

void
work_timer_remove()
{
	int fd = timers_random();
	int epfd = epfds_random();
	struct epoll_event evt;

	bzero(&evt, sizeof (evt));
	(void) epoll_ctl(fd, EPOLL_CTL_DEL, fd, &evt);
}

void
work_epoll_add()
{
	int fd = epfds_random();
	int epfd = epfds_random();
	struct epoll_event evt;

	bzero(&evt, sizeof (evt));
	evt.data.fd = fd;
	evt.events = EPOLLIN;

	(void) epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &evt);
}

void
work_epoll_remove()
{
	int fd = epfds_random();
	int epfd = epfds_random();
	struct epoll_event evt;

	bzero(&evt, sizeof (evt));
	(void) epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &evt);
}

void
work_epoll_wait()
{
	int epfd = epfds_random(), i, rval;
	uint64_t count;
	struct epoll_event evts[256];

	if ((rval = epoll_wait(epfd, evts,
	    sizeof (evts) / sizeof (evts[0]), 1000)) == -1)
		return;

	for (i = 0; i < rval; i++) {
		if (evts[i].data.fd > g_timermax)
			continue;

		(void) read(evts[i].data.fd, &count, sizeof (count));
	}
}

void
work_epoll_close()
{
	int epfd, i;

	pthread_mutex_lock(&g_lock);
	epfd = epfds_random();

	if (epfd == -1) {
		pthread_mutex_unlock(&g_lock);
		return;
	}

	for (i = 0; i < g_nepfds; i++) {
		if (g_epfds[i] == epfd) {
			g_epfds[i] = -1;
			break;
		}
	}

	pthread_mutex_unlock(&g_lock);

	close(epfd);
}

void
work_epoll_open()
{
	int i;

	pthread_mutex_lock(&g_lock);

	for (i = 0; i < g_nepfds; i++) {
		if (g_epfds[i] != -1)
			continue;

		g_epfds[i] = epoll_create(1);
		break;
	}

	pthread_mutex_unlock(&g_lock);
}

#define WORK(func) work_##func, #func

struct {
	void (*func)(void);
	char *desc;
	int weight;
} g_worktab[] = {
	{ WORK(timer_add), 12 },
	{ WORK(timer_remove), 10 },
	{ WORK(epoll_add), 15 },
	{ WORK(epoll_remove), 5 },
	{ WORK(epoll_close), 4 },
	{ WORK(epoll_open), 7 },
	{ WORK(epoll_wait), 30 },
	{ NULL }
};

void
work_init()
{
	int i;

	for (i = 0; g_worktab[i].func != NULL; i++) {
		g_weight += g_worktab[i].weight;
		g_worktab[i].weight = g_weight;
	}
}

void
work()
{
	int next = rand() % g_weight, i;

	for (i = 0; g_worktab[i].func != NULL; i++) {
		if (next < g_worktab[i].weight)
			break;
	}

/*
	printf("doing %s...\n", g_worktab[i].desc);
*/
	g_worktab[i].func();
}

void *
work_forever(void *arg)
{
	for (;;)
		work();
}

void
usage()
{
	printf("Usage: %s <thread count>\n", g_cmd);
	exit(EXIT_FAILURE);
}

void
parse_args(int argc, char **argv)
{
	if (argc != 2) {
		usage();
	}
	errno = 0;
	g_threadcount = strtol(argv[1], NULL, 10);
	if (errno != 0 || g_threadcount < 1) {
		usage();
	}
}

int
main(int argc, char **argv)
{
	int i;
	pthread_t tid;

	parse_args(argc, argv);

	timers_create();
	epfds_create();
	work_init();

	for (i = 0; i < g_threadcount; i++) {
		if (pthread_create(&tid, NULL, &work_forever, NULL) != 0) {
			fatal("could not spawn thread: %d", i);
		}
	}

	/* block main thread from exiting */
	(void) pthread_join(tid, NULL);
}
