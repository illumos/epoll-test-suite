/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright 2019 Joyent, Inc.
 */

#ifndef _COMMON_H_
#define _COMMON_H_

extern void test_init(int, char **);
extern void test_fail(char *, ...);
extern void test_pass();
extern void test_equal(int, int);
extern void test_ok(int, char *);
extern void test_fatal(char *, ...);
extern void test_done();
extern char *g_execname;

#endif /* _COMMON_H_ */
