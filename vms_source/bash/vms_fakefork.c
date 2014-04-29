/* File: vms_fakefork.c
 *
 * $Id: vms_fakefork.c,v 1.4 2013/06/14 05:02:56 robertsonericw Exp $
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
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <errno.h>

#include "config.h"

#include <unixlib.h>
#include "stdc.h"
#include "xmalloc.h"
#include "command.h"
#include "general.h"
#include "hashlib.h"
#include "variables.h"
#include "error.h"
#include "builtins/common.h"
#include "subst.h"
#include "bashjmp.h"
#include "input.h"
#include "make_cmd.h"
#include "dispose_cmd.h"
#include "command.h"
#include "externs.h"
#include "unwind_prot.h"
#include "bashhist.h"
#include "history.h"
#include "shell.h"
#include "hashlib.h"
#include "hashcmd.h"

/* For debugging routines */
#include <unixio.h>


#include <jpidef.h>
#include <stsdef.h>
#include <efndef.h>
#include <descrip.h>

/* These three are noise that the compiler generate for VMS specific code
 * in the more stricter modes
 */
#pragma message disable pragma
#pragma message disable dollarid
#pragma message disable valuepres

#pragma member_alignment save
#pragma nomember_alignment longword
struct item_list_3 {
	unsigned short len;
	unsigned short code;
	void * bufadr;
	unsigned short * retlen;
};

#pragma member_alignment

#pragma message save
#pragma message disable noparmlist
int SYS$GETJPIW
       (unsigned long efn,
	pid_t * pid,
	const struct dsc$descriptor_s * prcnam,
	const struct item_list_3 * itmlst,
	void * iosb,
	void (* astadr)(__unknown_params),
	void * astprm,
	void * nullarg);
#pragma message restore

/* Referenced routines from module NOJOBS.C */
extern void set_pid_status (int, int);
int wait_for (pid_t pid);

/* Reference routine from module VARIABLES.C */
void vms_restore_special_vars(void);

#define MOD_SUBST 1 /* Make special definitions visible */
#include "vms_fakefork.h"

/* OpenVMS-specific declarations needed to implement the storage */
/* and retrieval of last_made_pid values. */


#define PID_STACK_STORE_SIZE 20

struct vms_pid_stack {
   struct vms_pid_stack *stp_PrevStack;
   unsigned long ul_CurrentEntry;
   int pid_store[PID_STACK_STORE_SIZE];
   };

static struct vms_pid_stack *stp_GetNewStackStore();
static void DestroyStackStore(struct vms_pid_stack *stp_StackStore) {
    free((void *)stp_StackStore);
}

extern int vms_child_exit_pid = -1;

extern int vms_fake_fork_level = 0;

extern int vms_fake_exit_seen = 0;

extern WORD_LIST *subst_assign_varlist;
extern pid_t last_made_pid;
extern pid_t last_asynchronous_pid;
extern int export_env_index;
extern int export_env_size;
extern int bash_input_fd_changed;
extern int default_buffered_input;
extern int line_number;
extern int return_catch_flag;
extern int return_catch_value;
extern int last_command_exit_value;
extern int last_command_subst_pid;
extern int comsub_ignore_return ;
extern int restricted;
extern int restricted_shell;
extern int unbound_vars_is_error;
extern char *this_command_name;

extern int xtrace_fd;
extern int xtrace_fp;
extern char *rl_completer_word_break_characters;
extern int startup_state;
extern int funcnest_max;
extern int eof_encountered;
extern int eof_encountered_limit;
extern int ignoreeof;

extern void * unwind_protect_list;
extern HASH_TABLE *hashed_filenames;

void dispose_variable (SHELL_VAR *);

static pid_t vms_fake_child_pid = -1;

static char vms_imagename[256] = {0};

/* OpenVMS-specific Storage declaration and definition for */
/* storing last_made_pid values. */

static struct vms_pid_stack InitialPidStack;
static struct vms_pid_stack *stp_PidStack=NULL;

/* We need to get the image name in order to vfork/exec subshells */
/* We will cache it for frequent use. */
char *
vms_getjpi_imagename(void) {

struct item_list_3 itemlist[2];
int status;
unsigned short length;
unsigned short jpi_iosb[4];
char my_imagename[256];

    if (vms_imagename[0] != 0) {
	return vms_imagename;
    }

     /* Verify that the PID is valid */
    /*------------------------------*/
    itemlist[0].len = 255;
    itemlist[0].code = JPI$_IMAGNAME;
    itemlist[0].bufadr = my_imagename;
    itemlist[0].retlen = &length;
    itemlist[1].len = 0;
    itemlist[1].code = 0;

    status = SYS$GETJPIW
	   (EFN$C_ENF,
	    0,
	    NULL,
	    itemlist,
	    jpi_iosb,
	    NULL,
	    0,
	    0);

    if ($VMS_STATUS_SUCCESS(status) && $VMS_STATUS_SUCCESS(jpi_iosb[0])) {
    char * unix_name;
	my_imagename[length] = 0;

	/* Convert it to Unix as CRTL is in UNIX mode */
	unix_name = decc$translate_vms(my_imagename);

	/* Cache the Unix name */
	strcpy(vms_imagename, unix_name);
    } else {
	vms_imagename[0] = 0;
    }
    return vms_imagename;
}

