/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright 2020 Oxide Computer Company
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>

#include "common.h"


/*
 * Some software makes use of a self pipe to wake up an event loop; i.e., a
 * pipe where the read end is in the epoll set and the write end is available
 * to other threads in order to inject wakeups.
 *
 * On Linux systems, an edge-triggered wait on a pipe will fire for every write
 * to the pipe, even if that pipe has not been read after poll.  Some software
 * in the wild has been observed to make use of this behaviour, draining the
 * pipe only once the pipe buffer is full or nearing full.
 */
int
main(int argc, char **argv)
{
	int efd, pipefd[2], res, err;
	struct epoll_event ev;
	char b = '\0';

	test_init(argc, argv);

	if (pipe2(pipefd, O_NONBLOCK) != 0) {
		test_fatal("pipe()");
	}

	if ((efd = epoll_create1(EPOLL_CLOEXEC)) < 0) {
		test_fatal("epoll_create1()");
	}
	ev.events = EPOLLIN|EPOLLRDHUP|EPOLLET;
	ev.data.fd = pipefd[0];
	if (epoll_ctl(efd, EPOLL_CTL_ADD, pipefd[0], &ev) < 0) {
		test_fatal("epoll_ctl(EPOLL_CTL_ADD)");
	}

	/*
	 * No events should be pending to start:
	 */
	test_equal(epoll_wait(efd, &ev, 1, 0), 0);

	/*
	 * Trigger a wake-up on the pipe by writing a single byte:
	 */
	if (write(pipefd[1], &b, 1) != 1) {
		test_fatal("write()");
	}

	test_equal(epoll_wait(efd, &ev, 1, 0), 1);

	/*
	 * Trigger another wake-up on the pipe without having done a read:
	 */
	if (write(pipefd[1], &b, 1) != 1) {
		test_fatal("write()");
	}

	test_equal(epoll_wait(efd, &ev, 1, 0), 1);

	test_done();
}
