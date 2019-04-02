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
#include <sys/epoll.h>

#include "common.h"


int
main(int argc, char **argv)
{
	int fd, epfd, pipefd[2], targetfd, res, err;
	struct epoll_event ev;
	char filename[] = "/tmp/epolltst_XXXXXX";
	char b = '\0';

	test_init(argc, argv);

	if ((epfd = epoll_create1(0)) < 0) {
		test_fatal("epoll_create1()");
	}
	if ((fd = mkstemp(filename)) < 0) {
		test_fatal("mkstemp()");
	}
	if (unlink(filename) != 0) {
		test_fatal("unlink()");
	}
	if (pipe(pipefd) != 0) {
		test_fatal("pipe()");
	}
	if ((targetfd = dup(pipefd[0])) < 0) {
		test_fatal("dup()");
	}

	ev.events = EPOLLIN;
	ev.data.fd = targetfd;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, targetfd, &ev) < 0) {
		test_fatal("epoll_ctl(EPOLL_CTL_ADD)");
	}
	if (write(pipefd[1], &b, 1) != 1) {
		test_fatal("write()");
	}

	test_equal(epoll_wait(epfd, &ev, 1, 0), 1);

	/*
	 * Replace an fd in the epoll set to ensure that the "new" resource is not
	 * polled (unless explicitly added).
	 */
	if (dup2(fd, targetfd) < 0) {
		test_fatal("dup2()");
	}
	test_ok(epoll_wait(epfd, &ev, 1, 0) >= 0, "epoll_wait() no EPERM");

	(void) close(fd);
	(void) close(epfd);
	(void) close(pipefd[0]);
	(void) close(pipefd[1]);
	return (0);
}