static void
vms_save_current_directory (VMS_BASH_ENVIRON * buf) {
    char * tmpstr;

    tmpstr = get_string_value("PWD");
    buf->pwd    = (tmpstr != NULL) ? savestring(tmpstr) : NULL;
    buf->curdir = getcwd (NULL, VMS_PATH_MAX, 0);
}

static void
vms_restore_current_directory (VMS_BASH_ENVIRON * buf) {

    /* change old directory */

    /* Since we always set the curdir member, it should never be NULL */
    if (buf->curdir != NULL) {
	chdir (buf->curdir);

	/* pwd member is from original bash, curdir is VMS specific */
	if (buf->pwd != NULL) {
	    set_working_directory (buf->pwd);

	    /* Clean up curdir */
	    free(buf->curdir);
	    buf->curdir = NULL;
	} else {
	    set_working_directory (buf->curdir);
	}

    } else if (buf->pwd != NULL) {
    /* This is in the GNV 213 port.  If we get here something is wrong
       in the port.  Leave it in here for now and maybe add an assert */
	puts ("buf->curdir was NULL and that should not happen!");
	chdir (buf->pwd);
	    set_working_directory (buf->pwd);
    }

    /* If old directory was not set, clear out the saved pwd */
    if (buf->pwd) {
	free(buf->pwd);
	buf->pwd = NULL;
    }
}

/* Create a new shell variable with name NAME. */
static SHELL_VAR *
vms_new_shell_variable(const char * name) {
    SHELL_VAR *entry;

    entry = (SHELL_VAR *)xmalloc (sizeof (SHELL_VAR));

    entry->name = savestring (name);
    var_setvalue (entry, (char *)NULL);
    CLEAR_EXPORTSTR (entry);

    entry->dynamic_value = (sh_var_value_func_t *)NULL;
    entry->assign_func = (sh_var_assign_func_t *)NULL;

    entry->attributes = 0;

    /* Always assume variables are to be made at toplevel!
       make_local_variable has the responsibilty of changing the
       variable context. */
    entry->context = 0;

    return (entry);
}

static void
vms_dispose_shell_variables_hash_table (HASH_TABLE *table, int env_flag) {
    int i;
    BUCKET_CONTENTS *item, *next;

    if (!table) {
	return;
    }

    for (i = 0; i < table->nbuckets; i++) {
	item = table->bucket_array[i];
	while (item) {
	    if (env_flag) {
		SHELL_VAR *shell_var;
		shell_var = (SHELL_VAR *)item->data;
		dispose_variable(shell_var);
	    }

	    if (item->key)
		free (item->key);
	    next = item->next;
	    free (item);
	    item = next;
	}
    }
    free (table->bucket_array);
    free (table);
}

const char * NULL_STR = "(NULL)";

const char * CTX_NULL = "Null - Global";


char * debug_shell_var_attr(int attr) {
static char attr_string[200];

    attr_string[0] = 0;
    if (attr & att_exported)
	strcat(attr_string, " exported");
    if (attr & att_readonly)
	strcat(attr_string, " readonly");
    if (attr & att_array)
	strcat(attr_string, " array");
    if (attr & att_function)
	strcat(attr_string, " function");
    if (attr & att_integer)
	strcat(attr_string, " integer");
    if (attr & att_local)
	strcat(attr_string, " local");
    if (attr & att_assoc)
	strcat(attr_string, " assoc");
    if (attr & att_trace)
	strcat(attr_string, " trace");
    if (attr & att_uppercase)
	strcat(attr_string, " uppercase");
    if (attr & att_lowercase)
	strcat(attr_string, " lowercase");
    if (attr & att_capcase)
	strcat(attr_string, " capcase");
    return attr_string;
}

void debug_shell_var(const SHELL_VAR * shvar) {
char * sh_name;
char * sh_value;
char * sh_expstr;
char * sh_attr;

    if (shvar == NULL) {
	puts("SHELL_VAR * is null");
	return;
    }
    sh_name = shvar->name;
    if (sh_name == NULL)
	sh_name = NULL_STR;
    sh_value = shvar->value;
    if (sh_value == NULL)
	sh_value = NULL_STR;
    if (array_p (shvar)) {
	sh_value = array_to_string((ARRAY *)shvar->value, ",", 1);
    } else if (assoc_p (shvar)) {
	sh_value = assoc_to_string((HASH_TABLE *)shvar->value, ",", 1);
    }
    sh_expstr = shvar->exportstr;
    if (sh_expstr == NULL)
	sh_expstr = NULL_STR;
    printf("name: %s, value: %s, exportstr: %s\n",
	   sh_name,
	   sh_value,
	   sh_expstr);
    if (assoc_p (shvar)) {
	free(sh_value);
    }
    printf(
      "   dynamic_value: %d, assign_func: %d, attributes: %d, context: %d\n",
      (int)shvar->dynamic_value,
      (int)shvar->assign_func,
      shvar->attributes,
      shvar->context);
    sh_attr = debug_shell_var_attr(shvar->attributes);
    printf("%s\n", sh_attr);
}

