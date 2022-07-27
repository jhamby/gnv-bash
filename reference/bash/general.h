/* general.h -- defines that everybody likes to use. */

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

#if !defined (_GENERAL_H_)
#define _GENERAL_H_

#include "stdc.h"

#include "bashtypes.h"
#include "chartypes.h"

#if defined (HAVE_SYS_RESOURCE_H) && defined (RLIMTYPE)
#  if defined (HAVE_SYS_TIME_H)
#    include <sys/time.h>
#  endif
#  include <sys/resource.h>
#endif

#if defined (HAVE_STRING_H)
#  include <string.h>
#else
#  include <strings.h>
#endif /* !HAVE_STRING_H */

#if defined (HAVE_LIMITS_H)
#  include <limits.h>
#endif

#include "xmalloc.h"

/* NULL pointer type. */
#if !defined (NULL)
#  if defined (__STDC__)
#    define NULL ((void *) 0)
#  else
#    define NULL 0x0
#  endif /* !__STDC__ */
#endif /* !NULL */

/* Hardly used anymore */
#define pointer_to_int(x)	(int)((char *)x - (char *)0)

#if !defined (strcpy) && (defined (HAVE_DECL_STRCPY) && !HAVE_DECL_STRCPY)
extern char *strcpy (char *, const char *);
#endif

#if !defined (savestring)
#  define savestring(x) (char *)strcpy ((char *)xmalloc (1 + strlen (x)), (x))
#endif

#ifndef member
#  define member(c, s) ((c) ? ((char *)mbschr ((s), (c)) != (char *)NULL) : 0)
#endif

#ifndef whitespace
#define whitespace(c) (((c) == ' ') || ((c) == '\t'))
#endif

#ifndef CHAR_MAX
#  ifdef __CHAR_UNSIGNED__
#    define CHAR_MAX	0xff
#  else
#    define CHAR_MAX	0x7f
#  endif
#endif

#ifndef CHAR_BIT
#  define CHAR_BIT 8
#endif

/* Nonzero if the integer type T is signed.  */
#define TYPE_SIGNED(t) (! ((t) 0 < (t) -1))

/* The width in bits of the integer type or expression T.
   Padding bits are not supported; this is checked at compile-time below.  */
#define TYPE_WIDTH(t) (sizeof (t) * CHAR_BIT)

/* Bound on length of the string representing an unsigned integer
   value representable in B bits.  log10 (2.0) < 146/485.  The
      smallest value of B where this bound is not tight is 2621.  */
#define INT_BITS_STRLEN_BOUND(b) (((b) * 146 + 484) / 485)

/* Bound on length of the string representing an integer value of type T.
   Subtract one for the sign bit if T is signed;
   302 / 1000 is log10 (2) rounded up;
   add one for integer division truncation;
   add one more for a minus sign if t is signed.  */
#define INT_STRLEN_BOUND(t) \
  ((TYPE_WIDTH (t) - TYPE_SIGNED (t)) * 302 / 1000 \
   + 1 + TYPE_SIGNED (t))

/* Updated version adapted from gnulib/intprops.h, not used right now.
   Changes the approximation of log10(2) from 302/1000 to 146/485. */
#if 0
#define INT_STRLEN_BOUND(t) \
  (INT_BITS_STRLEN_BOUND (TYPE_WIDTH (t) - TYPE_SIGNED (t)) + TYPE_SIGNED(t))
#endif

/* Bound on buffer size needed to represent an integer type or expression T,
   including the terminating null.  */
#define INT_BUFSIZE_BOUND(t) (INT_STRLEN_BOUND (t) + 1)

/* Define exactly what a legal shell identifier consists of. */
#define legal_variable_starter(c) (ISALPHA(c) || (c == '_'))
#define legal_variable_char(c)	(ISALNUM(c) || c == '_')

/* Definitions used in subst.c and by the `read' builtin for field
   splitting. */
#define spctabnl(c)	((c) == ' ' || (c) == '\t' || (c) == '\n')

/* Here is a generic structure for associating character strings
   with integers.  It is used in the parser for shell tokenization. */
typedef struct {
  const char *word;
  int token;
} STRING_INT_ALIST;

/* A macro to avoid making an unnecessary function call. */
#define REVERSE_LIST(list, type) \
  ((list && list->next) ? (type)list_reverse ((GENERIC_LIST *)list) \
			: (type)(list))

