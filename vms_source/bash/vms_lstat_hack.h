/* File: vms_lstat_hack.h
 *
 * This module puts a wrapper around the lstat() to detect and attempt
 * to work around an issue where lstat() on VMS 8.4 is not handling
 * dangling symbolic links.
 *
 * Copyright 2013, John Malmberg
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

#define chdir hide_chdir
#define delete hide_delete  /* delete() not used but conflicts with tr.c */

#include <stat.h>
#include <stdlib.h>
#include <unixlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <unixio.h>
#undef lstat
#undef unlink
#undef readlink
#undef chdir
#undef delete
#include <stdio.h>
#include <descrip.h>
#include <libfildef.h>
#include <dscdef.h>
#include <stsdef.h>
#include <errno.h>

int decc$chdir(const char *__dir_spec, ...);

#define VMS_LSH_FNAME_LEN 4096

static int vms_lstat(const char * name, struct stat * st) {
    int result;
    int presult;
    int pcp_mode;
    int pathlen;
    char * slash;
    int cmp;

    /* check for bare filename bug */
    slash = strchr(name, '/');
    pathlen = strlen(name);

    cmp = 1;
    if (slash == NULL) {
        cmp = 0;
    } else if (pathlen > 4) {
        /* also check for ".dir" bug */
        int i;
        i = pathlen - 4;
        cmp = strcasecmp(".dir", &name[i]);
    }

    if (cmp == 0) {
        char * newpath;
        newpath = malloc(pathlen + 3);
        if (newpath == NULL) {
            return -1;
        }
        strcpy(newpath, name);
        strcat(newpath, "/.");
        result = lstat(newpath, st);
        free(newpath);
        if (result == 0) {
            return result;
        }
    }

    result = lstat(name, st);

    /* lstat not handling relative link to self or some directories */
    if ((result >= 0) && (!S_ISDIR(st->st_mode))) {
        return result;
    }
    /* Error?  May be a bug so try again */

    /* Save the PCP mode */
#if (__CRTL_VER >= 80300000)
    pcp_mode = decc$feature_get("DECC$POSIX_COMPLIANT_PATHNAMES",
                                __FEATURE_MODE_CURVAL);

    /* The lstat("/",x) fails in PCP mode if the POSIX root is not set up. */
    if (pcp_mode == 0) {
        int result1;
        struct stat psx_st;

        /* Work around for bug in VMS 8.3/8.4
         * lstat not handling relative link to self or some directories
         * when not in PCP mode. */
        decc$feature_set("DECC$POSIX_COMPLIANT_PATHNAMES",
                         __FEATURE_MODE_CURVAL, 2);

        result1 = lstat(name, &psx_st);

        decc$feature_set("DECC$POSIX_COMPLIANT_PATHNAMES",
                         __FEATURE_MODE_CURVAL,
                         pcp_mode);
        if (result1 == 0) {
            memcpy(st, &psx_st, sizeof(struct stat));
            result = result1;
        }
    }
#endif

    return result;
}

static int do_exe_stat(const char *path, struct stat *buf) {
   char *exepath = (char *)malloc(strlen(path) + 5);
   int retvalue;

   if (exepath != NULL) {
      strcpy(exepath, path);
      strcat(exepath, ".exe");
      retvalue = stat(exepath, buf);
      free(exepath);
   } else {
      retvalue = -1;
   }
   return retvalue;
}

static int vms_stat(const char * name, struct stat * st) {
    int result;
    int pathlen;
    char * slash;
    int cmp;

    /* check for bare filename bug */
    slash = strchr(name, '/');
    pathlen = strlen(name);

    cmp = 1;
    if (slash == NULL) {
        cmp = 0;
    } else if (pathlen > 4) {
        /* also check for ".dir" bug */
        int i;
        i = pathlen - 4;
        cmp = strcasecmp(".dir", &name[i]);
    }

    /* Either special case */
    if (cmp == 0) {
        char * newpath;
        newpath = malloc(pathlen + 3);
        if (newpath == NULL) {
            return -1;
        }
        strcpy(newpath, name);
        strcat(newpath, "/.");
        result = stat(newpath, st);
        free(newpath);
        if (result == 0) {
            return result;
        }
    }

    result = stat(name, st);

    /* Retry to see if an executable is present */
    if (result == -1) {
        char *slashptr = NULL;
        char *dotptr = NULL;

        if (errno == ENOENT) {
            if ((dotptr = strrchr(name, '.')) == NULL) {
                result = do_exe_stat(name, st);
            } else {
                if (((slashptr = strrchr(name, '/')) != NULL) &&
                    (dotptr < slashptr)) {

                    result = do_exe_stat(name, st);
                }
            }
        }
    }
    return result;
}

