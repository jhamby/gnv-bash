/* File: vms_getcwd_hack.h
 *
 * Copyright 2011, John Malmberg
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


#define getcwd decc$getcwd
#include <vms_sys_library/unistd.h>
#include <vms_sys_library/unixlib.h>
#undef getcwd

#define decc_getcwd decc$getcwd
char *decc$getcwd(char * buffer, size_t size, ...);

#include <vms_sys_library/string.h>
#include <vms_sys_library/stdlib.h>

#pragma message disable questcompare
#ifndef __VAX
#define VMS_MAX_PATH 4097
#pragma extern_prefix NOCRTL (getcwd)
#else
#define VMS_MAX_PATH 255
#endif

char * vms_to_unix(const char * vms_spec);

/* GNU open source code expects that the getcwd() with a NULL buffer and
 * and a buffer size of 0 will allocate a buffer of the size needed
 * for the result.
 *
 * VMS does not do this, so this wrapper fixes it.
 */

static char * vms_getcwd(char * buffer, unsigned int size, ...)
{
  int real_size;
  char * result;
  char * vms_result;
  char * unix_result;

    real_size = 0;
    if ((buffer == NULL) && (size == 0)) {
        real_size = VMS_MAX_PATH;
    }

    vms_result = decc_getcwd(NULL, VMS_MAX_PATH, 1);
    if (vms_result != NULL) {
        char * unix_result;

        unix_result = vms_to_unix(vms_result);
        if (unix_result > (char *)0)
            if (buffer == NULL) {
                result = unix_result;
            } else {
                strncpy(buffer, unix_result, size);
                buffer[size] = 0;
                result = buffer;
                free(unix_result);
            }
        else {
            result = NULL;
        }
        free(vms_result);
    }
    return result;
}

#define getcwd vms_getcwd
