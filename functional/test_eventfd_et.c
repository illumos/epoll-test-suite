/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright 2024 Oxide Computer Company
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>

#include "common.h"

/*
 * Some software makes use of an eventfd to wake up an event loop.
 *
 * On Linux systems, an edge-triggered wait on an eventfd descriptor will fire
 * for every write, even if that descriptor has not been read after poll.
 */
int
main(int argc, char **argv)
{
	int efd, evfd, res, err;
	struct epoll_event ev;
	eventfd_t val = 1;

	test_init(argc, argv);

	if ((evfd = eventfd(0, EFD_NONBLOCK)) == -1)
		test_fatal("eventfd()");

	if ((efd = epoll_create1(EPOLL_CLOEXEC)) < 0)
		test_fatal("epoll_create1()");

	ev.events = EPOLLIN|EPOLLRDHUP|EPOLLET;
	ev.data.fd = evfd;
	if (epoll_ctl(efd, EPOLL_CTL_ADD, evfd, &ev) < 0)
		test_fatal("epoll_ctl(EPOLL_CTL_ADD)");

	/*
	 * No events should be pending to start
	 */
	test_equal(epoll_wait(efd, &ev, 1, 0), 0);

	/*
	 * Trigger a wake-up by writing 1
	 */
	if (eventfd_write(evfd, val) == -1)
		test_fatal("eventfd_write()");

	test_equal(epoll_wait(efd, &ev, 1, 0), 1);
	test_equal(ev.events, EPOLLIN);
	test_equal(ev.data.fd, evfd);

	/*
	 * Trigger another wake-up without having done a read
	 */
	if (eventfd_write(evfd, val) == -1)
		test_fatal("eventfd_write()");

	test_equal(epoll_wait(efd, &ev, 1, 0), 1);
	test_equal(ev.events, EPOLLIN);
	test_equal(ev.data.fd, evfd);

	test_done();
}
