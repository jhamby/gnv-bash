/* externs.h -- extern function declarations which do not appear in their
   own header file. */

/* Copyright (C) 1993-2020 Free Software Foundation, Inc.

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

/* Make sure that this is included *after* config.h! */

#if !defined (_EXTERNS_H_)
#  define _EXTERNS_H_

#include "stdc.h"

/* Functions from expr.c. */
#define EXP_EXPANDED	0x01

extern intmax_t evalexp (const char *, int, bool *);

/* Functions from print_cmd.c. */
#define FUNC_MULTILINE	0x01
#define FUNC_EXTERNAL	0x02

extern char *make_command_string (COMMAND *);
extern char *named_function_string (const char *, COMMAND *, int);

extern void print_command (COMMAND *);
extern void print_simple_command (SIMPLE_COM *);
extern void print_word_list (WORD_LIST *, const char *);

/* debugger support */
extern void print_for_command_head (FOR_COM *);
#if defined (SELECT_COMMAND)
extern void print_select_command_head (SELECT_COM *);
#endif
extern void print_case_command_head (CASE_COM *);
#if defined (DPAREN_ARITHMETIC)
extern void print_arith_command (WORD_LIST *);
#endif
#if defined (COND_COMMAND)
extern void print_cond_command (COND_COM *);
#endif

/* set -x support */
extern void xtrace_init ();
#ifdef NEED_XTRACE_SET_DECL
extern void xtrace_set (int, FILE *);
#endif
extern void xtrace_fdchk (int);
extern void xtrace_reset ();
extern char *indirection_level_string ();
extern void xtrace_print_assignment (const char *, const char *, bool, int);
extern void xtrace_print_word_list (WORD_LIST *, int);
extern void xtrace_print_for_command_head (FOR_COM *);
#if defined (SELECT_COMMAND)
extern void xtrace_print_select_command_head (SELECT_COM *);
#endif
extern void xtrace_print_case_command_head (CASE_COM *);
#if defined (DPAREN_ARITHMETIC)
extern void xtrace_print_arith_cmd (WORD_LIST *);
#endif
#if defined (COND_COMMAND)
extern void xtrace_print_cond_term (int, int, WORD_DESC *, const char *, const char *);
#endif

/* Functions from shell.c. */
extern void exit_shell (int) __attribute__((__noreturn__));
extern void sh_exit (int) __attribute__((__noreturn__));
extern void subshell_exit (int) __attribute__((__noreturn__));
extern void set_exit_status (int);
extern void disable_priv_mode ();
extern void unbind_args ();

#if defined (RESTRICTED_SHELL)
extern bool shell_is_restricted (const char *);
extern bool maybe_make_restricted (const char *);
#endif

extern void unset_bash_input (int);
extern void get_current_user_info ();

/* Functions from eval.c. */
extern int reader_loop ();
extern int pretty_print_loop ();
extern int parse_command ();
extern int read_command ();

/* Functions from braces.c. */
#if defined (BRACE_EXPANSION)
extern char **brace_expand (char *);
#endif

/* Miscellaneous functions from parse.y */
extern int yyparse ();
extern int return_EOF ();
extern void push_token (int);
extern char *xparse_dolparen (const char *, char *, int *, int);
extern void reset_parser ();
extern void reset_readahead_token ();
extern WORD_LIST *parse_string_to_word_list (char *, int, const char *);

extern int parser_will_prompt ();
extern int parser_in_command_position ();

extern void free_pushed_string_input ();

extern int parser_expanding_alias ();
extern void parser_save_alias ();
extern void parser_restore_alias ();

extern void clear_shell_input_line ();

extern char *decode_prompt_string (const char *);

extern int get_current_prompt_level ();
extern int set_current_prompt_level (int);

#if defined (HISTORY)
extern const char *history_delimiting_chars (const char *);
#endif

/* Declarations for functions defined in locale.c */
extern void set_default_locale ();
extern void set_default_locale_vars ();
extern bool set_locale_var (const char *, const char *);
extern int set_lang (const char *, const char *);
extern void set_default_lang ();
extern const char *get_locale_var (const char *);
extern char *localetrans (const char *, int, int *);
extern char *mk_msgstr (char *, bool *);
extern char *localeexpand (const char *, int, int, int, int *);
#ifndef locale_decpoint
extern int locale_decpoint ();
#endif