void debug_bucket_content(const BUCKET_CONTENTS * bkt_content, int var) {
char * keystr;

    if (bkt_content == NULL) {
	puts("bkt_content is NULL");
	return;
    }
    keystr = bkt_content->key;
    if (keystr == NULL)
	keystr = NULL_STR;
    printf("next: %d, key: %s\n, data: %d, khash: %d, times_found: %d\n",
	   bkt_content->next,
	   keystr,
	   bkt_content->data,
	   bkt_content->khash,
	   bkt_content->times_found);
    if ((var != 0) && (bkt_content->data != NULL)) {
	debug_shell_var((const SHELL_VAR *)bkt_content->data);
    }
}

int debug_bucket_contents(const BUCKET_CONTENTS * bkt_content, int var) {
const BUCKET_CONTENTS * bkt_contptr;
int entry_count;

    entry_count = 0;
    bkt_contptr = bkt_content;
    while (bkt_contptr != NULL) {
	entry_count++;
	debug_bucket_content(bkt_contptr, var);
	bkt_contptr = bkt_contptr->next;
    }
    return entry_count;
}

void debug_hash_table(const HASH_TABLE * hashptr, int var) {
int bkt_index;
int entry_count;

    if (hashptr == NULL) {
	puts("HASH_TABLE * is NULL");
	return;
    }
    printf("nbuckets: %d, nentries: %d\n",
	   hashptr->nbuckets,
	   hashptr->nentries);

    bkt_index = 0;
    while (bkt_index < hashptr->nbuckets) {
	entry_count +=
	    debug_bucket_contents(hashptr->bucket_array[bkt_index], var);
	bkt_index++;
    }
    if (bkt_index != hashptr->nbuckets) {
	printf("  Expected %d buckets, found %d\n",
	       hashptr->nbuckets,
	       bkt_index);
    }
}

char * debug_var_context_flags(int flags) {
static char result[80];

     result[0] = 0;
     if (flags & VC_HASLOCAL)
	strcat(result, " VC_HASLOCAL");
     if (flags & VC_HASTMPVAR)
	strcat(result, " VC_HASTMPVAR");
     if (flags & VC_FUNCENV)
	strcat(result, " VC_FUNCENV");
     if (flags & VC_BLTNENV)
	strcat(result, " VC_BLTNENV");
     if (flags & VC_TEMPENV)
	strcat(result, " VC_TEMPENV");
     return result;
}

void debug_var_context(const VAR_CONTEXT *context) {
char * ctx_name;
char * ctx_flags;

    if (context == NULL) {
	puts("VAR_CONTEXT * is NULL");
	return;
    }

    ctx_name = context->name;
    if (ctx_name == NULL) {
	ctx_name = CTX_NULL;
    }
    ctx_flags = debug_var_context_flags(context->flags);
    printf("name: %s, scope: %d, flags: %d, up: %d, down: %d, table: %d\n",
	   ctx_name,
	   context->scope,
	   context->flags,
	   context->up,
	   context->down,
	   context->table);
    printf("%s\n", ctx_flags);
}

void debug_var_context_table(const VAR_CONTEXT *context) {
char * ctx_name;
char * ctx_flags;

    if (context == NULL) {
	puts("VAR_CONTEXT * is NULL");
	return;
    }
    ctx_name = context->name;
    if (ctx_name == NULL) {
	ctx_name = CTX_NULL;
    }
    ctx_flags = debug_var_context_flags(context->flags);
    printf("name: %s, scope: %d, flags: %d, up: %d, down: %d, table: %d\n",
	   ctx_name,
	   context->scope,
	   context->flags,
	   context->up,
	   context->down,
	   context->table);
    printf("%s\n", ctx_flags);
    debug_hash_table(context->table, 1);
}


void debug_filedesc(int fd) {
static char device_name[256];
char * retname;

    retname = getname(fd, device_name, 1);
    if (retname == NULL)
	retname = NULL_STR;
    fprintf(stderr, "File Descriptor: %d, name: %s\n", fd, retname);
}

void debug_stdfd(void) {
    debug_filedesc(0);
    debug_filedesc(1);
    debug_filedesc(2);
}

