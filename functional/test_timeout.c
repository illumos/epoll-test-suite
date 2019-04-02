/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright 2019 Joyent, Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <strings.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/time.h>

#ifdef _OS_LINUX
#include <time.h>
#endif /* _OS_LINUX */

#include "common.h"

static int
duration_within_range(struct timespec before, struct timespec after,
    int ms_lower, int ms_upper)
{
	after.tv_nsec += (after.tv_sec - before.tv_sec) * 1000000000;
	after.tv_nsec -= before.tv_nsec;
	after.tv_nsec /= 1000000;

	return (after.tv_nsec <= ms_upper && after.tv_nsec >= ms_lower);
}

static void
alarm_handler(int sig)
{
	return;
}

int
main(int argc, char **argv)
{
	int efd, pipefd[2], res, err;
	struct epoll_event ev;
	struct itimerval itv;
	struct timespec before, after;
	struct sigaction sa;

	test_init(argc, argv);

	if (pipe(pipefd) != 0) {
		test_fatal("pipe()");
	}

	if ((efd = epoll_create1(0)) < 0) {
		test_fatal("epoll_create1()");
	}
	ev.events = EPOLLIN;
	ev.data.fd = pipefd[0];
	if (epoll_ctl(efd, EPOLL_CTL_ADD, pipefd[0], &ev) < 0) {
		test_fatal("epoll_ctl(EPOLL_CTL_ADD)");
	}

	/* No events should be pending */
	test_equal(epoll_wait(efd, &ev, 1, 0), 0);

	/* setup appropriate noop handler for SIGALRM */
	sa.sa_handler = alarm_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	if (sigaction(SIGALRM, &sa, NULL) != 0) {
		test_fatal("sigaction(SIGALRM) failed");
	}

	clock_gettime(CLOCK_REALTIME, &before);
	bzero(&itv, sizeof (itv));
	itv.it_value.tv_usec = 500000;
	test_equal(setitimer(ITIMER_REAL, &itv, NULL), 0);

	/* Normal indefinite timeout */
	res = epoll_wait(efd, &ev, 1, -1);
	err = errno;
	clock_gettime(CLOCK_REALTIME, &after);


	test_equal(res, -1);
	test_equal(err, EINTR);
	test_ok(duration_within_range(before, after, 450, 550),
	    "exceeded timeout interval");

	clock_gettime(CLOCK_REALTIME, &before);
	test_equal(setitimer(ITIMER_REAL, &itv, NULL), 0);

	/* Excessively negative indefinite timeout */
	res = epoll_wait(efd, &ev, 1, -200);
	err = errno;
	clock_gettime(CLOCK_REALTIME, &after);

	test_equal(res, -1);
	test_equal(err, EINTR);
	test_ok(duration_within_range(before, after, 450, 550),
	    "exceeded timeout interval");

	clock_gettime(CLOCK_REALTIME, &before);
	itv.it_value.tv_sec = 1;
	itv.it_value.tv_usec = 0;
	test_equal(setitimer(ITIMER_REAL, &itv, NULL), 0);

	/* Normal timeout */
	res = epoll_wait(efd, &ev, 1, 500);
	clock_gettime(CLOCK_REALTIME, &after);

	test_equal(res, 0);
	test_ok(duration_within_range(before, after, 450, 550),
	    "exceeded timeout interval");

	/* clear timer (it should be uneeded) */
	itv.it_value.tv_sec = 0;
	itv.it_value.tv_usec = 0;
	test_equal(setitimer(ITIMER_REAL, &itv, NULL), 0);

	test_done();
}
