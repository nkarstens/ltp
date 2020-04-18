/*
 * Copyright (c) 2020 Garmin International, Inc.
 * Author: Nate Karstens <nate.karstens@garmin.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.
 */

/*
 * Test operation of fcntl(2) with the FD_CLOFORK file descriptor flag.
 */

#include <stdlib.h>
#include <unistd.h>

#include "tst_test.h"

#define TESTFILE "testfile"

#define TEST_STANDARD	0
#define TEST_CLOFORK	1
#define TEST_CNT	2

static void run(unsigned int test_nr)
{
	int fd;
	int fd_flags;
	pid_t pid;
	int status;
	int ttype;

	fd = SAFE_CREAT(TESTFILE, S_IRWXU);

	if (test_nr == TEST_CLOFORK)
		SAFE_FCNTL(fd, F_SETFD, FD_CLOFORK);

	fd_flags = SAFE_FCNTL(fd, F_GETFD);
	if ((fd_flags & FD_CLOFORK) && (test_nr != TEST_CLOFORK))
		tst_brk(TFAIL, "FD_CLOFORK should not be set");

	pid = SAFE_FORK();

	if (pid == -1) {
		tst_brk(TBROK | TERRNO, "Error forking child");
	} else if (pid == 0) {
		TEST(write(fd, &fd, sizeof(fd)));
		if (TST_RET == -1 && TST_ERR == EBADF)
			exit(TEST_CLOFORK);
		else
			exit(TEST_STANDARD);
	}

	SAFE_CLOSE(fd);
	SAFE_WAITPID(pid, &status, 0);

	if (!WIFEXITED(status))
		tst_brk(TBROK, "Child did not exit correctly");

	ttype = (WEXITSTATUS(status) == test_nr) ? TPASS : TFAIL;
	tst_res(ttype, "close-on-fork test with FD_CLOFORK %s",
		(test_nr == TEST_CLOFORK) ? "set" : "cleared");
}

static struct tst_test test = {
	.tcnt = TEST_CNT,
	.test = run,
//	.min_kver = "TODO: Update this",
	.needs_tmpdir = 1,
	.forks_child = 1,
};