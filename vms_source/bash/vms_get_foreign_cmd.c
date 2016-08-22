/* File: vms_get_foreign_command.c
 *
 * Returns the UNIX pathname for a VMS foreign command if that foreign
 * command exists.
 *
 * Copyright 2016, John Malmberg
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
 * 20-Aug-2016 J. Malmberg
 *==========================================================================
 */
#ifndef _POSIX_EXIT
#define _POSIX_EXIT=1
#endif

#include <vms_fake_path/ctype.h>
#include <vms_fake_path/errno.h>
#include <vms_fake_path/stdlib.h>
#include <vms_fake_path/string.h>
#include <vms_fake_path/unistd.h>
#include <vms_fake_path/unixlib.h>

#include <descrip.h>
#include <libclidef.h>
#include <ssdef.h>
#include <stsdef.h>

#pragma member_alignment save
#pragma nomember_alignment longword
struct item_list_3
{
  unsigned short len;
  unsigned short code;
  void * bufadr;
  unsigned short * retlen;
};

#pragma member_alignment

int
LIB$GET_SYMBOL (const struct dsc$descriptor_s * symbol,
                struct dsc$descriptor_s * value,
                unsigned short * value_len,
                unsigned long * table);

#define MAX_DCL_SYMBOL_LEN (255)
#if __CRTL_VER >= 70302000 && !defined(__VAX)
# define MAX_DCL_SYMBOL_VALUE (8192)
#else
# define MAX_DCL_SYMBOL_VALUE (1024)
#endif

/* Look up a name as a foreign command, sanitizing invalid characters.
 * Return NULL if no command is found.
 * Return a pointer to a string in thread specific memory if a command
 * is found.
 * A subsequent call to this routine or decc$translate_vms() will
 * over write this name.
 */
char * vms_get_foreign_cmd(const char * exec_name) {

    int status;
    unsigned long symtbl;
    int save_errno;
    char * value;
    char * exec_path;
    char * clean_name;
    int i;
    unsigned short value_len;
    struct dsc$descriptor_s name_desc;
    struct dsc$descriptor_s value_desc;

    exec_path = NULL;

    if (!isalpha(exec_name[0])) {
        errno = ENOENT;
        return NULL;
    }

    clean_name = strdup(exec_name);
    if (clean_name == NULL) {
        return NULL;
    }
    i = 0;
    while (clean_name[i] != 0) {
        if (!isalnum(clean_name[i])) {
            clean_name[i] = '_';
        }
        i++;
    }

    /* Look up the symbol */
    name_desc.dsc$a_pointer = (char *) clean_name;
    name_desc.dsc$w_length = strlen(clean_name);
    name_desc.dsc$b_dtype = DSC$K_DTYPE_T;
    name_desc.dsc$b_class = DSC$K_CLASS_S;

    value = malloc(MAX_DCL_SYMBOL_VALUE + 1);
    if (value == NULL) {
        save_errno = errno;
        free(clean_name);
        errno = save_errno;
        return exec_path;
    }
    value_desc.dsc$a_pointer = value;
    value_desc.dsc$w_length = MAX_DCL_SYMBOL_VALUE;
    value_desc.dsc$b_dtype = DSC$K_DTYPE_T;
    value_desc.dsc$b_class = DSC$K_CLASS_S;
    value_len = 0;

    status = LIB$GET_SYMBOL (&name_desc, &value_desc,
                               &value_len, &symtbl);
    if ($VMS_STATUS_SUCCESS(status) && (value_len > 0)) {
        value[value_len] = 0;

        /* Look for leading $ character */
        if (value[0] = '$') {
            int j;
            /* Look for end of word, discard parameters */
            j = 0;
            while (value[j] != 0) {
                if (isspace(value[j])) {
                    value[j] == 0;
                    break;
                }
                j++;
            }
            /* Translate path to Unix */
            exec_path = decc$translate_vms(&value[1]);

            /* Make sure that it is a viable executable */
            status = access(exec_path, X_OK);
            if (status < 0) {
                exec_path = NULL;
            }
        }
    } else {
        errno = ENOENT;
    }
    save_errno = errno;
    free(clean_name);
    free(value);
    errno = save_errno;
    return exec_path;
}

#ifdef DEBUG
#include <stdio.h>

int main(int argc, char ** argv) {

char * exe_path;

   if (argc < 2) {
       exit(EXIT_FAILURE);
   }

   exe_path = vms_get_foreign_cmd(argv[1]);
   if (exe_path == NULL) {
       perror("vms_get_foreign_cmd");
   } else {
       puts(exe_path);
   }
}
#endif
