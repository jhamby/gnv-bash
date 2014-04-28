/* File: vms_mailstat.c
 *
 * $Id: vms_mailstat.c,v 1.3 2013/06/09 17:07:00 wb8tyw Exp $
 *
 * These are routines to use the mail utility routines to simulate a
 * Unix/Linux mailstat() routine when passed a file specification that
 * matches a typical VMS user's mail.mai file.
 *
 * This module containts a unit test.
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

#include <stat.h>
#include <pwd.h>
#include <unistd.h>
#include <string.h>
#include <descrip.h>
#include <efndef.h>
#include <jpidef.h>
#include <errno.h>
#include <maildef.h>
#include <namdef.h>
#include <stsdef.h>
#include <unixlib.h>
#include <stdio.h>
#define VMS_MAXRSS NAM$C_MAXRSS
/* Note: The mail$ routines can not handle long filenames */

#ifdef TEST_VMS_MAILSTAT
#define mailstat(a,b) (1)
#include <stdlib.h>
#include <unixlib.h>
#else
int
mailstat(const char *path, struct stat *st);
#endif

#pragma member_alignment save
#pragma nomember_alignment longword
struct item_list_3 {
	unsigned short len;
	unsigned short code;
	void * bufadr;
	unsigned short * retlen;
};
#pragma member_alignment restore

#pragma message save
#pragma message disable noparmlist
int SYS$GETJPIW
       (unsigned long efn,
	pid_t * pid,
	const struct dsc$descriptor_s * prcnam,
	const struct item_list_3 * itmlst,
	void * iosb,
	void (* astadr)(__unknown_params),
	void * astprm,
	void * nullarg);
#pragma message restore

int MAIL$USER_BEGIN(unsigned long * context,
			const struct item_list_3 * in_item_list,
			const struct item_list_3 * out_item_list);

int MAIL$USER_GET_INFO(unsigned long * context,
			const struct item_list_3 * in_item_list,
			const struct item_list_3 * out_item_list);

int MAIL$USER_END(unsigned long * context,
			const struct item_list_3 * in_item_list,
			const struct item_list_3 * out_item_list);


int MAIL$MAILFILE_BEGIN(unsigned long * context,
			const struct item_list_3 * in_item_list,
			const struct item_list_3 * out_item_list);

int MAIL$MAILFILE_INFO_FILE(unsigned long * context,
			const struct item_list_3 * in_item_list,
			const struct item_list_3 * out_item_list);

int MAIL$MAILFILE_END(unsigned long * context,
			const struct item_list_3 * in_item_list,
			const struct item_list_3 * out_item_list);


