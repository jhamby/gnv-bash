/* File: temp_stub.c
 *
 * $Id: temp_stub.c,v 1.2 2013/05/27 01:26:12 wb8tyw Exp $
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

 */

#include <unistd.h>
#include "config.h"


/* The fpurge() routine allows flushing input buffers that have not
 * been read.
 * To do this on VMS probably requires putting a wrapper around all the
 * I/O calls for terminals and pipes so that the sys$qiow routines
 * can be used to do this.
 *
 * For now, fflush is good enough to do this.
 *
 */

/* Can not use a different name than fpurge easily as the gnu source code
 * is doing an undefine, so this will need to be updated in the unlikely
 * event that HP provides an fpurge() call in the VMS CRTL.
 */

/* TODO: Bash is placing a wrapper around this, so eventually we want
 * change all the places that the wrapper is called to directly call
 * what ever VMS code that we use to avoid unneeded procedure frame
 * save and restores
 */

int
fpurge (FILE * __file) {
    return fflush(__file);
}

/* PROCESS_SUBSTITUTION is calling mkfifo() to create a temp name for a pipe */
/* Bash 1.14.8 got away with using a real temp file. */
/* The Bash 1.14.8 code used a different tempfile name and honored the */
/* the TMPDIR logical name */
/* TODO: Investigate using the cluster interconnect services SYS$ICC* */
int
mkfifo(const char * path, mode_t mode) {
int fd;

    fd = creat(path, mode);
    if (fd < 0) {
	return -1;
    }
    close(fd);
    return 0;
}
