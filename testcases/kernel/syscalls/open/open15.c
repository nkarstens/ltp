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
 * Test operation of open(2) with the O_CLOFORK flag.
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
	int o_flags;
	int fd;
	int fd_flags;

	o_flags = O_CREAT | O_WRONLY;
	if (test_nr == TEST_CLOFORK)
		o_flags |= O_CLOFORK;

	fd = SAFE_OPEN(TESTFILE, o_flags, S_IRWXU);

	fd_flags = SAFE_FCNTL(fd, F_GETFD);
	if (!(fd_flags & FD_CLOFORK) != !(o_flags & O_CLOFORK))
		tst_brk(TFAIL, "FD flags do not match open flags");

	SAFE_CLOSE(fd);

	tst_res(TPASS, "open() test with close-on-fork %s",
		(test_nr == TEST_CLOFORK) ? "set" : "cleared");
}

static struct tst_test test = {
	.tcnt = TEST_CNT,
	.test = run,
//	.min_kver = "TODO: Update this",
	.needs_tmpdir = 1,
};