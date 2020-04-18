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
 * Test operation of accept4(2) with the SOCK_CLOFORK flag.
 */

#define _GNU_SOURCE

#include <sys/socket.h>

#include "tst_test.h"

#define TEST_STANDARD	0
#define TEST_CLOFORK	1
#define TEST_CNT	2

static void run(unsigned int test_nr)
{
	int sock_listener, sock_client, sock_incoming;
	struct sockaddr_in addr;
	socklen_t addrlen;
	int sock_flags;
	int fd_flags;

	sock_listener = SAFE_SOCKET(AF_INET, SOCK_STREAM, 0);

	addr.sin_family = AF_INET;
	addr.sin_port = 0;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	SAFE_BIND(sock_listener, (struct sockaddr *)&addr, sizeof(addr));
	SAFE_LISTEN(sock_listener, 1);

	addrlen = sizeof(addr);
	SAFE_GETSOCKNAME(sock_listener, (struct sockaddr *)&addr, &addrlen);

	sock_client = SAFE_SOCKET(AF_INET, SOCK_STREAM, 0);
	SAFE_CONNECT(sock_client, (struct sockaddr *)&addr, sizeof(addr));

	sock_flags = (test_nr == TEST_CLOFORK) ? SOCK_CLOFORK : 0;

	sock_incoming = accept4(sock_listener, NULL, NULL, sock_flags);
	if (sock_incoming < 0)
		tst_brk(TBROK | TERRNO, "accept4() failed");

	fd_flags = SAFE_FCNTL(sock_incoming, F_GETFD);
	if (!(fd_flags & FD_CLOFORK) != !(sock_flags & SOCK_CLOFORK))
		tst_brk(TFAIL, "FD flags do not match socket flags");

	SAFE_CLOSE(sock_incoming);
	SAFE_CLOSE(sock_client);
	SAFE_CLOSE(sock_listener);

	tst_res(TPASS, "accept4() test with SOCK_CLOFORK %s",
		(test_nr == TEST_CLOFORK) ? "set" : "cleared");
}

static struct tst_test test = {
	.tcnt = TEST_CNT,
	.test = run,
//	.min_kver = "TODO: Update this",
	.needs_tmpdir = 1,
};