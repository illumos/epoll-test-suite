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
	int efd;

	test_init(argc, argv);

	if ((efd = epoll_create1(0)) < 0) {
		test_fatal("epoll_create1()");
	} else {
		test_pass();
	}
	(void) close(efd);

	if ((efd = epoll_create1(EPOLL_CLOEXEC)) < 0) {
		test_fatal("epoll_create1()");
	} else {
		test_pass();
	}
	(void) close(efd);

	if ((efd = epoll_create1(~EPOLL_CLOEXEC)) >= 0) {
		test_fatal("epoll_create1() succeeded on bad flags");
	}
	test_equal(errno, EINVAL);

	test_done();
}
