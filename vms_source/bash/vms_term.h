/* File: vms_term.h
 *
 * $Id: vms_term.h,v 1.1.1.1 2012/12/02 19:25:23 wb8tyw Exp $
 *
 * Terminal capabilities based on the Open Group Single Unix Specification,
 * Version 2.
 *
 * This is initially a partial implementation in order to get GNV Bash
 * to build properly on VAX.
 *
 * On Alpha, these functions appear to be provided by curses.h, but tests
 * show that they always return the results for a generic VT terminal instead
 * of the expected results.
 *
 * Copyright 2010, John Malmberg
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
 * 30-Jun-2010	J. Malmberg	Original
 *
 ***************************************************************************/

#ifndef _VMS_TERM_H
#define _VMS_TERM_H

/* Supress this header from being included */
#define _RLTCAP_H_ 1

#include <unistd.h>  /* isatty, STDIN_FILENO */

#ifndef ERR
#define ERR (-1)  /* Ncurses uses -1, VMS curses uses 0 */
#endif
#ifndef OK
#define OK (0)    /* Ncurses uses 0, VMS curses uses 1 */
#endif

/* Now put in the replacement routines */
/* Bashline wants it to return int */

#ifdef _MINIX
typedef void (*vms_tputs_callback) (int);
#else
typedef int (*vms_tputs_callback) (int);
#endif


#define VMS_NOECHO   001
#define VMS_NONL     002
#define VMS_NOCRMODE 004

struct vms_kb_st {
	unsigned char flags;
	unsigned char termchan_valid;
	unsigned short termchan;
    };

extern struct vms_kb_st vms_keyboard;

#define tputs(p1, p2, p3) vms_tputs(p1, p2, p3)
#define tgoto(p1, p2, p3) vms_tgoto(p1, p2, p3)
#define tgetnum(p1)       vms_tgetnum(p1)
#define tgetstr(p1, p2)   vms_tgetstr(p1, p2)
#define tgetent(p1, p2)   vms_tgetent(p1, p2)
#define tgetflag(p1)      vms_tgetflag(p1)

/* We can not do tcflow, the driver will handle it */
#define tcflow(p1, p2)

#define echo()    (vms_keyboard.flags &= ~VMS_NOECHO)
#define noecho()  (vms_keyboard.flags |= VMS_NOECHO)
#define nl()      (vms_keyboard.flags &= ~VMS_NONL)
#define nonl()    (vms_keyboard.flags |= VMS_NONL)
#define crmode()  ((vms_keyboard.flags &= ~VMS_NOCRMODE), nonl())
#define nocrmode()  (vms_keyboard.flags |= VMS_NOCRMODE)

/* This is now a no-op */
#define initscr()

int vms_tputs(
	const char *str,
	int affcnt ,
	vms_tputs_callback my_puts);

#define getch() (!isatty(STDIN_FILENO) ? ERR : vms_getch(&vms_keyboard))

int vms_getch(struct vms_kb_st *vms_keyboard);

char * vms_tgoto(const char *id, int col, int row);

int vms_tgetnum(char * id);

char *vms_tgetstr(char * id, char **area);

int vms_tgetent(char * bp, const char * name);

int vms_tgetflag(char * id);


#endif /* _VMS_TERM_H */