int
vms_mail_info(struct stat *st) {
int status;
int end_status;
char full_directory[VMS_MAXRSS + 1];
char *full_directory_u;
unsigned short full_directory_len;
struct item_list_3 out_itemlist[2];
const struct item_list_3 null_list[] = {{0,0,0,0}};
unsigned long user_context = 0;
unsigned short new_messages;
unsigned short new_messages_len;

    out_itemlist[0].code = MAIL$_USER_FULL_DIRECTORY;
    out_itemlist[0].len = VMS_MAXRSS;
    out_itemlist[0].bufadr = full_directory;
    out_itemlist[0].retlen = &full_directory_len;
    out_itemlist[1].code = MAIL$_USER_NEW_MESSAGES;
    out_itemlist[1].len = sizeof new_messages;
    out_itemlist[1].bufadr = &new_messages;
    out_itemlist[1].retlen = &new_messages_len;
    out_itemlist[2].code = 0;
    out_itemlist[2].len = 0;

    /* Start the user context */
    status = MAIL$USER_BEGIN(&user_context, null_list, null_list);
    if (!$VMS_STATUS_SUCCESS(status)) {
	errno = ENOENT;
	return -1;
    }

    /* Get the mailbox file and new message count */
    status = MAIL$USER_GET_INFO(&user_context, null_list, out_itemlist);

    /* End the user context and check for errors */
    end_status = MAIL$USER_END(&user_context, null_list, null_list);
    if (!$VMS_STATUS_SUCCESS(end_status) || !$VMS_STATUS_SUCCESS(status)) {
	errno = ENOENT;
	return -1;
    }

    /* We need to do stat of the mail file to get the size and time */
    /* Which is really all Bash is looking at */
    full_directory[full_directory_len] = 0;
    strcat(full_directory, "mail.mai");
    if (full_directory_len > 0) {
	/* Bash had filename_unix_only active, so need to convert */
	full_directory_u = decc$translate_vms(full_directory);
	status = stat(full_directory_u, st);
	if (status < 0) {
	    return status;
	}
    }


    /* To do this properly, we have to look up all the messages
     * and total the results.
     * Bash just wants to know if there are new messages,
     * so just get the new message count.
     */

    /* First do stat of mail directory, then replace the following */
    st->st_nlink = 1;		/* Always 1 */
    /* st->st_size = 0; */	/* Total number of bytes in all files */
#ifndef __VAX
    st->st_blocks = new_messages; /* total number of messages */
#endif
    /* st->st_atime = 0; */	/* Access time of newest file in maildir */
    /* st->st_mtime = 0; */	/* Modify time of newest file in maildir */
    st->st_mode &= ~S_IFDIR;	/* Remove directory setting. */
    st->st_mode |= S_IFREG;	/* Add regular file setting. */
    return 0;
}

/* Really currently 12 and probably not changing */
#define VMS_USERNAME_LEN 255

static char vms_username[VMS_USERNAME_LEN+1] = {0};


int
is_vms_mail_file(const char * path) {
uid_t uid;
struct passwd * pwentry;
const char *lastslash;
int result;
int noslash;
int pw_dir_len;
int pw_name_len;

    /* Cache the username for future checks */
    if (vms_username[0] == 0) {
	struct item_list_3 itemlist[2];
	int status;
	unsigned short length;
	unsigned short jpi_iosb[4];

	itemlist[0].len = VMS_USERNAME_LEN;
	itemlist[0].code = JPI$_USERNAME;
	itemlist[0].bufadr = vms_username;
	itemlist[0].retlen = &length;
	itemlist[1].len = 0;
	itemlist[1].code = 0;

	status = SYS$GETJPIW(
		EFN$C_ENF,
		0,
		NULL,
		itemlist,
		jpi_iosb,
		NULL,
		0, 0);
	if ($VMS_STATUS_SUCCESS(status) && $VMS_STATUS_SUCCESS(jpi_iosb[0])) {
	    int name_len = length;
	    vms_username[name_len] = 0;
	    do {
		name_len--;
		if (vms_username[name_len] == ' ') {
		    vms_username[name_len] = 0;
		} else {
		    break;
		}
	    } while (name_len > 0); /* A returned name has at least 1 character */
	} else {
	    vms_username[0] = 0;
	}
	/* should not need to change case of usename */
    }

    pwentry = getpwnam(vms_username);

    pw_dir_len = strlen(pwentry->pw_dir);
    pw_name_len = strlen(pwentry->pw_name);


    /* First check, the path must end with mail.mai */
    /* and Bash tacks the username on the end */
    noslash = 0;
    lastslash = strrchr(path, '/');
    if (lastslash == NULL) {
	noslash = 1;
	lastslash = path;
    } else {
	lastslash++;
    }

    /* Check for username after a mail.mai file. */
    result = strncasecmp(lastslash, pwentry->pw_name, pw_name_len);
    if (result == 0) {
	/* Back up for a mail.mai name possible */
	if ((lastslash - path) > 9) {
	    lastslash -= 9;

	    /* Back up for an optional slash, if room */
	    if ((lastslash - path) > 1) {
		lastslash--;
	    }

	    /* Now we need to be at a slash or the start of the path */
	    if ((*lastslash == '/') || (lastslash == path)) {
		if (lastslash == path) {
		    noslash = 1;
		} else {
		    lastslash++;
		}
	    } else {
		/* Not a VMS mail file specification */
		return 0;
	    }
	} else {
	    /* Not a VMS mail file specification */
	    return 0;
	}
    }

    /* Look for a trailing null byte or slash after where mail.mai */
    /* may be expected */
    if ((lastslash[8] != 0) && (lastslash[8] != '/')) {
	return 0;
    }

    /* Check for the file */
    result = strncasecmp(lastslash, "mail.mai", 8);

    /* if the file is not mail.mai, then return not VMS special. */
    if (result != 0) {
	return 0;
    }

    /* If no slash is present, then VMS special */
    if (noslash == 1) {
	return 1;
    }

    /* There must be at least one slash to get this far */

    /* Check for simple case of ~/ */
    if ((path[0] == '~') && (path[1] == '/')) {
	return 1;
    }

    result = strncasecmp(path, "/sys$login/", 11);
    if (result == 0) {
	return 1;
    }

    /* Now finally check to see if the mail.mai is somewhere in the
     * default login directory
     */
    if (path[pw_dir_len] == '/') {
        result = strncasecmp(path, pwentry->pw_dir, pw_dir_len);
        if (result == 0) {
	    return 1;
	}
    }
    return 0;
}

