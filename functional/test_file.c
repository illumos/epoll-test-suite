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
	int fd, epfd, res, err;
	struct epoll_event epev;
	char filename[] = "/tmp/epolltst_XXXXXX";

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

	epev.events = EPOLLIN|EPOLLOUT;
	epev.data.fd = fd;

	res = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &epev);
	err = errno;
	test_equal(res, -1);
	test_equal(err, EPERM);

	(void) close(fd);
	(void) close(epfd);
	return (0);
}
