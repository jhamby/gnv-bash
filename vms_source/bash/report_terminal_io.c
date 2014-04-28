/* File: Report_terminal_io.c
 *
 * $Id: report_terminal_io.c,v 1.1.1.1 2012/12/02 19:25:22 wb8tyw Exp $
 *
 * This is a program to report significant termios attributes
 * a unit test for the emulation of termios VMS needed for GNV.
 *
 * 26-Mar-2012	J. Malmberg	Original
 *
 *************************************************************************/

#define __USE_BSD 1
#define __USE_GNU 1
#define __USE_MISC 1
#define __USE_XOPEN 1

#ifdef __VMS
#include "vms_terminal_io.h"
#endif

#ifdef __unix__
#include <term.h>   /* t*() routines */
#endif


#include <stdlib.h>
#include <string.h>
/* #include <stdio.h> */

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


static void
report_flag(const char * text, unsigned long flag, int mask) {
    if ((flag & mask) != 0) {
	printf("    %s is set\n", text);
    } else {
	printf("    %s is clear\n", text);
    }
}

static void
report_cc(const char * text, char code) {
    switch(code) {
    case 0:
	printf("    %s is not defined!\n", text);
	break;
    case 127:
	printf("    %s is set to DEL character.\n", text);
	break;
    default:
	if (code < 31) {
	    char ccode;
	    ccode = code + '@';
	    printf("    %s is set to Control-%c\n", text, ccode);
	} else {
	    printf("    %s is set to %d.\n", text, code);
	}
    }
}

static void
report_speed(const char * text, int speed) {
    printf("  %s is %d\n", text, speed);
}

static void
report_termios(const struct termios *buf) {

    puts("Input Flags:");
    report_flag("IGNBRK",  buf->c_iflag, IGNBRK);
    report_flag("BRKINT",  buf->c_iflag, BRKINT);
    report_flag("IGNPAR",  buf->c_iflag, IGNPAR);
    report_flag("PARMRK",  buf->c_iflag, PARMRK);
    report_flag("INPCK",   buf->c_iflag, INPCK);
    report_flag("ISTRIP",  buf->c_iflag, ISTRIP);
    report_flag("INLCR",   buf->c_iflag, INLCR);
    report_flag("IGNCR",   buf->c_iflag, IGNCR);
    report_flag("ICRNL",   buf->c_iflag, ICRNL);
    report_flag("IXON",    buf->c_iflag, IXON);
    report_flag("IXOFF",   buf->c_iflag, IXOFF);
    report_flag("IXANY",   buf->c_iflag, IXANY);
    report_flag("IMAXBEL", buf->c_iflag, IMAXBEL);
    report_flag("IUCLC",   buf->c_iflag, IUCLC);

    puts("Output Flags:");
    report_flag("OPOST",   buf->c_oflag, OPOST);
    report_flag("ONLCR",   buf->c_oflag, ONLCR);
    report_flag("OXTABS",  buf->c_oflag, OXTABS);
    report_flag("ONOEOT",  buf->c_oflag, ONOEOT);
    report_flag("OCRNL",   buf->c_oflag, OCRNL);
    report_flag("ONOCR",   buf->c_oflag, ONOCR);
    report_flag("ONLRET",  buf->c_oflag, ONLRET);
    report_flag("NL1",     buf->c_oflag, NL1);
    report_flag("TAB1",    buf->c_oflag, TAB1);
    report_flag("TAB2",    buf->c_oflag, TAB2);
    report_flag("TAB3",    buf->c_oflag, TAB3);
    report_flag("CR1",     buf->c_oflag, CR1);
    report_flag("CR2",     buf->c_oflag, CR1);
    report_flag("CR3",     buf->c_oflag, CR1);
    report_flag("FF1",     buf->c_oflag, FF1);
    report_flag("BS1",     buf->c_oflag, BS1);
    report_flag("VT1",     buf->c_oflag, VT1);
    report_flag("OLCUC",   buf->c_oflag, CR1);
    report_flag("OFILL",   buf->c_oflag, OFILL);

    puts("Control Modes:");
    report_flag("CIGNORE", buf->c_cflag, CIGNORE);
    report_flag("CS6",     buf->c_cflag, CS6);
    report_flag("CS7",     buf->c_cflag, CS7);
    report_flag("CSTOPB",  buf->c_cflag, CSTOPB);
    report_flag("CREAD",   buf->c_cflag, CREAD);
    report_flag("PARENB",  buf->c_cflag, PARENB);
    report_flag("PARODD",  buf->c_cflag, PARODD);
    report_flag("HUPCL",   buf->c_cflag, HUPCL);
    report_flag("CLOCAL",  buf->c_cflag, CLOCAL);
    report_flag("CRTSCTS", buf->c_cflag, CRTSCTS);
    report_flag("CDTRCTS", buf->c_cflag, CDTRCTS);
    report_flag("MDMBUF",  buf->c_cflag, MDMBUF);
    report_flag("CS6",     buf->c_cflag, CS6);
    report_flag("CS7",     buf->c_cflag, CS7);

    puts("Local Modes:");
    report_flag("ECHOKE",     buf->c_lflag, ECHOKE);
    report_flag("ECHOK",      buf->c_lflag, ECHOK);
    report_flag("ECHO",       buf->c_lflag, ECHO);
    report_flag("ECHONL",     buf->c_cflag, ECHONL);
    report_flag("ECHOPRT",    buf->c_cflag, ECHOPRT);
    report_flag("ECHOCTL",    buf->c_cflag, ECHOCTL);
    report_flag("ISIG",       buf->c_cflag, ISIG);
    report_flag("ICANON",     buf->c_cflag, ICANON);
    report_flag("ALTWERASE",  buf->c_cflag, ALTWERASE);
    report_flag("IEXTEN",     buf->c_cflag, IEXTEN);
    report_flag("EXTPROC",    buf->c_cflag, EXTPROC);
    report_flag("TOSTOP",     buf->c_cflag, TOSTOP);
    report_flag("FLUSHO",     buf->c_cflag, FLUSHO);
    report_flag("NOKERNINFO", buf->c_cflag, NOKERNINFO);
    report_flag("PENDIN",     buf->c_cflag, PENDIN);
    report_flag("NOFLSH",     buf->c_cflag, NOFLSH);

    puts("Control Characters");
    report_cc("VEOF", buf->c_cc[VEOF]);
    report_cc("VEOL", buf->c_cc[VEOL]);
    report_cc("VEOL2", buf->c_cc[VEOL2]);
    report_cc("VERASE", buf->c_cc[VERASE]);
    report_cc("VWERASE", buf->c_cc[VWERASE]);
    report_cc("VKILL", buf->c_cc[VKILL]);
    report_cc("VREPRINT", buf->c_cc[VREPRINT]);
    report_cc("VINTR", buf->c_cc[VINTR]);
    report_cc("VQUIT", buf->c_cc[VQUIT]);
    report_cc("VSUSP", buf->c_cc[VSUSP]);
    report_cc("VDSUSP", buf->c_cc[VDSUSP]);
    report_cc("VSTART", buf->c_cc[VSTART]);
    report_cc("VSTOP", buf->c_cc[VSTOP]);
    report_cc("VLNEXT", buf->c_cc[VLNEXT]);
    report_cc("VDISCARD", buf->c_cc[VDISCARD]);
    report_cc("VMIN", buf->c_cc[VMIN]);
    report_cc("VTIME", buf->c_cc[VTIME]);
    report_cc("VSTATUS", buf->c_cc[VSTATUS]);

    report_speed("TX Speed", buf->__ospeed);
    report_speed("RX Speed", buf->__ispeed);
}


