/* File: VMS_FAKEFORK.H
 *
 * $Id: vms_fakefork.h,v 1.5 2013/06/14 05:02:56 robertsonericw Exp $
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

/*
  This file is used in port of Bash to fake out doing a fork of bash.

  Bash appears to fork even internal commands, and then exits the forked copy
  instead of having to clean up after the command.

  With VMS, instead of forking, we save parts of the environment and then
  when the "parent" is resuming from the command, we restore them.

  The environment save and restore is mostly taken directly from the
  VMS 1.14.8 port that has been in GNV 2.1.3 and earlier.
*/

/* Include the trap.h file to get the extra definitions for the */
/* the vms-specific functions related to saving and restoring   */
/* the trap and signal state across subshells.                  */

#include <stdio.h>
#include <stat.h>
#include "trap.h"
#include "input.h"

/* OpenVMS-specific Declarations and for storing and retrieving */
/* last_made_pid values. */

void PushPid(int Pid);
int PopPid();
int GetCurrentChildExitPid();
void ExchangeParentChildPids();

extern int vms_child_exit_pid;

extern int vms_fake_fork_level;

extern int vms_fake_exit_seen;

int vms_fake_fork(void);

int vms_fake_fork_exit(int status);

int vms_fake_wait_for(pid_t child_pid);

char * vms_getjpi_imagename(void);

/* Debug routines */
void debug_var_context(const VAR_CONTEXT *context);
void debug_var_context_table(const VAR_CONTEXT *context);
void debug_shell_var(const SHELL_VAR * shvar);
void debug_bucket_content(const BUCKET_CONTENTS * bkt_content, int var);
int debug_bucket_contents(const BUCKET_CONTENTS * bkt_content, int var);
void debug_hash_table(const HASH_TABLE * hashptr, int var);
void debug_filedesc(int fd);
void debug_stdfd(void);


/* Expose routines added to trap.c */

int vms_get_signal_modes(int sig);
int vms_set_signal_modes(int sig, int modes);

#define VMS_SPECIAL_PID 0x10000

#define VMS_WAIT_CHILD 0
#define VMS_NOWAIT_CHILD 1
#define VMS_PARENT_EXIT 2

int
vms_shell_execve(char * command,
		 char **args,
		 char **env,
		 WORD_LIST * words,
		 int async, int *pExecSatus);

extern int vms_fake_fork_level;
extern int vms_fake_exit_seen;

#if defined(MOD_EXECUTE_CMD) || defined(MOD_SUBST)

/* Find out the maximum path */
#include <namdef.h>
#ifdef __VAX
#define VMS_PATH_MAX NAM$C_MAXRSS
#else
#ifdef NAML$C_MAXRSS
#define VMS_PATH_MAX NAML$C_MAXRSS
#else
#define VMS_PATH_MAX NAM$C_MAXRSS
#endif /* NAML$C_MAXRSS */
#endif /* __VAX */

#ifdef __VAX
/* This is needed for VAX to suppress multiple externs from causing build
   problems. */
#pragma message disable dupextern
#endif /* __VAX */


extern int interactive;
extern int interactive_shell;
extern int login_shell;
extern int subshell_level;
extern int subshell_environment;
extern int subshell_skip_commands;
extern int exit_immediately_on_error;
extern int variable_context;
extern int dollar_dollar_pid;
extern int array_needs_making;
extern char **export_env;

typedef struct vms_bash_environ {
    int sanity_count;
    unsigned int subshell_save;

    VAR_CONTEXT * shell_variables;
    VAR_CONTEXT * global_variables;
    HASH_TABLE * shell_functions;

    /* special variables */
    int xtrace_fd;
    int xtrace_fp;
#if defined (READLINE)
#if defined (STRICT_POSIX)
    int rl_screenwidth;
#endif
    char * rl_completer_word_break_characters;
#endif
    int startup_state;
    int funcnest_max;
#if defined (HISTORY)
    int history_control;
    int history_lines_this_session;
    int history_lines_in_file;
    int history_write_timestamps;
#endif
    SHELL_VAR *ifs_var;
    char * ifs_value;
    unsigned char ifs_cmap[UCHAR_MAX+1];
    int eof_encountered;
    int eof_encountered_limit;
    int ignoreeof;
#if defined (READLINE) && defined (STRICT_POSIX)
    int rl_screenheight
#endif
#if defined (HISTORY) && defined (BANG_HISTORY)
    char history_expansion_char;
    char history_subst_char;
    char history_comment_char;
#endif
    void * unwind_protect_list;

    procenv_t top_level;
    procenv_t subshell_top_level;
    procenv_t return_catch;

#if 1
    HASH_TABLE *temporary_env;
#endif
#if 0
    char **export_env;
    int export_env_index;
    int export_env_size;
#endif

    char *dollar_vars[10];
    WORD_LIST *rest_of_args;
    WORD_LIST *subst_assign_varlist;

    char *pwd;
    char *curdir;

    int interactive;
    int interactive_shell;
    int login_shell;
    int subshell_level;
    int subshell_environment;
    int subshell_skip_commands;
    int exit_immediately_on_error;
    int variable_context;
    int dollar_dollar_pid;
    int last_made_pid;
    int return_catch_flag;
    int return_catch_value;
    int last_command_exit_value;
    int last_command_subst_pid;
    int comsub_ignore_return;
    int restricted;
    int restricted_shell;
    int unbound_vars_is_error;
    int vms_fake_fork_level;
    int vms_fake_exit_seen;
    char *this_command_name;

    int fds[3];
    int line_number;

    BASH_INPUT bash_input;
    int bash_input_fd_changed;
    int default_buffered_input;
    COMMAND *global_command;
    mode_t umask;
    HASH_TABLE *hashed_filenames;
    struct VMS_trapnsigstate trapnsig_state;
} VMS_BASH_ENVIRON;

