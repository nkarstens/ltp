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
 * Test operation of unshare(2) with the FD_CLOFORK file descriptor flag.
 */

#define _GNU_SOURCE
#include <sched.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include "tst_clone.h"
#include "tst_test.h"

#define TESTFILE "testfile"

static const struct {
	int set_clofork;
	int parent_unshare;
	int parent_can_write;
	int child_can_write;
} test_params[] = {
	{
		.set_clofork		= 0,
		.parent_unshare		= 0,
		.parent_can_write	= 1,
		.child_can_write	= 1,
	},
	{
		.set_clofork		= 0,
		.parent_unshare		= 1,
		.parent_can_write	= 1,
		.child_can_write	= 1,
	},
	{
		.set_clofork		= 1,
		.parent_unshare		= 0,
		.parent_can_write	= 1,
		.child_can_write	= 0,
	},
	{
		.set_clofork		= 1,
		.parent_unshare		= 1,
		.parent_can_write	= 0,
		.child_can_write	= 1,
	}
};

static int file_fd;
static int event_fd;

static int clone_fn(void *arg)
{
	unsigned int test_nr = (uintptr_t)arg;
	eventfd_t event_val;

	if (test_params[test_nr].parent_unshare) {
		eventfd_read(event_fd, &event_val);
	} else {
		TEST(unshare(CLONE_FILES));
		if (TST_RET == -1)
			return TBROK;

		eventfd_write(event_fd, 1);
	}

	TEST(write(file_fd, &file_fd, sizeof(file_fd)));
	return ((TST_RET > 0 &&
		  test_params[test_nr].child_can_write) ||
		(TST_RET == -1 && TST_ERR == EBADF &&
		 !test_params[test_nr].child_can_write)) ? TPASS : TFAIL;
}

static void run(unsigned int test_nr)
{
	int thread_id;
	int status;
	int ttype;
	eventfd_t event_val;

	file_fd = SAFE_CREAT(TESTFILE, S_IRWXU);
	event_fd = eventfd(0, 0);

	if (test_params[test_nr].set_clofork)
		SAFE_FCNTL(file_fd, F_SETFD, FD_CLOFORK);

	thread_id = ltp_clone_quick(CLONE_FILES, clone_fn, (void *)(uintptr_t)test_nr);

	if (thread_id == -1)
		tst_brk(TBROK | TERRNO, "Error cloning child");

	if (test_params[test_nr].parent_unshare) {
		TEST(unshare(CLONE_FILES));
		if (TST_RET == -1)
			tst_brk(TBROK | TERRNO, "Error calling unshare");

		eventfd_write(event_fd, 1);
	} else {
		eventfd_read(event_fd, &event_val);
	}

	SAFE_WAITPID(thread_id, &status, __WCLONE);

	TEST(write(file_fd, &file_fd, sizeof(file_fd)));
	ttype = ((TST_RET > 0 &&
		  test_params[test_nr].parent_can_write) ||
		(TST_RET == -1 && TST_ERR == EBADF &&
		 !test_params[test_nr].parent_can_write)) ? TPASS : TFAIL;

	if (test_params[test_nr].parent_can_write)
		SAFE_CLOSE(file_fd);

	SAFE_CLOSE(event_fd);

	if (!WIFEXITED(status))
		tst_brk(TBROK, "Child did not exit correctly");

	if (ttype == TPASS)
		ttype = WEXITSTATUS(status);

	tst_res(ttype,
		"close-on-fork test with FD_CLOFORK %s and %s calls unshare",
		test_params[test_nr].set_clofork ? "set" : "cleared",
		test_params[test_nr].parent_unshare ? "parent" : "child");
}

static struct tst_test test = {
	.tcnt = sizeof(test_params) / sizeof(test_params[0]),
	.test = run,
//	.min_kver = "TODO: Update this",
	.needs_tmpdir = 1,
};