/* File: vfork_subshell.c
 *
 * Copyright 2022, Jake Hamby
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

#include "vfork_subshell.h"

/* Default values for consolidated global state. */
SHELL_STATE bash_state = {
    /* int values first, grouped by module, then alphabetically */

    /* shell.c */
    .current_command_number = 1,
#if defined (BUFFERED_INPUT)
    .default_buffered_input = -1,
#endif
    .indirection_level = 0,
    .shell_start_time = 0,
    .shellstart = { 0 },

    /* subst.c */
    .current_command_subst_pid = NO_PID,
    .ifs_cmap = { 0 },
#if defined (HANDLE_MULTIBYTE)
    .ifs_firstc = { 0 },
    .ifs_firstc_len = 0,
#else
    .ifs_firstc = 0,
#endif
    .last_command_subst_pid = NO_PID,

    /* boolean and char values, grouped by module, then alphabetically */

    /* alias.c */
    .alias_expand_all = 0,

    /* arrayfunc.c */
    .assoc_expand_once = 0,

    /* flags.c */
#if defined (BRACE_EXPANSION)
    .brace_expansion = 1,
#endif
    .disallow_filename_globbing = 0,
    .echo_command_at_execute = 0,
    .echo_input_at_read = 0,
    .errexit_flag = 0,
    .error_trace_mode = 0,
    .exit_immediately_on_error = 0,
    .forced_interactive = 0,
    .function_trace_mode = 0,
    .hashing_enabled = 1,
#if defined (BANG_HISTORY)
    .histexp_flag = 0,
    .history_expansion = HISTEXPAND_DEFAULT,
#endif
    .interactive_comments = 1,
    .just_one_command = 0,
    .mark_modified_vars = 0,
    .no_symbolic_links = 0,
    .noclobber = 0,
    .pipefail_opt = 0,
    .place_keywords_in_env = 0,
    .privileged_mode = 0,
    .read_but_dont_execute = 0,
#if defined (RESTRICTED_SHELL)
    .restricted = 0,
    .restricted_shell = 0,
#endif
    .unbound_vars_is_error = 0,
    .verbose_flag = 0,

    /* shell.c */
    .autocd = 0,
    .bash_argv_initialized = 0,
    .check_jobs_at_exit = 0,
    .debugging_login_shell = 0,
    .debugging_mode = 0,
    .dump_po_strings = 0,
    .dump_translatable_strings = 0,
    .executing = 0,
    .hup_on_exit = 0,
    .interactive = 0,
    .interactive_shell = 0,
    .login_shell = 0,
    .no_line_editing = 0,
    .posixly_correct = 0,
    .pretty_print_mode = 0,
    .protected_mode = 0,
    .read_from_stdin = 0,
    .reading_shell_script = 0,
    .running_under_emacs = 0,
    .shell_initialized = 0,
    .want_pending_command = 0,
    .wordexp_only = 0,

    /* builtins/shift.c */
    .print_shift_error;
    
    /* lib/glob/smatch.c */
    .glob_asciirange;
    
    /* builtins/source.c */
    .source_searches_cwd;
    .source_uses_path;

    /* subst.c */
    .allow_null_glob_expansion;
    .fail_glob_expansion;
    .ifs_is_null;
    .ifs_is_set;
    .inherit_errexit;        

    /* variables.c */
    .array_needs_making;

};
