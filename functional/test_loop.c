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
#include <sys/epoll.h>
#include <errno.h>

#include "common.h"

int
main(int argc, char **argv)
{
	int pipe_fd[2], dup_fd[2];
	int res, efd1, efd2;
	struct epoll_event ev;
	char buf;

	test_init(argc, argv);

	if (pipe(pipe_fd) != 0) {
		test_fatal("pipe()");
	}
	if (write(pipe_fd[1], "\0", 1) != 1) {
		test_fatal("write()");
	}

	if ((efd1 = epoll_create1(0)) < 0) {
		test_fatal("epoll_create1()");
	}
	if ((efd2 = epoll_create1(0)) < 0) {
		test_fatal("epoll_create1()");
	}

	ev.events = EPOLLIN;
	ev.data.fd = pipe_fd[0];
	if (epoll_ctl(efd1, EPOLL_CTL_ADD, pipe_fd[0], &ev) < 0) {
		test_fatal("epoll_ctl(EPOLL_CTL_ADD)");
	}

	ev.events = EPOLLIN;
	ev.data.fd = efd1;
	if (epoll_ctl(efd2, EPOLL_CTL_ADD, efd1, &ev) < 0) {
		test_fatal("epoll_ctl(EPOLL_CTL_ADD)");
	}

	ev.events = EPOLLIN;
	ev.data.fd = efd2;
	if (epoll_ctl(efd1, EPOLL_CTL_ADD, efd2, &ev) < 0) {
		test_equal(errno, ELOOP);
	} else {
		test_fail("created epoll loop");
	}

	test_done();
}
