/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright 2020 Oxide Computer Company
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/devpoll.h>

#include "common.h"

#if !defined(_OS_SUNOS)
#error "only to be built/run on illumos"
#endif

int
main(int argc, char **argv)
{
	int epfd, res, err;
	struct epoll_event epev;
	dvpoll_epollfd_t dpent[3];

	test_init(argc, argv);

	if ((epfd = epoll_create1(0)) < 0) {
		test_fatal("epoll_create1()");
	}

	/*
	 * Attempt to manually add 3 entries.  The internal limit should be two,
	 * in order to support the remove/add representing a modify action
	 */
	dpent[0].dpep_pollfd.fd = 0;
	dpent[0].dpep_pollfd.events = POLLIN;
	dpent[0].dpep_data = 0;
	dpent[1].dpep_pollfd.fd = 1;
	dpent[1].dpep_pollfd.events = POLLOUT;
	dpent[1].dpep_data = 1;
	dpent[2].dpep_pollfd.fd = 2;
	dpent[2].dpep_pollfd.events = POLLOUT;
	dpent[2].dpep_data = 2;

	res = write(epfd, dpent, sizeof (dpent));
	err = errno;

	test_equal(res, -1);
	test_equal(errno, EINVAL);

	/*
	 * But a normal add and then modify should work fine.
	 */
	epev.events = EPOLLIN;
	epev.data.fd = 0;

	res = epoll_ctl(epfd, EPOLL_CTL_ADD, 0, &epev);
	test_equal(res, 0);
	epev.events = EPOLLIN|EPOLLOUT;
	res = epoll_ctl(epfd, EPOLL_CTL_MOD, 0, &epev);
	test_equal(res, 0);

	(void) close(epfd);
	return (0);
}