static void
vms_save_shell_variables (VMS_BASH_ENVIRON * buf) {

    VAR_CONTEXT * shell_var;
    VAR_CONTEXT * new_shell_var;
    VAR_CONTEXT * prev_shell_var;
    VAR_CONTEXT * global_var;
    VAR_CONTEXT * new_global_var;
    VAR_CONTEXT * prev_global_var;
    WORD_LIST * tlist;
    WORD_LIST * nlist;

    maybe_make_export_env ();


    /* Shell and global Variables */
    if (buf->subshell_save == VMS_SAVE_SUBSHELL) {
	buf->shell_variables = shell_variables;
	buf->global_variables = global_variables;

	/* Global Variables */
	prev_global_var = NULL;
	new_global_var = NULL;
	buf->global_variables = global_variables;

	global_var = global_variables;
	global_variables = NULL;
	while (global_var != NULL) {

	    new_global_var = new_var_context(global_var->name,
					     global_var->flags);
	    if (global_variables == NULL) {
		global_variables = new_global_var;
	    }
	    new_global_var->scope = global_var->scope;
	    if (prev_global_var) {
		prev_global_var->down = new_global_var;
	    }
	    new_global_var->up = prev_global_var;
	    new_global_var->table = hash_copy(global_var->table,
					  (sh_string_func_t *)copy_variable);
	    global_var = global_var->down;
	    prev_global_var = new_global_var;
	}

	/* Shell variables */
	prev_shell_var = NULL;
	new_shell_var = NULL;
	shell_var = shell_variables;
	shell_variables = NULL;
	while (shell_var != NULL) {

	    if (shell_var == buf->global_variables) {
	        /* The global_variable table can be a part of the */
	        /* shell_variables table, so we must not make a new copy */
		new_shell_var = global_variables;

		/* Global variables must be the last in the table */
		shell_var = NULL;
	    } else {
                new_shell_var = new_var_context(shell_var->name,
						shell_var->flags);
		new_shell_var->scope = shell_var->scope;
		new_shell_var->table = hash_copy(shell_var->table,
					(sh_string_func_t *)copy_variable);
		shell_var = shell_var->down;
	    }
	    if (shell_variables == NULL) {
		shell_variables = new_shell_var;
	    }
	    if (prev_shell_var) {
	        prev_shell_var->down = new_shell_var;
	    }
	    new_shell_var->up = prev_shell_var;
	    prev_shell_var = new_shell_var;
	}

	/* Shell functions */
	buf->shell_functions = shell_functions;
	shell_functions = hash_copy (shell_functions,
				 (sh_string_func_t *)copy_variable);

#if 1
	buf->temporary_env = temporary_env;
	temporary_env = (temporary_env != NULL) ? hash_copy (temporary_env,
			       (sh_string_func_t *)copy_variable) : NULL;
#endif
#if 0
	buf->export_env = export_env;
	if (export_env != NULL) {
	    export_env = strvec_copy (buf->export_env);
	}
	buf->export_env_index = export_env_index;
	buf->export_env_size = export_env_size;
#endif
    }
    buf->subst_assign_varlist = subst_assign_varlist;
    subst_assign_varlist = (WORD_LIST *) NULL;

/* If the child finds anything in the subst_assign_varlist, it
 * deallocates it.  When we resume from a fake_fork, we need the
 * previously set up subst_assign_varlist
 */
#if 0
    tlist = buf->subst_assign_varlist;

    /* Copy the list */
    while (tlist != NULL) {
	char * tstring;
	WORD_LIST * alist;
	tstring = tlist->word->word;
	alist = add_string_to_list(tstring, (WORD_LIST *) NULL);
	alist->word->flags = tlist->word->flags;
	if (subst_assign_varlist == NULL) {
	    subst_assign_varlist = alist;
	    nlist = subst_assign_varlist;
	} else {
	    nlist->next = alist;
	    nlist = nlist->next;
	}
	tlist = tlist->next;
    }
#endif
}

static void
vms_restore_shell_variables (VMS_BASH_ENVIRON * buf) {

    VAR_CONTEXT * shell_var;
    VAR_CONTEXT * global_var;

    if (buf->subshell_save == VMS_SAVE_SUBSHELL) {
	buf->subshell_save = VMS_SAVE_DEALLOCATE;

	shell_var = shell_variables;
	while (shell_var != NULL) {
	    VAR_CONTEXT * next_shell_var;

	    next_shell_var = shell_var->down;

	    if (shell_var == global_variables) {
		/* Do not free the global variables copy here */
		shell_var = NULL;
	    } else {
		vms_dispose_shell_variables_hash_table(shell_var->table, 1);
		xfree(shell_var->name);
		xfree(shell_var);
		shell_var = next_shell_var;
	    }
	}
	shell_variables = buf->shell_variables;

	global_var = global_variables;
	while (global_var != NULL) {
	    VAR_CONTEXT * next_global_var;
	    next_global_var = global_var->down;
	    vms_dispose_shell_variables_hash_table(global_var->table, 1);
	    xfree(global_var->name);
	    xfree(global_var);
	    global_var = next_global_var;
	}
	global_variables = buf->global_variables;

	vms_dispose_shell_variables_hash_table(shell_functions, 0);
	shell_functions = buf->shell_functions;

#if 1
	if (temporary_env != NULL)
	   vms_dispose_shell_variables_hash_table(temporary_env, 0);
	temporary_env = buf->temporary_env;
#endif
#if 0
	free (export_env);
	export_env = buf->export_env;
	export_env_index = buf->export_env_index;
	export_env_size = buf->export_env_size;
	array_needs_making = 0;
#endif
    }
    /* If there is a subst_assign_varlist left from a child
     * a script error occurred and it needs to be deallocated.
     */
    if (subst_assign_varlist) {
	dispose_words (subst_assign_varlist);
    }
    subst_assign_varlist = buf->subst_assign_varlist;
}

