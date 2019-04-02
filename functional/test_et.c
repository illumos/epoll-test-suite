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
#include <pthread.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>

#include "common.h"

int evfd = -1;
int ready = 0;
pthread_mutex_t lock;

void *
writer_func(void *args) {
	uint64_t val = 1;

	ready = 1;
	pthread_mutex_lock(&lock);
	pthread_mutex_unlock(&lock);

	/*
	 * Allow the main thread time to enter the epoll_ctl.  Strictly speaking,
	 * this does have the potential to race.  There doesn't appear to be a
	 * convenient way to ensure proper ordering here.  The sleep should be
	 * adequate for all but the most pathological cases.
	 */
	usleep(1000);

	write(evfd, &val, sizeof (val));
	return (NULL);
}


int
main(int argc, char *argv[])
{
	pthread_t thread_write;
	struct epoll_event ev;
	int epfd;

	test_init(argc, argv);

	if ((evfd = eventfd(0, EFD_CLOEXEC|EFD_NONBLOCK)) < 0) {
		test_fatal("eventfd()");
	}
	if ((epfd = epoll_create1(0)) < 0) {
		test_fatal("epoll_create1()");
	}

	ev.events = EPOLLIN|EPOLLOUT|EPOLLET;
	ev.data.fd = evfd;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, evfd, &ev) < 0) {
		test_fatal("epoll_ctl(EPOLL_CTL_ADD)");
	}

	/* The eventfd is writable */
	test_equal(epoll_wait(epfd, &ev, 1, 0), 1);
	test_equal(ev.events, EPOLLOUT);
	test_equal(ev.data.fd, evfd);

	/* ET should yield no event now */
	test_equal(epoll_wait(epfd, &ev, 1, 0), 0);

	if (pthread_mutex_init(&lock, NULL) != 0) {
		test_fatal("pthread_mutex_init");
	}
	pthread_mutex_lock(&lock);
	pthread_create(&thread_write, NULL, writer_func, NULL);

	while (!ready) {
		usleep(1000);
	}
	pthread_mutex_unlock(&lock);

	/* The eventfd should report as readable once the above write succeeds */
	test_equal(epoll_wait(epfd, &ev, 1, 1000), 1);
	test_equal(ev.events, EPOLLIN|EPOLLOUT);
	test_equal(ev.data.fd, evfd);

	pthread_join(thread_write, NULL);
	test_done();
}