int
vms_mailstat(const char * path, struct stat *st) {

int vms_mail_file;
int status;

    /* if path is mail.mai, /sys$login/mail.mai, ~/mail.mai
     * then do a special VMS mail lookup.
     * otherwise call the defautl Unix mailcheck
     */

    vms_mail_file = is_vms_mail_file(path);
    if (!vms_mail_file) {
	return mailstat(path, st);
    }

    status = vms_mail_info(st);
    return status;
}

/* Diagnostic routine to aid debugging */
int
vms_print_mailstat(const struct stat *st) {

#ifndef __VAX
    printf("Mail status: %lld messages with %lld bytes, nlink: %d.\n",
	st->st_blocks,
	st->st_size,
	st->st_nlink);
#else
    printf("Mail status: unknown messages with %lld bytes, nlink: %d.\n",
	st->st_size,
	st->st_nlink);
#endif

    printf("   access time: %d, mod time: %d mode: %o\n",
	  st->st_atime,
	  st->st_mtime,
	  st->st_mode);
    return 0;
}


#ifdef TEST_VMS_MAILSTAT
int main(int argc, char ** argv) {

int status;
struct stat mailst;
char * home;
char * user;
char home_mail[4096];

    status = vms_mailstat("~/mail.mai", &mailst);
    if (status == 0) {
	vms_print_mailstat(&mailst);
    } else {
	perror("mailstat of ~/mail.mai");
    }

    status = vms_mailstat("mail.mai", &mailst);
    if (status == 0) {
	vms_print_mailstat(&mailst);
    } else {
	perror("mailstat of mail.mai");
    }

    status = vms_mailstat("/sys$login/mail.mai", &mailst);
    if (status == 0) {
	vms_print_mailstat(&mailst);
    } else {
	perror("mailstat of /sys$login/mail.mai");
    }

    home = getenv("HOME");
    if (home != NULL) {
	strcpy(home_mail, home);
	strcat(home_mail, "/mail.mai");
	status = vms_mailstat(home_mail, &mailst);
	if (status == 0) {
	    vms_print_mailstat(&mailst);
	} else {
	    perror("mailstat of $HOME/mail.mai");
	}
    } else {
	perror("$HOME is NULL?");
    }

    /* Bash appends $USER to the mail path */
    user = getenv("USER");
    if (user != NULL) {
	strcpy(home_mail, "/sys$login/mail.mai/");
	strcat(home_mail, user);
	status = vms_mailstat(home_mail, &mailst);
	if (status == 0) {
	    vms_print_mailstat(&mailst);
	} else {
	    perror("mailstat of /sys$login/mail.mai/$USER");
	}
    } else {
	perror("$USER is NULL?");
    }

}
#endif