/* This function saves (or copies) all global extern variables in */
/* preparation for subshell activity. */
static void
vms_save_extern_variables (VMS_BASH_ENVIRON * buf) {
    int i;

    /* Special variables */
    buf->xtrace_fd                 = xtrace_fd;
    buf->xtrace_fp                 = xtrace_fp;
#if defined (READLINE)
#if defined (STRICT_POSIX)
    buf->_rl_screenwidth           = rl_screenwidth;
#endif
    if (rl_completer_word_break_characters != NULL) {
	buf->rl_completer_word_break_characters =
	    savestring(rl_completer_word_break_characters);
    } else {
	buf->rl_completer_word_break_characters = NULL;
    }
#endif
    buf->startup_state             = startup_state;
    buf->funcnest_max              = funcnest_max;
    buf->ifs_var                   = ifs_var;
    buf->ifs_value                 = ifs_value;
    buf->history_control           = history_control;
    buf->history_lines_this_session = history_lines_this_session;
    buf->history_lines_in_file     = history_lines_in_file;
    buf->history_write_timestamps  = history_write_timestamps;
    buf->eof_encountered           = eof_encountered;
    buf->eof_encountered_limit     = eof_encountered_limit;
    buf->ignoreeof                 = ignoreeof;
    memcpy(buf->ifs_cmap, ifs_cmap, sizeof(buf->ifs_cmap));
#if defined (READLINE) && defined (STRICT_POSIX)
    buf->rl_screenheight               = _rl_screenheight;
#endif
    buf->history_expansion_char    = history_expansion_char;
    buf->history_subst_char        = history_subst_char;
    buf->history_comment_char      = history_comment_char;
    /* End of special variables */

    for (i = 0; i < 10; i++) {
	buf->dollar_vars[i] = dollar_vars[i];
	if (dollar_vars[i] != NULL) {
	    dollar_vars[i] = savestring (buf->dollar_vars[i]);
	} else {
	    dollar_vars[i] = NULL;
	}
    }
    buf->rest_of_args              = rest_of_args;
    rest_of_args = copy_word_list(rest_of_args);

    buf->interactive               = interactive;
    buf->interactive_shell         = interactive_shell;
    buf->login_shell               = login_shell;
    buf->subshell_level            = subshell_level;
    buf->subshell_environment      = subshell_environment;
    buf->subshell_skip_commands    = subshell_skip_commands;
    buf->exit_immediately_on_error = exit_immediately_on_error;
    buf->variable_context          = variable_context;
    buf->line_number               = line_number;
    buf->dollar_dollar_pid         = dollar_dollar_pid;
    buf->last_made_pid             = last_made_pid;
    buf->return_catch_flag         = return_catch_flag;
    buf->return_catch_value        = return_catch_value;
    buf->last_command_exit_value   = last_command_exit_value;
    buf->last_command_subst_pid    = last_command_subst_pid;
    buf->comsub_ignore_return      = comsub_ignore_return;
    buf->restricted                = restricted;
    buf->restricted_shell          = restricted_shell;
    buf->unbound_vars_is_error     = unbound_vars_is_error;
    buf->vms_fake_fork_level       = vms_fake_fork_level;
    buf->vms_fake_exit_seen        = vms_fake_exit_seen;
    buf->this_command_name         = this_command_name;
    buf->hashed_filenames          = hashed_filenames;
}

/* With the exception of the global variable hashed_filenames, this   */
/* function restores all global, extern variables after subshell      */
/* activity is completed. The lone exception for the global variable  */
/* hashed_variables is necessary because this variable references a   */
/* hash table the contents of which are cleared by the restoration    */
/* of the special environment variable PATH. If hashed_filenames were */
/* to be restored here, the contents of the hash table to which it    */
/* refers would be cleared by the restoration of the value for PATH   */
/* thus destroying its information instead of restoring it as desired.*/
/* Instead, we leave hashed_filenames pointing to the copy of the hash*/
/* table left by the completed subshell and let the later restoration */
/* of PATH clear the subshell's copy of the hash table which is no    */
/* longer needed. Then, after PATH is restored, the value of global   */
/* hashed_filenames (along with its referenced hash table values) is  */
/* restored to the state prior to subshell invocation. */

