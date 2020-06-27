/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright 2019 Joyent, Inc.
 * Copyright 2020 Oxide Computer Company
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
	int pipefd[2], epfd, res, err;
	struct epoll_event epev;
	char b = '\0';

	test_init(argc, argv);

	if (pipe(pipefd) != 0) {
		test_fatal("pipe()");
	}

	if ((epfd = epoll_create1(0)) < 0) {
		test_fatal("epoll_create1()");
	}

	epev.events = EPOLLIN|EPOLLEXCLUSIVE;
	epev.data.fd = pipefd[0];

	/* Exclusive add is OK */
	res = epoll_ctl(epfd, EPOLL_CTL_ADD, pipefd[0], &epev);
	test_equal(res, 0);

	epev.events = EPOLLOUT;
	epev.data.fd = pipefd[1];
	res = epoll_ctl(epfd, EPOLL_CTL_ADD, pipefd[1], &epev);
	test_equal(res, 0);

	/* Modify to exclusive is not allowed */
	epev.events = EPOLLOUT|EPOLLEXCLUSIVE;
	res = epoll_ctl(epfd, EPOLL_CTL_MOD, pipefd[1], &epev);
	err = errno;
	test_equal(res, -1);
	test_equal(err, EINVAL);

	res = epoll_ctl(epfd, EPOLL_CTL_DEL, pipefd[1], NULL);
	test_equal(res, 0);

	test_equal(epoll_wait(epfd, &epev, 1, 0), 0);

	if (write(pipefd[1], &b, 1) != 1) {
		test_fatal("write()");
	}

	/*
	 * The semantics of EPOLLEXCLUSIVE are pretty loose, so we only test
	 * that an event properly arrives.
	 */
	test_equal(epoll_wait(epfd, &epev, 1, 0), 1);
	test_equal(epev.data.fd, pipefd[0]);

	(void) close(pipefd[0]);
	(void) close(pipefd[1]);
	(void) close(epfd);
	return (0);
}
