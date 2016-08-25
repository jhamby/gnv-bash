/* File: vms_term.c
 *
 * Terminal capabilities based on the Open Group Single Unix Specification,
 * Version 2.
 *
 * This is initially a partial implementation in order to get GNV Bash
 * to build properly on VAX.
 *
 * On Alpha, these functions appear to be provided by curses.h, however
 * testing on OpenVMS Alpha 8.3 show that they do not provide accurate
 * answers and seem to be hardcoded to return settings for a basic
 * VT101 terminal.
 *
 * In addtion, Bash uses the curses input routines in non-window mode.
 * and the existing VAX curses or non-BSD curses routines only operate
 * in window mode, so they clear the screen before reading each keystroke.
 *
 * Based on http://www.gnu.org/software/termutils/manual/termcap-1.3
 * Some input from "The Single UNIX (R) Specifictaion, Version 2" was used
 * but that specification is incomplete and states that the termcap
 * values are being discontinued.
 * More information was found at:
 * http://www.gnu.org/software/termutils/manual/termutils-2.0/
 * html_chapter/tput_1.html#SEC5
 *
 * Copyright 2010, John Malmberg
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
 * 30-Jun-2010	J. Malmberg	Original
 *
 ***************************************************************************/

#include "vms_term.h"

#include <stdlib.h>
#include <string.h>

#ifdef __DECC
#pragma message save
    /* Bugs in smg*.h routines */
#pragma message disable dollarid
    /* Bug in smgdef_upcase.h */
#pragma message disable nestedcomment
    /* Bug in smgdef_upcase.h */
#pragma message disable misalgndstrct
    /* Bug in smgdef_upcase.h */
#endif
#include <smgdef.h>
#include <smgtrmptr.h>
#ifdef __DECC
#pragma message restore
#endif

#include <descrip.h>
#include <efndef.h>
#include <iodef.h>
#include <ssdef.h>
#include <stsdef.h>
#include <ttdef.h>

#ifdef __DECC
/* We are using mulitcharacter constants for readability */
#pragma message disable multichar
/* We are using VMS constants with dollarid */
#pragma message disable dollarid
/* The following is just noise */
#pragma message disable valuepres
#pragma message disable pragma
#pragma message disable hexoctunsign
#endif

#pragma member_alignment save
#pragma nomember_alignment longword

/* 8 bit Control Sequence Introducer */
#define ANSI_CSI ((char)0x9B)
#define ASCII_ESC 0x33

struct sense_st {
    unsigned char class;
    unsigned char type;
    unsigned short term_width;
    unsigned long tt_def;     /* Page length and characteristics */
    unsigned long tt2_def;    /* Extended terminal characteristics */
#if defined (__VMS_VER) && __VMS_VER >=80300000
    unsigned long tt3_def;	/* More extended terminal characteristics */
#endif
};

struct sense_iosb_st {
    unsigned short status;
    unsigned char transmit_speed;
    unsigned char receive_speed;
    unsigned char cr_fill;
    unsigned char lf_fill;
    unsigned char parity_flags;
    unsigned char fill0;
};

struct ttyread_iosb_st {
    unsigned short status;
    unsigned short terminator_offset;
    unsigned short terminator;
    unsigned short terminator_size;
};

#pragma member_alignment restore


int SMG$INIT_TERM_TABLE(const struct dsc$descriptor_s * dev_name,
			unsigned long * vms_termtable_entry);

int SMG$GET_NUMERIC_DATA(const unsigned long * vms_termtable_entry,
			 const unsigned long * request_code,
			 const long * return_value);

int SMG$GET_TERM_DATA(const unsigned long * vms_termtable_entry,
		      const unsigned long * request_code,
		      const long * max_buffer_length,
		      long * return_length,
		      const char * capability_data,
		      const unsigned long *input_args);

int SYS$ASSIGN(
	const struct dsc$descriptor_s * devnam,
	unsigned short * chan,
	unsigned long acmode,
	const struct dsc$descriptor_s * mbxnam,
	unsigned long flags);

int SYS$DASSGN(unsigned short chan);

unsigned long SYS$QIOW(
	unsigned long efn,
	unsigned short chan,
	unsigned long func,
	void * iosb,
	void (* astadr)(void *),
	...);

/* Simple wrapper for SYS$ASSIGN(SYS$OUTPUT, chan) */
static int sys_assign_stdout(unsigned short *channel)
{
const char *device_name = "SYS$OUTPUT:";
struct dsc$descriptor_s dev_desc;
int call_stat;

    /* Assign the channel */
    /*--------------------*/
    dev_desc.dsc$a_pointer = (char *)device_name;
    dev_desc.dsc$w_length = strlen(device_name);
    dev_desc.dsc$b_dtype = DSC$K_DTYPE_T;
    dev_desc.dsc$b_class = DSC$K_CLASS_S;
    call_stat = SYS$ASSIGN(&dev_desc, channel, 0, 0, 0);
    if ($VMS_STATUS_SUCCESS(call_stat)) {
	return 0;
    }

    return -1;
}

/* Simple wrapper for SYS$ASSIGN(SYS$INPUT, chan) */
static int sys_assign_stdin(unsigned short *channel)
{
const char *device_name = "SYS$INPUT:";
struct dsc$descriptor_s dev_desc;
int call_stat;

    /* Assign the channel */
    /*--------------------*/
    dev_desc.dsc$a_pointer = (char *)device_name;
    dev_desc.dsc$w_length = strlen(device_name);
    dev_desc.dsc$b_dtype = DSC$K_DTYPE_T;
    dev_desc.dsc$b_class = DSC$K_CLASS_S;
    call_stat = SYS$ASSIGN(&dev_desc, channel, 0, 0, 0);
    if ($VMS_STATUS_SUCCESS(call_stat)) {
	return 0;
    }
    return -1;
}

/* This is actually dependent on SYSGEN MAXBUF */
/* Q & D - This will always fit, and should handle anything bash needs */
#ifdef __VAX
#define KBUFSIZE 1024
#else
#define KBUFSIZE 2048
#endif
static int kbstrt = 0;
static int kbend = 0;
static char kbuffer[KBUFSIZE];

    /* Need a buffer for CRMODE the default and even in */
    /* nocrmode mode, terminators can be multiple characters */
static char rbuffer[KBUFSIZE];

extern struct vms_kb_st vms_keyboard = {VMS_NOCRMODE, 0, 0};

/* Routine to get a character from standard input, honoring CURSES settings.
 * Just enough here to make bash happy.
 */