/* Declarations for functions defined in list.c. */
#ifdef INCLUDE_UNUSED
extern void list_walk (GENERIC_LIST *, sh_glist_func_t *);
extern void wlist_walk (WORD_LIST *, sh_icpfunc_t *);
#endif
extern GENERIC_LIST *list_reverse (GENERIC_LIST *list);
extern int list_length (GENERIC_LIST *list);
extern GENERIC_LIST *list_append (GENERIC_LIST *head, GENERIC_LIST *tail);
#ifdef INCLUDE_UNUSED
extern GENERIC_LIST *list_remove (GENERIC_LIST **list, Function *comparer, char *arg);
#endif

/* Declarations for functions defined in stringlib.c */
extern int find_string_in_alist (char *, STRING_INT_ALIST *, int);
extern char *find_token_in_alist (int, STRING_INT_ALIST *, int);
extern int find_index_in_alist (char *, STRING_INT_ALIST *, int);

extern char *substring (const char *, int, int);
extern char *strsub (char *, char *, char *, int);
extern char *strcreplace (const char *, int, const char *, int);
extern void strip_leading (char *);
extern void strip_trailing (char *, int, int);
extern void xbcopy (const char *, char *, int);

/* Functions from version.c. */
extern char *shell_version_string ();
extern void show_shell_version (int);

/* Functions from the bash library, lib/sh/libsh.a.  These should really
   go into a separate include file. */

/* declarations for functions defined in lib/sh/casemod.c */
extern char *sh_modcase (const char *, const char *, int);

/* Defines for flags argument to sh_modcase.  These need to agree with what's
   in lib/sh/casemode.c */
#define CASE_LOWER	0x0001
#define CASE_UPPER	0x0002
#define CASE_CAPITALIZE	0x0004
#define CASE_UNCAP	0x0008
#define CASE_TOGGLE	0x0010
#define CASE_TOGGLEALL	0x0020
#define CASE_UPFIRST	0x0040
#define CASE_LOWFIRST	0x0080

#define CASE_USEWORDS	0x1000

/* declarations for functions defined in lib/sh/clktck.c */
extern long get_clk_tck ();

/* declarations for functions defined in lib/sh/clock.c */
extern void clock_t_to_secs ();
extern void print_clock_t ();

/* Declarations for functions defined in lib/sh/dprintf.c */
#if !defined (HAVE_DPRINTF)
extern void dprintf (int, const char *, ...)  __attribute__((__format__ (printf, 2, 3)));
#endif

/* Declarations for functions defined in lib/sh/fmtulong.c */
#define FL_PREFIX     0x01    /* add 0x, 0X, or 0 prefix as appropriate */
#define FL_ADDBASE    0x02    /* add base# prefix to converted value */
#define FL_HEXUPPER   0x04    /* use uppercase when converting to hex */
#define FL_UNSIGNED   0x08    /* don't add any sign */

extern char *fmtulong (unsigned long int, int, char *, size_t, int);

/* Declarations for functions defined in lib/sh/fmtulong.c */
#if defined (HAVE_LONG_LONG)
extern char *fmtullong (unsigned long long int, int, char *, size_t, int);
#endif

/* Declarations for functions defined in lib/sh/fmtumax.c */
extern char *fmtumax (uintmax_t, int, char *, size_t, int);

/* Declarations for functions defined in lib/sh/fnxform.c */
extern char *fnx_fromfs (char *, size_t);
extern char *fnx_tofs (char *, size_t);

/* Declarations for functions defined in lib/sh/fpurge.c */

#if defined NEED_FPURGE_DECL
#if !HAVE_DECL_FPURGE

#if HAVE_FPURGE
#  define fpurge _bash_fpurge
#endif
extern int fpurge (FILE *stream);

#endif /* HAVE_DECL_FPURGE */
#endif /* NEED_FPURGE_DECL */

