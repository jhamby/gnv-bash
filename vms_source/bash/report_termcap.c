/* File: Report_termcap.c
 *
 * $Id: report_termcap.c,v 1.1.1.1 2012/12/02 19:25:22 wb8tyw Exp $
 *
 * This is a program to report significant termcap items to mainly as
 * a unit test for the emulation of termcap on VAX/VMS.needed for GNV.
 *
 * 05-Jul-2010	J. Malmberg	Original
 *
 *************************************************************************/

#ifdef __VMS
#include "vms_term.h"
#endif

#ifdef __unix__
#include <term.h>   /* t*() routines */
#endif


#include <stdlib.h>
#include <stdio.h>

#ifdef __DECC
#pragma message disable pragma
#pragma member_alignment save
#pragma nomember_alignment longword
#endif
struct tflags_st {
	const char * idcode;
	const char * feature;
    };
#ifdef __DECC
#pragma member_alignment restore
#endif

static struct tflags_st tflags[] = {
	{"os", "Overstrike"},
	{"gn", "Generic"},
	{"hc", "Hardcopy"},
	{"bw", "auto_left_margin"},
	{"am", "auto_right_margin -Bash-"},
	{"LP", "Ignore_newline"},
	{"da", "memory above"},
	{"db", "memory below"},
	{"xn", "Dec margin wrap -Bash-"},
	{"bs", "Backspace can do cursor left"},
	{"hs", "Has status line"},
	{"es", "Escape in status line"},
	{NULL, NULL}
	};

static struct tflags_st tints[] = {
	{"co", "Columns -Bash-"},
	{"li", "lines -Bash-"},
	{"ws", "Width status line"},
	{"kn", "function keys"},
	{"Yn", "Wide size"},
	{NULL, NULL}
	};

static struct tflags_st tstrs[] = {
	{"ho", "home"},
	{"le", "left -Bash-"},
	{"nd", "right -Bash-"},
	{"up", "up"},
	{"do", "down"},
	{"nw", "next_line"},
	{"sc", "Save_cursor"},
	{"rc", "Restore_cursor"},
	{"ff", "Form_Feed"},
	{"sf", "Scroll up"},
	{"sr", "Scroll down"},
	{"cl", "erase screen -Bash-"},
	{"cd", "erase eos"},
	{"ce", "erase eol -Bash-"},
	{"cr", "Carriage Return -Bash-"},
	{"al", "insert line"},
	{"dl", "delete line"},
	{"im", "insert mode -Bash-"},
	{"ei", "end insert mode -Bash-"},
	{"ic", "insert char -Bash-"},
	{"ip", "insert padding"},
	{"mi", "Move in insert"},
	{"dc", "delete char -Bash-"},
	{"dm", "delete mode"},
	{"ed", "End delete mode"},
	{"so", "Standout mode"},
	{"se", "End standout"},
	{"ms", "Move in standout"},
	{"mb", "blink mode"},
	{"md", "bold mode"},
	{"mm", "Enable meta -Bash-"},
	{"mo", "Disable meta -Bash-"},
	{"mp", "protected mode"},
	{"mr", "reverse mode"},
	{"me", "Normal mode"},
	{"as", "Alt char mode"},
	{"ae", "Alt char end"},
	{"us", "Underline mode"},
	{"ue", "Underline end"},
	{"uc", "Underline char"},
	{"ul", "Underline overstrike"},
	{"vi", "Cursor off"},
	{"ve", "Cursor normal"},
	{"bl", "bell"},
	{"ZF", "Double wide"},
	{"ZQ", "Single wide"},
	{"kd", "Key down-arrow -Bash-"},
	{"kl", "Key left-arrow -Bash-"},
	{"kr", "Key right-arrow -Bash-"},
	{"ku", "Key up-arrow -Bash-"},
	{"ks", "Application keypad -Bash-"},
	{"ke", "Numeric keypad -Bash-"},
	{"pc", "pad character -Bash-"},
	{"vb", "visible bell -Bash-"},
	{NULL, NULL}
	};

static struct tflags_st tstrs1[] = {
	{"DO", "Down N"},
	{"UP", "Up N"},
	{"LE", "Left N"},
	{"RI", "Right N"},
	{"SF", "Scroll up N"},
	{"SR", "Scroll down N"},
	{"AL", "Insert N lines"},
	{"DL", "Delete N lines"},
	{"IC", "Insert N chars -Bash-"},
	{"DC", "Delete N chars -Bash-"},
	{NULL, NULL}
	};

static struct tflags_st tstrs2[] = {
	{"cm", "cursor move"},
	{"cs", "Scroll region"},
	{NULL, NULL}
	};

int main(int argc, char ** argv) {

    int status;
    char *bp;
    char bfptr[4096];
    char outbuf[4096];
    int i;

    /* Quck check of the arguments */
    if (argc < 2) {
	puts("Need terminal name to lookup in termcap");
	exit(1);
    }

    /* Get the termcap name and look it up */
    status = tgetent(bfptr, argv[1]);
    if (status == 1) {
	puts("Terminal type found.");
    } else if (status == 0) {
	puts("Terminal type is not known");
	exit(1);
    } else {
	puts("Unable to find the termcap emulation library");
    }

    /* Common flags for terminals commonly seen on VMS */
    i = 0;
    while (tflags[i].idcode != NULL) {
        status = tgetflag((char *)tflags[i].idcode);
	switch(status) {
	case 0:
	    printf("%s disabled\n", tflags[i].feature);
	    break;
	case 1:
	    printf("%s enabled\n", tflags[i].feature);
	    break;
	case -1:
	    printf("%s undefined\n", tflags[i].feature);
	    break;
	default:
	    printf("%s unknown\n", tflags[i].feature);
	    break;
	}
	i++;
    }

    /* Quick check for strings supported */
    i = 0;
    while (tstrs[i].idcode != NULL) {
	char * retstr;
        char * bufstr = outbuf;
        retstr = tgetstr((char *)tstrs[i].idcode, &bufstr);
	if (retstr == NULL) {
	    printf("%s not defined \n", tstrs[i].feature);
	} else {
	    printf("%s defined\n", tstrs[i].feature);
	}
	i++;
    }

    i = 0;
    while (tstrs1[i].idcode != NULL) {
	char * retstr;
        char * bufstr = outbuf;
        retstr = tgetstr((char *)tstrs1[i].idcode, &bufstr);
	if (retstr == NULL) {
	    printf("%s not defined \n", tstrs1[i].feature);
	} else {
	    printf("%s defined\n", tstrs1[i].feature);
	}
	i++;
    }
    i = 0;
    while (tstrs2[i].idcode != NULL) {
	char * retstr;
        char * bufstr = outbuf;
        retstr = tgetstr((char *)tstrs2[i].idcode, &bufstr);
	if (retstr == NULL) {
	    printf("%s not defined \n", tstrs2[i].feature);
	} else {
	    printf("%s defined\n", tstrs2[i].feature);
	}
	i++;
    }

    /* Common integer values */
    i = 0;
    while (tints[i].idcode != NULL) {
        status = tgetnum((char *)tints[i].idcode);
	switch(status) {
	case -1:
	    printf("%s undefined %d\n", tints[i].feature, status);
	    break;
	default:
	    printf("%s set to %d\n", tints[i].feature, status);
	    break;
	}
	i++;
    }

    /* Common Escape sequences - translated */

}
