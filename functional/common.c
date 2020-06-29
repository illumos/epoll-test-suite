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
#include <errno.h>
#include <string.h>
#include <stdarg.h>

static char *g_testname;
static int g_testnum = 0;

static int g_result = EXIT_SUCCESS;

void
test_init(int argc, char **argv)
{
	char *last_slash;
	if (argc < 1) {
		abort();
	}

	last_slash = strrchr(argv[0], '/');
	if (last_slash == NULL) {
		g_testname = argv[0];
	} else {
		g_testname = last_slash + 1;
	}
}

void
test_pass()
{
	printf("%s\t%d\tTPASS\n", g_testname, g_testnum);
	g_testnum++;
}

void
test_warn(char *reason, ...)
{
	va_list ap;
	printf("%s\t%d\tTWARN: ", g_testname, g_testnum);

	va_start(ap, reason);
	vprintf(reason, ap);
	printf("\n");
}

void
test_fail(char *reason, ...)
{
	va_list ap;
	printf("%s\t%d\tTFAIL: ", g_testname, g_testnum);

	va_start(ap, reason);
	vprintf(reason, ap);
	printf("\n");

	g_testnum++;
	g_result = EXIT_FAILURE;
}

void
test_equal(int a, int b) {
	if (a == b) {
		test_pass();
	} else {
		test_fail("%d != %d", a, b);
	}
}

void
test_ok(int cond, char *msg)
{
	if (cond) {
		test_pass();
	} else {
		test_fail(msg);
	}
}

void
test_done()
{
	exit(g_result);
}

void
test_fatal(char *fmt, ...)
{
	va_list ap;
	int error = errno;

	va_start(ap, fmt);

	(void) fprintf(stderr, "%s\t%d\tTBROK: ", g_testname, g_testnum);
	(void) vfprintf(stderr, fmt, ap);

	if (fmt[strlen(fmt) - 1] != '\n') {
		(void) fprintf(stderr, ": %s\n", strerror(error));
	}

	exit(EXIT_FAILURE);
}