int test_change(int fd,
		unsigned long *flag,
		int mask,
		const char *text,
		struct termios * buf) {

int status;
int expected;

    /* Flip the state of the flag to test */
    if ((*flag & mask) == 0) {
 	*flag |= mask;
	expected = mask;
	/* printf("%s is currently disabled.\n", text); */
    } else {
	/* printf("%s is currently enabled.\n", text); */
	*flag &= ~mask;
	expected = 0;
    }

    status = tcsetattr(fd, TCSANOW, buf);
    if (status != 0) {
	perror("tcsetattr failed ");
	printf("Setting of %s failed!\n", text);
	return -1;
    }

    status = tcgetattr(fd, buf);
    if (status != 0) {
	perror("tcgetattr failed ");
    }

    if ((*flag & mask) == expected) {
	printf("Setting of %s succeeded.\n", text);
	return 0;
    } else {
	printf("Setting of %s failed!\n", text);
	return -1;
    }
}


int main(int argc, char ** argv) {

    int status;
    struct termios buf;
    struct termios oldbuf;
    int fd;

    fd = STDOUT_FILENO;

    if (isatty(fd)) {
	status = tcgetattr(fd, &buf);
	if (status != 0) {
	    perror("tcgetattr failed ");
	}
	memcpy(&oldbuf, &buf, sizeof(struct termios));
	report_termios(&buf);

	test_change(fd, &buf.c_iflag, ISTRIP, "ISTRIP", &buf);
	test_change(fd, &buf.c_iflag, IXON,   "IXON",   &buf);
	test_change(fd, &buf.c_iflag, IXOFF,  "IXOFF",  &buf);

	test_change(fd, &buf.c_oflag, TAB3,   "TAB3",   &buf);
	test_change(fd, &buf.c_oflag, OLCUC,  "OLCUC",  &buf);


	buf.c_lflag &= ~(ECHOKE|ECHOE);
	test_change(fd, &buf.c_lflag, ECHOK,  "ECHOK",  &buf);
	test_change(fd, &buf.c_lflag, ECHO,   "ECHO",   &buf);

	/* Control-T processing */
	test_change(fd, &buf.c_lflag, NOKERNINFO, "NOKERNINFO", &buf);

	buf.c_lflag &= ~(ISIG|IEXTEN);
	test_change(fd, &buf.c_lflag, ICANON, "ICANON", &buf);

	/* Restore everything */
	status = tcsetattr(fd, TCSANOW, &oldbuf);
	if (status != 0) {
	    perror("tcsetattr restore failed ");
	}

    }

}
