/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright 2019 Joyent, Inc.
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>

#include "common.h"

#ifndef O_DIRECTORY
#define	O_DIRECTORY	0
#endif

int
main(int argc, char **argv)
{
	int fd, epfd, res, err;
	struct epoll_event epev;

	test_init(argc, argv);

	if ((epfd = epoll_create1(0)) < 0) {
		test_fatal("epoll_create1()");
	}
	if ((fd = open(".", O_RDONLY|O_DIRECTORY, 0)) < 0) {
		test_fatal("open()");
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
