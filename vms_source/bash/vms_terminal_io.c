/* File: vms_terminal_io.c
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

/* Define these options here as we always want to build with those
 * options on, even if the caller does not use them.
 * The code below will also use these macros to bracket the code
 * controlled by the options.
 *
 * In some cases the code will be testing and setting variables that
 * are effectively compile time constants.  This is done on purpose to
 * make the code match the documentation for the Unix paramters.
 * For example, cflag_active is always 1.
 *
 * The compiler will optimize all that extra code away.
 *
 * The tcgetattr and tcsetattr are on a best effort basis.  The tcgetattr()
 * call can not return all VMS terminal settings, and this means that the
 * tcsetattr() can not be used to restore the original settings.
 *
 * This version only does what is needed to report and set some VMS settings.
 * A version that better supports Unix attributes and behaviors will need to
 * intercept all the terminal I/O.
 *
 * These routines can be used to modify open source programs to actually
 * test for features like what the EOF character is instead of assuming
 * what that it is Control-D.
 */
#ifndef __USE_BSD
#define __USE_BSD 1
#endif
#ifndef __USE_GNU
#define __USE_GNU 1
#endif
#ifndef __USE_XOPEN
#define __USE_XOPEN 1
#endif
#ifndef __USE_MISC
#define __USE_MISC 1
#endif

#include "vms_lstat_hack.h"
#include "vms_getcwd_hack.h"
char * vms_to_unix(const char * vms_spec);

#include <errno.h>
#include <stropts.h>
#include "vms_terminal_io.h"
#include <stdlib.h>
#include <stsdef.h>
#include <descrip.h>
#include <efndef.h>
#include <string.h>
#include <iodef.h>
#include <unixlib.h>
#include <ssdef.h>
/* #include <inttypes.h> */
/* #include <libdef.h> */
#include <libclidef.h>
#define __NEW_STARLET 1
#include <ttdef.h>
/* #include <tt2def.h> */
#if (__CRTL_VER >= 080200000) && !defined (__VAX)
#include <tt3def.h>
#endif
#include <dirent.h>
#include <stat.h>
#include <stdarg.h>
#include <fcntl.h>
#include <fscndef.h>
#include <lnmdef.h>


/* Eventually this may need to be conditionally compiled for
 * versions of VMS that do not have it, and also set the size of
 * struct vms_termchar_st.
 */
#include <tt2def.h>

#ifdef __VAX
typedef union ttdef TTDEF;
typedef union tt2def TT2DEF;
#ifndef POLL_IN
#define POLL_IN 1
#endif
#ifndef POLL_OUT
#define POLL_OUT 2
#endif
#endif

#define MAX_DIR_PATH 4096
#define MAX_UNIX_DIR_PATH 8192

/* General structure of these routines is that they will check the file
 * descriptor to see if it is a tty and what its name is, and then use a
 * VMS channel for it to do the QIOW, and a structure of UNIX attributes
 * to actually do the I/O.  The first access of a tty file descriptor will
 * cause the VMS channel to be created and the structure of UNIX attributes
 * to be populated.
 */

/* The poll and select routines were copied from the ones that I wrote for
 * the port of glib.
 */

#pragma message disable pragma
#pragma message disable dollarid
#pragma message disable valuepres
#pragma message disable questcompare2

#pragma member_alignment save
#pragma nomember_alignment longword
#pragma message save
#pragma message disable misalgndmem

/* Generic VMS item list structure */
struct itmlst_3 {
  unsigned short int buflen;
  unsigned short int itmcode;
  void *bufadr;
  unsigned short int *retlen;
};

struct filescan_itmlst_2 {
    unsigned short length;
    unsigned short itmcode;
    char * component;
};

/* Packed structure for passing terminal information to other processes */
struct unix_term_log_st {
  int	version;	/* Sanity check - sizeof struct termios */
  struct termios unix_termios;
};

struct tt_framing_st {
    unsigned tt_v_framesize : 4;
    unsigned tt_v_altframe : 1;
    unsigned tt_v_altrpar : 1;
    unsigned tt_v_parity : 1;
    unsigned tt_v_odd : 1;
    unsigned tt_v_twostop : 1;
    unsigned tt_v_disparerr : 1;
    unsigned tt_v_altdispar : 1;
    unsigned tt_v_break : 1;
    unsigned tt_v_fill_43_ : 4;
};


struct term_mode_iosb_st {
    unsigned short sts;
    unsigned char txspeed;
    unsigned char rxspeed;
    unsigned char cr_fill;
    unsigned char lf_fill;
    struct tt_framing_st framing;
};

struct term_char_st {
    unsigned char class;
    unsigned char type;
    unsigned short page_width;
    TTDEF ttdef;
    TT2DEF tt2def;
#if (__CRTL_VER >= 080200000) && !defined (__VAX)
    TT3DEF tt3def;
#endif
};

struct term_speed_st {
    unsigned char txspeed;
    unsigned char rxspeed;
    unsigned short speed_fill_mbz;
};

struct term_fill_st {
    unsigned short cr_fill;
    unsigned short lf_fill;
};

struct term_read_iosb_st {
    unsigned short sts;
    unsigned short count;
    unsigned short terminator;
    unsigned short terminator_count;
};

struct term_write_iosb_st {
    unsigned short sts;
    unsigned short count;
    unsigned short unknown1;
    unsigned short unknown2;
};

#pragma message restore
#pragma member_alignment restore

int LIB$DISABLE_CTRL(const unsigned long * new_mask,
		     unsigned long * old_mask);

int LIB$ENABLE_CTRL(const unsigned long * new_mask,
		    unsigned long * old_mask);

int LIB$SIGNAL(int);

int SYS$ASSIGN(
	const struct dsc$descriptor_s * devnam,
        unsigned short * chan,
        unsigned long acmode,
        const struct dsc$descriptor_s * mbxnam,
        unsigned long flags);

int   SYS$CRELNM(
	const unsigned long * attr,
	const struct dsc$descriptor_s * table_dsc,
	const struct dsc$descriptor_s * name_dsc,
	const unsigned char * acmode,
	const struct itmlst_3 * item_list);

int SYS$DASSGN(unsigned short chan);

int SYS$FILESCAN
   (const struct dsc$descriptor_s * srcstr,
    struct filescan_itmlst_2 * valuelist,
    unsigned long * fldflags,
    struct dsc$descriptor_s *auxout,
    unsigned short * retlen);

unsigned long SYS$QIOW(
	unsigned long efn,
        unsigned short chan,
        unsigned long func,
        void * iosb,
        void (* astadr)(void *),
        ...);

int SYS$READEF(unsigned long efn, unsigned long * state);

int   SYS$TRNLNM(
	const unsigned long * attr,
	const struct dsc$descriptor_s * table_dsc,
	struct dsc$descriptor_s * name_dsc,
	const unsigned char * acmode,
	const struct itmlst_3 * item_list);

struct vms_info_st {
	unsigned short channel;		/* VMS channel assigned */
	unsigned short ref_cnt;		/* Number of references */
	char * device_name;		/* VMS device name for channel */
	unsigned long oldctrlmask;	/* Old Control Mask */
	struct termios * term_attr;	/* Unix terminal attributes */
	struct term_char_st *vms_char;  /* VMS terminal characteristics */
	struct term_mode_iosb_st * vms_iosb; /* IOSB from sense mode */
	char * vmscwd;			/* VMS format Current Working Dir */
	char * path;			/* Path dirfd simulation */
	DIR * dirptr;			/* Pointer for dirfd simulation */
	unsigned long vms_crtl;		/* VMS CRTL stuff */
					/* Add info for signals here */
	struct stat *st_buf;		/* Stat buffer for directories */
};


/* Array to hold file descriptors that we are intercepting I/O for. */
static struct vms_info_st * vms_info = NULL;
static int vms_info_size = -1;

/* Flag to indicate if we are expecting VMS behavior or UNIX behavior. */
static int vms_terminal_mode = -1;

#if 0
void dump_pointer_m(const void * ptr, const char * str, int psize) {
    long * ptr1;
    long msize;
    long flags;
    ptr1 = (long *)ptr;
    msize = ptr1[-4];
    flags = ptr1[-3];
    fprintf(stderr,
            "malloc %d, msize %d flags %xd size %d %s\n",
            ptr, msize, flags, psize, str);
}

void dump_pointer_f(const void * ptr, const char * str, int psize) {
    long * ptr1;
    long msize;
    long flags;
    ptr1 = (long *)ptr;
    msize = ptr1[-4];
    flags = ptr1[-3];
    fprintf(stderr,
            "free   %d, msize %d flags %xd size %d %s\n",
            ptr, msize, flags, psize, str);
}

#else

#define dump_pointer_m(__ptr, __str, __psize)

#define dump_pointer_f(__ptr, __str, __psize)

#endif

#if 0
struct stat lstat_probe_st;

char * lstat_probe(const char * str) {
int status;

    status = lstat(str, &lstat_probe_st);
    if (status < 0) {
        perror("lstat_probe");
    }
    return &lstat_probe_st;
}

char getpwd_probe_mem[8192];

char * getpwd_probe(const char * str) {
char * cwd;

    cwd = decc_getcwd(getpwd_probe_mem, 8192, 1);
    if (cwd == NULL) {
        perror("getpwd_probe");
    } else {
        puts(cwd);
    }
    return cwd;
}
#endif

 /* Internal routines */

/* fd lookup routine.  We need our special data with some file descriptors */

static struct vms_info_st * vms_lookup_fd(int fd) {

    if (fd < 0) {
        return NULL;
    }

    /* Make sure memory of file descriptors is set up. */
    if (vms_info == NULL) {

	/* Find out how many channels are possible */
	vms_info_size = getdtablesize();

	/* Allocate the array */
	vms_info = malloc(vms_info_size * sizeof(struct vms_info_st));
	if (vms_info == NULL) {

	    /* We are probably out of memory, so degrade gracefully */
	    vms_info_size = -1;
	} else {
/* debug */
            dump_pointer_m(vms_info, "vms_lookup_fd",
                           vms_info_size * sizeof(struct vms_info_st));
	    memset(vms_info, 0, vms_info_size * sizeof(struct vms_info_st));
	}
    }
    return &vms_info[fd];
}


int vms_open(const char *file_spec, int flags, ...) {
    mode_t mode;

    mode = 0;
    if (flags & O_CREAT) {
        va_list arg;
        va_start (arg, flags);

        mode = va_arg (arg, mode_t);

        va_end (arg);
    }
    int mask;
    mask = O_RDONLY | O_NOCTTY | O_NONBLOCK;
    if ((flags & ~mask) == 0) {
        int st_result;
        struct stat st_buf;
        st_result = vms_stat(file_spec, &st_buf);
        if ((st_result == 0) &&  S_ISDIR(st_buf.st_mode)) {
            int fd_result;
            fd_result = open("/dev/null", 0, 0);
            if (fd_result >= 0) {
                struct vms_info_st * info;
                info = vms_lookup_fd(fd_result);
                if (info == NULL) {
                    errno = EIO;
                    return -1;
                }
                /* Need to store cwd in VMS format for deep filenames */
                info->vmscwd = decc_getcwd(NULL, (MAX_DIR_PATH + 1), 1);
                if (info->vmscwd == NULL) {
                    return -1;
                }
/* debug */
                dump_pointer_m(info->vmscwd, "vms_open/decc_getcwd",
                               (MAX_DIR_PATH + 1));
                info->path = strdup(file_spec);
                if (info->path == NULL) {
                    free(info->vmscwd);
                    info->vmscwd = NULL;
                    errno = ENOMEM;
                    return -1;
                }
/* debug */
                dump_pointer_m(info->path, "vms_open/strdup",
                               strlen(file_spec)+1);

                info->st_buf = malloc(sizeof (struct stat));
                if (info->st_buf == NULL) {
                    free(info->vmscwd);
                    info->vmscwd = NULL;
                    free(info->path);
                    info->path = NULL;
                    errno = ENOMEM;
                    return -1;
                }
/* debug */
                dump_pointer_m(info->st_buf, "vms_open st_buf",
                               sizeof (struct stat));
                memcpy(info->st_buf, &st_buf, sizeof(struct stat));
            }
            return fd_result;
        }
    }
    return open(file_spec, flags, mode);
}