/* Declarations for functions defined in lib/sh/getcwd.c */
#if !defined (HAVE_GETCWD)
extern char *getcwd (char *, size_t);
#endif

/* Declarations for functions defined in lib/sh/input_avail.c */
extern int input_avail (int);

/* Declarations for functions defined in lib/sh/itos.c */
extern char *inttostr (intmax_t, char *, size_t);
extern char *itos (intmax_t);
extern char *mitos (intmax_t);
extern char *uinttostr (uintmax_t, char *, size_t);
extern char *uitos (uintmax_t);

/* declarations for functions defined in lib/sh/makepath.c */
#define MP_DOTILDE	0x01
#define MP_DOCWD	0x02
#define MP_RMDOT	0x04
#define MP_IGNDOT	0x08

extern char *sh_makepath (const char *, const char *, int);

/* declarations for functions defined in lib/sh/mbscasecmp.c */
#if !defined (HAVE_MBSCASECMP)
extern char *mbscasecmp (const char *, const char *);
#endif

/* declarations for functions defined in lib/sh/mbschr.c */
#if !defined (HAVE_MBSCHR)
extern char *mbschr (const char *, int);
#endif

/* declarations for functions defined in lib/sh/mbscmp.c */
#if !defined (HAVE_MBSCMP)
extern char *mbscmp (const char *, const char *);
#endif

/* declarations for functions defined in lib/sh/netconn.c */
extern bool isnetconn (int);

/* declarations for functions defined in lib/sh/netopen.c */
extern int netopen (const char *);

/* Declarations for  functions defined in lib/sh/oslib.c */

#if !defined (HAVE_DUP2) || defined (DUP2_BROKEN)
extern int dup2 (int, int);
#endif

#if !defined (HAVE_GETDTABLESIZE)
extern int getdtablesize ();
#endif /* !HAVE_GETDTABLESIZE */

#if !defined (HAVE_GETHOSTNAME)
extern int gethostname (char *, int);
#endif /* !HAVE_GETHOSTNAME */

extern int getmaxgroups ();
extern long getmaxchild ();

/* declarations for functions defined in lib/sh/pathcanon.c */
#define PATH_CHECKDOTDOT	0x0001
#define PATH_CHECKEXISTS	0x0002
#define PATH_HARDPATH		0x0004
#define PATH_NOALLOC		0x0008

extern char *sh_canonpath (const char *, int);

/* declarations for functions defined in lib/sh/pathphys.c */
extern char *sh_physpath (const char *, int);
extern char *sh_realpath (const char *, char *);

/* declarations for functions defined in lib/sh/random.c */
extern int brand ();
extern void sbrand (unsigned long);		/* set bash random number generator. */
extern void seedrand ();			/* seed generator randomly */
extern void seedrand32 ();
extern u_bits32_t get_urandom32 ();

/* declarations for functions defined in lib/sh/setlinebuf.c */
#ifdef NEED_SH_SETLINEBUF_DECL
extern int sh_setlinebuf (FILE *);
#endif

/* declarations for functions defined in lib/sh/shaccess.c */
extern int sh_eaccess (const char *, int);

/* declarations for functions defined in lib/sh/shmatch.c */
extern int sh_regmatch (const char *, const char *, int);

/* defines for flags argument to sh_regmatch. */
#define SHMAT_SUBEXP		0x001	/* save subexpressions in SH_REMATCH */
#define SHMAT_PWARN		0x002	/* print a warning message on invalid regexp */

/* declarations for functions defined in lib/sh/shmbchar.c */
extern size_t mbstrlen (const char *);
extern char *mbsmbchar (const char *);
extern int sh_mbsnlen (const char *, size_t, int);

/* declarations for functions defined in lib/sh/shquote.c */
extern char *sh_single_quote (const char *);
extern char *sh_double_quote (const char *);
extern char *sh_mkdoublequoted (const char *, int, int);
extern char *sh_un_double_quote (const char *);
extern char *sh_backslash_quote (const char *, const char *, int);
extern char *sh_backslash_quote_for_double_quotes (const char *);
extern char *sh_quote_reusable (const char *, int);
extern int sh_contains_shell_metas (const char *);
extern int sh_contains_quotes (const char *);