static void
vms_restore_extern_variables (VMS_BASH_ENVIRON * buf) {
int i;

    /* Special variables */
    xtrace_fd                 = buf->xtrace_fd;
    xtrace_fp                 = buf->xtrace_fp;
#if defined (READLINE)
#if defined (STRICT_POSIX)
    rl_screenwidth            = buf->rl_screenwidth;
#endif
    if (rl_completer_word_break_characters != NULL) {
	free (rl_completer_word_break_characters);
    }
    rl_completer_word_break_characters =
	buf->rl_completer_word_break_characters;
#endif
    startup_state             = buf->startup_state;
    funcnest_max              = buf->funcnest_max;
    sv_globignore("GLOBIGNORE");
    history_control           = buf->history_control;
    history_lines_this_session = buf->history_lines_this_session;
    history_lines_in_file     = buf->history_lines_in_file;
    eof_encountered           = buf->eof_encountered;
    eof_encountered_limit     = buf->eof_encountered_limit;
    ignoreeof                 = buf->ignoreeof;
    ifs_var		      = buf->ifs_var;
    ifs_value                 = buf->ifs_value;
    memcpy(ifs_cmap, buf->ifs_cmap, sizeof (buf->ifs_cmap));
#if defined (READLINE) && defined (STRICT_POSIX)
    _rl_screenheight               = buf->rl_screenheight;
#endif
    history_expansion_char    = buf->history_expansion_char;
    history_subst_char        = buf->history_subst_char;
    history_comment_char      = buf->history_comment_char;
    /* End of Special variables */

    for (i = 0; i < 10; i++) {
	if (dollar_vars[i] != NULL) {
	    free (dollar_vars[i]);
	}
	dollar_vars[i] = buf->dollar_vars[i];
    }
    if (rest_of_args != NULL) {
	dispose_words(rest_of_args);
    }
    rest_of_args              = buf->rest_of_args;

    interactive               = buf->interactive;
    interactive_shell         = buf->interactive_shell;
    login_shell               = buf->login_shell;
    subshell_level            = buf->subshell_level;
    subshell_environment      = buf->subshell_environment;
    subshell_skip_commands    = buf->subshell_skip_commands;
    exit_immediately_on_error = buf->exit_immediately_on_error;
    variable_context          = buf->variable_context;
    dollar_dollar_pid         = buf->dollar_dollar_pid;
    line_number               = buf->line_number;
    return_catch_flag         = buf->return_catch_flag;
    return_catch_value        = buf->return_catch_value;
    last_command_exit_value   = buf->last_command_exit_value;
    last_command_subst_pid    = buf->last_command_subst_pid;
    comsub_ignore_return      = buf->comsub_ignore_return;
    restricted                = buf->restricted;
    restricted_shell          = buf->restricted_shell;
    unbound_vars_is_error     = buf->unbound_vars_is_error;
    if (vms_fake_exit_seen == 0)
       printf("\r\n!!!restoring fork before exit!!!\r\n");
    if (vms_fake_fork_level != buf->vms_fake_fork_level)
       printf("\r\n!!!fork_level != saved fork_level (%ld != %ld)!!!\r\n",vms_fake_fork_level,buf->vms_fake_fork_level);
    vms_fake_fork_level       = buf->vms_fake_fork_level;
    vms_fake_exit_seen        = buf->vms_fake_exit_seen;
    this_command_name         = buf->this_command_name;
    last_made_pid             = PopPid();
    last_asynchronous_pid     = PopPid();
/*    vms_fake_child_pid        = last_made_pid; */
}

void
vms_save_bash_input(VMS_BASH_ENVIRON * buf) {

#ifdef BUFFERED_INPUT
    buf->bash_input_fd_changed = bash_input_fd_changed;
    buf->default_buffered_input = default_buffered_input;
#endif
    buf->bash_input.type = bash_input.type;
    buf->bash_input.name = bash_input.name;
    if (buf->bash_input.name != NULL) {
        bash_input.name = savestring(buf->bash_input.name);
    } else {
	bash_input.name = NULL;
    }
    buf->bash_input.location = bash_input.location;
    buf->bash_input.getter = bash_input.getter;
    buf->bash_input.ungetter = bash_input.ungetter;
}

void
vms_restore_bash_input(VMS_BASH_ENVIRON * buf) {
#ifdef BUFFERED_INPUT
    bash_input_fd_changed = buf->bash_input_fd_changed;
    default_buffered_input = buf->default_buffered_input;
#endif
    bash_input.type = buf->bash_input.type;
    if (bash_input.name) {
	free (bash_input.name);
    }
    bash_input.name = buf->bash_input.name;
    bash_input.location = buf->bash_input.location;
    bash_input.getter = buf->bash_input.getter;
    bash_input.ungetter = buf->bash_input.ungetter;
}