int vms_getch(struct vms_kb_st *vms_kb) {

    unsigned long iofunc;
    int ret_char;
    struct ttyread_iosb_st read_iosb;
    int read_limit;
    int i;
    int total_read;
    int status;

    /* Calling macro verified stdin is a terminal */

    /* See if we can just pull a character from the buffer */
    if (kbstrt != kbend) {
	ret_char = kbuffer[kbstrt];

	/* bump to the next character */
	kbstrt++;
	if (kbstrt >= KBUFSIZE) {
	    kbstrt = 0;
	}
	return ret_char;
    }

    /* Buffer is empty, have to read some more characters */
    iofunc = IO$_READVBLK;

    /* Make sure that a channel has been assigned */
    if (vms_kb->termchan_valid == 0) {
	status = sys_assign_stdin(&vms_kb->termchan);
	if (status != 0) {
	    return ERR;
	}
	vms_kb->termchan_valid = 1;
    }

    /* Should Echo be on? */
    if ((vms_kb->flags & VMS_NOECHO) == VMS_NOECHO) {
	iofunc |= IO$M_NOECHO;
    }

    /* Single character mode if VMS_NOCRMODE */
    if ((vms_kb->flags & VMS_NOCRMODE) == VMS_NOCRMODE) {

	/* Buffer in a full line including a terminator */
	read_limit = KBUFSIZE;

    } else {
	/* read a single character or escape terminator */
	read_limit = 1;

	/* Disable ^U, ^R, DEL, line editing */
	iofunc |= IO$M_NOFILTR;
    }

    /* Do the QIOW read of a character from the buffer */
    status = SYS$QIOW(
		EFN$C_ENF,
		vms_kb->termchan,
		iofunc,
		&read_iosb,
		NULL,
		NULL,
		rbuffer,
                read_limit, 0, 0, 0, 0, 0);

    if (!$VMS_STATUS_SUCCESS(status)) {
	return ERR;
    }

    if (!$VMS_STATUS_SUCCESS(read_iosb.status)
	&& (read_iosb.status != SS$_PARTESCAPE)) {
	return ERR;
    }

    /* Copy the keys to the buffer */
    /* Contrary to VMS curses documentation, nonl() is used */
    /* VMS_NONL means do not replace the CR with Line feeds */
    /* Handle the terminator escape sequence */
    /* Bash does not know about 8 bit CSI and such so we need to */
    /* convert them to escape sequences for compatibility */
    i = 0;
    total_read = read_iosb.terminator_offset + read_iosb.terminator_size;
    while (i < total_read) {
        if (rbuffer[i] == ANSI_CSI) {
	    /* Convert it to ESC + [ */
	    kbuffer[kbend] = ASCII_ESC;
	    kbend++;
	    if (kbend >= KBUFSIZE) {
		kbend = 0;
	    }
	    kbuffer[kbend] = '[';
        } else if ((rbuffer[i] == '\r') && ((vms_kb->flags & VMS_NONL) == 0)) {
	    /* Default is to replace a CR with a LF */
	    kbuffer[kbend] = '\n';
	} else {
	    kbuffer[kbend] = rbuffer[i];
	}

	/* Prepare for the next character input */
	kbend++;
	if (kbend >= KBUFSIZE) {
	    kbend = 0;
	}
	i++;
    }

    /* Pull off a character */
    ret_char = kbuffer[kbstrt];

    /* bump to the next character */
    kbstrt++;
    if (kbstrt >= KBUFSIZE) {
	kbstrt = 0;
    }
    return ret_char;
}


static unsigned long _vms_termtable_entry = 0;

/* Check caching for getting terminal characteristics */
static int term_lines = -1;
static int term_width = -1;
static int term_sl_width = -1;

/* Returns the termcap information
 * Called in readline.c - init_termianl_io()
 * Result is used by get_term_capabilities(). where it is parsed by tgetstr()
 * tgetstr does not need to use the contents of buffer
 * So VMS does not need this routine to do anything other than return 1.
 * For now, get the index from SMG$INIT_TERM_TABLE.
 */

int vms_tgetent(char * bp, const char * name) {

    /* This sets up the terminal capabilities */
    int status;
    int ret_stat;
    struct dsc$descriptor_s term_dsc;
    const char *smg_name;

    /* Clear these out */
    _vms_termtable_entry = 0;

    /* New terminal name means cache is invalid */
    term_lines = -1;
    term_width = -1;
    term_sl_width = -1;

    if (name == NULL) {
        return ERR; /* Failure code for this routine */
    }
    term_dsc.dsc$a_pointer = (char *) name;
    term_dsc.dsc$w_length = strlen(name);
    term_dsc.dsc$b_dtype = DSC$K_DTYPE_T;
    term_dsc.dsc$b_class = DSC$K_CLASS_S;

    status = SMG$INIT_TERM_TABLE(&term_dsc, &_vms_termtable_entry);
    switch(status) {
    case SS$_NORMAL:
    case SMG$_GBLSECMAP:
    case SMG$_PRISECMAP:
	ret_stat = 1;  /* All good */
	break;
    case SMG$_UNDTERNAM:
    case SMG$_UNDTERNOP:
    case SMG$_UNDTERNOS:

	/* We do not know this name, but there is a good chance that we */
	/* know what to do, and this hack is because the default */
	/* termtable.txt does not know about xterms, decterm, or putty */
	ret_stat = 0;
	smg_name = NULL;
	if (strncmp("vt", name, 2) == 0) {
	    switch(name[2]) {
	    case '1':  /* VT100 */
		smg_name = "VT101";
		break;
	    case '2':  /* VT220 */
		smg_name = "VT200_Series";
		break;
	    case '3':  /* VT320 */
		smg_name = "VT300_Series";
		break;
	    case '4':  /* VT420 */
		smg_name = "VT400_Series";
		break;
	    case '5':  /* VT520 */
		smg_name = "VT500_Series";
		break;
	    default: /* VT101 */
		smg_name = "VT101";
	    }
	} else {
	    /* This could probably be greatly improved */
	    if (strncmp(name, "putty", 5) == 0) {
		smg_name = "VT400_Series";
	    } else if (strncmp(name, "xterm", 4) == 0) {
		smg_name = "VT400_Series";
	    } else if (strncmp(name, "decterm", 7) == 0) {
		smg_name = "VT300_Series";
	    } else if (strncmp(name, "decwrit", 7) == 0) {
		smg_name = "LA120";
	    }

        }

	/* We have identified a simple alias, so try again */
	if (smg_name != NULL) {
	    term_dsc.dsc$a_pointer = (char *) smg_name;
	    term_dsc.dsc$w_length = strlen(smg_name);
	    status = SMG$INIT_TERM_TABLE(&term_dsc, &_vms_termtable_entry);
	    if ($VMS_STATUS_SUCCESS(status)) {
		ret_stat = OK;
	    }
	}
	break;
    default:
	ret_stat = ERR; /* Something is wrong */
    }

    return ret_stat;
}



/* The tputs() function outputs str to the terminal.
   str is a terminfo string from one of the other terminal routines.

   tputs() in rltty.c outputs to rl_outstream, which readline.c sets to stdout.
   tputs() in readline.c outputs to out_stream,
   which readline.c sets to rl_outstream.

   This may end up being implemented as a macro or a static inline
   routine for performance reasons, as long as the same is done for
   the callback routine, which in the two cases I have seen so far
   is simply a wrapper for the putc macro.
 */
/* int vms_tputs(char *, int, __tputs_callback); */

int vms_tputs(
	char * str,
	int affcnt ,
	vms_tputs_callback my_puts) {

    int i = 0;
    int result;

    /* If the string is null then there is nothing to do */
    if (str == NULL) {
	return ERR;
    }
    while(str[i] != 0) {
	my_puts(str[i]);
	i++;
    }
    return OK;
}

enum smg_actions {SMG_UNKNOWN,	 /* Not a known SMG string */
		 SMG_FLAG,	 /* True if SMG equivalent found or is true */
		 SMG_NEGFLAG,    /* True if SMG equivalent not found or false */
		 SMG_INT,	 /* Returns an integer value */
		 SMG_STRING,	 /* Returns a string constant */
		 SMG_STRING1,	 /* Returns a string with 1 parameter */
		 SMG_STRING2,	 /* Returns a string with 2 parameters */
		 SMG_STRING_NEG, /* True if no string returned */
		 SPECIAL_WIDE2,	 /* Terminal has wide characters - return 2 */
		 ASCII_CHAR,	 /* Returns an ASCII character */
		 SMG_TRUE,	 /* Forced return true with out calling SMG */
		 QIO_READSENSE}; /* Use QIO READSENSE to get value */