static int vms_unlink(const char * name) {
    int result;
    int pcp_mode;
    int pathlen;

    /* First check for ".dir" bug */
    pathlen = strlen(name);
    if (pathlen > 4) {
        int i, cmp;
        char * newpath;

        i = pathlen - 4;
        cmp = strcasecmp(".dir", &name[i]);
        if (cmp == 0) {
            newpath = malloc(pathlen + 3);
            if (newpath == NULL) {
                return -1;
            }
            strcpy(newpath, name);
            strcat(newpath, "/.");
            result = unlink(newpath);
            free(newpath);
            if (result == 0) {
                return result;
            }
        }
    }


    /* Save the PCP mode */
#if (__CRTL_VER >= 80300000)
    pcp_mode = decc$feature_get("DECC$POSIX_COMPLIANT_PATHNAMES",
                                __FEATURE_MODE_CURVAL);

    /* Already non-zero, nothing to fix up */
    if (pcp_mode == 0) {

        /* Work around for bug in VMS 8.3/8.4 */
        decc$feature_set("DECC$POSIX_COMPLIANT_PATHNAMES",
                         __FEATURE_MODE_CURVAL, 2);
        result = unlink(name);

        decc$feature_set("DECC$POSIX_COMPLIANT_PATHNAMES",
                         __FEATURE_MODE_CURVAL,
                         pcp_mode);
        if (result >= 0) {
            return result;
        }
    }
    /* Error?  May be a bug so try again */
#endif
    result = unlink(name);

    return result;
}

static int vms_readlink(const char * path, char *buf, size_t bufsize) {
    int result;
    int pcp_mode;

    result = readlink(path, buf, bufsize);
    if (result >= 0) {
        return result;
    }
    /* Error?  May be a bug so try again */

#if (__CRTL_VER >= 80300000)
    /* Save the PCP mode */
    pcp_mode = decc$feature_get("DECC$POSIX_COMPLIANT_PATHNAMES",
                                __FEATURE_MODE_CURVAL);

    /* Already non-zero, nothing to fix up */
    if (pcp_mode != 0) {
        return result;
    }

    /* Work around for bug in VMS 8.3/8.4 */
    decc$feature_set("DECC$POSIX_COMPLIANT_PATHNAMES",
                      __FEATURE_MODE_CURVAL, 2);

    result = readlink(path, buf, bufsize);

    decc$feature_set("DECC$POSIX_COMPLIANT_PATHNAMES",
                      __FEATURE_MODE_CURVAL,
                      pcp_mode);
#endif
    return result;
}

int  decc$access (const char *__file_spec, int __mode);

