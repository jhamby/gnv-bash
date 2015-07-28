/* File: vms_fname_to_unix.c
 *
 * This is a replacement for translate_vms that handles DID format
 * directory specifications for deep directories.
 *
 * It returns a malloc() string that the caller must free when done.
 *
 * Copyright 2015, John Malmberg
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

#include <descrip.h>
#include <stsdef.h>
#include <dscdef.h>
#include <vms_sys_library/stdlib.h>
#include <vms_sys_library/string.h>
#include <vms_sys_library/stdio.h>
#include <vms_sys_library/unixlib.h>

#define lib$fid_to_name LIB$FID_TO_NAME

int LIB$FID_TO_NAME(const struct dsc$descriptor_s * devnam,
                    const unsigned short *fid,
                    const struct dsc$descriptor_s * filespec,
                    unsigned short *length,
                    const unsigned short *did,
                    int * acp_status);

#pragma message disable questcompare
#ifndef __VAX
#define VMS_MAX_PATH 4097
#pragma extern_prefix NOCRTL (getcwd)
#else
#define VMS_MAX_PATH 255
#endif

char * vms_to_unix(const char * vms_spec) {
    char *unix_spec = NULL;
    char *unix_spec2;
#ifndef __VAX
    char *vms_spec2;
    char dir_delim;
    struct dsc$descriptor_s dev_desc;
    unsigned long did[3];
    unsigned short fid[3];
    const char *dir_strt;
    char *did_found = NULL;
    char *sdir_strt = NULL;
    const char *c_ptr;

    c_ptr = vms_spec;
    while (*c_ptr != 0) {
       c_ptr = strpbrk(c_ptr, "^[<");
       if ((c_ptr == NULL) || (*c_ptr != '^'))
           break;
    }
    if (c_ptr && *c_ptr != '^')
        dir_strt = c_ptr;
    else
        dir_strt = NULL;
    dev_desc.dsc$a_pointer = (char *)vms_spec;
    dev_desc.dsc$b_dtype = DSC$K_DTYPE_T;
    dev_desc.dsc$b_class = DSC$K_CLASS_S;
    dev_desc.dsc$w_length = 0;
    if (dir_strt != NULL) {
        int dev_len;
        dev_desc.dsc$w_length  = dir_strt - vms_spec - 1;
        if (dev_desc.dsc$w_length < 0) {
            dev_desc.dsc$w_length = 0;
        }
    }
    /* Worst case parse: [1,2,3.12345][4,5,6.12345] */
    if ((*dir_strt == '[') || (*dir_strt == '<')) {
        char *dir_strt2;
        dir_delim = *dir_strt;
        dir_strt++;
        /* Check for second DID first. */
        dir_strt2 = strstr(dir_strt,".][");
        if (dir_strt2 == NULL)
            dir_strt2 == strstr(dir_strt,".><");
        if (dir_strt2 != NULL) {
            dir_strt2 += 3;
            did_found = strpbrk(dir_strt2, "^,.]");
            if (did_found && (*did_found != ',')) {
                did_found = NULL;
            } else {
                dir_strt = dir_strt2;
            }
        }
        /* If needed, look for the first DID */
        if (did_found == NULL) {
            did_found = strpbrk(dir_strt, "^,.]");
            if (did_found && (*did_found != ',')) {
                did_found = NULL;
            }
        }

        if (did_found) {
            int status;
            int acp_status;
            unsigned short length;
            struct dsc$descriptor_s file_desc;

            sscanf(dir_strt, "%d,%d,%d", &did[0], &did[1], &did[2]);

            vms_spec2 = malloc(VMS_MAX_PATH);
            if (vms_spec2 == NULL)
                return vms_spec2;

            file_desc.dsc$a_pointer = vms_spec2;
            file_desc.dsc$w_length = VMS_MAX_PATH - 1;
            file_desc.dsc$b_dtype = DSC$K_DTYPE_T;
            file_desc.dsc$b_class = DSC$K_CLASS_S;

            if (did[0] < 65535) {
                fid[0] = did[0] & 0xFFFF;
                fid[1] = did[1] & 0xFFFF;
                fid[2] = did[2] & 0xFFFF;
            } else {
                fid[0] = did[0] & 0xFFFF;
                fid[1] = did[1] & 0xFFFF;
                fid[2] = (did[2] & 0xFF) | ((did[0] >> 8) & 0xFF00);
            }

            status = lib$fid_to_name(&dev_desc, fid, &file_desc,
                                     &length, NULL, &acp_status);
            if ($VMS_STATUS_SUCCESS(status)) {
                char *unix_spec3;
                char * vptr;
                char end_delim = 0;
                vms_spec2[length] = 0;
                /* Fix directory spec */
                vptr = strrchr(vms_spec2, ']');
                if (vptr == NULL)
                    vptr = strrchr(vms_spec2, '>');
                if (vptr != NULL) {
                    end_delim = *vptr;
                    *vptr = '.';
                }
                /* Chop off the ".DIR;1" */
                vptr = strrchr(vms_spec2, '.');
                if (vptr != NULL && end_delim != 0) {
                   *vptr = end_delim;
                   vptr[1] = 0;
                }

                /* Now we have three components to deal with */
                /* device name, directory base, and subdirs */
                unix_spec = malloc(VMS_MAX_PATH);
                unix_spec2 = decc$translate_vms(vms_spec2);
                if (unix_spec2 <= (char *) 0) {
                    free(unix_spec);
                    free(vms_spec2);
                    return NULL;
                }
                if (unix_spec2 != NULL) {
                    strcpy(unix_spec, unix_spec2);
                }
                /* Now handle the subdirectories, which start at the */
                /* first dot in a directory after did_found */
                sdir_strt = strpbrk(did_found,".[<");
                if (sdir_strt && (*sdir_strt != '.'))
                    sdir_strt = NULL;
                if (sdir_strt != NULL) {
                    vms_spec2[0] = dir_delim;
                    strcpy(&vms_spec2[1], sdir_strt);
                    unix_spec2 = decc$translate_vms(vms_spec2);
                    if (unix_spec2 > (char *) 0) {
                        if ((unix_spec2[0] == '.') && (unix_spec2[1] == '/')) {
                            unix_spec2 += 2;
                        }
                        strcat(unix_spec, "/");
                        strcat(unix_spec, unix_spec2);
                    }
                }
                free(vms_spec2);
                return unix_spec;
            }
            free(vms_spec2);
        }
    }
#endif
    /* TODO VAX ODS2/NFS/PATHWORKS code */

    /* We either could not parse it or did not find a DID */
    /* Punt it back to decc$translate_vms with a malloced buffer */
    unix_spec2 = decc$translate_vms(vms_spec);
    if (unix_spec2 > (char *)0) {
        unix_spec = strdup(unix_spec2);
    }
    return unix_spec;
}