/* Translate a termcap code to an SMG$K_ value */
static int vms_get_smg_request_code(
    const char * id,
    unsigned long * smg_code,
    enum smg_actions * smg_action) {

    int ret_stat = SS$_NORMAL;

    /* Make a short selector */
    unsigned short id_code = id[0] + (id[1] * 256);

    *smg_code = 0;
    *smg_action = SMG_UNKNOWN;

    switch(id_code) {
    case 'os':  /* os, over_strike, Terminal can overstrike - boolean */
	*smg_code = SMG$K_OVERSTRIKE;
	*smg_action = SMG_FLAG;
	/* Note not set in VMS predefined terminal types */
	break;
    case 'eo':	/* eo, erase_overstrike, Space erases overstrike if present */
	break;
    case 'gn':  /* gn, generic_type, Terminal is generic */
	/* Not exactly accurate, but probably close enough */
	*smg_code = SMG$K_ANSI_CRT;
	*smg_action = SMG_NEGFLAG;
	break;
    case 'hc':  /* hc, hard_copy, Hardcopy terminal */
	*smg_code = SMG$K_SCOPE;  /* Need to invert */
	*smg_action = SMG_NEGFLAG;
	break;
    case 'rp':	/* String of commands to output repeating character */
    case 'hz':	/* hz, tilde_glitch, Can not output tilde -Hazeltine- */
    case 'CC':	/* cmdch, command_character, Terminal settable cmd character */
    case 'xb':	/* xsg, no_esc_ctlc, */
		/* Superbee terminals without ESC or Control-C */
	/* VMS termtable does not track these items */
	break;
    case 'co':  /* cols, columns, Width of screen in character cells */
	*smg_code = SMG$K_COLUMNS;
	*smg_action = QIO_READSENSE;
	break;
    case 'li':  /* lines, lines, Number of rows or lines */
	*smg_code = SMG$K_ROWS;
	*smg_action = QIO_READSENSE;
	break;
    case 'cm':  /* cup, cursor_address, Absolute cursor move */
	*smg_code = SMG$K_SET_CURSOR_ABS;
	*smg_action = SMG_STRING2;
	break;
    case 'ho':  /* home, cursor_home, Home cursor */
	*smg_code = SMG$K_HOME;
	*smg_action = SMG_STRING;
	break;
    case 'll':  /* ll, cursor_to_ll, Move to lower left */
	break;
    case 'cr':  /* cr, carriage_return, Move cursor to beginning of line */
		/* GNU says to only define if not ASCII CR */
		/* ncurses sets to \r */
	*smg_code = '\r';
	*smg_action = ASCII_CHAR;
	break;
    case 'le':  /* cubl, cursor_left, Cursor move left 1 */
		/* if 'bw' set 'le' can backup past start of line */
	*smg_code = SMG$K_CURSOR_LEFT;
	*smg_action = SMG_STRING;
	break;
    case 'bc':  /* Very obsolete form of 'le' */
    case 'bs':  /* Backspace can do cursor left - obsoleted by 'le' */
	*smg_code = SMG$K_BACKSPACE;
	*smg_action = SMG_FLAG;
	break;
    case 'nd':  /* cufl, cursor right, Cursor move right */
	*smg_code = SMG$K_CURSOR_RIGHT;
	*smg_action = SMG_STRING;
	break;
    case 'up':  /* cuul, cursor_up, cursor move up */
	*smg_code = SMG$K_CURSOR_UP;
	*smg_action = SMG_STRING;
	break;
    case 'do':  /* cudl, cursor_down, cursor down */
	*smg_code = SMG$K_CURSOR_DOWN;
	*smg_action = SMG_STRING;
	break;
    case 'bw':  /* bw, auto_left_margin */
		/* le can backup to previous line - need to check */
	break;
    case 'nw':  /* Move to start of next line, possibly clearing it */
	*smg_code = SMG$K_NEXT_LINE;
	*smg_action = SMG_STRING;
	break;
    case 'DO':  /* Down n rows */
	*smg_code = SMG$K_CURSOR_DOWN;
	*smg_action = SMG_STRING1;
	break;
    case 'UP':  /* Up n rows */
	*smg_code = SMG$K_CURSOR_UP;
	*smg_action = SMG_STRING1;
	break;
    case 'LE':  /* Left n columns */
	*smg_code = SMG$K_CURSOR_LEFT;
	*smg_action = SMG_STRING1;
	break;
    case 'RI':  /* Right n columns */
	*smg_code = SMG$K_CURSOR_RIGHT;
	*smg_action = SMG_STRING1;
	break;
    case 'CM':	/* mrcpu, cursor_mem_address */
		/* Gnu: Relative move - Not sure about this */
		/* X/open: Memory relative cursor addressing */
    case 'ch':	/* Move to column N in same row - Not sure */
    case 'cv':	/* Move to row N of the same column - Not sure */
	break;
    case 'sc':  /* Save cursor */
	*smg_code = SMG$K_SAVE_CURSOR;
	*smg_action = SMG_STRING;
	break;
    case 'rc':  /* Restore cursor */
	*smg_code = SMG$K_RESTORE_CURSOR;
	*smg_action = SMG_STRING;
	break;
    case 'ff':  /* ff, form_feed, Form Feed */
	*smg_code = '\f';
	*smg_action = ASCII_CHAR;
	break;
    case 'ta':	/* Move to next tab stop if not expanding tabs */
		/* Do we want to look up a per terminal setting? */
    case 'bt':	/* cbt, back_tab, Move to prev tab stop if not expanding tabs */
    case 'nc':	/* Terminal does not support CR */
    case 'xt':	/* xt, dest_tabs_magic_smso */
		/* This is a Teleray 1061 - do not use TAB chars. */
		/* Cursor can not be positioned in front of magic cookie */
	break;
    /* case 'bc' and 'bs' done earlier - repeated in GNU docs in this order */
    case 'nl':  /* New line character if \n does not do the job */
	break;
    case 'am':	/* am, auto_right_margin */
		/* Write to last column causes wrap */
	*smg_code = SMG$K_ANSI_CRT;
	*smg_action = SMG_FLAG;
	break;
    case 'xn':  /* xenl, eat_newline_glitch */
    case 'LP':	/* Dec type of wrapping - considered strange */
	*smg_code = SMG$K_DEC_CRT;
	*smg_action = SMG_FLAG;
	break;
    case 'sf':  /* Scroll up */
	*smg_code = SMG$K_SCROLL_FORWARD;
	*smg_action = SMG_STRING;
	break;
    case 'SF':
	*smg_code = SMG$K_SCROLL_FORWARD;
	*smg_action = SMG_STRING1;
	break;
    case 'sr':  /* Scroll down */
	*smg_code = SMG$K_SCROLL_REVERSE;
	*smg_action = SMG_STRING;
	break;
    case 'SR':  /* Scroll down N */
	*smg_code = SMG$K_SCROLL_REVERSE;
	*smg_action = SMG_STRING1;
	break;
    case 'cs':  /* csr, change_scroll_region, change to lines #1 through 2 */
		/* (VT100) */
	*smg_code = SMG$K_SET_SCROLL_REGION;
	*smg_action = SMG_STRING2;
	break;
    case 'cS':  /* Ann Arbor Ambassador set scroll region */
    case 'ns':	/* Terminal does not scroll, starts over at top */
	break;
    case 'da':  /* da, memory_above */
		/* lines may come back from scrolling - assume yes */
    case 'db':  /* db, memory_below, Lines may come back from scrolling */
	*smg_code = 1;
	*smg_action = SMG_TRUE;
	break;
    case 'lm':  /* lm, lines_of_memory, Terminal memory */
		/* VMS termtable does not track */
    case 'wi':	/* Terminal supports windows, not implementing now */
	break;
    case 'cl':  /* clear, clear_screen, clear entire screen */
	*smg_code = SMG$K_ERASE_WHOLE_DISPLAY;
	*smg_action = SMG_STRING;
	break;
    case 'cd':  /* ed, clr_eos, clear to end display */
	*smg_code = SMG$K_ERASE_TO_END_DISPLAY;
	*smg_action = SMG_STRING;
	break;
    case 'ce':  /* el, crl_eol, Clear to end of line */
	*smg_code = SMG$K_ERASE_TO_END_LINE;
	*smg_action = SMG_STRING;
	break;
    case 'ec':  /* ech, erase_chars, Clear n characters */
	break;
    case 'al':  /* ill, insert_line, Insert blank line before cursor */
	*smg_code = SMG$K_INSERT_LINE;
	*smg_action = SMG_STRING;
	break;
    case 'AL':  /* Insert blank lines before cursor */
	*smg_code = SMG$K_INSERT_LINE;
	*smg_action = SMG_STRING1;
	break;
    case 'dl':  /* dll, delete_line, Delete line */
	*smg_code = SMG$K_DELETE_LINE;
	*smg_action = SMG_STRING;
	break;
    case 'DL': /* Delete lines */
	*smg_code = SMG$K_DELETE_LINE;
	*smg_action = SMG_STRING1;
	break;
    case 'im':  /* smir, enter_insert_mode, Start insert mode */
	*smg_code = SMG$K_BEGIN_INSERT_MODE;
	*smg_action = SMG_STRING;
	break;
    case 'ei':  /* rmir, end_insert_mode, End insert mode */
	*smg_code = SMG$K_END_INSERT_MODE;
	*smg_action = SMG_STRING;
	break;
    case 'ic': /* ichl, insert_character, Insert character */
	*smg_code = SMG$K_INSERT_CHAR;
	*smg_action = SMG_STRING;
	break;
    case 'IC':	/* GNU: Insert characters */
		/* X/Open: initc, initialize_color, set color #1 to RGB 2,3,4 */
		/* Will use GNU definition here */
	*smg_code = SMG$K_INSERT_CHAR;
	*smg_action = SMG_STRING1;
	break;
    case 'ip':  /* ip, insert_padding, Insert padding */
	*smg_code = SMG$K_INSERT_PAD;
	*smg_action = SMG_STRING;
	break;
    case 'mi':  /* mir, move_insert_mode, */
		/* Safe to move in insert mode - Don't know */
	*smg_code = SMG$K_END_INSERT_MODE;
	*smg_action = SMG_STRING;
	break;
    case 'in':	/* in, insert_null_glitch */
		/* Terminal distinguishes between space and cleared cells */
	break;
    case 'dc':  /* dchl, delete_character, Delete one character */
	*smg_code = SMG$K_DELETE_CHAR;
	*smg_action = SMG_STRING;
	break;
    case 'DC':  /* Delete n characters */
	*smg_code = SMG$K_DELETE_CHAR;
	*smg_action = SMG_STRING1;
	break;
    case 'dm':  /* smdc, enter_delete_mode, Enter delete mode */
	*smg_code = SMG$K_BEGIN_DELETE_MODE;
	*smg_action = SMG_STRING;
	break;
    case 'ed':  /* rmdc, exit_delete_mode, End delete mode */
	*smg_code = SMG$K_END_DELETE_MODE;
	*smg_action = SMG_STRING;
	break;
    case 'so':  /* smso, enter_standout_mode, */
		/* enter Standout mode - VMS does not define */
	break;
    case 'se':  /* rmso, exit_standout_mode, end standout mode */
	*smg_code = SMG$K_BEGIN_NORMAL_RENDITION;
	*smg_action = SMG_STRING;
	break;
    case 'ms':  /* msgr, move_standout_mode, */
		/* Safe to move cursor in appearance mode - Unknown */
	break;
    case 'xs':  /* xhp, ceol_standout_glitch, Reset by clearing line */
    case 'mb':  /* blink, enter_blink_mode, Begin blink */
	*smg_code = SMG$K_BEGIN_BLINK;
	*smg_action = SMG_STRING;
	break;
    case 'md':   /* bold, enter_bold_mode, Begin bold */
	*smg_code = SMG$K_BEGIN_BOLD;
	*smg_action = SMG_STRING;
	break;
    case 'mh':   /* dim, enter_dim_mode, Begin dim */
    case 'mk':   /* invis, enter_secure_mode, Begin invisible */
	break;
    case 'mp':   /* prot, enter_protected_mode, Enter protected mode */
	*smg_code = SMG$K_SET_CHAR_NOT_SEL_ERASE;
	*smg_action = SMG_STRING;
	break;
    case 'mr':   /* rev, enter_reverse_mode, Begin reverse */
	*smg_code = SMG$K_BEGIN_REVERSE;
	*smg_action = SMG_STRING;
	break;
    case 'me':   /* sgr0, exit_attribute_mode, End all appearance modes */
	*smg_code = SMG$K_BEGIN_NORMAL_RENDITION;
	*smg_action = SMG_STRING;
	break;
    case 'as':	/* smacs, enter_alt_charset_mode, */
		/* Begin Alternate character set mode */
	*smg_code = SMG$K_BEGIN_ALTERNATE_CHAR;
	*smg_action = SMG_STRING;
	break;
    case 'ae':	/* rmacs, exit_alt_charset_mode, */
		/*End Alternate character set mode */
	*smg_code = SMG$K_END_ALTERNATE_CHAR;
	*smg_action = SMG_STRING;
	break;
    case 'sa':   /* Start multiple appearance modes */
	break;
    case 'us':   /* smul, enter_underline_mode, Start underline mode */
	*smg_code = SMG$K_BEGIN_UNDERSCORE;
	*smg_action = SMG_STRING;
	break;
    case 'ue':   /* rmul, exit_underline_mode, End underline mode */
	*smg_code = SMG$K_END_UNDERSCORE;
	*smg_action = SMG_STRING;
	break;
    case 'ug':   /* Width of magic cookie */
	break;
    case 'uc':   /* Underline single character */
	*smg_code = SMG$K_UNDERLINE_CHAR;
	*smg_action = SMG_STRING;
	break;
    case 'ul':   /* ul, transparent_underline, Underline by overstrike */
	*smg_code = SMG$K_OVERSTRIKE;
	*smg_action = SMG_STRING;
	break;
    case 'vs':   /* cvvis, cursor visible, Enhance cursor */
	break;
    case 'vi':   /* civis, cursor_invisible, Cursor invisible */
	*smg_code = SMG$K_SET_CURSOR_OFF;
	*smg_action = SMG_STRING;
	break;
    case 've':   /* cnorm, cursor_normal, Cursor normal */
	*smg_code = SMG$K_SET_CURSOR_ON;
	*smg_action = SMG_STRING;
	break;
    case 'bl':   /* bel, bell, Audible signal (bell) */
	*smg_code = 0x07;
	*smg_action = ASCII_CHAR;
	break;
    case 'vb':   /* Flash Screen */
	break;
    case 'ks':   /* Keypad mode on */
	*smg_code = SMG$K_SET_APPLICATION_KEYPAD;
	*smg_action = SMG_STRING;
	break;
    case 'ke':   /* Numeric keypad */
	*smg_code = SMG$K_SET_NUMERIC_KEYPAD;
	*smg_action = SMG_STRING;
	break;
    case 'kl':   /* left arrow key */
	*smg_code = SMG$K_KEY_LEFT_ARROW;
	*smg_action = SMG_STRING;
	break;
    case 'kr':   /* Right arrow key */
	*smg_code = SMG$K_KEY_RIGHT_ARROW;
	*smg_action = SMG_STRING;
	break;
    case 'ku':   /* Up arrow key */
	*smg_code = SMG$K_KEY_UP_ARROW;
	*smg_action = SMG_STRING;
	break;
    case 'kd':   /* kcudl, key_down, Down arrow key */
	*smg_code = SMG$K_KEY_DOWN_ARROW;
	*smg_action = SMG_STRING;
	break;
    case 'kh':   /* Home key */
	break;
    case 'K1':  /* ka1, key_al Edit key array, upper left*/
	*smg_code = SMG$K_KEY_E1;
	*smg_action = SMG_STRING;
	break;
    case 'K2':  /* kb2, key_b2, Edit key array, center */
	*smg_code = SMG$K_KEY_E2;
	*smg_action = SMG_STRING;
	break;
    case 'K3':  /* ka3, key_a3, Edit key array, upper right */
	*smg_code = SMG$K_KEY_E3;
	*smg_action = SMG_STRING;
	break;
    case 'K4':  /* kc1, key_c1, Edit key array, lower left */
	*smg_code = SMG$K_KEY_E4;
	*smg_action = SMG_STRING;
	break;
    case 'K5':  /* kc3, key_c3, Edit key array, lower right */
	*smg_code = SMG$K_KEY_E5;
	*smg_action = SMG_STRING;
	break;
    case 'k0': /* kf0, key_f0, Function Key 0 or 10 */
	*smg_code = SMG$K_KEY_F10;
	*smg_action = SMG_STRING;
	break;
    case 'k1': /* Function keys */
	*smg_code = SMG$K_KEY_F1;
	*smg_action = SMG_STRING;
	break;
    case 'k2': /* Function keys */
	*smg_code = SMG$K_KEY_F2;
	*smg_action = SMG_STRING;
	break;
    case 'k3': /* Function keys */
	*smg_code = SMG$K_KEY_F3;
	*smg_action = SMG_STRING;
	break;
    case 'k4': /* Function keys */
	*smg_code = SMG$K_KEY_F4;
	*smg_action = SMG_STRING;
	break;
    case 'k5': /* Function keys */
	*smg_code = SMG$K_KEY_F5;
	*smg_action = SMG_STRING;
	break;
    case 'k6': /* Function keys */
	*smg_code = SMG$K_KEY_F6;
	*smg_action = SMG_STRING;
	break;
    case 'k7': /* Function keys */
	*smg_code = SMG$K_KEY_F7;
	*smg_action = SMG_STRING;
	break;
    case 'k8': /* Function keys */
	*smg_code = SMG$K_KEY_F8;
	*smg_action = SMG_STRING;
	break;
    case 'k9': /* Function keys */
	*smg_code = SMG$K_KEY_F9;
	*smg_action = SMG_STRING;
	break;
	/* X/Open defines up to 63 keys VMS knows about 20. */
	/* It gives an end sequence of 'Fq', 'Fr', but not a start */
	/* Counting backwards, indicates F10 should be 'F=' */
	/* GNU tput documents these */
    case 'k;': /*kf10 - Function keys */
	*smg_code = SMG$K_KEY_F10;
	*smg_action = SMG_STRING;
	break;
    case 'F1': /* F11 - Function keys */
	*smg_code = SMG$K_KEY_F11;
	*smg_action = SMG_STRING;
	break;
    case 'F2': /* F12 - Function keys */
	*smg_code = SMG$K_KEY_F12;
	*smg_action = SMG_STRING;
	break;
    case 'F3': /* F13 - Function keys */
	*smg_code = SMG$K_KEY_F13;
	*smg_action = SMG_STRING;
	break;
    case 'F4': /* F14 - Function keys */
	*smg_code = SMG$K_KEY_F14;
	*smg_action = SMG_STRING;
	break;
    case 'F5': /* F15 - Function keys */
	*smg_code = SMG$K_KEY_F15;
	*smg_action = SMG_STRING;
	break;
    case 'F6': /* F16 - Function keys */
	*smg_code = SMG$K_KEY_F16;
	*smg_action = SMG_STRING;
	break;
    case 'F7': /* F17 - Function keys */
	*smg_code = SMG$K_KEY_F17;
	*smg_action = SMG_STRING;
	break;
    case 'F8': /* F18 - Function keys */
	*smg_code = SMG$K_KEY_F18;
	*smg_action = SMG_STRING;
	break;
    case 'F9': /* F19 - Function keys */
	*smg_code = SMG$K_KEY_F19;
	*smg_action = SMG_STRING;
	break;
    case 'FA': /* F20 - Function keys */
	*smg_code = SMG$K_KEY_F20;
	*smg_action = SMG_STRING;
	break;
    case 'kn': /* Number of function keys */
	*smg_code = SMG$K_NUMBER_FN_KEYS;
	*smg_action = SMG_INT;
	break;
    case 'l0': /* Keyboard function key labels */
	*smg_code = SMG$K_LABEL_F10;
	*smg_action = SMG_STRING;
	break;
    case 'l1':
	*smg_code = SMG$K_LABEL_F1;
	*smg_action = SMG_STRING;
	break;
    case 'l2':
	*smg_code = SMG$K_LABEL_F2;
	*smg_action = SMG_STRING;
	break;
    case 'l3':
	*smg_code = SMG$K_LABEL_F3;
	*smg_action = SMG_STRING;
	break;
    case 'l4':
	*smg_code = SMG$K_LABEL_F4;
	*smg_action = SMG_STRING;
	break;
    case 'l5':
	*smg_code = SMG$K_LABEL_F5;
	*smg_action = SMG_STRING;
	break;
    case 'l6':
	*smg_code = SMG$K_LABEL_F6;
	*smg_action = SMG_STRING;
	break;
    case 'l7':
	*smg_code = SMG$K_LABEL_F7;
	*smg_action = SMG_STRING;
	break;
    case 'l8':
	*smg_code = SMG$K_LABEL_F8;
	*smg_action = SMG_STRING;
	break;
    case 'l9':
	*smg_code = SMG$K_LABEL_F9;
	*smg_action = SMG_STRING;
	break;
	break;
    case 'kH':  /* Home key.  Home on PC, Find on VT/LKxxx keyboard */
	*smg_code = SMG$K_KEY_E1;
	*smg_action = SMG_STRING;
	break;
    case 'kb':  /* kbs, key_backspace, Backspace key sends this */
	*smg_code = SMG$K_KEY_BACKSPACE;
	*smg_action = SMG_STRING;
	break;
    case 'kB':	/* kcbt, key_btab, back tab key - Not in GNU */
    case 'ka':	/* ktbc, key_catab, Clear all tabs key */
    case 'kt':  /* kctab, key_ctab, Clear tab stop key */
    case 'kC':  /* kclr, key_clear, Clear screen key */
	break;
    case 'kD':  /* kdchl, key_dc, Delete Character key */
	*smg_code = SMG$K_KEY_E2;
	*smg_action = SMG_STRING;
	break;
    case 'kL':  /* kdll, key_dl, Delete Line key */
    case 'kM':  /* krmir, key_eic, Exit insert mode key */
    case 'kE':  /* kel, key_eol, clear to end of line key */
    case 'kS':  /* ked, key_oes, clear to end of screen key */
	break;
    case 'kI':  /* kill, Insert mode or Insert character key */
	*smg_code = SMG$K_KEY_E2;
	*smg_action = SMG_STRING;
	break;
    case 'kA':  /* Insert line key */
	break;
    case 'kN':  /* Next page */
	*smg_code = SMG$K_KEY_E6;
	*smg_action = SMG_STRING;
	break;
    case 'kP':  /* Previous Page */
	*smg_code = SMG$K_KEY_E5;
	*smg_action = SMG_STRING;
	break;
    case 'kF':  /* Scroll forward */
    case 'kR':  /* Scroll reverse */
    case 'kT':  /* Set tab stop */
    case 'ko':  /* Other functions */
    case 'km':  /* km, has_meta_key */
		/* Terminal has a Meta key - xterm or putty? */
    case 'mm':  /* Enable Meta key */
    case 'mo':  /* Disable Meta key */
	break;
    case 'ti':  /* smcup, enter_ca_mode, Terminal init string */
	*smg_code = SMG$K_INIT_STRING;
	*smg_action = SMG_STRING;
	break;
    case 'te':  /* rmcup, exit_ca_mode, Terminal un-init string */
    case 'is':  /* is2, init_2_string, Per login terminal init string */
    case 'if':  /* if, init_file, Terminal init file */
    case 'i1':  /* is1, init_1string, init before 'is' */
    case 'i3':  /* is3, init_3_string, init after 'is' */
    case 'rs':  /* Terminal reset */
    case 'it':  /* it, init_tabs, Initial tab stop */
    case 'ct':  /* tbc, clear_all_tabs, clear all tab stops */
	break;
    case 'st':  /* Tab stop */
	*smg_code = SMG$K_SET_TAB;
	*smg_action = SMG_STRING;
	break;
    case 'NF':  /* Does not support XON/XOFF */
    case 'pb':  /* pb, padding_baud_rate, Lowest baud rate needing padding */
	break;
    case 'pc':  /* Pad character */
	*smg_code = SMG$K_PAD_CHAR;
	*smg_action = SMG_STRING;
	break;
    case 'dC':  /* CR padding */
    case 'dN':  /* LF padding */
    case 'dB':  /* BS padding */
    case 'dF':  /* FF padding */
    case 'dT':  /* TAB padding */
	break;
    case 'hs':  /* hs, has_status_line, Has status line */
	/* Use begin status line to see if available */
	*smg_code = SMG$K_BEGIN_STATUS_LINE;
	*smg_action = SMG_STRING;
	break;
    case 'ts':  /* Start status line */
	*smg_code = SMG$K_BEGIN_STATUS_LINE;
	*smg_action = SMG_STRING;
	break;
    case 'fs':  /* fsl, from_status_line, End status line */
	*smg_code = SMG$K_END_STATUS_LINE;
	*smg_action = SMG_STRING;
	break;
    case 'ds':  /* dsl, dis_status_line, Disable status line */
	break;
    case 'ws':  /* wsl, width_status_line, Width of status line */
	/* Valid if SMG$K_BEGIN_STATUS_LINE returns a string */
	*smg_code = SMG$K_COLUMNS;
	*smg_action = QIO_READSENSE;
	break;
    case 'hu':  /* Half line scroll up */
    case 'hd':  /* hd, down_half_line, Half line scroll down */
	break;
    case 'ps':  /* Print screen */
	*smg_code = SMG$K_PRINT_SCREEN;
	*smg_action = SMG_STRING;
	break;
    case 'po':  /* Send output to printer */
	*smg_code = SMG$K_SET_PRINTER_OUTPUT;
	*smg_action = SMG_STRING;
	break;
    case 'pf':  /* Send output to screen */
	*smg_code = SMG$K_SET_SCREEN_OUTPUT;
	*smg_action = SMG_STRING;
	break;
    case 'p0':  /* Send n characters to printer */
	break;
    /* Termcap codes from X-OPEN below */
    case 'ut':  /* bce, back_color_erase, Screen erased with background color */
    case 'cc':  /* ccc, can_change, terminal can re-define existing color */
    case 'YA':  /* xhpa, col_addr_glitch, Only pos. motion for hpa/mhpa caps */
    case 'YF':  /* cpix, cpi_changes_res, Chang char pitch changes res. */
    case 'YB':  /* crxm, cr_cancels_micro_mode, Using cr turns off micro mode */
    case 'HC':  /* chts, hard_cursor, Cursor is hard to see */
    case 'YC':  /* daisy, has_print_wheel, Printer needs operator to change */
		/* character set */
    case 'hl':  /* hls, hue_lightness_saturation, */
		/* Terminal uses only HLS color notation - Tektronix */
		/* VT34x also have HLS color notation */
    case 'YG':  /* lpix, lpi_changes_res, Changing line pitch changes res. */
    case 'nx':  /* nxon, needs_xon_xoff, Padding won't work, xon/xoff req. */
	break;
    case 'NP':  /* npc, no_pad_char, Pad char doesn't exist */
	*smg_code = SMG$K_PAD_CHAR;
	*smg_action = SMG_STRING_NEG;
	break;
    case 'ND':  /* ndscr, non_dest_scroll_region */
		/* Scrolling region is non-destructive */
    case 'NR':  /* nrrmc, non_rev_mcup, smcup does not reverse rmcup */
    case '5i':  /* mc5i, ptr_silent, Printer won't echo on screen */
    case 'YD':  /* xvpa, row_addr_glitch, Only pos motion for vpa/mvpa caps */
    case 'YE':  /* sam, semi_auto_right_margin */
		/* Printing in last column causes cr */
	break;
    case 'es':  /* eslok, status_line_esc_ok */
		/* Escape can be used on the status line */
		/* Assume yes if have status line */
	*smg_code = SMG$K_BEGIN_STATUS_LINE;
	*smg_action = SMG_STRING;
	break;
    case 'xo':  /* xon, xon_xoff, Terminal uses xon/xoff handshaking */
    case 'Yo':  /* bitwin, bit_image_entwining */
		/* Number of passes for each bit-map row */
    case 'Yp':  /* bittype, bit_image_type, Type of bit image device */
    case 'BT':  /* btns, buttons, Number of buttons on the mouse */
    case 'Yc':  /* spinh, dot_horz_spacing */
		/* Spacing of dots horizontally in dots per inch */
    case 'Yb':  /* spinv, dot_vert_spacing */
		/* Spacing of pins vertically iin pins per inch */
    case 'lh':  /* lh, label_height, Number of rows in each label */
    case 'lw':  /* lw, label_width, Number of columns in each label */
    case 'ma':  /* ma, max_attributes, Maximim combined video attributes */
		/* that the terminal can display */
    case 'sg':  /* xmc, magic_cookie_glitch */
		/* Number of blank characters left by smso or rmso */
    case 'Co':  /* colors, max_colors, Maxium number of colors on the screen */
    case 'Yd':  /* maddr, max_micro_address */
		/* Maximum value in micro_..._address */
    case 'Ye':  /* mjump, max_micro_jump */
		/* Maximum value in parm_..._micro */
    case 'pa':  /* pairs, max_pairs, Maximum number of color-pairs on screen */
    case 'MW':  /* wnum, maximum_windows, Max number of definable windows */
    case 'Yf':  /* mcs, micro_col_size, Char. step size when in micro mode */
    case 'Yg':  /* mls, micro_line_size, Line step size when in micro mode */
    case 'NC':  /* ncv, no_color_video, Video attritues that can't be used */
		/* with colors */
    case 'Nl':  /* nlab, num_labels, Number of labels on screen, start at 1 */
    case 'Yh':  /* npins, number_of_pins, Number of pins in print-head */
    case 'Yi':  /* orc, output_res_char */
		/* Horizontal resolution in units per character. */
    case 'Yj':  /* orl, output_rel_line */
		/* Vertical resolution in units per inch. */
    case 'Yk':  /* orhi, output_res_horz_inch */
		/* Horizontal resolution in units per inch */
    case 'Yl':  /* orvi, output_res_vert_inch */
		/* Vertical resolution in units per inch */
    case 'Ym':  /* cps, print_rate, Print rate in characters per second */
    case 'vt':  /* vt, virtual_terminal, Virtual terminal number */
	break;
    case 'Yn':  /* widcs, Yn, Character step size when in double wide mode */
	*smg_code = SMG$K_DOUBLE_WIDE;
	*smg_action = SPECIAL_WIDE2;
	break;
    case 'ac':  /* acsc, acs_chars, Graphic charset pairs aAbBcC */
    case 'S8':  /* scesa, alt_scancode_esc, alternate escape for scancode */
		/* emulation.  (Default is for VT100) */
    case 'Yv':  /* bicr, bit_image_crriage_return */
		/* Move to beginning of same row */
    case 'Zz':  /* binel, bit_image_newline */
		/* Move to next row of the bit image */
    case 'Xy':	/* birep, bit_image_repeat, repeat bit-image cell #1 #2 times */
    case 'ZA':	/* cpi, change_char_pitch, */
		/* change number of characters per inch */
    case 'ZB':  /* lpi, change_line_pitch, change number of lines per inch */
    case 'ZC':  /* chr, change_res_horz, change horizontal resolution */
    case 'ZD':  /* cvr, change_res_vert, change vertical resolution */
    case 'rP':  /* rmp, char_padding, like 'ip' but when in replace mode */
    case 'Zy':  /* csnm, char_set_names, */
		/* Returns a list of character set names */
    case 'MC':	/* mgc, clear_margins, Clear all margins (top, bottom, sides) */
    case 'cb':	/* ell, clr_bol, clear to beginning of line, inclusive */
    case 'ci':	/* csin, code_set)init, Init sequence for multiple codesets */
    case 'Yw':	/* colornm, color_names, give name for color #1 */
    case 'Yx':	/* defbi, define_bit_image_region */
		/* Define retangular bit-image region */
    case 'ZE':	/* defc, define_char, Define a character in a character set */
    case 'dv':	/* devt, device_type, Indicate language/codeset support */
    case 'DI':	/* dial, dial_phone, Dial phone number #1 */
    case 'DK':	/* dclk, display_clock, display time-of-day clock */
    case 'S1':	/* dispc, display_pc_char, Display PC character */
    case 'eA':	/* enacs, ena_acs, Enable alternate character set */
    case 'Yy':	/* endbi, Enable alternate character set */
    case 'SA':	/* smam, enter am mode, Turn on automatic margins */
	break;
    case 'ZF':	/* swidm, enter_doublewide_mode, double wide printing */
	*smg_code = SMG$K_DOUBLE_WIDE,
	*smg_action = SMG_STRING;
	break;
    case 'ZG':	/* sdrfq, enter_draft_quality, set draft quality print */
    case 'ZH':	/* sitm, enter_italics_mode, enable italics */
    case 'ZI':	/* slm, enter_leftward_mode, Enable leftward carriage motion */
    case 'ZJ':	/* smicm, enable_micro_mode, Enable micro motion capabilities*/
    case 'S2':	/* smpch, enter_pc_charset_mode, */
		/* Enter PC character display mode */
    case 'S4':	/* smsc, enter_scancode_mode, Enter PC scancode mode */
    case 'ZM':	/* sshm, enter_shadow_mode, Enable shadow printing */
    case 'ZN':	/* ssubm, enter_subscript_mode, Enable supscript printing */
    case 'ZO':	/* ssubm, enter_superscript_mode, enable superscript printing */
    case 'ZP':	/* sum, enter_upward_mode, Enable upward carriage modtion */
    case 'SX':	/* smxon, enter_xon_mode, Turn on xon/xoff handshaking */
    case 'RA':	/* rmam, exit_am_mode, Turn off automatic margins */
	break;
    case 'ZQ':	/* rwidm, exit_doublewide_mode, disable doublewide printing */
	*smg_code = SMG$K_SINGLE_HIGH;
	*smg_action = SMG_STRING;
	break;
    case 'ZR':	/* ritm, exit_italics_mode, disable italics */
    case 'ZS':	/* rlm, exit_leftward_mode, Rightward cursor motion (normal) */
    case 'ZT':	/* rmicm, exit_micro_mode, Disable micro motion capabilities */
    case 'S3':	/* rmpch, exit_pc_charset_mode, Disable PC character display */
    case 'S5':	/* rmsc, exit_scancode_mode, Disable PC scancode mode */
    case 'ZU':	/* rshm, exit_shadow_mode, Disable shadow printing */
    case 'ZV':	/* rsubm, exit_subscript_mode, Disable subscript printing */
    case 'ZW':	/* rsupm, exit_superscript_mode, Disable superscript printing */
    case 'ZX':	/* rum, exit_upward_mode, enable downward (normal) carriage */
    case 'RX':	/* rmxon, exit_xon_mode, Turn off xon/xoff handshaking */
    case 'PA':	/* pause, fixed_pause, pause for 2-3 seconds */
    case 'fh':	/* hook, flash_hook, flash the switch hook */
    case 'Gm':	/* getm, get_mouse, Curses should get button events */
    case 'WG':	/* wingo, goto_window, go to window #1 */
    case 'HU':	/* hup, hangup, hang up phone */
    case 'iP':	/* iprog, init_prog, Path name of program for initization */
    case 'Ip':	/* initp, initialize pair, set color pair 1 to fg 2 to fg 3 */
	break;
    case '@8':	/* Enter/send key */
	*smg_code = SMG$K_TRM_ENTER;
	*smg_action = SMG_STRING;
	break;
    case '@0':	/* kfnd, Find key */
	*smg_code = SMG$K_TRM_FIND;
	*smg_action = SMG_STRING;
	break;
    case '%1':	/* khlp, Help key */
	*smg_code = SMG$K_TRM_HELP;
	*smg_action = SMG_STRING;
	break;
    case '*6':	/* kslt, Select key */
	*smg_code = SMG$K_TRM_SELECT;
	*smg_action = SMG_STRING;
	break;
    /* case 'MT': */ /* Set top and bottom margins */
	/* This is odd, Bash source code indicates that this is flag */
	/* indicating that the terminal has Meta keys.*/
	/* This is not documented in GNU term */
	/* X/Open says Set top and bottom margins */
	/* So just default to returning as not present */
	break;
    default:
	return SS$_BADPARAM;
    }

    /* Let caller know we found something */
    if (*smg_code != 0) {
	return SS$_NORMAL;
    }

    return SS$_BADPARAM;
}


