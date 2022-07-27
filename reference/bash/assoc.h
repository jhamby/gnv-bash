/* assoc.h -- definitions for the interface exported by assoc.c that allows
   the rest of the shell to manipulate associative array variables. */

/* Copyright (C) 2008,2009-2020 Free Software Foundation, Inc.

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

#ifndef _ASSOC_H_
#define _ASSOC_H_

#include "stdc.h"
#include "hashlib.h"

#define ASSOC_HASH_BUCKETS	1024

#define assoc_empty(h)		((h)->nentries == 0)
#define assoc_num_elements(h)	((h)->nentries)

#define assoc_create(n)		(hash_create((n)))

#define assoc_copy(h)		(hash_copy((h), 0))

#define assoc_walk(h, f)	(hash_walk((h), (f))

extern void assoc_dispose (HASH_TABLE *);
extern void assoc_flush (HASH_TABLE *);

extern int assoc_insert (HASH_TABLE *, char *, const char *);
extern PTR_T assoc_replace (HASH_TABLE *, char *, const char *);
extern void assoc_remove (HASH_TABLE *, const char *);

extern char *assoc_reference (HASH_TABLE *, const char *);

extern char *assoc_subrange (HASH_TABLE *, arrayind_t, arrayind_t, int, int, int);
extern char *assoc_patsub (HASH_TABLE *, const char *, const char *, int);
extern char *assoc_modcase (HASH_TABLE *, const char *, int, int);

extern HASH_TABLE *assoc_quote (HASH_TABLE *);
extern HASH_TABLE *assoc_quote_escapes (HASH_TABLE *);
extern HASH_TABLE *assoc_dequote (HASH_TABLE *);
extern HASH_TABLE *assoc_dequote_escapes (HASH_TABLE *);
extern HASH_TABLE *assoc_remove_quoted_nulls (HASH_TABLE *);

extern char *assoc_to_kvpair (HASH_TABLE *, int);
extern char *assoc_to_assign (HASH_TABLE *, int);

extern WORD_LIST *assoc_to_word_list (HASH_TABLE *);
extern WORD_LIST *assoc_keys_to_word_list (HASH_TABLE *);

extern char *assoc_to_string (HASH_TABLE *, const char *, int);
#endif /* _ASSOC_H_ */
