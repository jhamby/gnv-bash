/* File: test_getpwuid.c
 *
 * $Id: test_getpwuid.c,v 1.1.1.1 2012/12/02 19:25:22 wb8tyw Exp $
 *
 * This program is to test a wrapper to the getpwd() function.
 *
 * Copyright 2012, John Malmberg
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */


#ifdef VMS_PASSWORD_WRAPPER
#pragma message disable pragma
#pragma message disable unusedtop
#include "vms_pwd_hack.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>

int main(int argc, char ** argv, char **env) {

struct passwd * pwentry;
uid_t uid;

    uid = getuid();

    pwentry = getpwuid(uid);

    if (pwentry != NULL) {
	printf("pw_name = %s\n", pwentry->pw_name);
	printf("pw_shell = %s\n", pwentry->pw_shell);
	printf("pw_dir = %s\n", pwentry->pw_dir);
	printf("pw_uid = %d\n", pwentry->pw_uid);
	printf("pw_gid = %d\n", pwentry->pw_gid);
#ifdef VMS_PASSWORD_WRAPPER
	printf("pw_gecos = %s\n", pwentry->pw_gecos);
	printf("pw_passwd = %s\n", pwentry->pw_passwd);
#endif
    } else {
	perror("getpwduid failed");
    }

    exit(0);
}
