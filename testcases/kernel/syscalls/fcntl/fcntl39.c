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
 * Test operation of fcntl(2) with the F_DUPFD_CLOFORK command.
 */

#include "tst_test.h"

#define TESTFILE "testfile"

static void run(void)
{
	int fd;
	int fd_dup;
	int fd_flags;

	fd = SAFE_CREAT(TESTFILE, S_IRWXU);

	fd_dup = SAFE_FCNTL(fd, F_DUPFD);
	fd_flags = SAFE_FCNTL(fd_dup, F_GETFD);
	if (fd_flags & FD_CLOFORK)
		tst_brk(TFAIL, "FD_CLOFORK should not be set");
	SAFE_CLOSE(fd_dup);

	fd_dup = SAFE_FCNTL(fd, F_DUPFD_CLOFORK);
	fd_flags = SAFE_FCNTL(fd_dup, F_GETFD);
	if (!(fd_flags & FD_CLOFORK))
		tst_brk(TFAIL, "FD_CLOFORK should not be set");
	SAFE_CLOSE(fd_dup);

	SAFE_CLOSE(fd);

	tst_res(TPASS, "fcntl F_DUPFD_CLOFORK test completed");
}

static struct tst_test test = {
	.test_all = run,
//	.min_kver = "TODO: Update this",
	.needs_tmpdir = 1,
};