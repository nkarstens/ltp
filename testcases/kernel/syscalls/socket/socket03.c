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
 * Test operation of socket(2) with the SOCK_CLOFORK flag.
 */

#include "tst_test.h"

#define TEST_STANDARD	0
#define TEST_CLOFORK	1
#define TEST_CNT	2

static void run(unsigned int test_nr)
{
	int sock_flags;
	int sock;
	int fd_flags;

	sock_flags = (test_nr == TEST_CLOFORK) ? SOCK_CLOFORK : 0;
	sock = SAFE_SOCKET(AF_INET, SOCK_STREAM | sock_flags, 0);

	fd_flags = SAFE_FCNTL(sock, F_GETFD);
	if (!(fd_flags & FD_CLOFORK) != !(sock_flags & SOCK_CLOFORK))
		tst_brk(TFAIL, "FD flags do not match socket flags");

	SAFE_CLOSE(sock);

	tst_res(TPASS, "socket() test with SOCK_CLOFORK %s",
		(test_nr == TEST_CLOFORK) ? "set" : "cleared");
}

static struct tst_test test = {
	.tcnt = TEST_CNT,
	.test = run,
//	.min_kver = "TODO: Update this",
};