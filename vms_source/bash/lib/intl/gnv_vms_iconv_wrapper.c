/* File: gnv_vms_iconv_wrapper.c
 *
 * $Id: gnv_vms_iconv_wrapper.c,v 1.1.1.1 2012/12/02 19:25:23 wb8tyw Exp $
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

#include <iconv.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

/* Maximum fake CD values for special handling */
#define GNV_ICONV_MAX_SELF_CD 8192


static char * vms_iconv_charset_sub(const char * charset, int *malloc_flag)
{
/* ASCII => ISO8859-1 */
/* GB2312 => DECHANZI */
/* EUC-KR => "DECKOREAN" */
/* EUC- => EUC */
/* ISO- => ISO */
/* DEC- => DEC */
/* UCS-2LE => UCS-2 */
/* UTF-16 => UCS-2 */
/* CP1255 => ISO8859-8 */

const unsigned long * lcharptr;
const unsigned short * scharptr;
const char * charptr;
char * result;
int result_len;

    lcharptr = (const unsigned long *) charset;
    *malloc_flag = 0;
    switch(*lcharptr) {
    case 'ASCI':
	scharptr = (const unsigned short *) &lcharptr[1];
	if (*scharptr == 'I\0') {
	    return "ISO8859-1";
	}
	return (char *) charset;
	break;
    case 'CP12':
	scharptr = (const unsigned short *) &lcharptr[1];
	charptr = (const char *) &scharptr[1];
	if ((*scharptr == '55') && (*charptr == '\0')) {
	    return "ISO8859-8";
	}
	return (char *) charset;
	break;
    case 'UTF-':
	scharptr = (const unsigned short *) &lcharptr[1];
	charptr = (const char *) &scharptr[1];
	if ((*scharptr == '16') && (*charptr == '\0')) {
	    return "UCS-2";
	}
	return (char *) charset;
	break;
    case 'GB23':
	scharptr = (const unsigned short *) &lcharptr[1];
	charptr = (const char *) &scharptr[1];
	if ((*scharptr == '12') && (*charptr == '\0')) {
	    return "DECHANZI";
	}
	return (char *) charset;
	break;
    case 'EUC-':
	scharptr = (const unsigned short *) &lcharptr[1];
	charptr = (const char *) &scharptr[1];
	if ((*scharptr == 'KR') && (*charptr == '\0')) {
	    return "DECKOREAN";
	}
    case 'ISO-':
    case 'DEC-':
	result_len = strlen(charset) - 1;
	result = (char *)malloc(result_len);
	if (result == NULL) {
	    return (char *)charset;
	}
	*malloc_flag = 1;
	strncpy(result, charset, 3);
	strncpy(&result[3], &charset[4], result_len - 3);
	result[result_len] = 0;
	return result;
	break;
    case 'UCS-':
	if (lcharptr[1] == '2LE\0')
	    return "UCS-2";
    default:
	return (char *)charset;
    }
}


static int vms_iconv_charset_encode(const char * charset)
{

/* The wrapper needs to know how many bytes a character can be. */
/* ASCII = 1 */
/* ISO2022JP = ISO = 1 */
/* ISO8859-1-EURO = ISO-8859-1-EURO = 1 */
/* ISO8859-1,-2,-5,-9,-15 = ISO-*-* = 1 */
/* UTF-8 = varying 1 to 4 bytes */
/* UTF-16 = varying 2 or more bytes */
/* UCS-2 = 2 bytes */
/* UCS-4 = 4 bytes */


const unsigned long * lcharptr;
const unsigned short * scharptr;
const char * charptr;
char * result;
int result_len;

    lcharptr = (const unsigned long *) charset;
    scharptr = (const unsigned short *) &lcharptr[1];
    charptr = (const char *) &scharptr[1];
    switch(*lcharptr) {
    case 'ASCI':
	return 1;
    case 'ISO-':
	return 1;
    case 'ISO8':
	return 1;
    case 'UTF-':
#if 0
	if ((*scharptr == '16') && (*charptr == '\0')) {
	    return 16;
	}
#endif
	if (*scharptr == '8\0') {
	    return 8;
	}
	return -1;
	break;
    case 'UCS-':
	if (lcharptr[1] == '2LE\0')
	    return 2;
	if (*scharptr == '2\0')
	    return 2;
	if (*scharptr == '4\0')
	    return 4;
    default:
	return -1;
    }
}

