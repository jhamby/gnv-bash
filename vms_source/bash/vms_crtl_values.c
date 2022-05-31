#include <errno.h>
#include <unixlib.h>

#define __NEW_STARLET 1
#include <descrip.h>
#include <stsdef.h>
#include <lib$routines.h>

/*
** Sets current value for a feature
*/
static void set(const char *name, int value) {
    errno = 0;
    int index = decc$feature_get_index(name);
    if (index > 0) {
        decc$feature_set_value(index, 1, value);
    }
}

void vms_set_crtl_values(void) {
    set ("DECC$UNIX_LEVEL", 100);

    const char *disable_feature[] = {
        "DECC$EFS_CASE_SPECIAL",
        "DECC$DETACHED_CHILD_PROCESS",
        "DECC$POSIX_STYLE_UID",         // else getpwuid() doesn't work
    };

    const char *enable_feature[] = {
        "DECC$EFS_FILE_TIMESTAMPS",
        "DECC$ENABLE_GETENV_CACHE",
        "DECC$EXIT_AFTER_FAILED_EXEC",
        "DECC$FILE_SHARING",
        "DECC$MAILBOX_CTX_STM",
        "DECC$POPEN_NO_CRLF_REC_ATTR",
        "DECC$POSIX_SEEK_STREAM_FILE",

        "DECC$ARGV_PARSE_STYLE",
        "DECC$DISABLE_TO_VMS_LOGNAME_TRANSLATION",
        "DECC$EFS_CASE_PRESERVE",
        "DECC$EFS_CHARSET",
        "DECC$FILENAME_UNIX_NO_VERSION",
        "DECC$FILENAME_UNIX_REPORT",
        "DECC$READDIR_DROPDOTNOTYPE",
        "DECC$RENAME_NO_INHERIT",
    };

    for(int i = 0; i < sizeof(disable_feature)/sizeof(disable_feature[0]); ++i) {
        set (disable_feature[i], 0);
    }

    for(int i = 0; i < sizeof(enable_feature)/sizeof(enable_feature[0]); ++i) {
        set (enable_feature[i], 1);
    }
    // set ("DECC$POSIX_COMPLIANT_PATHNAMES", 1);  // required for realpath(), but getcwd() is failed
     /* Pipe feature settings are no longer needed with virtual memory pipe
        code. Programs that use pipe need to be converted to use the
        virtual memory pipe code, which effectively removes the hangs and
        left over temporary files.

        Comment left here to prevent regressions, as the larger pipe size
        actually hurts memory usage with the new algorithm.
     */
    /* do_not_set ("DECC$PIPE_BUFFER_SIZE", 65535); */

    // decc$set_reentrancy(C$C_MULTITHREAD);    // not required for bash

    // Define GNV$UNIX_SHELL like the previous vms_crtl_init.c did for bash.
    $DESCRIPTOR(unix_shell, "GNV$UNIX_SHELL");
    $DESCRIPTOR(shell_value, "bash");
    unsigned int status = lib$set_logical(&unix_shell, &shell_value);
    if (!$VMS_STATUS_SUCCESS(status)) {
        /* We have a big problem */
        lib$signal(status);
    }
}