/* Buffer used by tgoto for return values */
#define VMS_TERM_BUFFER_LEN 1000 /* Per X/Open xcurses/terminal.html */
static char _vms_tgoto_buffer[VMS_TERM_BUFFER_LEN + 1];

/* Return the escape sequence to move to specific position */
char * vms_tgoto(char * id, int col, int row) {

    int status;
    unsigned long request_code;
    int ret_stat;
    unsigned long input[3];
    long return_length;
    const long buffer_len = VMS_TERM_BUFFER_LEN;
    enum smg_actions smg_action;

    if ((_vms_termtable_entry == 0) || (id == NULL)) {
	return NULL;
    }

    /* The right way to do this is to lookup the mapping */
    status = vms_get_smg_request_code(id, &request_code, &smg_action);
    if (!$VMS_STATUS_SUCCESS(status))
	return NULL;

    /* We assume that SMG will do the correct thing, so ignore the vms_UP
     * and vms_BC that may be stored.
     */

    input[0] = 0;
    switch (smg_action) {
    case SMG_STRING2:
	input[0]++;
	input[2] = col;
    case SMG_STRING1:
	input[0]++;
	input[1] = row;
	break;
    default:
	return NULL;
    }

    /* Get the string for the terminal */
    status = SMG$GET_TERM_DATA(&_vms_termtable_entry,
			       &request_code,
			       &buffer_len,
			       &return_length,
			       _vms_tgoto_buffer,
			       input);


    /* tgoto uses a static buffer and returns it. */
    if ($VMS_STATUS_SUCCESS(status)) {
	/* Make sure it is null terminated */
	_vms_tgoto_buffer[return_length] = 0;
	return _vms_tgoto_buffer;
    }
    return NULL;
}