/* declarations for functions defined in lib/sh/spell.c */
extern int spname (const char *, char *);
extern char *dirspell (const char *);

/* declarations for functions defined in lib/sh/strcasecmp.c */
#if !defined (HAVE_STRCASECMP)
extern int strncasecmp (const char *, const char *, size_t);
extern int strcasecmp (const char *, const char *);
#endif /* HAVE_STRCASECMP */

/* declarations for functions defined in lib/sh/strcasestr.c */
#if ! HAVE_STRCASESTR
extern char *strcasestr (const char *, const char *);
#endif

/* declarations for functions defined in lib/sh/strchrnul.c */
#if ! HAVE_STRCHRNUL
extern char *strchrnul (const char *, int);
#endif

/* declarations for functions defined in lib/sh/strerror.c */
#if !defined (HAVE_STRERROR) && !defined (strerror)
extern char *strerror (int);
#endif

/* declarations for functions defined in lib/sh/strftime.c */
#if !defined (HAVE_STRFTIME) && defined (NEED_STRFTIME_DECL)
extern size_t strftime (char *, size_t, const char *, const struct tm *);
#endif

/* declarations for functions and structures defined in lib/sh/stringlist.c */

/* This is a general-purpose argv-style array struct. */
typedef struct _list_of_strings {
  char **list;
  int list_size;
  int list_len;
} STRINGLIST;

typedef int sh_strlist_map_func_t (char *);

extern STRINGLIST *strlist_create (int);
extern STRINGLIST *strlist_resize (STRINGLIST *, int);
extern void strlist_flush (STRINGLIST *);
extern void strlist_dispose (STRINGLIST *);
extern int strlist_remove (STRINGLIST *, char *);
extern STRINGLIST *strlist_copy (STRINGLIST *);
extern STRINGLIST *strlist_merge (STRINGLIST *, STRINGLIST *);
extern STRINGLIST *strlist_append (STRINGLIST *, STRINGLIST *);
extern STRINGLIST *strlist_prefix_suffix (STRINGLIST *, char *, char *);
extern void strlist_print (STRINGLIST *, char *);
extern void strlist_walk (STRINGLIST *, sh_strlist_map_func_t *);
extern void strlist_sort (STRINGLIST *);

/* declarations for functions defined in lib/sh/stringvec.c */

extern char **strvec_create (int);
extern char **strvec_resize (char **, int);
extern char **strvec_mcreate (int);
extern char **strvec_mresize (char **, int);
extern void strvec_flush (char **);
extern void strvec_dispose (char **);
extern int strvec_remove (char **, char *);
extern size_t strvec_len (char **);
extern int strvec_search (char **, char *);
extern char **strvec_copy (char **);
extern int strvec_posixcmp (char **, char **);
extern int strvec_strcmp (char **, char **);
extern void strvec_sort (char **, int);

extern char **strvec_from_word_list (WORD_LIST *, int, int, int *);
extern WORD_LIST *strvec_to_word_list (char **, int, int);

/* declarations for functions defined in lib/sh/strnlen.c */
#if !defined (HAVE_STRNLEN)
extern size_t strnlen (const char *, size_t);
#endif

/* declarations for functions defined in lib/sh/strpbrk.c */
#if !defined (HAVE_STRPBRK)
extern char *strpbrk (const char *, const char *);
#endif

/* declarations for functions defined in lib/sh/strtod.c */
#if !defined (HAVE_STRTOD)
extern double strtod (const char *, char **);
#endif

/* declarations for functions defined in lib/sh/strtol.c */
#if !HAVE_DECL_STRTOL
extern long strtol (const char *, char **, int);
#endif

/* declarations for functions defined in lib/sh/strtoll.c */
#if defined (HAVE_LONG_LONG) && !HAVE_DECL_STRTOLL
extern long long strtoll (const char *, char **, int);
#endif

/* declarations for functions defined in lib/sh/strtoul.c */
#if !HAVE_DECL_STRTOUL
extern unsigned long strtoul (const char *, char **, int);
#endif

