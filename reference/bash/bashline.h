/* bashline.h -- interface to the bash readline functions in bashline.c. */

/* Copyright (C) 1993-2019 Free Software Foundation, Inc.

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

#if !defined (_BASHLINE_H_)
#define _BASHLINE_H_

#include "stdc.h"

extern bool bash_readline_initialized;
extern bool hostname_list_initialized;

/* these are controlled via shopt */
extern char perform_hostname_completion;
extern char no_empty_command_completion;
extern char force_fignore;
extern char dircomplete_spelling;
extern char dircomplete_expand;
extern char dircomplete_expand_relpath;
extern char complete_fullquote;

extern void posix_readline_initialize (int);
extern void reset_completer_word_break_chars (void);
extern int enable_hostname_completion (int);
extern void initialize_readline (void);
extern void bashline_reset (void);
extern void bashline_reinitialize (void);
extern int bash_re_edit (const char *);

extern void bashline_set_event_hook (void);
extern void bashline_reset_event_hook (void);

extern int bind_keyseq_to_unix_command (const char *);
extern int bash_execute_unix_command (int, int);
extern int print_unix_command_map (void);
extern int unbind_unix_command (const char *);

extern char **bash_default_completion (const char *, int, int, int, int);

void set_directory_hook (void);

/* Used by programmable completion code. */
extern char *command_word_completion_function (const char *, int);
extern char *bash_groupname_completion_function (const char *, int);
extern char *bash_servicename_completion_function (const char *, int);

extern char **get_hostname_list (void);
extern void clear_hostname_list (void);

extern char **bash_directory_completion_matches (const char *);
extern char *bash_dequote_text (const char *);

#endif /* _BASHLINE_H_ */