static int vms_access_root (const char *file_spec, int mode) {
int result;
struct stat st;
int stat_result;
mode_t x_mask;
mode_t r_mask;
mode_t w_mask;
gid_t my_gid;
uid_t my_uid;
gid_t maxsysgroup;

    /* First try the access() routine to see if it works */

    result = decc$access(file_spec, mode);
    if (result == 0) {
        return result;
    }

    /* The access() routine fails for the root directories */
    stat_result = stat(file_spec, &st);
    if (stat_result < 0) {
        /* If stat fails, return original access status */
        return result;
    }
    if (!S_ISDIR(st.st_mode)) {
        /* If this is not a directory, return original access status */
        return result;
    }

    my_gid = getgid();
    my_uid = getuid();
    result = 0;
    x_mask = S_IXOTH;
    w_mask = S_IWOTH;
    r_mask = S_IROTH;
    if (my_gid == st.st_gid) {
        x_mask |= S_IXGRP;
        w_mask |= S_IWGRP;
        r_mask |= S_IRGRP;
    }
    if (my_uid == st.st_uid) {
        x_mask |= S_IXUSR;
        w_mask |= S_IWUSR;
        r_mask |= S_IRUSR;
    }

    /* To be complete, should check if READALL, SYSPRV, or in the */
    /* SYSTEM group.  Just doing UID/GID check here for now */

    /* Check for each request */
    if (mode & X_OK) {
        if ((x_mask & st.st_mode) == 0) {
            return -1;
        }
    }
    if (mode & W_OK) {
        if ((w_mask & st.st_mode) == 0) {
            return -1;
        }
    }
    if (mode & R_OK) {
        if ((r_mask & st.st_mode) == 0) {
            return -1;
        }
    }

return result;
}

static int vms_access(const char * file, int mode) {
    int result;
    int pathlen;

    char * slash;
    int cmp;

    /* check for bare filename bug */
    slash = strchr(file, '/');
    pathlen = strlen(file);

    cmp = 1;
    if (slash == NULL) {
        cmp = 0;
    } else if (pathlen > 4) {
        /* also check for ".dir" bug */
        int i;
        i = pathlen - 4;
        cmp = strcasecmp(".dir", &file[i]);
    }

    pathlen = strlen(file);
    if (cmp ==  0) {
        char * newpath;
        newpath = malloc(pathlen + 3);
        if (newpath == NULL) {
            return -1;
        }
        strcpy(newpath, file);
        strcat(newpath, "/.");
        result = vms_access_root(newpath, mode);
        free(newpath);
        if (result == 0) {
            return result;
        }
        if ((result == -1) && (errno != ENOENT)) {
            return result;
        }
    }

    result = vms_access_root(file, mode);
    return result;
}

#if __CRTL_VER >= 70301000
# define transpath_parm transpath
#else
static char transpath[VMS_LSH_FNAME_LEN - 1];
#endif

/* Helper callback routine for converting Unix paths to VMS */
static int
to_vms_action (char * vms_spec, int flag, char * transpath_parm)
{
  strncpy (transpath, vms_spec, VMS_LSH_FNAME_LEN -2);
  transpath[VMS_LSH_FNAME_LEN - 2] = 0;
  return 0;
}

#ifdef __DECC
# pragma message save
  /* Undocumented extra parameter use triggers a ptrmismatch warning */
# pragma message disable ptrmismatch
#endif


static int vms_chdir(const char * file) {
    int result;
    int pathlen;

    pathlen = strlen(file);
    if (pathlen > 4) {
        int i, cmp;
        char * newpath;

        i = pathlen - 4;
        cmp = strcasecmp(".dir", &file[i]);
        if (cmp == 0) {
            newpath = malloc(pathlen + 3);
            if (newpath == NULL) {
                return -1;
            }
            strcpy(newpath, file);
            strcat(newpath, "/.");
            result = decc$chdir(newpath);
            free(newpath);
            return result;
        }
    }

    result = decc$chdir(file);
    return result;
}

static int vms_mkdir(const char * file, mode_t mode) {
    int result;
    int pathlen;

    pathlen = strlen(file);
    if (pathlen > 4) {
        int i, cmp;
        char * newpath;

        i = pathlen - 4;
        cmp = strcasecmp(".dir", &file[i]);
        if (cmp == 0) {
            newpath = malloc(pathlen + 3);
            if (newpath == NULL) {
                return -1;
            }
            strcpy(newpath, file);
            strcat(newpath, "/.");
            result = mkdir(newpath, mode);
            free(newpath);
            return result;
        }
    }

    result = mkdir(file, mode);
    return result;
}

/* TODO: backporting to older VMS is mostly not compiling these hacks
   since the long directory paths and DID format paths do not exist */