/* This is the copy function to be used by the code in hashlib to copy */
/* the hash entry payload from one hash entry to another within the    */
/* hashlib\hash_copy() function when copying the hash entries pointed  */
/* to by hashed_filenames. */

char *vms_copy_path_data(char *cpdata)
   {
   PATH_DATA *Src=(PATH_DATA *)cpdata;
   PATH_DATA *Dest=(PATH_DATA *)xmalloc (sizeof (PATH_DATA));
   
   if (Dest != NULL)
      {
      Dest->path = savestring (Src->path);
      Dest->flags = Src->flags;
      }
   return (char *)Dest;
   }
   
VMS_BASH_ENVIRON *
vms_save_bash_environ(int subshell_save) {
    VMS_BASH_ENVIRON *buf;
    buf = malloc(sizeof (VMS_BASH_ENVIRON));
    buf->sanity_count = VMS_SAVE_SANITY_VALUE;
    buf->subshell_save = subshell_save;

    buf->unwind_protect_list = unwind_protect_list;
    unwind_protect_list = NULL;

    vms_save_procenv (buf->top_level, top_level);
    vms_save_procenv (buf->subshell_top_level, subshell_top_level);
    vms_save_procenv (buf->return_catch, return_catch);

    vms_save_std_fds (buf->fds);
    vms_save_current_directory (buf);
    vms_save_extern_variables (buf);
    vms_save_shell_variables (buf);
    
    /* The above call to vms_save_extern_variables() saved the original */
    /* hash table pointer contained in hashed_filenames. Now, we create */
    /* a copy of this hash table for use by the subshell. */
    hashed_filenames = hash_copy (buf->hashed_filenames, vms_copy_path_data);
    
    vms_save_bash_input (buf);
    buf->global_command = global_command;
    if (global_command) {
        global_command = copy_command(buf->global_command);
    }
    buf->umask = umask (022);
    umask (buf->umask);

    vms_save_trap_signal_state(&buf->trapnsig_state);

/* Special shell variables are sometimes cached in global program     */
/* variables. If so we must update the values in the cache because    */
/* storage locations of shell variables change after saving/restoring */
/* the variables.                                                     */
/* [should this be done for all entries in VARIABLES\special_vars?]   */
    stupidly_hack_special_variables("IFS");
/*    buf->pipestatus = save_pipestatus_array(); */
/*    fprintf(stderr, "\n*DEBUG* vms_save_bash_environ %x\n", buf); */
    return buf;
}


void
vms_restore_bash_environ(VMS_BASH_ENVIRON * buf) {

    /* We should never enter here with out a save, or after a restore */
    if (buf->sanity_count != VMS_SAVE_SANITY_VALUE) {
	fprintf(stderr, "\nRestore of Bash Environment sanity count = %x\n\n",
	       buf->sanity_count);
	return;
    }
    buf->sanity_count = VMS_SAVE_SANITY_DEALLOCATE;
    umask (buf->umask);
    vms_restore_shell_variables (buf);
    vms_restore_extern_variables (buf);
    vms_restore_current_directory (buf);
    vms_restore_std_fds (buf->fds);

    /* Clear any new unwind protects that have been left over */
    clear_unwind_protect_list(0);
    unwind_protect_list = buf->unwind_protect_list;

    vms_restore_procenv (buf->top_level, top_level);
    vms_restore_procenv (buf->subshell_top_level, subshell_top_level);
    vms_restore_procenv (buf->return_catch, return_catch);

    vms_restore_bash_input (buf);
/*    restore_pipestatus_array(buf->pipestatus); */
/*    fprintf(stderr, "\n*DEBUG* vms_restore_bash_environ %x\n", buf); */
    if (global_command) {
	dispose_command(global_command);
    }
    global_command = buf->global_command;
    vms_restore_trap_signal_state(&buf->trapnsig_state);
/* Special shell variables are sometimes cached in global program     */
/* variables. So we must update the values in the cache because       */
/* storage locations of shell variables change after saving/restoring */
/* the variables.                                                     */
    vms_restore_special_vars();
    
/* The above call to vms_restore_special_vars() restored the cached value*/
/* for the special environment variable PATH. This also caused the sub-  */
/* shell's filename hash table (pointed to by hashed_filenames) to be    */
/* cleared. So, now we can deallocate the subshell's hash table and re-  */
/* store the original value of the global hashed_filenames which points  */
/* to the original hash table associated with the restored value of PATH.*/
    if (hashed_filenames != NULL) hash_dispose(hashed_filenames);
    hashed_filenames = buf->hashed_filenames;
    free(buf);
}


int vms_fake_fork(void) {
    vms_fake_fork_level++;
    vms_fake_exit_seen = 0;
/*    fprintf(stderr,
	     "\n*DEBUG* vms_fake_fork level %d\n", vms_fake_fork_level);
*/
    return 0;
}

