/* File: vfork_subshell.h
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

/*
 * This file defines functions and a data structure for transferring the
 * shell's state from a parent to a subshell using a pipe passed to the
 * child as a file descriptor, passed as a private command-line argument.
 *
 * This is loosely based on vms_fakefork.h, except subshells are executed as
 * subprocesses, for better POSIX compatibility, concurrency, and isolation.
 */

#if !defined (_VFORK_SUBSHELL_H)
#define _VFORK_SUBSHELL_H

/* Consolidate pass-by-value globals into one struct for easier copying. */
typedef struct shell_state {
    /* int values first, grouped by module, then alphabetically */

    /* shell.c */
    int current_command_number;
#if defined (BUFFERED_INPUT)
    int default_buffered_input;
#endif
    int indirection_level;
    time_t shell_start_time;
    struct timeval shellstart;

    /* subst.c */
    pid_t current_command_subst_pid;
    unsigned char ifs_cmap[UCHAR_MAX + 1];
#if defined (HANDLE_MULTIBYTE)
    unsigned char ifs_firstc[MB_LEN_MAX];
    size_t ifs_firstc_len;
#else
    unsigned char ifs_firstc;
#endif
    pid_t last_command_subst_pid;

    /* boolean and char values next, grouped by module, then alphabetically */

    /* alias.c */
    char alias_expand_all;

    /* arrayfunc.c */
    char assoc_expand_once;

    /* bashhist.c */

    /* flags.c */
#if defined (BRACE_EXPANSION)
    char brace_expansion;
#endif
    char disallow_filename_globbing;
    char echo_command_at_execute;
    char echo_input_at_read;
    char errexit_flag;
    char error_trace_mode;
    char exit_immediately_on_error;
    char forced_interactive;
    char function_trace_mode;
    char hashing_enabled;
#if defined (BANG_HISTORY)
    char histexp_flag;
    char history_expansion;
#endif
    char interactive_comments;
    char just_one_command;
    char mark_modified_vars;
    char no_symbolic_links;
    char noclobber;
    char pipefail_opt;
    char place_keywords_in_env;
    char privileged_mode;
    char read_but_dont_execute;
#if defined (RESTRICTED_SHELL)
    char restricted;
    char restricted_shell;
#endif
    char unbound_vars_is_error;
    char verbose_flag;

    /* shell.c */
    char autocd;
    char bash_argv_initialized;
    char check_jobs_at_exit;
    char debugging_login_shell;
    char debugging_mode;
    char dump_po_strings;
    char dump_translatable_strings;
    char executing;
    char hup_on_exit;
    char interactive;
    char interactive_shell;
    char login_shell;
    char no_line_editing;
    char posixly_correct;
    char pretty_print_mode;
    char protected_mode;
    char read_from_stdin;
    char reading_shell_script;
    char running_under_emacs;
    char shell_initialized;
    char want_pending_command;
    char wordexp_only;

    /* builtins/shift.c */
    char print_shift_error;
    
    /* lib/glob/smatch.c */
    char glob_asciirange;
    
    /* builtins/source.c */
    char source_searches_cwd;
    char source_uses_path;

    /* subst.c */
    char allow_null_glob_expansion;
    char fail_glob_expansion;
    char ifs_is_null;
    char ifs_is_set;
    char inherit_errexit;        

    /* variables.c */
    char array_needs_making;

    /* finally, the sizes of the serialized global data to receive. */
    size_t global_variable_bytes;	/* VAR_CONTEXT */
    size_t shell_variable_bytes;	/* VAR_CONTEXT */

    size_t shell_function_bytes;	/* HASH_TABLE */
    size_t temporary_env_bytes;		/* HASH_TABLE */

} SHELL_STATE;

/* Global state that we have to serialize before sending to subshells. */
typedef struct shell_state_ptr {

    /* parse.y */

    /* shell.c */

    /* variables.c */
    HASH_TABLE *shell_functions;
    HASH_TABLE *temporary_env;
    VAR_CONTEXT *global_variables;
    VAR_CONTEXT *shell_variables;

} SHELL_STATE_PTR;

/* Consolidated state to send to subshells via mailbox write. */
extern SHELL_STATE bash_state;

/* Pointers to state to serialize and send after the fixed-size state. */
extern SHELL_STATE_PTR bash_state_ptr;

/* alias.c global redirections */
#define alias_expand_all	    (bash_state.alias_expand_all)

/* arrayfunc.c global redirections */
#define assoc_expand_once	    (bash_state.assoc_expand_once)