/* Routine to return and invalidate any cached values
 * The second time that a code is looked up, we need to
 * re-read the values.  But if a cached code has not been
 * read, we want to just return it, because we look up both
 * of these at the same time.
 * TODO: Handle other parameters like xon.
 */
static int return_cached_value(unsigned short id_code) {

    int ret_val = -1;

    /* check to see if we need to refresh the values */
    switch(id_code) {
    case 'li': /* lines */
	/* See if we have cached the value */
	if (term_lines != -1) {
	    ret_val = term_lines;
	    term_lines = -1;  /* Get fresh next time */
	    return ret_val;
	}
	break;
    case 'co': /* columns */
	/* See if we have cached the value */
	if (term_width != -1) {
	    ret_val = term_width;
	    term_width = -1;  /* Get fresh next time */
	    return ret_val;
	}
	break;
    case 'ws': /* Status line colums */
	/* Set ws read flag */
	/* If someone asks with out checking if a status line is */
	/* actually supported, then that is there problem */
	if (term_sl_width != -1) {
	    ret_val = term_sl_width;
	    term_sl_width = -1;  /* Get fresh next time */
	    return ret_val;
	}
	break;
    }
    return -1;
}




static int vms_qio_sensmode(unsigned short chan) {
int status;
struct sense_iosb_st sense_iosb;
struct sense_st term_char;

    status = SYS$QIOW(
		EFN$C_ENF,
		chan,
		IO$_SENSEMODE,
		&sense_iosb,
		NULL,
		NULL,
		&term_char,
                sizeof term_char, 0, 0, 0, 0, 0);

    if (!$VMS_STATUS_SUCCESS(status) ||
	!$VMS_STATUS_SUCCESS(sense_iosb.status)) {
	return -1;
    }

    term_lines = (term_char.tt_def & TT$M_PAGE) >> 24;
    term_width = term_char.term_width;
    term_sl_width = term_char.term_width ;
    return 0;
}