/* Need a wrapper for fstat() */
int vms_fstat(int fd, struct stat * st_buf) {
    struct vms_info_st * info;
    info = vms_lookup_fd(fd);
    if ((info != NULL) && (info->path != NULL)) {
        if (info->st_buf != NULL) {
            memcpy(st_buf, info->st_buf, sizeof(struct stat));
            return 0;
        }
    }
    return fstat(fd, st_buf);
}

#if 0
/* Debug free */
int decc$free(void *);

void vms_free(void * foo) {
int free_stat;

    errno = 0;
    free_stat = decc$free(foo);
    if (free_stat != 0) {
        perror("free");
    }
}
#endif

DIR * decc$opendir(const char * name);

DIR * vms_opendir(const char * name) {
    int pathlen;
    char * newpath;
    DIR * dirptr;

    /* First check for ".dir" bug */
    pathlen = strlen(name);
    if (pathlen > 4) {
        int i, cmp;
        i = pathlen - 4;
        cmp = strcasecmp(".dir", &name[i]);
        if (cmp == 0) {
            newpath = malloc(pathlen + 3);
            if (newpath == NULL) {
                return NULL;
            }
/* debug */
            dump_pointer_m(newpath, "vms_opendir ", pathlen + 3);

            strcpy(newpath, name);
            strcat(newpath, "/.");
            dirptr = decc$opendir(newpath);
/* debug */
            dump_pointer_f(newpath, "vms_opendir ", strlen(newpath)+1);

            free(newpath);
            return dirptr;
        }
   }
   dirptr = decc$opendir(name);
   return dirptr;
}


/* Fake an opendir */
DIR * vms_fdopendir(int fd) {
    struct vms_info_st * info;
    info = vms_lookup_fd(fd);
    if (info == NULL) {
        errno = EIO;
        return NULL;
    }
    /* char dir_path[MAX_UNIX_DIR_PATH + 1]; */
    char * dir_path;
    dir_path = malloc(MAX_UNIX_DIR_PATH + 1);
    if (dir_path == NULL) {
        return NULL;
    }
    dir_path[0] = 0;
    if (info->path != NULL) {
        if (info->path[0] != '/') {
            if (info->vmscwd != NULL) {
                int len;
                char * unix_path;
                unix_path = vms_to_unix(info->vmscwd);
                strcpy(dir_path, unix_path);
/* debug */
                dump_pointer_f(unix_path, "vms_fdopendir ",
                               strlen(unix_path)+1);
                free(unix_path);

                len = strlen(dir_path);
                if (dir_path[len -1] != '/')
                    strcat(dir_path, "/");
            }
        }
        if (!((info->path[0] == '.') && info->path[1] == 0)) {
            int len1,len2;
            len1 = strlen(dir_path);
            len2 = strlen(info->path);
            if (len1 + len2 > MAX_UNIX_DIR_PATH) {
                char * dir_path2;
                dir_path2 = malloc(len1 + len2 + 1);
                if (dir_path2 == NULL) {
                    free(dir_path);
                    errno = ENOMEM;
                    return NULL;
                }
                strcpy(dir_path2, dir_path);
                free(dir_path);
                dir_path = dir_path2;
            }
            strcat(dir_path, info->path);
        }
        info->dirptr = vms_opendir(dir_path);
        free(dir_path);
        return info->dirptr;
    }
    free(dir_path);
    errno = ENOTDIR;
    return NULL;
}