/* declarations for functions defined in lib/sh/strtoull.c */
#if defined (HAVE_LONG_LONG) && !HAVE_DECL_STRTOULL
extern unsigned long long strtoull (const char *, char **, int);
#endif

/* declarations for functions defined in lib/sh/strimax.c */
#if !HAVE_DECL_STRTOIMAX
extern intmax_t strtoimax (const char *, char **, int);
#endif

/* declarations for functions defined in lib/sh/strumax.c */
#if !HAVE_DECL_STRTOUMAX
extern uintmax_t strtoumax (const char *, char **, int);
#endif

/* declarations for functions defined in lib/sh/strtrans.c */
extern char *ansicstr (const char *, int, int, int *, int *);
extern char *ansic_quote (const char *, int, int *);
extern bool ansic_shouldquote (const char *);
extern char *ansiexpand (const char *, int, int, int *);

/* declarations for functions defined in lib/sh/timeval.c */
extern void timeval_to_secs (struct timeval *tvp, time_t *sp, int *sfp);
extern void print_timeval (FILE *fp, struct timeval *tvp);

/* declarations for functions defined in lib/sh/tmpfile.c */
#define MT_USETMPDIR		0x0001
#define MT_READWRITE		0x0002
#define MT_USERANDOM		0x0004
#define MT_TEMPLATE		0x0008

extern char *sh_mktmpname (char *, int);
extern int sh_mktmpfd (const char *, int, char **);
/* extern FILE *sh_mktmpfp (const char *, int, char **); */
extern char *sh_mktmpdir (char *, int);

/* declarations for functions defined in lib/sh/uconvert.c */
extern int uconvert (char *, long *, long *, char **);

/* declarations for functions defined in lib/sh/ufuncs.c */
extern unsigned int falarm (unsigned int, unsigned int);
extern unsigned int fsleep (unsigned int, unsigned int);

/* declarations for functions defined in lib/sh/unicode.c */
extern int u32cconv (unsigned long, char *);
extern void u32reset ();

/* declarations for functions defined in lib/sh/utf8.c */
extern char *utf8_mbschr (const char *, int);
extern int utf8_mbscmp (const char *, const char *);
extern char *utf8_mbsmbchar (const char *);
extern int utf8_mbsnlen (const char *, size_t, int);
extern int utf8_mblen (const char *, size_t);
extern size_t utf8_mbstrlen (const char *);

/* declarations for functions defined in lib/sh/wcsnwidth.c */
#if defined (HANDLE_MULTIBYTE)
extern int wcsnwidth (const wchar_t *, size_t, size_t);
#endif

/* declarations for functions defined in lib/sh/winsize.c */
extern void get_new_window_size (int, int *, int *);

/* declarations for functions defined in lib/sh/zcatfd.c */
extern int zcatfd (int, int, const char *);

/* declarations for functions defined in lib/sh/zgetline.c */
extern ssize_t zgetline (int, char **, size_t *, int, bool);

/* declarations for functions defined in lib/sh/zmapfd.c */
extern int zmapfd (int, char **, const char *);

/* declarations for functions defined in lib/sh/zread.c */
extern ssize_t zread (int, char *, size_t);
extern ssize_t zreadretry (int, char *, size_t);
extern ssize_t zreadintr (int, char *, size_t);
extern ssize_t zreadc (int, char *);
extern ssize_t zreadcintr (int, char *);
extern ssize_t zreadn (int, char *, size_t);
extern void zreset ();
extern void zsyncfd (int);

/* declarations for functions defined in lib/sh/zwrite.c */
extern int zwrite (int, char *, size_t);

/* declarations for functions defined in lib/glob/gmisc.c */
extern bool match_pattern_char (const char *, const char *, int);
extern int umatchlen (const char *, size_t);

#if defined (HANDLE_MULTIBYTE)
extern bool match_pattern_wchar (const wchar_t *, const wchar_t *, int);
extern int wmatchlen (const wchar_t *, size_t);
#endif

#endif /* _EXTERNS_H_ */
