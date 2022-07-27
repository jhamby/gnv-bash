/* execute_cmd.h - functions from execute_cmd.c. */

/* Copyright (C) 1993-2017 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

   Bash is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bash is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash.  If not, see <http://www.gnu.org/licenses/>.
*/

#if !defined (_EXECUTE_CMD_H_)
#define _EXECUTE_CMD_H_

#include "stdc.h"

#if defined (ARRAY_VARS)
struct func_array_state
  {
    ARRAY *funcname_a;
    SHELL_VAR *funcname_v;
    ARRAY *source_a;
    SHELL_VAR *source_v;
    ARRAY *lineno_a;
    SHELL_VAR *lineno_v;
  };
#endif

/* Placeholder for later expansion to include more execution state */
/* XXX - watch out for pid_t */
struct execstate
  {
    pid_t pid;
    int subshell_env;
  };


/* Variables declared in execute_cmd.c, used by many other files */
extern int return_catch_flag;
extern int return_catch_value;
extern volatile int last_command_exit_value;
extern int last_command_exit_signal;
extern bool builtin_ignoring_errexit;
extern int executing_builtin;
extern int executing_list;		/* list nesting level */
extern int comsub_ignore_return;	/* can be greater than 1 */
extern int subshell_level;
extern bool match_ignore_case;
extern bool executing_command_builtin;
extern int funcnest, funcnest_max;
extern int evalnest, evalnest_max;
extern int sourcenest, sourcenest_max;
extern bool stdin_redir;
extern int line_number_for_err_trap;

extern char *the_printed_command_except_trap;

extern const char *this_command_name;
extern SHELL_VAR *this_shell_function;

/* Functions declared in execute_cmd.c, used by many other files */

extern struct fd_bitmap *new_fd_bitmap (int);
extern void dispose_fd_bitmap (void *);
extern void close_fd_bitmap (void *);
extern int executing_line_number ();
extern int execute_command (COMMAND *);
extern int execute_command_internal (COMMAND *, bool, int, int, struct fd_bitmap *);
extern int shell_execve (char *, char **, char **);
extern void setup_async_signals ();
extern void async_redirect_stdin ();

extern void undo_partial_redirects ();
extern void dispose_partial_redirects ();
extern void dispose_exec_redirects ();

extern int execute_shell_function (SHELL_VAR *, WORD_LIST *);

extern struct coproc *getcoprocbypid (pid_t);
extern struct coproc *getcoprocbyname (const char *);

extern void coproc_init (struct coproc *);
extern struct coproc *coproc_alloc (char *, pid_t);
extern void coproc_dispose (struct coproc *);
extern void coproc_flush ();
extern void coproc_close (struct coproc *);
extern void coproc_closeall ();
extern void coproc_reap ();
extern pid_t coproc_active ();

extern void coproc_rclose (struct coproc *, int);
extern void coproc_wclose (struct coproc *, int);
extern void coproc_fdclose (struct coproc *, int);

extern void coproc_checkfd (struct coproc *, int);
extern void coproc_fdchk (int);

extern void coproc_pidchk (pid_t, int);

extern void coproc_fdsave (struct coproc *);
extern void coproc_fdrestore (struct coproc *);

extern void coproc_setvars (struct coproc *);
extern void coproc_unsetvars (struct coproc *);

#if defined (PROCESS_SUBSTITUTION)
extern void close_all_files ();
#endif

#if defined (ARRAY_VARS)
extern void restore_funcarray_state (void *);
#endif

#endif /* _EXECUTE_CMD_H_ */