int vmsmode_stat(const char * name, struct stat *st) {
    int result;
    int file_ux_only_mode;
#if (__CRTL_VER >= 80300000)
    int pcp_mode;
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
    result = stat(name, st);
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

int vmsmode_chdir(const char * newdir) {
    int result;
    int file_ux_only_mode;
#if (__CRTL_VER >= 80300000)
    int pcp_mode;
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
    result = decc$chdir(newdir);
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

/* The VMS CRTL will sometimes access violate when:
   1. The current working directory is a search list.
   2. A subdirectory at the start of the search list is not in all
      directories of the search list.
   Since fchdir() caches the default directory, if the directory only exists
   at the start of the search list, cache the start of the search list
   instead of the search list.
 */
char * vms_crtl_searchlist_getcwd_hack(void) {

    const $DESCRIPTOR(table_desc, "LNM$FILE_DEV");
    const unsigned long attr = LNM$M_CASE_BLIND;
    struct dsc$descriptor_s path_desc;
    int status;
    unsigned long field_flags;
    struct filescan_itmlst_2 fs_list[5];
    char * volume;
    char * name;
    int name_len;
    char * ext;
    char * original_cwd;
    /* const int show up in debugger, macros do not */
    const int FS_FULL = 0;
    const int FS_DEV = 1;
    const int FS_ROOT = 2;
    const int FS_DIR = 3;
    const int FS_END = 4;

    /* Get the current working directory in VMS format */
    original_cwd = decc_getcwd(NULL, MAX_DIR_PATH + 1, 1);

/* debug */
    dump_pointer_m(original_cwd, "v_c_searchlist ", (MAX_DIR_PATH + 1));

    path_desc.dsc$a_pointer = original_cwd;
    path_desc.dsc$w_length = strlen(original_cwd);
    path_desc.dsc$b_dtype = DSC$K_DTYPE_T;
    path_desc.dsc$b_class = DSC$K_CLASS_S;

    /* Don't actually need to initialize anything buf itmcode */
    /* I just do not like uninitialized input values */

    /* Sanity check, this must be the same length as input */
    fs_list[FS_FULL].itmcode = FSCN$_FILESPEC;
    fs_list[FS_FULL].length = 0;
    fs_list[FS_FULL].component = NULL;

    /* Only device and dir should be present */
    fs_list[FS_DEV].itmcode = FSCN$_DEVICE;
    fs_list[FS_DEV].length = 0;
    fs_list[FS_DEV].component = NULL;

    /* we need any root and directory */
    fs_list[FS_ROOT].itmcode = FSCN$_ROOT;
    fs_list[FS_ROOT].length = 0;
    fs_list[FS_ROOT].component = NULL;

    fs_list[FS_DIR].itmcode = FSCN$_DIRECTORY;
    fs_list[FS_DIR].length = 0;
    fs_list[FS_DIR].component = NULL;

    /* End the list */
    fs_list[FS_END].itmcode = 0;
    fs_list[FS_END].length = 0;
    fs_list[FS_END].component = NULL;

    status = SYS$FILESCAN(
        (const struct dsc$descriptor_s *)&path_desc,
        fs_list, &field_flags, NULL, NULL);

    if ($VMS_STATUS_SUCCESS(status) &&
        (fs_list[FS_FULL].length == path_desc.dsc$w_length) &&
        (fs_list[FS_DEV].length != 0)) {

        /* Check if device is a search list */
        {
            struct dsc$descriptor_s dev_desc;
            struct itmlst_3 item_list[3];
            int max_index;
            unsigned short max_index_len;
            int status;
            int dir_len;

            item_list[0].buflen = 4;
            item_list[0].itmcode = LNM$_MAX_INDEX;
            item_list[0].bufadr = &max_index;
            item_list[0].retlen = &max_index_len;

            item_list[1].buflen = 0;
            item_list[1].itmcode = 0;

            dev_desc.dsc$w_length = fs_list[FS_DEV].length - 1;
            dev_desc.dsc$a_pointer = fs_list[FS_DEV].component;
            dev_desc.dsc$b_dtype = DSC$K_DTYPE_T;
            dev_desc.dsc$b_class = DSC$K_CLASS_S;

            status = SYS$TRNLNM(&attr, &table_desc, &dev_desc, 0, item_list);
            if ($VMS_STATUS_SUCCESS(status) && (max_index > 0)) {

                int cur_index;
                char *new_dir_path;
                new_dir_path = malloc(MAX_DIR_PATH + 1);
                if (new_dir_path == NULL) {
                    return NULL;
                }
/* debug */
                dump_pointer_m(new_dir_path, "v_c_searchlist ",
                               (MAX_DIR_PATH + 1));

                cur_index = max_index;
                /* Loop through search list to see if the dir exists
                 * We only do one level deep iteration, not multiple
                 * levels which could be possible.
                 */
                while (cur_index >= 0) {

                    /* dir exist at this device, but not earlier */
                    int new_path_len;
                    int dir_exists;
                    unsigned short new_dev_len;
                    int fs_dir_len;

                    dir_exists = 0;
                    item_list[0].buflen = 4;
                    item_list[0].itmcode = LNM$_INDEX;
                    item_list[0].bufadr = &cur_index;
                    item_list[0].retlen = 0;

                    item_list[1].buflen = MAX_DIR_PATH;
                    item_list[1].itmcode = LNM$_STRING;
                    item_list[1].bufadr = new_dir_path;
                    item_list[1].retlen = &new_dev_len;

                    item_list[2].buflen = 0;
                    item_list[2].itmcode = 0;

                    fs_dir_len = fs_list[FS_ROOT].length +
                        fs_list[FS_DIR].length;
                    status = SYS$TRNLNM(&attr, &table_desc, &dev_desc,
                                        0, item_list);
                    if ($VMS_STATUS_SUCCESS(status) && (new_dev_len > 0) &&
                        ((new_dev_len + fs_dir_len) < MAX_DIR_PATH)) {
                        struct stat st_buf;
                        int st_result;
                        int cur_len;

                        new_dir_path[new_dev_len] = 0;
                        cur_len = new_dev_len;
                        if (fs_list[FS_ROOT].length > 0) {
                            strncat(new_dir_path,
                                fs_list[FS_ROOT].component,
                                fs_list[FS_ROOT].length);
                            cur_len += fs_list[FS_ROOT].length;
                            new_dir_path[cur_len] = 0;
                        }

                        /* This should always be > 0 */
                        if (fs_list[FS_DIR].length > 0) {
                            strncat(new_dir_path,
                                fs_list[FS_DIR].component,
                                fs_list[FS_DIR].length);
                            cur_len += fs_list[FS_DIR].length;
                            new_dir_path[cur_len] = 0;
                        }

                        st_result = vmsmode_stat(new_dir_path, &st_buf);
                        if (st_result == 0) {
                            dir_exists = 1;
                        }
                    }

                    if (dir_exists && (cur_index == max_index)) {
                        /* It exists at the end of the search list
                         * assume good.
                         */
                        break;
                    }

                    /* If we get here, the directory was missing from
                     * somewhere later in the search list, so if it is here
                     * it is the one that we want to use.
                     */
                    if (dir_exists) {
                        char * old_dir;

                        /* Swap the directory pointers */
                        old_dir = original_cwd;
                        original_cwd = new_dir_path;
                        new_dir_path = old_dir;

                        /* Adjust the actual cwd */
                        vmsmode_chdir(original_cwd);
                        break;
                    }
                    cur_index--;
                } /* End whiie */

                /* Either we did not find the directory
                 * or the directory is in all search paths
                 */
/* debug */
                dump_pointer_f(new_dir_path, "v_c_searchlist ",
                               strlen(new_dir_path)+1);

                free(new_dir_path);

            } /* End of device is a logical name */
        } /* sys$filescan succeeded - should never fail for a current dir */
    }
    return original_cwd;
}


/* Need a fchdir */
int vms_fchdir(int fd) {
    struct vms_info_st * info;
    info = vms_lookup_fd(fd);
    if (info == NULL) {
        errno = EIO;
        return -1;
    }

    /* First need to go to saved wd */
    if (info->vmscwd != NULL) {
        int err;
        err = vmsmode_chdir(info->vmscwd);
    }
    /* Then to the desired directory */
    if (info->path != NULL) {
        if (!((info->path[0] == '.') && info->path[1] == 0)) {
            int err;
            err = chdir(info->path);
            if (err == 0) {
                /* Need to replace the saved path */

/* debug */
                dump_pointer_f(info->vmscwd, "vms_fchdir ",
                               strlen(info->vmscwd)+1);

                free(info->vmscwd);
                info->vmscwd = vms_crtl_searchlist_getcwd_hack();
                if (info->vmscwd == NULL) {
                    return -1;
                }
/* debug */
                dump_pointer_f(info->path, "vms_fchdir ",
                               strlen(info->path)+1);

                free(info->path);
                info->path = strdup(".");
                if (info->path == NULL) {
                    free(info->vmscwd);
                    info->vmscwd == NULL;
                    errno = ENOMEM;
                    return -1;
                }
/* debug */
                dump_pointer_m(info->path, "vms_fchdir/strdup ", 2);
            }
            return err;
        } else {
            return 0;
        }
    }
    errno = ENOTDIR;
    return -1;
}


/* Close up the file descriptor info on close */

int vms_close(int fd) {
    int result;
    struct vms_info_st * info;
    info = vms_lookup_fd(fd);
    if (info != NULL) {
        if (info->vmscwd != NULL) {
/* debug */
            dump_pointer_f(info->vmscwd, "vms_close", strlen(info->vmscwd)+1);

            free(info->vmscwd);
            info->vmscwd = NULL;
        }
        if (info->path != NULL) {
/* debug */
            dump_pointer_f(info->path, "vms_close", strlen(info->path)+1);
            free(info->path);
            info->path = NULL;
        }
        if (info->st_buf != NULL) {
/* debug */
            dump_pointer_f(info->st_buf, "vms_close", sizeof(struct stat));

            free(info->st_buf);
            info->st_buf = NULL;
        }
    }
    result = close(fd);
    if ((info != NULL) && (info->dirptr != NULL)) {
        result = closedir(info->dirptr);
        info->dirptr = NULL;
    }
    return result;
}

/* have to handle dup */
int vms_dup(int fd1) {
    int fd2;
    struct vms_info_st * info1;
    struct vms_info_st * info2;
    fd2 = dup(fd1);
    if (fd2 < 0) {
        return fd2;
    }
    info1 = vms_lookup_fd(fd1);
    info2 = vms_lookup_fd(fd2);
    if ((info1 == NULL) || (info2 == NULL)) {
        errno = EIO;
        return -1;
    }
    if (info1->vmscwd != NULL) {
        info2->vmscwd = strdup(info1->vmscwd);
        if (info2->vmscwd == NULL) {
            return -1;
        }
/* debug */
        dump_pointer_m(info2->vmscwd, "vms_dup/strdup",
                       strlen(info1->vmscwd)+1);
    }
    if (info1->path != NULL) {
        info2->path = strdup(info1->path);
        if (info2->path == NULL) {
            if (info2->vmscwd != NULL) {
                free(info2->vmscwd);
                info2->vmscwd = NULL;
            }
            errno = ENOMEM;
            return -1;
        }
/* debug */
        dump_pointer_m(info2->path, "vms_dup/strdup",
                       strlen(info1->path)+1);
    }
    if (info1->st_buf != NULL) {
        info2->st_buf = malloc(sizeof (struct stat));
        if (info2->st_buf == NULL) {
            if (info2->vmscwd != NULL) {
                free(info2->vmscwd);
                info2->vmscwd = NULL;
            }
            if (info2->path == NULL) {
                free(info2->path);
                info2->path = 0;
            }
            errno = ENOMEM;
            return -1;
        }
/* debug */
        dump_pointer_m(info2->st_buf, "vms_dup/malloc", sizeof(struct stat));

        memcpy(info2->st_buf, info1->st_buf, sizeof(struct stat));
    }
    return fd2;
}

/* and handle dup2 */
int vms_dup2(int fd1, int fd2) {
    int fd;
    struct vms_info_st * info1;
    struct vms_info_st * info2;
    info1 = vms_lookup_fd(fd1);
    info2 = vms_lookup_fd(fd2);
    if ((info1 == NULL) || (info2 == NULL)) {
        errno = EIO;
        return -1;
    }
    if (info2->path != NULL) {
        vms_close(fd2);
    }
    fd = dup2(fd1, fd2);
    if (fd2 < 0) {
        return fd2;
    }
    if (info1->vmscwd != NULL) {
        info2->vmscwd = strdup(info1->vmscwd);
        if (info2->vmscwd == NULL) {
            return -1;
        }
/* debug */
        dump_pointer_m(info2->vmscwd, "vms_dup/strdup",
                       strlen(info1->vmscwd)+1);
    }
    if (info1->path != NULL) {
        info2->path = strdup(info1->path);
        if (info2->path == NULL) {
            if (info2->vmscwd != NULL) {
                free(info2->vmscwd);
                info2->vmscwd = NULL;
            }
            errno = ENOMEM;
            return -1;
        }
/* debug */
        dump_pointer_m(info2->path, "vms_dup/strdup",
                       strlen(info1->path)+1);
    }
    if (info1->st_buf != NULL) {
        info2->st_buf = malloc(sizeof (struct stat));
        if (info2->st_buf == NULL) {
            if (info2->vmscwd != NULL) {
                free(info2->vmscwd);
                info2->vmscwd = NULL;
            }
            if (info2->path == NULL) {
                free(info2->path);
                info2->path = NULL;
            }
            errno = ENOMEM;
            return -1;
        }
/* debug */
        dump_pointer_m(info2->st_buf, "vms_dup/malloc", sizeof(struct stat));
        memcpy(info2->st_buf, info1->st_buf, sizeof(struct stat));
    }
    return fd2;
}

/* Need a dirfd */
int vms_dirfd(DIR * dirp) {
    int i;

    /* No interceptions - Trivial */
    if (vms_info == NULL) {
        errno = ENOTDIR;
        return -1;
    }

    /* Hard way, look it up */
    for (i = 0; i < vms_info_size; i++) {
        if (vms_info[i].dirptr == dirp) {
            return i;
        }
    }

    /* We may end up having to allocate a fd and return it */
    errno = ENOTDIR;
    return -1;
}

/* Cleanup a file descriptor on closedir */
int vms_closedir(DIR * dirp) {
    int fd;

    fd = vms_dirfd(dirp);
    if (fd >= 0) {
        return vms_close(fd);
    }

    /* Still not found? */
    return closedir(dirp);
}

/* When we encounter a terminal device, we need to see if it is one
 * that we already know about.
 * The first time we encounter a terminal device, we need to look up its
 * Unix terminal characteristics so we can do terminal I/O properly on it.
 * Poll/select from glib are modified to only deassign channels that they
 * assigned.  Eventually this module should become common to glib so that
 * glib can get the better terminal support
 */


static int vms_channel_lookup(int fd, unsigned short *channel)
{
int status;
char device_name[256];
char * retname;
struct dsc$descriptor_s dev_desc;
int call_stat;
unsigned short chan;
struct vms_info_st * info;

    status = -1;

    if (fd < 0) {
        errno = EIO;
        return -1;
    }

    /* Make sure memory of file descriptors is set up. */
    if (vms_info == NULL) {

	/* Find out how many channels are possible */
	vms_info_size = getdtablesize();

	/* Allocate the array */
	vms_info = malloc(vms_info_size * sizeof(struct vms_info_st));
	if (vms_info == NULL) {

	    /* We are probably out of memory, so degrade gracefully */
	    vms_info_size = -1;
	} else {
	    memset(vms_info, 0, vms_info_size * sizeof(struct vms_info_st));
	}
    }

    /* Do we know about this one? */
    info = vms_lookup_fd(fd);
    if (info == NULL) {
        errno = EIO;
        return -1;
    }
    if (info->ref_cnt > 0) {
	/* We found it */
	*channel = info->channel;

	/* Do not increment the ref count for STDIN/STDOUT/STDERR */
	if (fd > 2) {
	    info->ref_cnt++;
	}
	return 0;
    }

     /* get the name */
    /*--------------*/
    retname = getname(fd, device_name, 1);
    if (retname != NULL) {

	/* Store the name */
	info->device_name = strdup(device_name);
        if (info->device_name == NULL) {
            return -1;
        }

	 /* Assign the channel */
	/*--------------------*/
	dev_desc.dsc$a_pointer = device_name;
	dev_desc.dsc$w_length = strlen(device_name);
	dev_desc.dsc$b_dtype = DSC$K_DTYPE_T;
	dev_desc.dsc$b_class = DSC$K_CLASS_S;
	call_stat = SYS$ASSIGN(&dev_desc, &chan, 0, 0, 0);
	if ($VMS_STATUS_SUCCESS(call_stat)) {
	    *channel = chan;
	    info->channel = chan;
	    info->ref_cnt = 1;
	    status = 0;
	}

	/* Initialize the rest of the structure */
	info->term_attr = NULL;
    }

    return status;
}


/* Close the vms_channel on the last lookup */

static int vms_channel_close(int fd) {
    int status = 0;

    if (fd < 0) {
	errno = EIO;
	status = -1;
    }

    if (vms_info[fd].ref_cnt) {
	if (fd > 2) {
	    vms_info[fd].ref_cnt--;
	}
    }
    if (vms_info[fd].ref_cnt == 0) {
    int call_stat;

	/* All references done, time to clean up */

	/* Cancel any ASTs for signals */

	/* Remove the channel */
	call_stat = SYS$DASSGN(vms_info[fd].channel);
	if ($VMS_STATUS_SUCCESS(call_stat)) {
	    status = 0;
	} else {
	    errno = EIO;
	    status = -1;
	}

	/* Clean up the cached device name */
	if (vms_info[fd].device_name != NULL) {
	    free(vms_info[fd].device_name);
	    vms_info[fd].device_name = NULL;
	}

	/* Clean up the VMS terminal attributes */
	if (vms_info[fd].vms_char != NULL) {
	    free(vms_info[fd].vms_char);
	    vms_info[fd].vms_char = NULL;
	}
	if (vms_info[fd].vms_iosb != NULL) {
	    free(vms_info[fd].vms_iosb);
	    vms_info[fd].vms_iosb = NULL;
	}

	/* Clean up the Unix terminal attributes */
	if (vms_info[fd].term_attr != NULL) {
	    free(vms_info[fd].term_attr);
	    vms_info[fd].term_attr = NULL;
	}
    }
    return status;
}

#ifdef __VAX
struct fake_pollfd {
    int fd;
    short events;
    short revents;
};
typedef unsigned int fake_nfds_t;
#define pollfd fake_pollfd
#define nfds_t fake_nfds_t
#endif


struct vms_pollfd_st {
    struct pollfd *fd_desc_ptr;
    unsigned short channel;
    unsigned short pad;
    int fd;
};

static int vms_poll_terminal(const struct vms_pollfd_st *term_array, int ti)
{
int i;
int ret_stat;
int count;
int status;
#pragma member_alignment save
#pragma nomember_alignment longword

struct typeahead_st {
    unsigned short numchars;
    unsigned char firstchar;
    unsigned char reserved0;
    unsigned long reserved1;
} typeahead;
#pragma member_alignment restore
struct term_mode_iosb_st mode_iosb;

    ret_stat = 0;

    /* Loop through the terminal channels */
    for (i = 0; i < ti; i++) {
	term_array[i].fd_desc_ptr->revents = 0;

	/* assume output is always available */
	term_array[i].fd_desc_ptr->revents =
		term_array[i].fd_desc_ptr->events & POLL_OUT;

	/* Poll input status */
	if (term_array[i].fd_desc_ptr->events & POLL_IN)
	{
	    status = SYS$QIOW
			   (EFN$C_ENF,
			    term_array[i].channel,
			    IO$_SENSEMODE | IO$M_TYPEAHDCNT,
			    &mode_iosb,
			    NULL,
			    NULL,
			    &typeahead, 8, 0, 0, 0, 0);
	    if ($VMS_STATUS_SUCCESS(status) &&
		$VMS_STATUS_SUCCESS(mode_iosb.sts)) {
		if (typeahead.numchars != 0) {
		    term_array[i].fd_desc_ptr->revents =
			term_array[i].fd_desc_ptr->events & POLL_IN;
		}
	    }
	    else {
		/* Something really wrong */
		ret_stat = -1;
		errno = EIO;
		break;
	    }
	}
	/* Increment the return status */
	if (term_array[i].fd_desc_ptr->revents != 0)
	    ret_stat++;
    }

    return ret_stat;
}

static int vms_poll_pipe(const struct vms_pollfd_st * pipe_array, int pi)
{
int i;
int ret_stat;
int status;
#pragma member_alignment save
#pragma nomember_alignment longword
struct mbx_gmif_iosb_st {
    unsigned short sts;
    unsigned short num_msg;
    unsigned long num_bytes;
} mbx_iosb;
#pragma member_alignment restore

    ret_stat = 0;

    /* Loop through the pipes */
    for (i = 0; i < pi; i++) {
	pipe_array[i].fd_desc_ptr->revents = 0;

	/* Check the mailbox status */
	if (pipe_array[i].fd_desc_ptr->events & (POLL_IN | POLL_OUT)) {
	    status = SYS$QIOW
			   (EFN$C_ENF,
			    pipe_array[i].channel,
			    IO$_SENSEMODE,
			    &mbx_iosb,
			    NULL,
			    NULL,
			    0, 0, 0, 0, 0, 0);
	    if ($VMS_STATUS_SUCCESS(status) &&
		$VMS_STATUS_SUCCESS(mbx_iosb.sts)) {

		/* Got some information */

		if (mbx_iosb.num_msg != 0) {
		    /* There is data to read */
		    pipe_array[i].fd_desc_ptr->revents =
			pipe_array[i].fd_desc_ptr->events & POLL_IN;
		}
		else {
		    /* Pipe is empty, ok to write */
		    pipe_array[i].fd_desc_ptr->revents =
			pipe_array[i].fd_desc_ptr->events & POLL_OUT;
		}
	    }
	    else {
		/* Something really wrong */
		ret_stat = -1;
		errno = EIO;
		break;
	    }
	}
	/* Increment the return status */
	if (pipe_array[i].fd_desc_ptr->revents != 0)
	    ret_stat++;
    }
    return ret_stat;
}

static int vms_poll_x11_efn(const struct vms_pollfd_st * efn_array, int xi)
{
int i;
int ret_stat;
int status;
unsigned long state;

    ret_stat = 0;

    /* Loop through the event flags */
    for (i = 0; i < xi; i++) {
	efn_array[i].fd_desc_ptr->revents = 0;

	/* assume output is always available */
	efn_array[i].fd_desc_ptr->revents =
		efn_array[i].fd_desc_ptr->events & POLL_OUT;

	/* Check the mailbox status */
	if (efn_array[i].fd_desc_ptr->events & (POLL_IN | POLL_OUT)) {
	    status = SYS$READEF(efn_array[i].fd_desc_ptr->fd, &state);
	    if ($VMS_STATUS_SUCCESS(status)) {

		/* Got some information */
		if (status == SS$_WASSET) {
		    /* There is data to read */
		    efn_array[i].fd_desc_ptr->revents =
			efn_array[i].fd_desc_ptr->events & POLL_IN;
		}
	    }
	    else {
		/* Something really wrong */
		ret_stat = -1;
		errno = EIO;
		break;
	    }
	}
	/* Increment the return status */
	if (efn_array[i].fd_desc_ptr->revents != 0)
	    ret_stat++;
    }
    return ret_stat;
}


/* We need to know if we are in UNIX mode or VMS mode */
int get_vms_terminal_mode (void) {
char * shell;

    if (vms_terminal_mode != -1) {
	return vms_terminal_mode;
    }

    shell = getenv("SHELL");
    if (shell == NULL) {
	vms_terminal_mode = 1;
    }

    if (strncmp("DCL", shell, 4) == 0) {
	vms_terminal_mode = 1;
    } else {
	vms_terminal_mode = 0;
    }
    return vms_terminal_mode;
}


/* Implement guts of a unix like IOCTL for terminals */

/* We could attempt to actually do the IOCTLs for terminals but do not.
 * 1. No documentation for some of the IOCTL calls.
 * 2. "r" argument is bit encoded, hard to avoid conflicts
 * According to web search, apparently only bash is using them and when
 * bash uses them instead of termios, major functionality is lost.
 * Also the VMS prototype for ioctl() is wrong, the two arguments are
 * mandatory, and then a varying number of optional arguments based
 * on the "r" argument.
 */
int vms_term_qio_ioctl(int fd, int r, void * argp) {
    errno = ENOSYS;
    return -1;
}

/* vms_speed_to_unix
 *
 * speed - VMS Speed value
 *
 * returns Unix speed value or 0 if speed can not be calculated.
 *
 */

static int vms_speed_to_unix_speed(unsigned char speed) {
    switch(speed) {
    case TT$C_BAUD_50:		return B50;
    case TT$C_BAUD_75:		return B75;
    case TT$C_BAUD_110: 	return B110;
    case TT$C_BAUD_134: 	return B134;
    case TT$C_BAUD_150: 	return B150;
    case TT$C_BAUD_300: 	return B300;
    case TT$C_BAUD_600: 	return B600;
    case TT$C_BAUD_1200:	return B1200;
    case TT$C_BAUD_2400:	return B2400;
    case TT$C_BAUD_4800:	return B4800;
    case TT$C_BAUD_7200:	return B7200;
    case TT$C_BAUD_9600:	return B9600;
    case TT$C_BAUD_19200:	return B19200;
    case TT$C_BAUD_38400:	return B38400;
    case TT$C_BAUD_57600:	return B57600;
    case TT$C_BAUD_76800:	return B76800;
    case TT$C_BAUD_115200:	return B115200;
    }
    return 0;
}

/* unix_speed_to_vms
 *
 * speed - Unix Speed value
 *
 * returns VMS speed value or 0 if speed can not be calculated.
 *
 */

static int unix_speed_to_vms_speed(unsigned char speed) {
    switch(speed) {
    case B50:		return TT$C_BAUD_50;
    case B75:		return TT$C_BAUD_75;
    case B110: 		return TT$C_BAUD_110;
    case B134: 		return TT$C_BAUD_134;
    case B150: 		return TT$C_BAUD_150;
    case B300: 		return TT$C_BAUD_300;
    case B600: 		return TT$C_BAUD_600;
    case B1200:		return TT$C_BAUD_1200;
    case B2400:		return TT$C_BAUD_2400;
    case B4800:		return TT$C_BAUD_4800;
    case B7200:		return TT$C_BAUD_7200;
    case B9600:		return TT$C_BAUD_9600;
    case B19200:	return TT$C_BAUD_19200;
    case B38400:	return TT$C_BAUD_38400;
    case B57600:	return TT$C_BAUD_57600;
    case B76800:	return TT$C_BAUD_76800;
    case B115200:	return TT$C_BAUD_115200;
    }
    return 0;
}

/* vms_term_qio_tcgetattr
 * Does the actual work of getting those VMS terminal characteristics
 * that can be mapped to Unix terminal characteristics.
 *
 * vms_info - on input, the VMS device channel, and other information.
 *            on output, updated to reflect the current status.
 *
 * my_attr  - Structure containing the equivalent Unix characteristics.
 *
 * Returns 0 for success, -1 for an error and sets errno on failure.
 *
 */
int vms_term_qio_tcgetattr(struct vms_info_st * vms_info,
			   struct termios *my_attr) {
int status;
struct term_mode_iosb_st * mode_iosb;
struct term_char_st * termchar;
const unsigned long newmask = 0;
unsigned int opost_active = 0;
unsigned int any_fill = 0;
unsigned int cflag_active = 0;
int ret_stat = 0;

    /* Get the old control character mask */
    status = LIB$ENABLE_CTRL(&newmask, &vms_info->oldctrlmask);
    if (!$VMS_STATUS_SUCCESS(status)) {
	errno = EIO;
	return -1;
    }

    /* Set up the structures to hold the information if not already there */
    if (vms_info->vms_char == NULL) {
	vms_info->vms_char = malloc(sizeof(struct term_char_st));
	if (vms_info->vms_char == NULL) {
	    return -1;
	}
    }
    termchar = vms_info->vms_char;

    if (vms_info->vms_iosb == NULL) {
	vms_info->vms_iosb = malloc(sizeof(struct term_mode_iosb_st));
	if (vms_info->vms_iosb == NULL) {
	    return -1;
	}
    }
    mode_iosb = vms_info->vms_iosb;

    if (vms_info->term_attr == NULL) {
	vms_info->term_attr = malloc(sizeof(struct termios));
	if (vms_info->term_attr == NULL) {
	    return -1;
	}
    }

    status = SYS$QIOW
       (EFN$C_ENF,
	vms_info->channel,
	IO$_SENSEMODE,
	vms_info->vms_iosb,
	NULL,
	NULL,
	termchar, sizeof(struct term_char_st), 0, 0, 0, 0);
    if ($VMS_STATUS_SUCCESS(status) &&
	$VMS_STATUS_SUCCESS(mode_iosb->sts)) {
	    /* We have data */
	    ret_stat = 0;
    } else {
	/* Something really wrong */
	ret_stat = -1;
	errno = EIO;
	return ret_stat;
    }

    memset(my_attr, 0, sizeof(struct termios));

    /* Input Flags */
    my_attr->c_iflag |= IGNBRK; /* VMS driver handles BREAK */
/*			BKRINT     VMS driver handles BREAK */
/*			IGNPAR     VMS fatal error on bad parity */
/*                      PARMRK     VMS can not mark parity errors */
    if (mode_iosb->framing.tt_v_disparerr) {
        my_attr->c_iflag |= INPCK;  /* Parity is enabled */
    }
    if (!termchar->ttdef.tt$v_eightbit) {
	my_attr->c_iflag |= ISTRIP;  /* Strip 8th bit off characters */
    }
/*			INLCR	VMS terminates on linefeed and passes through */
/*			IGNCR   VMS can not ignore CR */
    my_attr->c_iflag |= ICRNL;   /* VMS RMS maps CR to LF on input */
    if (termchar->ttdef.tt$v_ttsync) {
	my_attr->c_iflag |= IXON;  /* XON/XOFF on output */
    }
    if (termchar->ttdef.tt$v_hostsync) {
	my_attr->c_iflag |= IXOFF; /* XON/XOFF on input */
    }
#ifdef __USE_BSD
/*			IXANY	VMS will not do this. */
    my_attr->c_iflag |= IMAXBEL;    /* VMS rings bell when queue full */
#endif
#ifdef __USE_GNU
/*			IUCLC   VMS will not translate upper to lower */
#endif


    /* Output flags */
#if defined __USE_BSD || defined __USE_XOPEN
    my_attr->c_oflag |= ONLCR;  /* VMS RMS does this automatically */
    opost_active = 1;
#endif
#ifdef __USE_BSD
/*			OXTABS	 (ALIAS for TAB3)  Expand tabs to spaces */
/*			ONOEOT		VMS does not pass through ^Z EOF */
/*					to output.			 */
/*					Ignore for now.			 */
#endif
#if defined __USE_BSD || defined __USE_XOPEN
/*			OCRNL	VMS will not map CR to NL on output */
/*			ONOCR   Will VMS discard CRs when on column 0? */
    my_attr->c_oflag |= ONLRET; /*VMS RMS adds a CR to NL on output */
    opost_active = 1;
#endif
#if defined __USE_MISC || defined __USE_XOPEN
/* NLDLY */
/*				NL0 - No delay */
/*				NL1 - Delay 0.10 seconds (2 chars) */
/* FFDLY */
/*				FF0 - No delay */
/*				FF1 - Delay about 2 seconds */
/* VTDLY */
/*				VT0 - No delay */
/*				VT1 - Delay about 2 seconds */
    if (mode_iosb->lf_fill != 0) {
	any_fill = 1;
	opost_active = 1;
	my_attr->c_oflag |= (VT1|FF1|VT1);
    }
/* CRDLY */
/*				CR0 - No delay */
/*				CR1 - column dependent (2 chars) */
/*				CR2 - Delay 0.10 seconds (4 chars) */
/*				CR3 - Delay 0.15 seconds */
    if (mode_iosb->cr_fill != 0) {
	any_fill = 1;
	opost_active = 1;
	if (mode_iosb->cr_fill > 4) {
	    my_attr->c_oflag |= CR3;
	} else if (mode_iosb->cr_fill > 2) {
	    my_attr->c_oflag |= CR2;
	} else {
	    my_attr->c_oflag |= CR1;
	}
    }
/* TABDLY */
/*				TAB0 - No delay */
/*				TAB1 - column dependent */
/*				TAB2 - Delay 0.10 seconds (2 chars) */
/*				TAB3 - Expand TAB to spaces */
    if (!termchar->ttdef.tt$v_mechtab) {
	my_attr->c_oflag |= TAB3;	/* Expand tabs to spaces */
	opost_active = 1;
    }
/* BSDLY Not present on VMS */
#endif
#ifdef __USE_GNU
    if (!termchar->ttdef.tt$v_lower) {
	my_attr->c_oflag |= OLCUC;	/* VMS output all upper case */
	opost_active = 1;
    }
#endif
/*			    OFDEL	X/Open says use DEL instead of NULL */
/*					GNU/Linux does not have a definition */
#ifdef __USE_XOPEN
    if (any_fill) {
					/* X/Open Says this means 2	  */
					/* characters sent for delay	  */
	my_attr->c_oflag |= OFILL;	/* Send fill characters for delay */
	opost_active = 1;
    }
#endif

    if (opost_active) {
	my_attr->c_oflag |= OPOST;  /* VMS At least one post feature active */
    }

    /* Control Modes */
    if (mode_iosb->framing.tt_v_altframe) {
	switch(mode_iosb->framing.tt_v_framesize) {
	case 6: my_attr->c_cflag |= CS6;
		cflag_active = 1;
		break;
	case 7: my_attr->c_cflag |= CS7;
		cflag_active = 1;
		break;
	case 8: my_attr->c_cflag |= CS8;
		cflag_active = 1;
		break;
	}
    }
    if (mode_iosb->framing.tt_v_twostop) {  /* two stop bits */
	my_attr->c_cflag |= CSTOPB;
	cflag_active = 1;
    }
    my_attr->c_cflag |= CREAD;	/* VMS not easily available, so say ready */
    cflag_active = 1;
    if (mode_iosb->framing.tt_v_parity) { /* Enable parity */
	my_attr->c_cflag |= PARENB;
	cflag_active = 1;
    }
    if (mode_iosb->framing.tt_v_odd) { /* Odd parity */
	my_attr->c_cflag |=  PARODD;
	cflag_active = 1;
    }
    if (!termchar->ttdef.tt$v_modem ) {
	my_attr->c_cflag |= CLOCAL;	/* Local terminal */
	cflag_active = 1;
    } else {
	my_attr->c_cflag |= HUPCL;	/* Hang up on last close */
    }

#ifdef __USE_BSD
#if (__CRTL_VER >= 080200000) && !defined (__VAX)
    if (termchar->tt3def.tt3$v_rts_flow) {
        my_attr->c_cflag |= CRTSCTS;	/* VMS 8.2 has CTRCTS flow control */
	cflag_active = 1;
    }
#else
    if (termchar->tt2def.tt2$v_commsync) {
	my_attr->c_cflag |= CRTSCTS;	/* Comm sync uses hardware flow ctrl */
	cflag_active = 1;
    }
#endif
    if (termchar->tt2def.tt2$v_commsync) {
	my_attr->c_cflag |= CDTRCTS;	/* Comm sync uses hardware flow ctrl */
	cflag_active = 1;
    }
    if (termchar->ttdef.tt$v_modem) {
	my_attr->c_cflag |= MDMBUF;	/* Modem connected terminal */
	cflag_active = 1;
    }

    if (!cflag_active) {
	my_attr->c_cflag |= CIGNORE; /* Only if no control flags are set */
    }
#endif

    /* Local Modes */
    if (termchar->ttdef.tt$v_scope) {   /* VMS terminal SCOPE set */
	my_attr->c_lflag |= ECHOKE;
	my_attr->c_lflag |= ECHOE;
    } else {
	my_attr->c_lflag |= ECHOK;
    }

    if (!termchar->ttdef.tt$v_noecho) {  /* Characters are echoed */
	my_attr->c_lflag |= ECHO;
    }
/*			ENCHONL   VMS will not echo NL with echo off */
#ifdef __USE_BSD
    my_attr->c_lflag |= ECHOPRT;	/* VMS echos erase characters */
    my_attr->c_lflag |= ECHOCTL;	/* VMS echos control characters */
#endif

					/* VMS pass all input characters */
    if (!termchar->ttdef.tt$v_passall && !termchar->tt2def.tt2$v_pasthru) {
	my_attr->c_lflag |= ISIG;	/* VMS normal mode */
	my_attr->c_lflag |= ICANON;
    }
#ifdef __USE_BSD
/*			ALTWERASE	VMS is hard coded */
#endif
    if (!termchar->ttdef.tt$v_passall && !termchar->tt2def.tt2$v_pasthru) {
	my_attr->c_lflag |= IEXTEN; /* VMS does ^O and ^V normally */
    } else {
	my_attr->c_lflag |= EXTPROC; /* Application, not interactive */
    }
/*			TOSTOP		VMS does not have SIGTTOU */
#ifdef __USE_BSD
/*			FLUSHO		VMS and Linux do not do this. */
    if ((vms_info->oldctrlmask & LIB$M_CLI_CTRLT) == 0) {
	my_attr->c_lflag |= NOKERNINFO; /* VMS Control-T Disabled */
    }
/*			PENDIN		Not sure how to do on VMS */
#endif
/*			NOFLSH		VMS does not do this */

/*			XCASE		VMS does not have this */

    /* Control Character Array */
    my_attr->c_cc[VEOF] = 26;		/* VMS EOF is Control-Z */
					/* Unix EOF is Control-D */
    my_attr->c_cc[VEOL] = 0;		/* Not used with VMS/Linux */
#ifdef __USE_BSD
    my_attr->c_cc[VEOL2] = 0;		/* Not used with VMS/Linux */
#endif
    my_attr->c_cc[VERASE] = 127;	/* VMS default is DEL */
#if (__CRTL_VER >= 080200000) && !defined (__VAX)
    if (termchar->tt3def.tt3$v_bs) {
	my_attr->c_cc[VERASE] = 8;	/* VMS 8.2 can set BS */
    }
#endif
#ifdef __USE_BSD
    my_attr->c_cc[VWERASE] = 10;	/* VMS uses LF to delete word */
					/* Unix uses Control-W */
#endif
    my_attr->c_cc[VKILL] = 21;		/* VMS uses Control-U to delete line */
    my_attr->c_cc[VREPRINT] = 18;	/* VMS uses Control-R to reprint */
    my_attr->c_cc[VINTR] = 3;		/* VMS uses Control-C */
    my_attr->c_cc[VQUIT] = 0;		/* VMS does not have Quit Character */
					/* Unix uses Control-\ */
    my_attr->c_cc[VSUSP] = 0;		/* VMS does not have Suspend */
#ifdef __USE_BSD
    my_attr->c_cc[VDSUSP] = 0;	/* VMS does not have a Delayed suspend */
				/* Unix uses Control-y */
#endif
    my_attr->c_cc[VSTART] = 17;		/* VMS uses Control-Q */
    my_attr->c_cc[VSTOP] = 19;		/* VMS uses Control-S */
    my_attr->c_cc[VLNEXT] = 22;		/* VMS uses Control-V */
    my_attr->c_cc[VDISCARD] = 15;	/* VMS uses Control-O */
    my_attr->c_cc[VSTATUS] = 20;	/* VMS uses Control-T */

    /* Speed section */
    my_attr->__ospeed = vms_speed_to_unix_speed(mode_iosb->txspeed);
    if (mode_iosb->rxspeed == 0) {
	my_attr->__ispeed = my_attr->__ospeed;
    } else {
	my_attr->__ispeed = vms_speed_to_unix_speed(mode_iosb->rxspeed);
    }

    memcpy(vms_info->term_attr, my_attr, sizeof(struct termios));

    return ret_stat;
}


/* vms_term_qio_tcsetattr
 * Does the actual work of setting those VMS terminal characteristics
 * that can be set, ignoring the rest.
 * The vms_term_qio_tcgetattr routine needs to initially populate the vms_info
 * structure that is passed by reference before this call.
 * The vms_info structure will be updated to reflect the changes.
 *
 * vms_info - on input, the VMS device channel, and other information.
 *            on output, updated to reflect any change requested.
 *
 * my_attr  - Readonly structure containing the desired Unix characteristics.
 *
 * We do not seem to be able to set fill here and have not tested
 * Setting speed.  Attempting to set parity returns a SS$_BADPARAM
 * On VMS these should be set before running the program.
 *
 * Returns 0 for success, -1 for an error and sets errno on failure.
 *
 */
int vms_term_qio_tcsetattr(struct vms_info_st * vms_info,
			   const struct termios *my_attr) {
int status;
struct term_mode_iosb_st * mode_iosb;
struct term_char_st * termchar;
const unsigned long newmask = LIB$M_CLI_CTRLT;
unsigned long oldmask;
int is_modem;
int is_scope;
int ret_stat;
struct term_mode_iosb_st set_mode_iosb;

    /* Set up the structures to hold the information if not already there */
    if (vms_info->vms_char == NULL) {
	struct termios old_attr;
	ret_stat = vms_term_qio_tcgetattr(vms_info, &old_attr);
	if (ret_stat < 0) {
	    return ret_stat;
	}
    }

    termchar = vms_info->vms_char;
    mode_iosb = vms_info->vms_iosb;

    is_modem = termchar->ttdef.tt$v_modem;
    is_scope = termchar->ttdef.tt$v_scope;

    /* Input Flags */
/*			IGNBRK     VMS driver handles BREAK */
/*			BKRINT     VMS driver handles BREAK */
/*			IGNPAR     VMS fatal error on bad parity */
/*                      PARMRK     VMS can not mark parity errors */

/* On VMS, we are not setting the framing or parity this way */
/*			INPCK	   Parity is enabled ? */

    if ((my_attr->c_iflag & ISTRIP) != 0) {  /* Strip 8th bit off characters */
        termchar->ttdef.tt$v_eightbit = 0;
    } else {
        termchar->ttdef.tt$v_eightbit = 1;
    }
/*			INLCR	VMS terminates on linefeed and passes through */
/*			IGNCR   VMS can not ignore CR */
/*			ICRNL   VMS RMS maps CR to LF on input */

    if ((my_attr->c_iflag & IXON) != 0) {  /* XON/XOFF on output */
        termchar->ttdef.tt$v_ttsync = 1;
    } else {
        termchar->ttdef.tt$v_ttsync = 0;
    }
    if ((my_attr->c_iflag & IXOFF) != 0) { /* XON/XOFF on input */
        termchar->ttdef.tt$v_hostsync = 1;
    } else {
        termchar->ttdef.tt$v_hostsync = 0;
    }
#ifdef __USE_BSD
/*			IXANY	VMS will not do this. */
/*			IMAXBEL VMS rings bell when queue full */
#endif
#ifdef __USE_GNU
/*			IUCLC   VMS will not translate upper to lower */
#endif


    /* Output flags */
#if defined __USE_BSD || defined __USE_XOPEN
/*			ONLCR	VMS RMS does this automatically */
#endif

#ifdef __USE_BSD
/*			OXTABS	Alias for TAB3 */
/*			ONOEOT		VMS does not pass through ^Z EOF */
/*					to output.			 */
/*					Ignore for now.			 */
#endif
#if defined __USE_BSD || defined __USE_XOPEN
/*			OCRNL	VMS will not map CR to NL on output */
/*			ONOCR   Will VMS discard CRs when on column 0? */
/*			ONLRET  VMS RMS adds a CR to NL on output */
#endif
#if defined __USE_MISC || defined __USE_XOPEN
/* NLDLY */
/*				NL0 - No delay */
/*				NL1 - Delay 0.10 seconds (2 chars) */
/* FFDLY */
/*				FF0 - No delay */
/*				FF1 - Delay about 2 seconds */
/* VTDLY */
/*				VT0 - No delay */
/*				VT1 - Delay about 2 seconds */
/* CRDLY */
/*				CR0 - No delay */
/*				CR1 - column dependent (2 chars) */
/*				CR2 - Delay 0.10 seconds (4 chars) */
/*				CR3 - Delay 0.15 seconds */
/* TABDLY */
/*				TAB0 - No delay */
/*				TAB1 - column dependent */
/*				TAB2 - Delay 0.10 seconds (2 chars) */
/*				TAB3 - Expand TAB to spaces */
    if ((my_attr->c_oflag & TAB3) != 0) {	/* Expand tabs to spaces */
	termchar->ttdef.tt$v_mechtab = 0;
    } else {
	termchar->ttdef.tt$v_mechtab = 1;
    }


/* BSDLY Not present on VMS */
#endif
#ifdef __USE_GNU
    if ((my_attr->c_oflag & OLCUC) != 0) {  /* VMS output all upper case */
	termchar->ttdef.tt$v_lower = 0;
    } else {
	termchar->ttdef.tt$v_lower = 1;
    }
#endif
/*			    OFDEL	X/Open says use DEL instead of NULL */
/*					GNU/Linux does not have a definition */
#ifdef __USE_XOPEN
					/* X/Open Says this means 2	  */
					/* characters sent for delay	  */
/*			OFILL	*/	/* Send fill characters for delay */
#endif

/*			OPOST		VMS At least one post feature active */

    /* Control Modes */

#ifdef __USE_BSD
/*			CIGNORE		Only if no control flags are set */
    if ((my_attr->c_cflag & CIGNORE) != 0) {
#endif

/*		CSIZE == CS6 , CS7, CS8  Frame size not set here on VMS */
/*			CSTOPB		Stop bits not set here on VMS */

/*  			CREAD	    VMS not easily available, so say ready */

/*			PARENB	    VMS not setting parity here */
/*			PARODD	    VMS not setting Odd parity here */

/*			HUPCL		Hangup modem on logout */
/*			CLOCAL		Local terminal - Not modem */
/* Resolving conflict by having HUPCL override CLOCAL */
/* Setting modem by sensemode should disconnect or logout the process */
	if ((my_attr->c_cflag & CLOCAL) != 0) {
	    is_modem = 0;
	}
	if ((my_attr->c_cflag & HUPCL) != 0) {
	    is_modem = 1;
	}

#ifdef __USE_BSD
	/* VMS 8.2 has CTRCTS flow control */
#if (__CRTL_VER >= 080200000) && !defined (__VAX)
	if ((my_attr->c_cflag & CRTSCTS) != 0) {
	    termchar->tt3def.tt3$v_rts_flow = 1;
	} else {
	    termchar->tt3def.tt3$v_rts_flow = 0;
	}
#else
	if ((my_attr->c_cflag & CRTSCTS) != 0) {
	    /* Comm sync uses hardware flow ctrl */
	    termchar->tt2def.tt2$v_commsync = 1;
	} else {
	   termchar->tt2def.tt2$v_commsync = 0;
	}
#endif
	/* Comm sync uses hardware flow ctrl */
	if ((my_attr->c_cflag & CDTRCTS) != 0) {
	    termchar->tt2def.tt2$v_commsync = 1;
	} else {
	    termchar->tt2def.tt2$v_commsync = 0;
	}
	if ((my_attr->c_cflag & MDMBUF) != 0) {	/* Modem connected terminal */
	    is_modem = 1;
	}
#endif

#ifdef __USE_BSD
    }
#endif

    /* Local Modes */
    if (((my_attr->c_lflag & ECHOKE) != 0) ||
	((my_attr->c_lflag & ECHOE) != 0)) {
	is_scope = 1;			/* VMS terminal SCOPE set */
    } else if ((my_attr->c_lflag & ECHOK) != 0) {
	is_scope = 0;			/* VMS terminal SCOPE clear */
    }


/*			ECHOK		Echo NL after kill (non-scope) */

    if ((my_attr->c_lflag & ECHO) != 0) {
	termchar->ttdef.tt$v_noecho = 0;  /* Characters are echoed */
    } else {
	termchar->ttdef.tt$v_noecho = 1;  /* Characters are not echoed */
    }
/*			ENCHONL   VMS will not echo NL with echo off */
#ifdef __USE_BSD
/*			ECHOPRT   VMS hardcopy echos erase characters */
/*			ECHOCTL	  VMS echos control characters */
#endif

    if (((my_attr->c_lflag & ISIG) != 0) ||
	((my_attr->c_lflag & ICANON) != 0) ||
	((my_attr->c_lflag & IEXTEN) != 0)) {

	/* VMS interpret input characters */
	termchar->tt2def.tt2$v_pasthru = 0;
    } else {
	/* VMS pass-through input characters */
	termchar->tt2def.tt2$v_pasthru = 1;
    }
#ifdef __USE_BSD
/*			ALTWERASE	VMS is hard coded */
#endif
/*			IEXTEN		VMS does ^O and ^V normally */

/*			EXTPROC		Application, not interactive */
/*					Need to set before entering */
/*			TOSTOP		VMS does not have SIGTTOU */
#ifdef __USE_BSD
/*			FLUSHO		VMS and Linux do not do this. */
    if ((my_attr->c_lflag & NOKERNINFO) != 0) {  /* VMS Control-T handling */
	if ((vms_info->oldctrlmask & LIB$M_CLI_CTRLT) != 0) {
	    status = LIB$DISABLE_CTRL(&newmask, &oldmask);
	}
    } else {
	if ((vms_info->oldctrlmask & LIB$M_CLI_CTRLT) == 0) {
	    status = LIB$ENABLE_CTRL(&newmask, &oldmask);
	}
    }
/*			PENDIN		Not sure how to do on VMS */
#endif
/*			NOFLSH		VMS does not do this */

/*			XCASE		VMS does not have this */

    /* Control Character Array */
/*    my_attr->c_cc[VEOF] = 26; */	/* VMS EOF is Control-Z */
					/* Unix EOF is Control-D */
/*    my_attr->c_cc[VEOL] = 0; */	/* Not used with VMS/Linux */
#ifdef __USE_BSD
/*    my_attr->c_cc[VEOL2] = 0; */	/* Not used with VMS/Linux */
#endif
/*    my_attr->c_cc[VERASE] = 127; */	/* VMS default is DEL */
#if (__CRTL_VER >= 080200000) && !defined (__VAX)
    if (my_attr->c_cc[VERASE] == 8) {	/* VMS 8.2 can set BS */
    	termchar->tt3def.tt3$v_bs = 1;
    } else {
	termchar->tt3def.tt3$v_bs = 0;
    }
#endif
#ifdef __USE_BSD
/*  my_attr->c_cc[VWERASE] = 10; */	/* VMS uses LF to delete word */
					/* Unix uses Control-W */
#endif
/*    my_attr->c_cc[VKILL] = 21; */	/* VMS uses Control-U to delete line */
/*    my_attr->c_cc[VREPRINT] = 18; */	/* VMS uses Control-R to reprint */
/*    my_attr->c_cc[VINTR] = 3;	*/	/* VMS uses Control-C */
/*    my_attr->c_cc[VQUIT] = 0;	*/	/* VMS does not have Quit Character */
					/* Unix uses Control-\ */
/*    my_attr->c_cc[VSUSP] = 0; */	/* VMS does not have Suspend */
#ifdef __USE_BSD
/*    my_attr->c_cc[VDSUSP] = 0; */ /* VMS does not have a Delayed suspend */
				/* Unix uses Control-y */
#endif
/*    my_attr->c_cc[VSTART] = 17; */	/* VMS uses Control-Q */
/*    my_attr->c_cc[VSTOP] = 19; */	/* VMS uses Control-S */
/*    my_attr->c_cc[VLNEXT] = 22; */	/* VMS uses Control-V */
/*    my_attr->c_cc[VDISCARD] = 15; */	/* VMS uses Control-O */
/*    my_attr->c_cc[VSTATUS] = 20; */	/* VMS uses Control-T */

    /* Is this a video terminal? */
    termchar->ttdef.tt$v_scope = is_scope;

    /* Is this a modem? */
    termchar->ttdef.tt$v_modem = is_modem;

    status = SYS$QIOW
       (EFN$C_ENF,
	vms_info->channel,
	IO$_SETMODE,
	&set_mode_iosb,
	NULL,
	NULL,
	termchar, sizeof(struct term_char_st),
	0, 0, 0, 0);
    if ($VMS_STATUS_SUCCESS(status) &&
	$VMS_STATUS_SUCCESS(set_mode_iosb.sts)) {

	    /* We set the data, cache it */
	    vms_info->term_attr->c_iflag = my_attr->c_iflag;
	    vms_info->term_attr->c_oflag = my_attr->c_oflag;
	    vms_info->term_attr->c_lflag = my_attr->c_lflag;
	    if (termchar->tt2def.tt2$v_pasthru) {
	        vms_info->term_attr->c_lflag &= ~(ISIG|ICANON|IEXTEN);
	    }

	    /* Except if CIGNORE set for changes */
	    if ((my_attr->c_cflag & CIGNORE) == 0) {
		vms_info->term_attr->c_cflag = my_attr->c_cflag;
	    }

	    /* VMS 8.2 can set BS, so this could have changed */
	    vms_info->term_attr->c_cc[VERASE] = my_attr->c_cc[VERASE];

    } else {
	/* Something really wrong */
	ret_stat = -1;
	errno = EIO;
    }

    return ret_stat;
}

/* Routine to do raw/cooked reads on a terminal */

size_t
vms_terminal_qio_read(struct vms_info_st * vms_info,
		      void * buf,
		      size_t nbytes) {
int status;
int ret_stat;
struct term_read_iosb_st read_iosb;
int command;

    command = IO$_READVBLK;
    if (vms_info->term_attr != NULL) {
 	if ((vms_info->term_attr->c_lflag & ICANON) == 0) {
	    command |= IO$M_NOFILTR;
	}
    }

    status = SYS$QIOW
       (EFN$C_ENF,
	vms_info->channel,
	command,
	&read_iosb,
	NULL,
	NULL,
	buf, nbytes, 0, 0, 0, 0);
    if ($VMS_STATUS_SUCCESS(status) &&
	$VMS_STATUS_SUCCESS(read_iosb.sts)) {
	    /* We have data, include the terminator if it is in the buffer */
	    ret_stat = read_iosb.count + read_iosb.terminator_count;
    } else {
	/* Something really wrong */
	ret_stat = -1;
	errno = EIO;
	return ret_stat;
    }

    return ret_stat;
}

size_t
vms_terminal_qio_write(struct vms_info_st * vms_info,
		       const void * buf,
		       size_t nbytes) {
int status;
int ret_stat;
struct term_write_iosb_st write_iosb;

    status = SYS$QIOW
       (EFN$C_ENF,
	vms_info->channel,
	IO$_WRITEVBLK,
	&write_iosb,
	NULL,
	NULL,
	buf, nbytes, 0, 0, 0, 0);
    if ($VMS_STATUS_SUCCESS(status) &&
	$VMS_STATUS_SUCCESS(write_iosb.sts)) {
	    /* We have data */
	    ret_stat = write_iosb.count;
    } else {
	/* Something really wrong */
	ret_stat = -1;
	errno = EIO;
	return ret_stat;
    }
    return ret_stat;
}



/* Routine to set the Unix terminal attributes for an external program */
int export_unix_terminal_attributes(int fd) {

    /* Create a logical name GNV$<terminal>$ATTR in the process table */
    /* It will hold the binary contents of the termchar structure */
    return -1;
}

/* Routine to fetch the Unix terminal attributes from external program */
int fetch_unix_terminal_attributes(int fd) {

    /* Get the logical name GNV$<terminal>$ATTR and  cache it */
    return -1;
}

/* The first time that we need to get the terminal attributes */
int vms_init_term_attr(int fd) {
int result = 0;

    /* Have we set or looked up the attributes yet? */
    if (vms_info[fd].term_attr == NULL) {

	struct termios my_attr;

	/* We need to first get the current settings */
	result = vms_term_qio_tcgetattr(&vms_info[fd], &my_attr);
	if (result != 0) {
	    return -1;
	}

	/* TODO: */
        /* If we are under a UNIX shell, then we need to get the terminal */
	/* settings from the parent process.  This needs to be passed */
	/* by some mechanism like symbols or logical names or lock value */
	/* blocks */

    }
    return result;
}


  /******************************/
 /* stdio replacement routines */
/******************************/
/* This is a replacement for the CRTL fileno() routine.
 *
 * The first terminal access in ncurses is using the fileno() routine,
 * so we intercept it and open a VMS channel.  While it does not appear
 * needed, some sanity checks have been added to see if the device was
 * known before or a different device is connected.
 */
int vms_terminal_fileno(FILE * stream) {
    int fd;

    fd = fileno(stream);

    /* fileno() worked */
    if (fd >= 0) {

	/* For ncurses, we only care about terminals */
	if (isatty(fd)) {
            int result;
	    unsigned short channel;

	    /* Is this fd still the same terminal? */
	    if ((vms_info != NULL) && (vms_info[fd].ref_cnt > 0)) {
		char device_name[256];
		char *retname;

		retname = getname(fd, device_name, 1);
		if (retname == NULL) {
		    /* We can not get the VMS info at this time */
		    return fd;
		}

		result = strcmp(device_name, vms_info[fd].device_name);
		if (result == 0) {
		    /* This is the same, we already have a VMS channel */
		    return fd;
		}

		/* Force a cleanup of the old VMS information */
		vms_info[fd].ref_cnt = 1;
		result = vms_channel_close(fd);

	    }

	    /* Assign a VMS channel for this if needed */
	    result = vms_channel_lookup(fd, &channel);
	}
    }
    return fd;
}


  /********************************/
 /* termios replacement routines */
/********************************/
int vms_terminal_tcgetattr(int fd, struct termios * buf) {
int result;
unsigned short chan;
int status;

    /* Would someone call this on a non-terminal? */
    if (!isatty(fd)) {
	errno = EBADF;
	return -1;
    }

    /* Open the channel if needed */
    status = vms_channel_lookup(fd, &chan);
    if (status == 0) {


	/* We need to first get the current settings */
	status = vms_term_qio_tcgetattr(&vms_info[fd], buf);

	if (status != 0) {
	    vms_channel_close(fd);
	    return -1;
	}


	/* Unix mode, just look up the last cached value */
	memcpy(buf, vms_info[fd].term_attr, sizeof (struct termios));
	status = 0;

	/* close the channel if we opened it */
	vms_channel_close(fd);
    }
    return status;
}



int vms_terminal_tcsetattr(int fd, int action, const struct termios * buf) {

int result;
unsigned short chan;
int status;

    /* Would someone call this on a non-terminal? */
    if (!isatty(fd)) {
	errno = EBADF;
	return -1;
    }

    if (action != TCSANOW) {
	/* Unless we totally take over the I/O, the best that */
	/* we can do is attempt a a fsync() */
	fsync(fd);
    }

    /* Open the channel if needed */
    status = vms_channel_lookup(fd, &chan);
    if (status == 0) {

	/* Now attempt to set them */
	status = vms_term_qio_tcsetattr(&vms_info[fd], buf);
    }

    /* close the channel if we opened it */
    vms_channel_close(fd);
    return status;
}

int vms_terminal_tcflush(int fd, int queue_selector) {
    errno = ENOSYS;
    return -1;
}

  /*******************************/
 /* stropts replacement routine */
/*******************************/
int vms_terminal_ioctl(int fd, int r, void * argp) {
    size_t result;

    if (fd >= 0) {
	if (isatty(fd)) {
	    result = vms_term_qio_ioctl(fd, r, argp);
        }
    } else {
	result = ioctl(fd, r, argp);
    }

    return result;
}



  /*******************************/
 /* socket replacement routines */
/*******************************/
int vms_terminal_select(int nfds, fd_set * readfds, fd_set * writefds,
                        fd_set * exceptfds, struct timeval * timeout) {
int ret_stat;
int siif_index;
int old_siif_value;
int new_siif_value;
int i;
#ifndef __VAX /* Fix this later */
const char * select_ignores_invalid_fd = "DECC$SELECT_IGNORES_INVALID_FD";

    /* Get old ignore setting and enable new */
    /* threaded applications need this setting enabled before this routine */
    siif_index = decc$feature_get_index(select_ignores_invalid_fd);
    if (siif_index >= 0)
	old_siif_value = decc$feature_set_value(siif_index, 1, 1);
#endif

    if (nfds != 0) {
    int i;
    struct pollfd *poll_array;
    struct vms_pollfd_st *term_array;
    struct vms_pollfd_st *pipe_array;
    struct vms_pollfd_st *xefn_array;
    int ti;
    int si;
    int pi;
    int xi;
    int status;

	/* Need structures to separate terminals and pipes */
	poll_array = malloc(sizeof(struct pollfd) * (nfds + 1));
	if (poll_array == NULL) {
	    return -1;
	}
	term_array = malloc(sizeof(struct vms_pollfd_st) * (nfds + 1));
	if (term_array == NULL) {
	    free(poll_array);
	    return -1;
	}
	pipe_array = malloc(sizeof(struct vms_pollfd_st) * (nfds + 1));
	if (pipe_array == NULL) {
	    free(term_array);
	    free(poll_array);
	    return -1;
	}
	xefn_array = malloc(sizeof(struct vms_pollfd_st) * (nfds + 1));
	if (xefn_array == NULL) {
	    free(pipe_array);
	    free(term_array);
	    free(poll_array);
	    return -1;
	}

	/* Find all of the terminals and pipe fds  for polling */
	for (i = 0, pi = 0, ti = 0, xi = 0; i <= nfds; i++) {

	    /* Copy file descriptor arrays into a poll structure */
	    poll_array[i].fd = i;
	    poll_array[i].events = 0;
	    poll_array[i].revents = 0;

	    /* Now separate out the pipes and terminals */
	    if (isapipe(i) == 1) {
		pipe_array[pi].fd_desc_ptr = &poll_array[i];
		pipe_array[pi].fd = i;
		if (readfds != NULL) {
		    if (FD_ISSET(i, readfds))
			poll_array[i].events |= POLL_IN;
		}
		if (writefds != NULL) {
		    if (FD_ISSET(i, writefds))
			poll_array[i].events |= POLL_OUT;
		}
		/* Only care about something with read/write events */
		if (poll_array[i].events != 0) {
		    status = vms_channel_lookup(i, &pipe_array[pi].channel);
		    if (status == 0)
			pi++;
		}
	    }
	    /* Not a pipe, see if a terminal */
	    else if (isatty(i) == 1) {
		term_array[ti].fd_desc_ptr = &poll_array[i];
		term_array[ti].fd = i;
		if (readfds != NULL) {
		    if (FD_ISSET(i, readfds))
			poll_array[i].events |= POLL_IN;
		}
		if (writefds != NULL) {
		    if (FD_ISSET(i, writefds))
			poll_array[i].events |= POLL_OUT;
		}
		/* Only care about something with read/write events */
		if (poll_array[i].events != 0) {
		    status = vms_channel_lookup
			(i, &term_array[ti].channel);
		    if (status == 0)
			ti++;
		}
	    }
	    else if (decc$get_sdc(i) != 0) {
		    /* Not pipe or terminal, use built in select on this */
		    si++;
	    }
	    /* What's left? X11 event flags */
	    else {
		xefn_array[xi].fd_desc_ptr = &poll_array[i];
		if (readfds != NULL) {
		    if (FD_ISSET(i, readfds))
			poll_array[i].events |= POLL_IN;
		}
		if (writefds != NULL) {
		    if (FD_ISSET(i, writefds))
			poll_array[i].events |= POLL_OUT;
		}
		/* Only care about something with read/write events */
		if (poll_array[i].events != 0) {
		    xi++;
		}
	    }
	}
	if ((pi == 0) && (ti == 0) && (xi == 0)) {
	    /* All sockets, let select do everything */
	    ret_stat = select
		    (nfds, readfds, writefds, exceptfds, timeout);
	}
	else {
	time_t stimeleft; /* Seconds left */
	int utimeleft; /* Microseconds left */
	int ti_stat;
	int pi_stat;
	int si_stat;
	int xi_stat;

	    ti_stat = 0;
	    pi_stat = 0;
	    si_stat = 0;
	    xi_stat = 0;
	    if (timeout != NULL) {
		stimeleft = timeout->tv_sec;
		utimeleft = timeout->tv_usec;
		if (stimeleft == (time_t)-1)
		    utimeleft = 0;
	    }
	    else {
		stimeleft = (time_t)-1;
		utimeleft = 0;
	    }
	    ret_stat = 0;

	     /* Terminals and or pipes and or sockets  */
	    /* Now we have to periodically poll everything with timeout */
	    while (ret_stat == 0) {
	    int sleeptime;
	    struct timeval sleep_timeout;

		if (ti != 0) {
		    ti_stat = vms_poll_terminal(term_array, ti);
		}
		if (pi != 0) {
		    pi_stat = vms_poll_pipe(pipe_array, pi);
		}
		if (xi != 0) {
		    xi_stat = vms_poll_x11_efn(xefn_array, xi);
		}

		sleep_timeout.tv_sec = 0;
		if (ti_stat != 0 || pi_stat != 0) {
		    sleeptime = 0;
		    sleep_timeout.tv_usec = 0;
		}
		else {
		    sleeptime = 100 * 1000;
		    sleep_timeout.tv_sec = 0;
		    sleep_timeout.tv_usec = sleeptime;
		    if ((stimeleft == 0) && (utimeleft <sleeptime)) {
			sleeptime = utimeleft;
			sleep_timeout.tv_usec = utimeleft;
		    }
		}
		if (si == 0) {
		    /* sleep for shorter of 100 Ms or timeout and retry */
		    if (sleeptime > 0)
			usleep(sleeptime);
		}
		else {
		    si_stat = select
			       (nfds,
				readfds,
				writefds,
				exceptfds, &sleep_timeout);
		}
		 /* one last poll of terminals and pipes */
		if ((sleeptime > 0) || (si_stat > 0)) {
		    if ((ti != 0) && (ti_stat == 0)) {
			ti_stat = vms_poll_terminal(term_array, ti);
		    }
		    if ((pi != 0) && (pi_stat == 0)) {
			pi_stat = vms_poll_pipe(pipe_array, pi);
		    }
		    if ((xi != 0) && (xi_stat == 0)) {
			xi_stat = vms_poll_x11_efn(xefn_array, xi);
		    }
		}

		/* how much time is left? */
		if (utimeleft > 0) {
		    utimeleft -= sleeptime;
		    if ((utimeleft <= 0) && (stimeleft != (time_t)-1) &&
			(stimeleft > 0)) {
			stimeleft--;
			utimeleft = 1000 * 1000; /* 1 second */
		    }
		    else {
			stimeleft = 0;
			utimeleft = 0;
		    }
		}

		/* Gather up any results */
		if ((ti_stat == -1) || (pi_stat == -1) || (si_stat == -1) ||
		    (xi_stat == -1)) {
		    ret_stat = -1;
		}
		else {
		    ret_stat = ti_stat + pi_stat + si_stat + xi_stat;
		}

		/* Copy the pipe and terminal information */
		if ((ti_stat > 0) || (pi_stat > 0) || (xi_stat > 0)) {
		int j;
		    for (j = 0; j < nfds; j++) {
			if (poll_array[j].events != 0) {
			    if (readfds != NULL) {
				if (poll_array[j].revents & POLL_IN)
				    FD_SET(poll_array[j].fd, readfds);
				else
				    FD_CLR(poll_array[j].fd, readfds);
			    }
			    if (writefds != NULL) {
				if (poll_array[j].revents & POLL_OUT)
				    FD_SET(poll_array[j].fd, writefds);
				else
				    FD_CLR(poll_array[j].fd, writefds);
			    }
			}
		    }
		}
		/* Timed out? */
		if ((stimeleft == 0) && (utimeleft == 0)) {
		    break;
		}
	    }
	}
	while (pi > 0) {
	    pi--;
	    vms_channel_close(pipe_array[ti].fd);
	}
	free(poll_array);
	while (ti > 0) {
	    ti--;
	    vms_channel_close(term_array[ti].fd);
	}
	free(term_array);
	free(pipe_array);
	free(xefn_array);
    }
    else {

       ret_stat = select(nfds, readfds, writefds, exceptfds, timeout);
    }

    /* Restore old setting */
#ifndef __VAX
    if (siif_index >= 0)
	new_siif_value = decc$feature_set_value(siif_index, 1, old_siif_value);
#endif

    return ret_stat;
}



  /*****************************/
 /* poll replacement routines */
/*****************************/
#ifndef __VAX
int vms_terminal_poll(struct pollfd fd_array[], nfds_t nfds, int timeout)
{
int ret_stat;

struct vms_pollfd_st *sock_array;
struct pollfd *sfd_array;
struct vms_pollfd_st *term_array;
struct vms_pollfd_st *pipe_array;
struct vms_pollfd_st *xefn_array;

    /* Sort out the pipes, terminals from the array */
    if ((nfds != 0) && (fd_array != NULL)) {
    int i;
    int pi;
    int ti;
    int si;
    int xi;
    int status;
    int ti_stat;
    int pi_stat;
    int si_stat;
    int xi_stat;

	ti_stat = 0;
	pi_stat = 0;
	si_stat = 0;
	xi_stat = 0;

	/* Need 5 new arrays */
	sock_array = malloc(sizeof(struct vms_pollfd_st) * nfds);
	if (sock_array == NULL)
	    return -1;
	sfd_array = malloc(sizeof(struct pollfd) * nfds);
	if (sfd_array == NULL) {
	    free(sock_array);
	    return -1;
	}
	term_array = malloc(sizeof(struct vms_pollfd_st) * nfds);
	if (term_array == NULL) {
	    free(sock_array);
	    free(sfd_array);
	    return -1;
	}
	pipe_array = malloc(sizeof(struct vms_pollfd_st) * nfds);
	if (pipe_array == NULL) {
	    free(term_array);
	    free(sock_array);
	    free(sfd_array);
	    return -1;
	}
	xefn_array = malloc(sizeof(struct vms_pollfd_st) * nfds);
	if (xefn_array == NULL) {
	    free(pipe_array);
	    free(term_array);
	    free(sock_array);
	    free(sfd_array);
	    return -1;
	}
	memset(sock_array, 0, sizeof(struct vms_pollfd_st) * nfds);
	memset(sfd_array, 0, sizeof(struct pollfd) * nfds);
	memset(term_array, 0, sizeof(struct vms_pollfd_st) * nfds);
	memset(pipe_array, 0, sizeof(struct vms_pollfd_st) * nfds);
	memset(xefn_array, 0, sizeof(struct vms_pollfd_st) * nfds);

	/* Now actually separate things out */
	for (i = 0, pi = 0, si = 0, ti = 0, xi = 0; i < nfds; i++) {
	    fd_array[i].revents = 0;

	    /* Only care about devices that are waiting for something */
	    if (fd_array[i].events != 0) {

		/* First look for pipes */
	        if (isapipe(fd_array[i].fd) == 1) {
		    pipe_array[pi].fd_desc_ptr = NULL;
		    if (fd_array[i].events & (POLL_IN | POLL_OUT)) {
			pipe_array[pi].fd_desc_ptr = &fd_array[i];
			status = vms_channel_lookup
				(fd_array[i].fd, &pipe_array[pi].channel);
			if (status == 0)
			   pi++;
		    }
		}
		/* Then look for terminals */
		else if (isatty(fd_array[i].fd) == 1) {
		    term_array[ti].fd_desc_ptr = NULL;
		    if (fd_array[i].events & (POLL_IN | POLL_OUT)) {
			term_array[ti].fd_desc_ptr = &fd_array[i];
			status = vms_channel_lookup
				(fd_array[i].fd, &term_array[ti].channel);
			if (status == 0)
			    ti++;
		    }
		}
		/* Should be a socket */
		else if (decc$get_sdc(fd_array[i].fd) != 0) {
		    sfd_array[si] = fd_array[i];
		    sock_array[si].fd_desc_ptr = &fd_array[i];
		    si++;
		}
		/* If not a file descriptor or socket, X11 event flag */
		else {
		    xefn_array[xi].fd_desc_ptr = NULL;
		    if (fd_array[i].events & (POLL_IN | POLL_OUT)) {
			xefn_array[xi].fd_desc_ptr = &fd_array[i];
			xi++;
		    }
		}
	    }
	}
	if ((ti == 0) && (pi == 0) && (si == 0) && (xi == 0)) {
	    /* Trivial case, all sockets */
	    ret_stat = poll(fd_array, nfds, timeout);
	}
	else {
	int timeleft;

	    timeleft = timeout;
	    ret_stat = 0;

	    /* Terminals and or pipes and or sockets  */
	    /* Now we have to periodically poll everything with timeout */
	    while (ret_stat == 0)
	    {
	    int sleeptime;

		if (ti != 0) {
		    ti_stat = vms_poll_terminal(term_array, ti);
		}
		if (pi != 0) {
		    pi_stat = vms_poll_pipe(pipe_array, pi);
		}
		if (xi != 0) {
		    xi_stat = vms_poll_x11_efn(xefn_array, xi);
		}

		if ((ti_stat != 0) || (pi_stat != 0) || (xi_stat != 0))
		    sleeptime = 0;
		else {
		    sleeptime = 100;
		    if ((timeleft < sleeptime) && (timeleft > -1))
			sleeptime = timeleft;
		}

		if (si == 0) {
		    /* sleep for shorter of 100 Ms or timeout and retry */
		    if (sleeptime > 0)
			usleep(sleeptime * 1000);
		}
		else {
		int j;

		     /* select for shorter of 100 Ms or timeout and retry */
		     si_stat = poll(sfd_array, si, sleeptime);
		     if (si_stat != 0) {

			/* Need to copy the results back to original array */
			for (j = 0; j < si; j++)
			{
			    sock_array[j].fd_desc_ptr[0] = sfd_array[j];
			}
		     }
		}
		/* one last poll of terminals and pipes */
		if ((sleeptime > 0) || (si_stat > 0)) {
		    if ((ti != 0) && (ti_stat == 0)) {
			ti_stat = vms_poll_terminal(term_array, ti);
		    }
		    if ((pi != 0) && (pi_stat == 0)) {
			pi_stat = vms_poll_pipe(pipe_array, pi);
		    }
		    if ((xi != 0) && (xi_stat == 0)) {
			xi_stat = vms_poll_x11_efn(xefn_array, xi);
		    }
		}
		if (timeleft > 0) {
		    timeleft -= sleeptime;
		    if (timeleft < 0)
			timeleft = 0;
		}

		/* Gather up any results */
		if ((ti_stat == -1) || (pi_stat == -1) || (si_stat == -1) ||
		    (xi_stat == -1)) {
		    ret_stat = -1;
		}
		else {
		    ret_stat = ti_stat + pi_stat + si_stat + xi_stat;
		}

		/* Out of time? */
	        if (timeleft <= 0)
		    break;
	    }
	}

	/* Clean up channels */
	free(xefn_array);
	while (ti > 0) {
	    ti--;
	    if (term_array[ti].fd_desc_ptr != NULL) {
		vms_channel_close(term_array[ti].fd);
		term_array[ti].fd_desc_ptr = NULL;
	    }
	}
	free(term_array);
	while (pi > 0) {
	    pi--;
	    if (pipe_array[ti].fd_desc_ptr != NULL) {
		vms_channel_close(pipe_array[ti].fd);
		pipe_array[pi].fd_desc_ptr = NULL;
	    }
	}
	free(pipe_array);
	free(sock_array);
	free(sfd_array);

    }
    else {
	/* Why would this be called with an empty array? */
	ret_stat = poll(fd_array, nfds, timeout);
    }

    return ret_stat;
}
#endif


  /*******************************/
 /* unixio replacement routines */
/*******************************/
ssize_t vms_terminal_read(int fd, void * buf, size_t nbytes) {

    size_t result;

    if (fd >= 0) {
	if (isatty(fd)) {
	    int status;
	    unsigned short chan;

	    /* Open the channel if needed */
	    status = vms_channel_lookup(fd, &chan);
	    if (status == 0) {
		result = vms_terminal_qio_read(&vms_info[fd], buf, nbytes);
		return result;
	    }

	    /* close the channel if we opened it */
	    vms_channel_close(fd);
	}
    }

    result = read(fd, buf, nbytes);

    return result;
}

ssize_t vms_terminal_write(int fd, const void * buf, size_t nbytes) {
    size_t result;

    if (fd >= 0) {
	if (isatty(fd)) {
	    int status;
	    unsigned short chan;

	    /* Open the channel if needed */
	    status = vms_channel_lookup(fd, &chan);
	    if (status == 0) {
	        result = vms_terminal_qio_write(&vms_info[fd], buf, nbytes);
		return result;
	    }

	    /* close the channel if we opened it */
	    vms_channel_close(fd);
	}
    }

    /* TODO: Because of VMS differences, not all bytes may have been */
    /* written.  To emulate Unix better, it should loop until either */
    /* an I/O error or all bytes are written */

    result = write(fd, buf, nbytes);

    return result;
}