static int tovms_rmdir(const char * unix_dir) {
    int result;
    int exists;
    int file_ux_only_mode;
    struct stat st;
    long did[3];
    int dir_len;
    char vms_dir[VMS_LSH_FNAME_LEN + 1];

    result = stat(".", &st);
    if (result < 0) {
        if (errno == ENOENT) {
            return 0;  /* parent does not exist? */
        } else {
            return result;  /* Something really wrong here, give up */
        }
    }

    /* Use a DID for the directory, since this is a hack to support
       unlinkat, the default device can be used.
     */
    did[0] = ((S_INO_RVN_NMX(st.st_ino) << 16) & 0xFF0000) |
        S_INO_NUM(st.st_ino);
    did[1] = S_INO_SEQ(st.st_ino);
    did[2] = S_INO_RVN_RVN(st.st_ino);
    sprintf(vms_dir, "[%d,%d,%d]", did[0], did[1], did[2]);
    dir_len = strlen(vms_dir);

#if (__CRTL_VER >= 80300000)
    int pcp_mode;
    int len;
    /* Save the PCP mode */
    pcp_mode = decc$feature_get("DECC$POSIX_COMPLIANT_PATHNAMES",
                                __FEATURE_MODE_CURVAL);
    if (pcp_mode > 0) {
        decc$feature_set("DECC$POSIX_COMPLIANT_PATHNAMES",
                         __FEATURE_MODE_CURVAL, 0);
    }
#endif
    file_ux_only_mode = decc$feature_get("DECC$FILENAME_UNIX_ONLY",
                                         __FEATURE_MODE_CURVAL);
    if (file_ux_only_mode > 0) {
        decc$feature_set("DECC$FILENAME_UNIX_ONLY",
                         __FEATURE_MODE_CURVAL, 0);
    }
#if __CRTL_VER >= 70301000
    /* Current decc$to_vms is reentrant */
    decc$to_vms (unix_dir, to_vms_action, 0, 1, &vms_dir[dir_len]);
#else
    /* Older decc$to_vms is not reentrant */
    decc$to_vms (unix_dir, to_vms_action, 0, 1);
    strncpy (&vms_dir[dir_len], transpath, VMS_LSH_FNAME_LEN - dir_len);
    vms_dir[VMS_LSH_FNAME_LEN] = 0;
#endif
    len = strlen(vms_dir);
    if (len < (VMS_LSH_FNAME_LEN - 6)) {
        strcat(vms_dir, ".dir;1");
    }

    exists = access(vms_dir, F_OK);
    if (exists != 0) {
        result = 0;
    } else {
        result = unlink(vms_dir);
    }
    if (file_ux_only_mode > 0) {
        decc$feature_set("DECC$FILENAME_UNIX_ONLY",
                         __FEATURE_MODE_CURVAL, file_ux_only_mode);
    }
#if (__CRTL_VER >= 80300000)
    if (pcp_mode > 0) {
        decc$feature_set("DECC$POSIX_COMPLIANT_PATHNAMES",
                         __FEATURE_MODE_CURVAL, pcp_mode);
    }
#endif
    return result;
}

static int vms_rmdir(const char * file) {
    int result;
    int pathlen;

    pathlen = strlen(file);
    if (pathlen > 4) {
        int i, cmp;
        char * newpath;

        i = pathlen - 4;
        cmp = strcasecmp(".dir", &file[i]);
        if (cmp == 0) {
            newpath = malloc(pathlen + 3);
            if (newpath == NULL) {
                return -1;
            }
            strcpy(newpath, file);
            strcat(newpath, "/.");
            result = rmdir(newpath);
            free(newpath);
            return result;
        }
    }

    result = rmdir(file);
    if (result != 0) {
        /* helper for unlinkat */
        char * slash;
        slash = strchr(file, '/');
        if (slash == NULL) {
            result = tovms_rmdir(file);
        }
    }
    return result;
}

#define stat(file, buf) vms_stat(file, buf)
#define lstat vms_lstat
#define unlink vms_unlink
#define remove vms_unlink
#define readlink vms_readlink
#define access vms_access
#define chdir vms_chdir
#define mkdir vms_mkdir
#define rmdir vms_rmdir