#if __GNUC__ > 1
#  define FASTCOPY(s, d, n)  __builtin_memcpy ((d), (s), (n))
#else /* !__GNUC__ */
#  if !defined (HAVE_BCOPY)
#    if !defined (HAVE_MEMMOVE)
#      define FASTCOPY(s, d, n)  memcpy ((d), (s), (n))
#    else
#      define FASTCOPY(s, d, n)  memmove ((d), (s), (n))
#    endif /* !HAVE_MEMMOVE */
#  else /* HAVE_BCOPY */
#    define FASTCOPY(s, d, n)  bcopy ((s), (d), (n))
#  endif /* HAVE_BCOPY */
#endif /* !__GNUC__ */

/* String comparisons that possibly save a function call each. */
#define STREQ(a, b) ((a)[0] == (b)[0] && strcmp(a, b) == 0)
#define STREQN(a, b, n) ((n == 0) ? (1) \
				  : ((a)[0] == (b)[0] && strncmp(a, b, n) == 0))

/* More convenience definitions that possibly save system or libc calls. */
#define STRLEN(s) (((s) && (s)[0]) ? ((s)[1] ? ((s)[2] ? strlen(s) : 2) : 1) : 0)
#define FREE(s)  do { if (s) free (s); } while (0)
#define MEMBER(c, s) (((c) && c == (s)[0] && !(s)[1]) || (member(c, s)))

/* A fairly hairy macro to check whether an allocated string has more room,
   and to resize it using xrealloc if it does not.
   STR is the string (char *)
   CIND is the current index into the string (int)
   ROOM is the amount of additional room we need in the string (int)
   CSIZE is the currently-allocated size of STR (int)
   SINCR is how much to increment CSIZE before calling xrealloc (int) */

#define RESIZE_MALLOCED_BUFFER(str, cind, room, csize, sincr) \
  do { \
    if ((cind) + (room) >= csize) \
      { \
	while ((cind) + (room) >= csize) \
	  csize += (sincr); \
	str = (char *) xrealloc (str, csize); \
      } \
  } while (0)

/* Function pointers can be declared as (Function *)foo. */
#if !defined (_FUNCTION_DEF)
#  define _FUNCTION_DEF
typedef int Function ();
typedef void VFunction ();
typedef char *CPFunction ();		/* no longer used */
typedef char **CPPFunction ();		/* no longer used */
#endif /* _FUNCTION_DEF */

#ifndef SH_FUNCTION_TYPEDEF
#  define SH_FUNCTION_TYPEDEF

/* Shell function typedefs with prototypes */
/* `Generic' function pointer typedefs */

typedef int sh_intfunc_t (int);
typedef int sh_ivoidfunc_t ();
typedef int sh_icpfunc_t (char *);
typedef int sh_icppfunc_t (char **);
typedef int sh_iptrfunc_t (PTR_T);

typedef void sh_voidfunc_t ();
typedef void sh_vintfunc_t (int);
typedef void sh_vcpfunc_t (char *);
typedef void sh_vcppfunc_t (char **);
typedef void sh_vptrfunc_t (PTR_T);

typedef int sh_wdesc_func_t (WORD_DESC *);
typedef int sh_wlist_func_t (WORD_LIST *);

typedef int sh_glist_func_t (GENERIC_LIST *);

typedef char *sh_string_func_t (const char *);	/* like savestring, et al. */

typedef int sh_msg_func_t (const char *, ...);		/* printf(3)-like */
typedef void sh_vmsg_func_t (const char *, ...);	/* printf(3)-like */

/* Specific function pointer typedefs.  Most of these could be done
   with #defines. */
typedef void sh_sv_func_t (const char *);	/* sh_vcpfunc_t */
typedef void sh_free_func_t (PTR_T);		/* sh_vptrfunc_t */
typedef void sh_resetsig_func_t (int);		/* sh_vintfunc_t */

typedef bool sh_ignore_func_t (const char *);	/* sh_icpfunc_t */

typedef int sh_assign_func_t (const char *);
typedef int sh_wassign_func_t (WORD_DESC *, int);

typedef int sh_load_func_t (const char *);
typedef void sh_unload_func_t (const char *);

typedef int sh_builtin_func_t (WORD_LIST *); /* sh_wlist_func_t */

#endif /* SH_FUNCTION_TYPEDEF */

#define NOW	((time_t) time ((time_t *) 0))
#define GETTIME(tv)	gettimeofday(&(tv), NULL)