iconv_t libintl_vms_iconv_open
	(const char *tocode, const char * fromcode)
{
iconv_t result;
char * new_tocode;
char * new_fromcode;
int to_malloc;
int from_malloc;
int save_errno;

    save_errno = errno;

    /* Try it once to see if things work. */
    result = iconv_open(tocode, fromcode);

    if (result != (iconv_t) -1) {
	return result;
    }

    /* Any other error than EINVAL means do not bother to retry */
    if (errno != EINVAL)
	return result;

    /* restore errno like no error actually happened */
    errno = save_errno;

    /* Now attempt to replace the character set names. */
    new_tocode = vms_iconv_charset_sub(tocode, &to_malloc);
    new_fromcode = vms_iconv_charset_sub(fromcode, &from_malloc);

    result = iconv_open(new_tocode, new_fromcode);
    if (errno != EINVAL) {
	if (to_malloc) free(new_tocode);
	if (from_malloc) free(new_fromcode);
	return result;
    }

    /* restore errno like no error actually happened */
    errno = save_errno;

    /* Still no match? look for a self converter */
    if ((new_tocode != tocode) || (new_fromcode != fromcode)) {
	if (strcmp(new_tocode, new_fromcode) == 0) {
	    result = (iconv_t)vms_iconv_charset_encode(new_tocode);
	    if (result == (iconv_t)-1) {
		/* We can not do that conversion */
		/* And it is not likely we will be asked to */
		errno = EINVAL;
	    }
	}
    }

    if (to_malloc) free(new_tocode);
    if (from_malloc) free(new_fromcode);
    return result;
}
#define iconv_open(__t, __f) libintl_vms_iconv_open(__t, __f)

int libintl_vms_iconv_close(iconv_t cd)
{
    if (cd <= (iconv_t)GNV_ICONV_MAX_SELF_CD)
	return 0;
    return iconv_close(cd);
}
#define iconv_close(__cd) libintl_vms_iconv_close(__cd)


static int vms_utf8_move(char *outbuf, char *inbuf, int *move_size)
{
int ret_stat;
int i;
int rq_move;

    i = 0;
    rq_move = *move_size;
    while (i < rq_move) {
	if ((inbuf[i] & 0x7F) != 0) {
	   outbuf[i] = inbuf[i];
	}
	else if ((inbuf[i] & 0xE0) == 0xC0) {
	   /* 2 byte copy attempt */
	   if ((i + 1) > rq_move) {
		errno = EINVAL;
		*move_size = i;
		return -1;
	   }
	   outbuf[i] = inbuf[i];
	   i++;
	   outbuf[i] = inbuf[i];
	}
	else if ((inbuf[i] & 0xF0) == 0xE0) {
	   /* 3 byte copy attempt */
	   if ((i + 2) > rq_move) {
		errno = EINVAL;
		*move_size = i;
		return -1;
	   }
	   outbuf[i] = inbuf[i];
	   i++;
	   outbuf[i] = inbuf[i];
	   i++;
	   outbuf[i] = inbuf[i];
	}
	else if ((inbuf[i] & 0xF8) == 0xF0) {
	   /* 4 byte copy attempt */
	   if ((i + 3) > rq_move) {
		errno = EINVAL;
		*move_size = i;
		return -1;
	   }
	   outbuf[i] = inbuf[i];
	   i++;
	   outbuf[i] = inbuf[i];
	   i++;
	   outbuf[i] = inbuf[i];
	   i++;
	   outbuf[i] = inbuf[i];
	}
	else if ((inbuf[i] & 0xFC) == 0xF8) {
	   /* 5 byte copy attempt */
	   if ((i + 4) > rq_move) {
		errno = EINVAL;
		*move_size = i;
		return -1;
	   }
	   outbuf[i] = inbuf[i];
	   i++;
	   outbuf[i] = inbuf[i];
	   i++;
	   outbuf[i] = inbuf[i];
	   i++;
	   outbuf[i] = inbuf[i];
	   i++;
	   outbuf[i] = inbuf[i];
	}
	else if ((inbuf[i] & 0xFE) == 0xFC) {
	   /* 6 byte copy attempt */
	   if ((i + 5) > rq_move) {
		errno = EINVAL;
		*move_size = i;
		return -1;
	   }
	   outbuf[i] = inbuf[i];
	   i++;
	   outbuf[i] = inbuf[i];
	   i++;
	   outbuf[i] = inbuf[i];
	   i++;
	   outbuf[i] = inbuf[i];
	   i++;
	   outbuf[i] = inbuf[i];
	   i++;
	   outbuf[i] = inbuf[i];
	}
	i++;
    }
    return 0;
}