/* flags.c global redirections */
#define brace_expansion		    (bash_state.brace_expansion)
#define disallow_filename_globbing  (bash_state.disallow_filename_globbing)
#define echo_command_at_execute	    (bash_state.echo_command_at_execute)
#define echo_input_at_read	    (bash_state.echo_input_at_read)
#define errexit_flag		    (bash_state.errexit_flag)
#define error_trace_mode	    (bash_state.error_trace_mode)
#define exit_immediately_on_error   (bash_state.exit_immediately_on_error)
#define forced_interactive	    (bash_state.forced_interactive)
#define function_trace_mode	    (bash_state.function_trace_mode)
#define hashing_enabled		    (bash_state.hashing_enabled)
#define histexp_flag		    (bash_state.histexp_flag)
#define history_expansion	    (bash_state.history_expansion)
#define interactive_comments	    (bash_state.interactive_comments)
#define just_one_command	    (bash_state.just_one_command)
#define mark_modified_vars	    (bash_state.mark_modified_vars)
#define noclobber		    (bash_state.noclobber)
#define no_symbolic_links	    (bash_state.no_symbolic_links)
#define pipefail_opt		    (bash_state.pipefail_opt)
#define place_keywords_in_env	    (bash_state.place_keywords_in_env)
#define privileged_mode		    (bash_state.privileged_mode)
#define read_but_dont_execute	    (bash_state.read_but_dont_execute)
#define restricted		    (bash_state.restricted)
#define restricted_shell	    (bash_state.restricted_shell)
#define unbound_vars_is_error	    (bash_state.unbound_vars_is_error)
#define verbose_flag		    (bash_state.verbose_flag)

/* shell.c global redirections */
#define autocd			    (bash_state.autocd)
#define bash_argv_initialized	    (bash_state.bash_argv_initialized)
#define check_jobs_at_exit	    (bash_state.check_jobs_at_exit)
#define current_command_number	    (bash_state.current_command_number)
#define debugging_login_shell	    (bash_state.debugging_login_shell)
#define debugging_mode		    (bash_state.debugging_mode)
#define default_buffered_input	    (bash_state.default_buffered_input)
#define dump_po_strings		    (bash_state.dump_po_strings)
#define dump_translatable_strings   (bash_state.dump_translatable_strings)
#define executing		    (bash_state.executing)
#define hup_on_exit		    (bash_state.hup_on_exit)
#define indirection_level	    (bash_state.indirection_level)
#define interactive		    (bash_state.interactive)
#define interactive_shell	    (bash_state.interactive_shell)
#define login_shell		    (bash_state.login_shell)
#define no_line_editing		    (bash_state.no_line_editing)
#define posixly_correct		    (bash_state.posixly_correct)
#define pretty_print_mode	    (bash_state.pretty_print_mode)
#define protected_mode		    (bash_state.protected_mode)
#define read_from_stdin		    (bash_state.read_from_stdin)
#define reading_shell_script	    (bash_state.reading_shell_script)
#define running_under_emacs	    (bash_state.running_under_emacs)
#define shellstart		    (bash_state.shellstart)
#define shell_initialized	    (bash_state.shell_initialized)
#define shell_start_time	    (bash_state.shell_start_time)
#define want_pending_command	    (bash_state.want_pending_command)
#define wordexp_only		    (bash_state.wordexp_only)

/* builtins/shift.c */
#define print_shift_error	    (bash_state.print_shift_error)
 
/* lib/glob/smatch.c */
#define glob_asciirange		    (bash_state.glob_asciirange)

/* builtins/source.c */
#define source_searches_cwd	    (bash_state.source_searches_cwd)
#define source_uses_path	    (bash_state.source_uses_path)
 
/* subst.c */
#define allow_null_glob_expansion   (bash_state.allow_null_glob_expansion)
#define current_command_subst_pid   (bash_state.current_command_subst_pid)
#define fail_glob_expansion	    (bash_state.fail_glob_expansion)
#define ifs_cmap		    (bash_state.ifs_cmap)
#define ifs_firstc		    (bash_state.ifs_firstc)
#define ifs_firstc_len		    (bash_state.ifs_firstc_len)
#define ifs_is_null		    (bash_state.ifs_is_null)
#define ifs_is_set		    (bash_state.ifs_is_set)
#define inherit_errexit		    (bash_state.inherit_errexit)
#define last_command_subst_pid	    (bash_state.last_command_subst_pid)

/* variables.c */
#define array_needs_making	    (bash_state.array_needs_making)
#define global_variable_bytes	    (bash_state_ptr.global_variable_bytes)
#define shell_function_bytes	    (bash_state_ptr.shell_function_bytes)
#define shell_variable_bytes	    (bash_state_ptr.shell_variable_bytes)
#define temporary_env_bytes	    (bash_state_ptr.temporary_env_bytes)

#endif /* _VFORK_SUBSHELL_H */
