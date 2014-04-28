/* File: gnv_vms_iconv.h
 *
 * $Id: gnv_vms_iconv.h,v 1.1.1.1 2012/12/02 19:25:23 wb8tyw Exp $
 *
 * There is an issue where GNU named the character sets differently
 * then OpenVMS and a few others did, so we have to compensate until
 * OpenVMS gets their character sets updated.
 *
 * Also handle conversions to the same codesets with out using a
 * converter.
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

#include <types.h>
#include <iconv.h>

iconv_t libintl_vms_iconv_open
	(const char *__tocode, const char * __fromcode);

int libintl_vms_iconv_close(iconv_t cd);

size_t libintl_vms_iconv
   (iconv_t cd,
#if _XOPEN_SOURCE == 500
    const
#endif
    char ** inbuf,
    size_t * inbytesleft,
    char ** outbuf,
    size_t * outbytesleft);

#define iconv_open(__t, __f) libintl_vms_iconv_open(__t, __f)

#define iconv_close(__cd) libintl_vms_iconv_close(__cd)

#define iconv(__cd, __ib, __ibl, __ob, __obl) \
	libintl_vms_iconv(__cd, __ib, __ibl, __ob, __obl)