/* There is an issue with the BIG5 character on OpenVMS set on some versions,
   when given a NULL input it access violates.  The code that trips it should
   not be compiled in, as it is a work around for a different bug that does
   not occur on OpenVMS.

   This hack makes the original source code work until the gettext source is
   fixed or the OpenVMS BIG5 character set is fixed.

   In addition, if the two character sets are the same, this code handles
   the moving of the data instead of actually attempting to process the
   character sets.
 */


size_t libintl_vms_iconv
   (iconv_t cd,
#if _XOPEN_SOURCE == 500
    const
#endif
    char ** inbuf,
    size_t * inbytesleft,
    char ** outbuf,
    size_t * outbytesleft)
{
   if (inbuf != NULL) {
      if (cd < (iconv_t)GNV_ICONV_MAX_SELF_CD) {
	int move_size;
	int in_left;
	int out_left;
	out_left = *outbytesleft;
	in_left = *inbytesleft;
	int ret_stat;
	int rem;

	/* Character sets are the same, so this is just a copy operation */

	move_size = out_left;
	if (in_left < move_size)
	    move_size = in_left;
	if (move_size > 0) {
	    switch((int)cd) {
	    case 2: /* Two bytes */
		if ((move_size % 2) != 0)
		   move_size--;
		if (move_size > 0) {
		    memmove(*outbuf, *inbuf, move_size);
		    ret_stat = 0;
		}
		if (move_size < in_left) {
		    errno = EINVAL;
		    ret_stat = -1;
		}
		break;
	    case 4: /* Four bytes */
		rem = move_size % 4;
		if (rem != 0)
		   move_size -= rem;
		if (move_size > 0) {
		    memmove(*outbuf, *inbuf, move_size);
		    ret_stat = 0;
		}
		if (move_size < in_left) {
		    errno = EINVAL;
		    ret_stat = -1;
		}
		break;
	    case 8:  /* Varying */
		ret_stat = vms_utf8_move(*outbuf, *inbuf, &move_size);
		break;
#if 0
	    case 16:  /* Varying */
		ret_stat = vms_utf16_move(*outbuf, *inbuf, &move_size);
		break;
#endif
	    default:
		/* Unknown - punt */
		memmove(*outbuf, *inbuf, move_size);
		ret_stat = 0;
	    }
	}

	/* Adjust the amount transfered */
	*inbytesleft = in_left - move_size;
	*outbytesleft = out_left - move_size;
	*inbuf = *inbuf + move_size;
	*outbuf = *outbuf + move_size;

	/* Did all the input get transfered? */
	if (in_left > out_left) {
	    errno = E2BIG;
	    return (size_t)-1;
	}
	return ret_stat;
      }
      else
         return iconv(cd, inbuf, inbytesleft, outbuf, outbytesleft);
   }
   else
      return 0;
}


#define iconv(__cd, __ib, __ibl, __ob, __obl) \
	libintl_vms_iconv(__cd, __ib, __ibl, __ob, __obl)