static void
vms_save_procenv (procenv_t save, procenv_t now) {
   memcpy(save, now, sizeof (procenv_t));
}

static void
vms_restore_procenv(procenv_t save, procenv_t now) {
   memcpy(now, save, sizeof (procenv_t));
}

static void
vms_save_std_fds (int fds[3]) {
    int i;

    /* save stdin/out/err */
    for (i = 0; i < 3; i++) {
	if ((fds[i] = dup (i)) < 0) {
	    internal_error ("Cannot duplicate fd %d: %s", i, strerror (errno));
	}
    }
}

static void
vms_restore_std_fds (int fds[3]) {
    int i;

     /* restore stdin/out/err */
    for (i = 0; i < 3; i++) {
	if (fds[i] >= 0) {
	    if (dup2 (fds[i], i) < 0) {
 		internal_error ("cannot duplicate fd %d to fd %d: %s",
 				fds[i], i, strerror (errno));
	    }
	}
	close (fds[i]);
	fds[i] = -1;
    }
}

#define VMS_SAVE_NORMAL (0)
#define VMS_SAVE_SUBSHELL (1)
#define VMS_SAVE_DEALLOCATE (0xdeadbeef)

#define VMS_SAVE_SANITY_VALUE (0xbeef)
#define VMS_SAVE_SANITY_DEALLOCATE (0xfeed)

VMS_BASH_ENVIRON *
vms_save_bash_environ(unsigned int subshell_save);

void
vms_restore_bash_environ(VMS_BASH_ENVIRON * buf);

static int vms_vfork_exec(char * command,
			  char** args,
                          char **env,
		          int asynch,
			  int *status) {
    pid_t real_pid;
    char* cp_ccend=NULL;
    struct stat chkexe_stat;
    int cmd_len;
    char *vms_command = command;
    char *dotptr = strrchr(command, '.');
    unsigned long IsCompiler=0;
    
    /* The OpenVMS CRTL exec* functions favor .exe and then .com files when */
    /* the command string does not have an explicit extension. This can be  */
    /* a problem when the directory in which a command resides has both the */
    /* file with the command name and no extension as well as the file with */
    /* a .com extension. In this case the file with the .com extension will */
    /* be executed by the OpenVMS CRTL instead of the intended file with no */
    /* extension. So, in the cases where the command does not contain an    */
    /* explicit extension, an explicit "." is appended at the end of the    */
    /* command when the file with the .exe extension does not exist. This   */
    /* will prevent the .com file from executing under these circumstances. */
    
    if ((dotptr == NULL) || (dotptr < strrchr(command, '/')))
       {
       cmd_len = strlen(command);
       vms_command = (char *)malloc(cmd_len + 5);
       strcpy(vms_command, command);
       strcat(vms_command, ".exe");
       if (stat(vms_command, &chkexe_stat) != 0)
          {
          vms_command[cmd_len + 1] = '\0';
          }
       else
          {
          free(vms_command);
          vms_command=command;
          }
       }
    *status = 0;
    real_pid = vfork ();
     /* execve() always returns here with success or failure */
    if (real_pid == 0) {
	*status = execve (vms_command, args, env);
	/* DECC$EXIT_AFTER_FAILED_EXEC enabled lets us _exit() on failure */
	_exit (EXIT_FAILURE);
    }

    if (vms_command != command) free(vms_command);

    /* Are we a successful child? */
    if (real_pid > 0) {
	/* Come here on successful execve() */
	switch (asynch) {
	case VMS_WAIT_CHILD:
	    /* Wait for child */
	    waitpid (real_pid, status, 0);
	    return 0;
	    break;
	case VMS_NOWAIT_CHILD:
	    /* Do not wait for child. But, set the real OpenVMS process ID */
	    /* so that the status can be retrieved later when required.    */
	    *status = 0;
	    v_set_pid_list_vms_pid(GetCurrentChildExitPid(), real_pid);
	    return 0;
	    break;
	case VMS_PARENT_EXIT:
	    /* Intends to exit parent when child exits */
	    /* Wait for child */
	    waitpid (real_pid, status, 0);
	    exit (EXIT_SUCCESS);
	    break;
	default:
	    internal_error(
		"Bad asynch value of %d seen - treating as 0 ", asynch);
	    waitpid (real_pid, status, 0);
	    return 0;
	}
    }

    /* We should only get here on a failed exec() as the parent. */
    /* This means that the only return status possible is for a failure. */
    return (-1);
}

#endif

#include <stdlib.h> /* We redefine the usual exit() to account for fake forks */

/* VMS does not actually fork, so do not actually exit. */
#define exit(foo) vms_fake_fork_exit(foo)