int vms_tgetnum(char * id) {

    int status;
    unsigned long request_code;
    long return_value;
    enum smg_actions smg_action;
    long return_length;
    const long buffer_len = VMS_TERM_BUFFER_LEN;
    char return_buffer[VMS_TERM_BUFFER_LEN + 1];
    unsigned long input[1];
    unsigned short id_code;
    int ret_val;
    unsigned short term_chan;

    if ((_vms_termtable_entry == 0) || (id == NULL)) {
	return -1;
    }

    /* The right way to do this is to lookup the mapping */
    status = vms_get_smg_request_code(id, &request_code, &smg_action);
    if (!$VMS_STATUS_SUCCESS(status)) {
	/* XOPEN says to return ERR, GNU.ORG says to return -1 */
	/* Experience has shown that in a conflict, GNU.ORG */
	/* is more accurate in what programs expect */
	return -1;
    }

    /* Handle the hard coded cases */
    switch(smg_action) {
	case ASCII_CHAR:
	case SMG_TRUE:
	    return request_code;
	case SMG_INT:
	    break;
	case SPECIAL_WIDE2:
	    /* Width of double wide characters if supported */
	    input[0] = 0;
	    status = SMG$GET_TERM_DATA(&_vms_termtable_entry,
				       &request_code,
				       &buffer_len,
				       &return_length,
				       return_buffer,
				       input);
	    if ($VMS_STATUS_SUCCESS(status) && (return_length > 0)) {
		return 2;
	    }
	    return -1;
	case QIO_READSENSE:

	    id_code = id[0] + (id[1] * 256);

	    /* Check if stdout is a terminal, if not return -1 */
	    if (!isatty(STDOUT_FILENO)) {
		return -1;
	    }

	    /* Check to see if we have a cached value */
	    ret_val = return_cached_value(id_code);
	    if (ret_val != -1) {
		return ret_val;
	    }

	    /* Open a channel to stdout */
	    ret_val = sys_assign_stdout(&term_chan);
	    if (ret_val == -1) {
		return ret_val;
	    }

	    /* TODO: When integrated with other VMS extensions, reuse the */
	    /* channel that they has opened and tracked with a fileno. */

	    /* Do a SYS$QIOW to get the terminal characteristics */
	    ret_val = vms_qio_sensmode(term_chan);
	    if (ret_val == -1) {
		return ret_val;
	    }

	    /* Close the channel to stdout */
	    ret_val = SYS$DASSGN(term_chan);
	    /* Not much to do if this fails, but that should never happen */

	    /* Return the selected value and invalidate the cache */
	    ret_val = return_cached_value(id_code);
	    return ret_val;
	default:
	    return -1;
    }


    /* SMG does not update if value not found */
    return_value = -1;

    /* Look it up with SMG */
    status = SMG$GET_NUMERIC_DATA(&_vms_termtable_entry,
				  &request_code,
				  &return_value);

    /* We have a return value */
    if ($VMS_STATUS_SUCCESS(status)) {
	return return_value;
    }

    return -1;
}

