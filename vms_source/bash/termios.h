/* File: termios.h
 *
 * $Id: termios.h,v 1.1.1.1 2012/12/02 19:25:22 wb8tyw Exp $
 *
 * Simulate this on VMS.
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
 * 20-Mar-2012	J. Malmberg	Original
 *
 ***************************************************************************/

#ifndef _TERMIOS_H
#define _TERMIOS_H 1

/* Supress _RLTCAP_H_ inclusion. */
#if !defined (_RLTCAP_H_)
#define _RLTCAP_H_ 1
#endif

/* Need VMS terminal support */
#include "vms_term.h"

/* termios.h section */
#include "bits_termios.h"

#include "vms_terminal_io.h"


#endif /* _TERMIOS_H */