int vms_fake_fork_exit(int status) {
/*    fprintf(stderr,
	    "\n*DEBUG* vms_fake_fork_exit level %d status %d\n",
	    vms_fake_fork_level, status);
*/
    if (vms_fake_fork_level || vms_fake_exit_seen)
       {
       if (vms_fake_exit_seen == 0)
          {
          if (subshell_level) subshell_skip_commands = 1;
          vms_fake_fork_level--;
          vms_fake_exit_seen = 1;
          set_pid_status(GetCurrentChildExitPid(), status << 8);
          }
       }
    else
#if !defined(__VAX)
       __posix_exit(status);
#else
       decc$exit(status);
#endif
    
    return status;
}

/* Need to set the pid that Bash is checking the status of */
int vms_fake_wait_for(pid_t child_pid) {
    vms_fake_child_pid = child_pid;
    return wait_for(child_pid);
}

/* OpenVMS-specific function definitions for allocating, initializing */
/* new storage for holding previous last_made_pid values. */

static struct vms_pid_stack *stp_GetNewStackStore() {
struct vms_pid_stack *stp_NewStack=NULL;

    stp_NewStack = malloc(sizeof(struct vms_pid_stack));
    memset(stp_NewStack, 0, sizeof(struct vms_pid_stack));
    return stp_NewStack;
}

/* OpenVMS-specific function definitions for storing and */
/* retrieving last_made_pid values. */

void PushPid(int Pid) {
struct vms_pid_stack *stp_NewStackStore=NULL;

    if (stp_PidStack == NULL) {
	memset(&InitialPidStack, 0, sizeof(struct vms_pid_stack));
	stp_PidStack = &InitialPidStack;
    }

    stp_PidStack->pid_store[stp_PidStack->ul_CurrentEntry++] = Pid;
    if (stp_PidStack->ul_CurrentEntry == PID_STACK_STORE_SIZE) {
	stp_NewStackStore = stp_GetNewStackStore();
	stp_NewStackStore->stp_PrevStack = stp_PidStack;
	stp_PidStack = stp_NewStackStore;
    }
}

int PopPid() {
struct vms_pid_stack *stp_OldStackStore=NULL;
int Pid = -1;

    if (stp_PidStack != NULL) {
	if (!stp_PidStack->ul_CurrentEntry--) {
	    stp_OldStackStore = stp_PidStack;
	    stp_PidStack = stp_PidStack->stp_PrevStack;
	    if (stp_PidStack != NULL) {
		stp_PidStack->ul_CurrentEntry--;
	    }
	    if (stp_OldStackStore != &InitialPidStack) {
		DestroyStackStore(stp_OldStackStore);
	    }
	}
    if (stp_PidStack != NULL)
	Pid = stp_PidStack->pid_store[stp_PidStack->ul_CurrentEntry];
    }
    return Pid;
}

int GetCurrentChildExitPid() {
struct vms_pid_stack *stp_CurStackStore=stp_PidStack;
int Pid = -1;

    if (stp_CurStackStore != NULL) {
	if (!stp_CurStackStore->ul_CurrentEntry) {
	    if (stp_CurStackStore->stp_PrevStack != NULL) {
		int pid_entry;
		stp_CurStackStore = stp_CurStackStore->stp_PrevStack;
		pid_entry = stp_CurStackStore->ul_CurrentEntry - 1;
		Pid = stp_CurStackStore->pid_store[pid_entry];
	    }
	} else {
	    int pid_entry;
	    pid_entry = stp_CurStackStore->ul_CurrentEntry - 1;
	    Pid = stp_CurStackStore->pid_store[pid_entry];
	}
    }
    return Pid;
}

void ExchangeParentChildPids() {
int parents_last_made_pid,parents_last_asynchronous_pid;

    /* Temporarily save the effective values for the parent using */
    /* local stack storage. */

    parents_last_made_pid = last_made_pid;
    parents_last_asynchronous_pid = last_asynchronous_pid;

    /* Set the current values effective for the fake child that were */
    /* saved earlier in NOJOBS/make_child(). */

    last_made_pid = PopPid();
    last_asynchronous_pid = PopPid();

    /* Save the parent values in the global PID stack for later retrieval */
    /* when vms_restore_bash_environ is called. NOTE: THE VARIABLE        */
    /* parents_last_made_pid MUST ALWAYS BE THE LAST VALUE PUSHED BECAUSE */
    /* THE FUNCTION GetCurrentChildExitPid() RETURNS THIS VALUE WHEN FAKE */
    /* EXITING the child. This is because the parents_last_made_pid value */
    /* is the same pid that needs to be specified when performing fake    */
    /* child exits so that the exit status of the child is correctly made */
    /* to the entry corresponding to the child's pid.                     */

    PushPid(parents_last_asynchronous_pid);
    PushPid(parents_last_made_pid);
}