char *vms_tgetstr(char * id, char **area) {

/* If area is not NULL, write bytes into it and increment it by number
   of bytes returned after the call.
   If area is NULL, malloc() the data for the result.
 */

    int status;
    unsigned long request_code;
    int ret_stat;
    unsigned long input[3];
    long return_length;
    const long buffer_len = VMS_TERM_BUFFER_LEN;
    enum smg_actions smg_action;
    char * return_buffer = *area;

    if ((_vms_termtable_entry == 0) || (id == NULL)) {
	return NULL;
    }

    /* The right way to do this is to lookup the mapping */
    status = vms_get_smg_request_code(id, &request_code, &smg_action);
    if (!$VMS_STATUS_SUCCESS(status)) {
	return NULL;
    }

    /* Needs to be a string returned */
    /* If the SMG code for the string takes parameters, fake it */
    /* These results should only be used by programs that are   */
    /* trying to determine if the capability is supported       */
    /* Actual output will be by SMG routines */

    input[0] = 0;
    switch (smg_action) {

    case SMG_STRING2:
	input[0]++;
	input[2] = 1;
    case SMG_STRING1:
	input[0]++;
	input[1] = 2;
    case SMG_STRING:
	break;
    case ASCII_CHAR:
	/* All terminals on VMS basically need to do these */
	if (return_buffer == NULL) {
	    return_buffer = malloc(2);
	}
	return_buffer[0] = request_code & 0xFF;
	return_buffer[1] = 0;
	if (*area != NULL) {
	    *area += 2;
	}
	return return_buffer;
    case SMG_UNKNOWN:
    case SMG_FLAG:
    case SMG_NEGFLAG:
    case SMG_INT:
    case SMG_TRUE:
    default:
	return NULL;
    }

    /* Allocate memory for the return value if needed */
    if (return_buffer == NULL) {
	return_buffer = malloc(VMS_TERM_BUFFER_LEN + 1);
    }

    /* Get the string for the terminal */
    status = SMG$GET_TERM_DATA(&_vms_termtable_entry,
			       &request_code,
			       &buffer_len,
			       &return_length,
			       return_buffer,
			       input);


    /* We have a result! */
    if ($VMS_STATUS_SUCCESS(status) && (return_length > 0)) {

	/* Make sure it is null terminated */
	return_buffer[return_length] = 0;

	/* Increment pointer if needed */
	if (*area != NULL) {
	    /* Need to include the null terminator */
	    *area += return_length + 1;
	}
	return return_buffer;
    }

    /* Free memory if needed */
    if (*area == NULL) {
	free(return_buffer);
    }
    return NULL;
}


