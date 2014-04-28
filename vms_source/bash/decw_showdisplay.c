/* File: decw_showdisplay.c
 *
 * $Id: decw_showdisplay.c,v 1.2 2013/06/09 18:05:03 wb8tyw Exp $
 *
 * This module is to report the settings for the DECW$DISPLAY
 *
 * Based on http://labs.hoffmanlabs.com/node/375
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
 *
 */


#define __NEW_STARLET 1
#include <descrip.h>
#include <efndef.h>
#include <iodef.h>
#ifndef __VAX
#include <iosbdef.h>
#endif
#include <ssdef.h>
#include <stdio.h>
#include <stsdef.h>
#ifdef TEST_DECW_SHOWDISPLAY
#include <stdlib.h>
#endif

#ifndef IO$M_WS_DISPLAY
#define IO$M_WS_DISPLAY         64
#define DECW$C_WS_DSP_NODE       1
#define DECW$C_WS_DSP_TRANSPORT  2
#define DECW$C_WS_DSP_SERVER     3
#define DECW$C_WS_DSP_SCREEN     4
#endif
#define NODE_LEN 255
#define TRANSPORT_LEN 64

int SYS$ASSIGN
       (const struct dsc$descriptor_s * devnam,
	unsigned short * chan,
	unsigned long acmode,
	const struct dsc$descriptor_s * mbxnam,
	unsigned long flags);

int SYS$DASSGN(unsigned short chan);

#pragma message save
#pragma message disable noparmlist
#ifdef __VAX
struct qio_iosb {
    unsigned short iosb$w_status;
    unsigned short iosb_w_something;
    unsigned long iosb$l_dev_depend;
};
typedef struct qio_iosb IOSB;
#endif

int SYS$QIOW
       (unsigned long efn,
	unsigned short chan,
	unsigned long func,
	IOSB * iosb,
	void (* astadr)(__unknown_params),
	void *,
	void * p1,
	int p2,
	int p3,
	int p4,
	int p5,
	int p6);

#pragma message restore


/* Need space for hostname, two colons, 5 digit server, period, 5 digit screen
 */
#define DISPLAY_LEN (NODE_LEN + 13)
static char vms__display[DISPLAY_LEN + 1];

char * get_decw_display(void) {

int status;
unsigned short chan;
IOSB iosb;
char node[NODE_LEN + 1];
char transport[TRANSPORT_LEN + 1];
char server[6];
char screen[6];
$DESCRIPTOR( ws_device, "DECW$DISPLAY");

    /* See if the DECW$DEVICE exist by trying to assign a channel to it */
    status = SYS$ASSIGN(&ws_device, &chan, 0, 0, 0);
    if (!$VMS_STATUS_SUCCESS(status)) {
	return NULL;
    }

    /* Get the node or host name */
    status = SYS$QIOW(EFN$C_ENF, chan, IO$_SENSEMODE | IO$M_WS_DISPLAY,
		      &iosb, 0, 0, node, NODE_LEN,
		      DECW$C_WS_DSP_NODE, 0, 0, 0 );
    if (!$VMS_STATUS_SUCCESS(status))
 	return NULL;
    if (!$VMS_STATUS_SUCCESS(iosb.iosb$w_status))
	return NULL;
    node[iosb.iosb$l_dev_depend] = 0;

    status = SYS$QIOW(EFN$C_ENF, chan, IO$_SENSEMODE | IO$M_WS_DISPLAY,
		      &iosb, 0, 0, transport, TRANSPORT_LEN,
		      DECW$C_WS_DSP_TRANSPORT, 0, 0, 0 );
    if (!$VMS_STATUS_SUCCESS(status))
	return NULL;
    if (!$VMS_STATUS_SUCCESS(iosb.iosb$w_status))
	return NULL;
    transport[iosb.iosb$l_dev_depend] = 0;

    status = SYS$QIOW(EFN$C_ENF, chan, IO$_SENSEMODE | IO$M_WS_DISPLAY,
		      &iosb, 0, 0, server, (sizeof server)-1,
		      DECW$C_WS_DSP_SERVER, 0, 0, 0 );
    if (!$VMS_STATUS_SUCCESS(status))
	return NULL;
    if (!$VMS_STATUS_SUCCESS(iosb.iosb$w_status))
	return NULL;
    server[iosb.iosb$l_dev_depend] = 0;

    status = SYS$QIOW(EFN$C_ENF, chan, IO$_SENSEMODE | IO$M_WS_DISPLAY,
                       &iosb, 0, 0, screen, (sizeof screen)-1,
                       DECW$C_WS_DSP_SCREEN, 0, 0, 0 );
    if (!$VMS_STATUS_SUCCESS(status))
	return NULL;
    if (!$VMS_STATUS_SUCCESS(iosb.iosb$w_status))
	return NULL;
    screen[iosb.iosb$l_dev_depend] = 0;

    status = SYS$DASSGN(chan);

    /* www.xfree.org/current/X.7.html covers these three cases
     * local  - The hostname should be empty, single colon.
     * TCP/IP - The hostname or IP address followed by a single colon.
     * DECNET - The node name or address followed by a double colon.
     *
     * We do not have a case for LAT, but LAT is no longer supported
     * as of VMS 8.x.
     */

    {
    char * separator;
    char * host;
    int result;

	host = node;
	separator = ":";
	switch(transport[0]) {
	case 'l':
	case 'L':
	    host = "";
	    break;
	case 'd':
	case 'D':
	    separator = "::";
	    break;
	}
#ifdef __VAX
        result = sprintf(vms__display, "%s%s%s.%s",
	    host, separator, server, screen);
#else
        result = snprintf(vms__display, DISPLAY_LEN, "%s%s%s.%s",
	    host, separator, server, screen);
#endif
	vms__display[DISPLAY_LEN] = 0;

        return vms__display;
    }
}

#ifdef TEST_DECW_SHOWDISPLAY
int main(int argc, char ** arv) {

char * display;

    /* Look up the display */
    display = get_decw_display();

    if (display != NULL) {
	printf("display = %s\n", display);
    } else {
	puts("No DECW$DISPLAY device found.");
    }
    exit(0);
}
#endif