/* Some defines for calling file status functions. */
#define FS_EXISTS	  0x1
#define FS_EXECABLE	  0x2
#define FS_EXEC_PREFERRED 0x4
#define FS_EXEC_ONLY	  0x8
#define FS_DIRECTORY	  0x10
#define FS_NODIRS	  0x20
#define FS_READABLE	  0x40

/* Default maximum for move_to_high_fd */
#define HIGH_FD_MAX	256

/* The type of function passed as the fourth argument to qsort(3). */
#ifdef __STDC__
typedef int QSFUNC (const void *, const void *);
#else
typedef int QSFUNC ();
#endif

/* Some useful definitions for Unix pathnames.  Argument convention:
   x == string, c == character */

#if !defined (__CYGWIN__)
#  define ABSPATH(x)	((x)[0] == '/')
#  define RELPATH(x)	((x)[0] != '/')
#else /* __CYGWIN__ */
#  define ABSPATH(x)	(((x)[0] && ISALPHA((unsigned char)(x)[0]) && (x)[1] == ':') || ISDIRSEP((x)[0]))
#  define RELPATH(x)	(ABSPATH(x) == 0)
#endif /* __CYGWIN__ */

#define ROOTEDPATH(x)	(ABSPATH(x))

#define DIRSEP	'/'
#if !defined (__CYGWIN__)
#  define ISDIRSEP(c)	((c) == '/')
#else
#  define ISDIRSEP(c)	((c) == '/' || (c) == '\\')
#endif /* __CYGWIN__ */
#define PATHSEP(c)	(ISDIRSEP(c) || (c) == 0)

#define DOT_OR_DOTDOT(s)	(s[0] == '.' && (s[1] == 0 || (s[1] == '.' && s[2] == 0)))
#if defined (HANDLE_MULTIBYTE)
#define WDOT_OR_DOTDOT(w)	(w[0] == L'.' && (w[1] == L'\0' || (w[1] == L'.' && w[2] == L'\0')))
#endif

#if 0
/* Declarations for functions defined in xmalloc.c */
extern PTR_T xmalloc (size_t);
extern PTR_T xrealloc (void *, size_t);
extern void xfree (void *);
#endif

/* Declarations for functions defined in general.c */
extern void posix_initialize (bool);

extern int num_posix_options ();
extern char *get_posix_options (char *);
extern void set_posix_options (const char *);

extern void save_posix_options ();

#if defined (RLIMTYPE)
extern RLIMTYPE string_to_rlimtype (char *);
extern void print_rlimtype (RLIMTYPE, int);
#endif

extern bool all_digits (const char *);
extern bool legal_number (const char *, intmax_t *);
extern bool legal_identifier (const char *);
extern bool importable_function_name (const char *, size_t);
extern bool exportable_function_name (const char *);
extern bool check_identifier (WORD_DESC *, int);
extern bool valid_nameref_value (const char *, int);
extern bool check_selfref (const char *, const char *, int);
extern bool legal_alias_name (const char *, int);
extern bool line_isblank (const char *);
extern int assignment (const char *, int);

extern int sh_unset_nodelay_mode (int);
extern int sh_setclexec (int);
extern int sh_validfd (int);
extern int fd_ispipe (int);
#if 0
extern void check_dev_tty ();
#endif
extern int move_to_high_fd (int, int, int);
extern int check_binary_file (const char *, int);

#ifdef _POSIXSTAT_H_
extern int same_file (const char *, const char *, struct stat *, struct stat *);
#endif

extern int sh_openpipe (int *);
extern int sh_closepipe (int *);

extern bool file_exists (const char *);
extern bool file_isdir (const char  *);
extern bool file_iswdir (const char  *);
extern bool path_dot_or_dotdot (const char *);
extern bool absolute_pathname (const char *);
extern bool absolute_program (const char *);

extern char *make_absolute (const char *, const char *);
extern const char *base_pathname (const char *);
extern char *full_pathname (char *);
extern const char *polite_directory_format (const char *);
extern char *trim_pathname (char *, int);
extern char *printable_filename (const char *, int);

extern char *extract_colon_unit (char *, int *);

extern void tilde_initialize ();
extern char *bash_tilde_find_word (const char *, int, int *);
extern char *bash_tilde_expand (const char *, int);

/* extern int group_member (gid_t); */
extern char **get_group_list (int *);
extern int *get_group_array (int *);

extern char *conf_standard_path ();
extern int default_columns ();

#endif	/* _GENERAL_H_ */
