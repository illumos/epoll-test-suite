#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>

#include "common.h"

#define	TARGETEVENT	EPOLLERR
#if defined(_OS_SUNOS)
/*
 * Unfortunately, the behavior of pipe closure differs between Linux and
 * illumos.  Linux will emit EPOLLERR when the pipe writer (fd[1]) is closed
 * and EPOLLHUP when the reader is closed.  On illumos, EPOLLHUP is emitted
 * when either side is closed on the other.
 *
 * Redefining this on SunOS makes this test a little redundant with
 * test_hupevent, but it ensures proper coverage on Linux.
 */
#undef	TARGETEVENT
#define	TARGETEVENT	EPOLLHUP
#endif

int
main(int argc, char **argv)
{
	int efd, pipefd[2], res, err;
	struct epoll_event ev;
	char b = '\0';

	test_init(argc, argv);

	if (pipe(pipefd) != 0) {
		test_fatal("pipe()");
	}

	if ((efd = epoll_create1(0)) < 0) {
		test_fatal("epoll_create1()");
	}
	ev.events = 0;
	ev.data.fd = pipefd[1];
	if (epoll_ctl(efd, EPOLL_CTL_ADD, pipefd[1], &ev) < 0) {
		test_fatal("epoll_ctl(EPOLL_CTL_ADD)");
	}

	/* No events should be pending */
	test_equal(epoll_wait(efd, &ev, 1, 0), 0);

	if (close(pipefd[0]) != 0) {
		test_fatal("close()");
	}

	/*
	 * According to the Linux man page, the EPOLLERR event is reported event
	 * when it has not been set in the events for the subscription.
	 */
	test_equal(epoll_wait(efd, &ev, 1, 0), 1);
	test_equal(ev.events, TARGETEVENT);

	if (epoll_ctl(efd, EPOLL_CTL_DEL, pipefd[1], NULL) < 0) {
		test_fatal("epoll_ctl(EPOLL_CTL_DEL)");
	}

	ev.events = EPOLLONESHOT;
	ev.data.fd = pipefd[1];
	if (epoll_ctl(efd, EPOLL_CTL_ADD, pipefd[1], &ev) < 0) {
		test_fatal("epoll_ctl(EPOLL_CTL_ADD)");
	}

	/* Make sure it behaves as expected in oneshot mode too */
	test_equal(epoll_wait(efd, &ev, 1, 0), 1);
	test_equal(ev.events, TARGETEVENT);
	test_equal(epoll_wait(efd, &ev, 1, 0), 0);

	test_done();
}
