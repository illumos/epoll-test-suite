#ifndef _COMMON_H_
#define _COMMON_H_

extern void test_init(int, char **);
extern void test_fail(char *, ...);
extern void test_pass();
extern void test_equal(int, int);
extern void test_fatal(char *, ...);
extern void test_done();
extern char *g_execname;

#endif /* _COMMON_H_ */
