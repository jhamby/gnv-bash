/* File: vms_terminal_io.h
 *
 * Wrappers to intercept IO to terminals to better emulate what Unix programs
 * expect.
 *
 * Copyright 2009, John Malmberg
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
 * 09-Oct-2009	J. Malmberg
 *==========================================================================
 */

#ifndef _VMS_TERMINAL_IO_H
#define _VMS_TERMINAL_IO_H

  /******************************/
 /* stdio replacement routines */
/******************************/
#ifdef VMS_NEED_FILENO
#define fileno hide_fileno
#undef fileno
#include <stdio.h>
#define fileno(stream) vms_terminal_fileno(stream)
#else
#include <stdio.h>
#endif /* VMS_NEED_FILENO */
int vms_terminal_fileno(FILE * stream);



  /********************************/
 /* termios replacement routines */
/********************************/

#define tcgetattr hide_tcgetattr
#define tcsetattr hide_tcsetattr
#define tcflush hide_tcflush

#include <termios.h>
#undef tcgetattr
#undef tcsetattr
#undef tcflush

#define tcgetattr(fd, buf) vms_terminal_tcgetattr(fd, buf)
#define tcsetattr(fd, action, buf) vms_terminal_tcsetattr(fd, action, buf)
#define tcflush(fd, queue_selector) vms_terminal_tcflush(fd, queue_selector)
int vms_terminal_tcgetattr(int fd, struct termios * buf);
int vms_terminal_tcsetattr(int fd, int action, const struct termios * buf);
int vms_terminal_tcflush(int fd, int queue_selector);

#define cfsetispeed(buf, speed) (buf->__ispeed = speed)
#define cfsetospeed(buf, speed) (buf->__ospeed = speed)
#define cfgetispeed(buf) (buf->__ispeed)
#define cfgetospeed(buf) (buf->__ospeed)

  /*******************************/
 /* stropts replacement routine */
/*******************************/
#if 0  /* No point in trying to do this */
#define ioctl hide_ioctl
#include <stropts.h>
#undef ioctl
#define ioctl(fd, r, argp) vms_terminal_ioctl(fd, r, argp)
int vms_terminal_ioctl(int fd, int r, void * argp);
#endif

  /*******************************/
 /* socket replacement routines */
/*******************************/
#ifdef VMS_NEED_SELECT
#define select hide_select
#include <socket.h>
#undef select
#define select vms_terminal_select
#else
#include <socket.h>
#endif /* VMS_NEED_SELECT */
int vms_terminal_select(int nfds, fd_set * readfds, fd_set * writefds,
                        fd_set * exceptfds, struct timeval * timeout);

  /*****************************/
 /* poll replacement routines */
/*****************************/
#ifdef VMS_NEED_POLL
#define poll hide_poll
#include <poll.h>
#undef poll
#define poll vms_terminal_poll
#else
#ifndef __VAX
#include <poll.h>
#endif
#endif
#ifndef __VAX
int poll (struct pollfd fd_array[], nfds_t nfds, int timeout);
#endif

  /*******************************/
 /* unixio replacement routines */
/*******************************/
#ifdef VMS_NEED_READ_WRITE
#define read hide_read
#define write hide_write
#include <unixio.h>
#include <unistd.h>
#undef read
#undef write
#else
#include <unixio.h>
#include <unistd.h>
#endif /* VMS_NEED_READ_WRITE */

#include <dirent.h>

int vms_open(const char *file_spec, int flags, ...);
DIR * vms_fdopendir(int fd);
int vms_close(int fd);
int vms_dirfd(DIR * dirp);
int vms_dup(int fd1);
int vms_dup2(int fd1, int fd2);
int vms_closedir(DIR * dirp);
int vms_fstat(int fd, struct stat * st_buf);

FILE * vms_popen_helper(int *pipeno, pid_t pid, const char *mode);
int vms_pclose(FILE * stream);

#define read(fd, buf, nbytes) vms_terminal_read(fd, buf, nbytes)
#define write(fd, buf, nbytes) vms_terminal_write(fd, buf, nbytes)
ssize_t vms_terminal_read(int fd, void * buf, size_t nbytes);
ssize_t vms_terminal_write(int fd, const void * buf, size_t nbytes);

#endif /* _VMS_TERMINAL_IO_H */