int vms_tgetflag(char * id) {

    int status;
    unsigned long request_code;
    int ret_stat;
    unsigned long input[3];
    long return_length;
    const long buffer_len = VMS_TERM_BUFFER_LEN;
    char return_buffer[VMS_TERM_BUFFER_LEN + 1];
    long return_value;
    enum smg_actions smg_action;
    unsigned short id_code;

    if ((_vms_termtable_entry == 0) || (id == NULL)) {
	return -1;
    }

    /* The right way to do this is to lookup the mapping to SMG */
    status = vms_get_smg_request_code(id, &request_code, &smg_action);
    if (!$VMS_STATUS_SUCCESS(status)) {
	/* If we can not translate it to SMG, it does not exist */
	return 0;
    }

    input[0] = 0;
    switch (smg_action) {
    case SMG_STRING2:
	/* The documentation states that this call can be used to
	 * determine a capability is present on a terminal.
	 * I have found no documentation that indicates if this should
	 * work on all capabilities, or just boolean ones.
	 */
	input[0]++;
	input[2] = 1;
    case SMG_STRING1:
	input[0]++;
	input[1] = 1;
    case SMG_STRING:
    case SMG_STRING_NEG:
    case SPECIAL_WIDE2:
	/* Get the string for the terminal */
	status = SMG$GET_TERM_DATA(&_vms_termtable_entry,
				   &request_code,
				   &buffer_len,
				   &return_length,
				   return_buffer,
				   input);
	if ($VMS_STATUS_SUCCESS(status) && (return_length > 0)) {
	    if (smg_action == SMG_STRING_NEG) {
		return 0;
	    }
	    return 1;
	} else if (smg_action == SMG_STRING_NEG) {
	    return 1;
	}
	return 0;
    case QIO_READSENSE:
	/* VMS can look up this value */
    case SMG_TRUE:
    case ASCII_CHAR:
	/* Value hard coded for OpenVMS */
	/* Terminals these do not work for are not very usable on VMS */
	return 1;
    case SMG_FLAG:
    case SMG_NEGFLAG:
    case SMG_INT:
	break;
    case SMG_UNKNOWN:
    default:
	/* We have no idea what this is */
	return 0;
    }

    /* SMG does not update if value not found */
    id_code = id[0] + (id[1] * 256);
    return_value = -1;
    status = SMG$GET_NUMERIC_DATA(&_vms_termtable_entry,
				  &request_code,
				  &return_value);

    if ($VMS_STATUS_SUCCESS(status)) {
	switch(smg_action) {
	case SMG_INT:
	     /* Just checking to see if it is known to SMG */
	     return 1;
	case SMG_FLAG:
	    /* We know about this as an SMG capability */
	    if (return_value > 0) {
		return 1;
	    } else {
		if ((id_code == 'os') && (return_value == -1)) {
		    /* default SMGTERMS.TXT is missing this parameter */
		    /* So assume if not scope than overstrike is available */
		    /* This hack will allow correct operation if the */
		    /* SMGTERMS.TXT is fixed. */
		    request_code = SMG$K_SCOPE;
		    return_value = -1;
		    status = SMG$GET_NUMERIC_DATA(&_vms_termtable_entry,
						  &request_code,
						  &return_value);
		    if ($VMS_STATUS_SUCCESS(status)) {
			if (return_value == 0) {
			    return 1;
			}
		    }
		}
		return 0;
	    }
	case SMG_NEGFLAG:
	    /* We know this is the reverse of a SMG capability */
	    if (return_value == 0) {
		return 1;
	    } else {
		return 0;
	}
	default:
	    /* We should not get here. */
	    return 0;
	}
    }
    return 0;

}
