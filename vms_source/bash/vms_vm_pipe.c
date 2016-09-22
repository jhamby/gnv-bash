/* File: VMS_VM_PIPE.C */

/*************************************************************************
 *                                                                       *
 * © Copyright 2005 Hewlett-Packard Development Company, L.P.		 *
 *                                                                       *
 * Confidential computer software.  Valid license from HP and/or         *
 * its subsidiaries required for possession, use, or copying.            *
 *                                                                       *
 * Consistent with FAR 12.211 and 12.212, Commercial Computer Software,  *
 * Computer Software Documentation, and Technical Data for Commercial    *
 * Items are licensed to the U.S. Government under vendor's standard     *
 * commercial license.                                                   *
 *                                                                       *
 * Neither HP nor any of its subsidiaries shall be liable for technical  *
 * or editorial errors or omissions contained herein.  The information   *
 * in this document is provided "as is" without warranty of any kind and *
 * is subject to change without notice.  The warranties for HP products  *
 * are set forth in the express limited warranty statements accompanying *
 * such products.  Nothing herein should be construed as constituting an *
 * additional warranty.                                                  *
 *                                                                       *
 *************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

/*
 *	VMS special pipe handling code for BASH
 *
 *	This provides a way of creating larger pipes than what
 *	the users bytlim would normally allow by putting the
 *	data from the pipes in virtual memory.
 *
 *	Special code is needed for creating the pipe that is
 *	used to write to the child as the FIFO must be established
 *	before enough data is written to cause the parent process
 *	to go into a RWMBX wait.
 *
 *	For reading from the child, a procedure vms_execve is
 *	provided that can be a wrapper for the execve() function
 *	as stdout/stderr needs to be redirected to a VMS mailbox
 *	just before the child process is execed.
 *
 *	Note that the use of this doubles the amount of pipes
 *	and mailboxes in use.
 *
 *  Usage:
 *
 *     #include <unistd.h>
 *
 *	If you are borrowing this code for a Posix Thread application then
 *	You will need to put some guards in place to prevent VMS channel
 *	use collision.
 *
 *	Resource limitations are the only things that should cause error
 *	returns to be seen for most subroutine calls, and those usually
 *	mean there is no way back off and wait for those limitations to
 *	clear.  So to prevent hangs, any unexpected error in the AST routines
 *	will be treated as fatal.
 *
 *	Author: John Malmberg/HP
 *
 *	08-Sep-2002	JEM008	J. Malmberg
 *
 *			The idea is born, with assistance others including
 *			Paul Cerqua, Charlie McCutcheon, and Michael Boucher.
 *
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <unixio.h>
#include <string.h>
#include <errno.h>
#ifndef __VAX
#include <inttypes.h>
#endif

#include <descrip.h>
#include <iodef.h>
#include <ssdef.h>
#include <agndef.h>
#include <dvidef.h>
#include <stsdef.h>
#include <jpidef.h>
#include <efndef.h>
#include <builtins.h>
#include <unixlib.h>

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

struct mbx_iosb {
	unsigned short status;
	unsigned short count;
	unsigned long value;
};

#pragma member_alignment

struct mbx_buffer_hdr {
    struct mbx_buffer_hdr *flink;
    struct mbx_buffer_hdr *blink;
    int flags;
    int size;
    void * buffer;
};


struct vms_mbx {
    int read_ast_status;
    int write_ast_status;
    unsigned short read_chan; /*  Read from MBX, write to FIFO  */
    unsigned short write_chan; /* Read from FIFO - write to MBX */
    struct mbx_iosb read_iosb; /* AST status for reading from MBX */
    struct mbx_iosb write_iosb; /* AST status for writing to MBX */
    int read_devbufsiz;		/* How big are the buffers to transfer */
    int write_devbufsiz;	/* write must be >= read size */
    int direction;		/* 1 = parent is writer */
    char *read_buffer;
    struct mbx_buffer_hdr *rbuf_active; /* rbuf being written to parent */
    struct mbx_buffer_hdr *rbuf_flink; /* Chain of buffers to read from */
    struct mbx_buffer_hdr *rbuf_blink;
    int child_watch_ast_status;
    int status;
    unsigned short jpi_iosb[4];	/* AST status for polling if child is alive */
    pid_t child_pid;		/* Child Pid */
    pid_t result_pid;		/* Looked up Pid */
};

#pragma member_alignment restore

int LIB$SIGNAL(int status);
int LIB$STOP(int status);

int SYS$ASSIGN
       (const struct dsc$descriptor_s * devnam,
	unsigned short * chan,
	unsigned long acmode,
	const struct dsc$descriptor_s * mbxnam,
	unsigned long flags);

int SYS$CANCEL(unsigned short chan);


int SYS$CREMBX
       (unsigned long prmflg,
	unsigned short *chan,
	unsigned long maxmsg,
	unsigned long bufquo,
	unsigned long promsk,
	unsigned long acmode,
	const struct dsc$descriptor_s * lognam,
	unsigned long flags,
	unsigned long nullarg);

int SYS$DASSGN(unsigned short chan);

int SYS$GETDVIW
       (unsigned long efn,
	unsigned short chan,
	const struct dsc$descriptor_s * devnam,
	const struct item_list_3 * itmlst,
	void * iosb,
	void (* astadr)(unsigned long),
	unsigned long astprm,
	void * nullarg);

#pragma message save
#pragma message disable noparmlist
int SYS$GETJPI
       (unsigned long efn,
	pid_t * pid,
	const struct dsc$descriptor_s * prcnam,
	const struct item_list_3 * itmlst,
	void * iosb,
	void (* astadr)(__unknown_params),
	void * astprm,
	void * nullarg);

int SYS$QIOW
       (unsigned long efn,
	unsigned short chan,
	unsigned long func,
	void * iosb,
	void (* astadr)(__unknown_params),
	void *,
	void * p1,
	int p2,
	int p3,
	int p4,
	int p5,
	int p6);

int SYS$QIO
       (unsigned long efn,
	unsigned short chan,
	unsigned long func,
	void * iosb,
	void (* astadr)(__unknown_params),
	void *,
	void * p1,
	int p2,
	int p3,
	int p4,
	int p5,
	int p6);

int SYS$SETIMR
       (unsigned long efn,
	const void * daytime,
	void (* astadr)(__unknown_params),
	void * reqid,
	unsigned long flags);
#pragma message restore

#ifndef __VAX
const int64_t one_second = -10000000;
#else
#pragma member_alignment save
#pragma member_alignment
const struct {
   long low_longword;
   long high_longword;
} one_second = {-1, -10000000};
#pragma member_alignment restore
#endif
static void check_if_child_is_alive_ast(struct vms_mbx * context);

#ifdef __VAX
#   define INSQUEL(pred, new) _INSQUE(new, pred)
#   define REMQUEL(entry, removed) \
	((_REMQUE(entry, removed) == _remque_empty) ? -1 : 1)
#else
#   define INSQUEL(pred, new) __PAL_INSQUEL(pred, new);
#   define REMQUEL(entry, removed) __PAL_REMQUEL(entry, removed)
#endif



 /* AST to keep watch for a child dying */
/*=====================================*/
static void vms_watch_child_ast(struct vms_mbx * context)
{
int status;

     /* Is the read ast still going?  */
    /*-------------------------------*/
    if (context->read_ast_status >= 0) {

	 /* Only need to do anything if the child is not alive */
	/*----------------------------------------------------*/
	if (!$VMS_STATUS_SUCCESS(context->jpi_iosb[0])) {

	     /* Cancel the AST reading from the child */
	    /*---------------------------------------*/
	    context->child_watch_ast_status = -1;

	    /* This AST is done */
	    return;
	}

	 /* Look once a second */
	/*--------------------*/
	context->child_watch_ast_status = 2;
	status = SYS$SETIMR
	   (EFN$C_ENF,
	    &one_second,
	    check_if_child_is_alive_ast,
	    context,
	    0);
	return;
    }

     /* All done? */
    /*-----------*/
    if (context->write_ast_status < 0) {

	 /* Only can get here if this is the last AST left of the FIFO
	  * Everything else should be cleaned up, so free the context
	  * and be done.
	  */
	free(context);
    }

}

static void vms_write_mbx_ast(struct vms_mbx * context);


 /* This routine is used to check to see if a child is still alive. */
/*-----------------------------------------------------------------*/
static void check_if_child_is_alive_ast(struct vms_mbx * context)
{
struct item_list_3 itemlist[2];
int status;
int status1;

     /* Is the read ast still going?  */
    /*-------------------------------*/
    if (context->read_ast_status >= 0) {

	 /* Verify that the PID is valid */
	/*------------------------------*/
	itemlist[0].len = 4;
	itemlist[0].code = JPI$_PID;
	itemlist[0].bufadr = &context->result_pid;
	itemlist[0].retlen = NULL;
	itemlist[1].len = 0;
	itemlist[1].code = 0;

	context->child_watch_ast_status = 1;
	status = SYS$GETJPI
	   (EFN$C_ENF,
	    &context->child_pid,
	    NULL,
	    itemlist,
	    context->jpi_iosb,
	    vms_watch_child_ast,
	    context,
	    0);

         /* Check if child has already exited */
	/*----------------------------------*/
	if (!$VMS_STATUS_SUCCESS(status)) {

            /* Child has exited, mark for future EOF to be added
             * to the end of the read buffer.
             */

            struct mbx_buffer_hdr * rbuf;

	     /* Cancel the AST reading from the child */
	    /*---------------------------------------*/
	    context->child_watch_ast_status = -1;
	    status = SYS$DASSGN(context->read_chan);
	}

	return;
    }

     /* All done? */
    /*-----------*/
    if (context->write_ast_status < 0) {

	 /* Only can get here if this is the last AST left of the FIFO
	  * Everything else should be cleaned up, so free the context
	  * and be done.
	  */
	free(context);
    }
}

/* Start up watching the child if not already started.
 *
 */
static void start_child_watcher(struct vms_mbx * context)
{
struct timr_req * tcontext;


      /* The child_fd file descriptor can not be      */
     /*  closed until after the child has started up. */
    /*-----------------------------------------------*/
    if (context->child_watch_ast_status == 0) {
	context->child_watch_ast_status = 1;

	/* Now in an ideal world the child should be the only other
	 * with a unidirectional channel attached to the mailbox.  When that
	 * channel is is gone, then cleanup can be started.
	 */

	/* because the world is not ideal, get the pid of the child
	 * to check if it is still alive when the pipe is empty.
	 */

	if (context->direction) {
	    /* Direction = 1, child is reader */
	    context->child_pid = context->write_iosb.value;
	}
	else {
	    /* Direction = 0, child is writer */
	    context->child_pid = context->read_iosb.value;
	}
	context->jpi_iosb[1] = SS$_NORMAL;

	 /* Start watching for the child to exit */
	/*--------------------------------------*/
	check_if_child_is_alive_ast(context);
    }
}

static void vms_write_mbx_ast(struct vms_mbx * context)
{
const unsigned long write_it = IO$_WRITEVBLK | IO$M_READERCHECK;
const unsigned long write_eof = IO$_WRITEOF | IO$M_READERCHECK;
int status;

     /* if write_iosb.status is 0, then we are starting or restarting */
    /*----------------------------------------------------------------*/
    if (context->write_iosb.status == 0) {
    }
    else {
	if (!$VMS_STATUS_SUCCESS(context->write_iosb.status)) {

	     /* If no reader then just discard the data */
	    /*---------------------------------*/
	    if (context->write_iosb.status == SS$_NOREADER) {
		struct mbx_buffer_hdr * rbuf;

		/* No one is reading from the PIPE.  This means that the
		 * other side broke the pipe. so dump the contents in bit bucket
		 * until the writer shuts down it's side.
		 * If we do not keep reading and dumping the data, the writing
		 * process will may go into RWMBX wait.
		 *
		 * No point in keeping the data, as the UNIX routines do not
		 * have any way of reconnecting back to the same pipe to read it
		 *
		 * It is quite possible that a parent program could
		 * intentionally cause this condition, so deal with it
		 * gracefully.
		 */

		context->write_ast_status = 0;
		while( REMQUEL
			((void *)context->rbuf_flink,(void*) &rbuf) >= 0) {
#ifdef VMS_BP_POISON
	memset(rbuf, 2, rbuf->size + sizeof(struct mbx_buffer_hdr));
#endif
		    free(rbuf);
		}

	    }
	    else {

		/* Should only get here if process is in bad shape
		 * there was a programming error.
		 */
		LIB$STOP(context->write_ast_status);
	    }
	}
    }

     /* Free up the buffer just written */
    /*---------------------------------*/
    if (context->rbuf_active != NULL) {

	 /* Now that we have a PID, watch for the child to die */
	/*----------------------------------------------------*/

	    /* Direction = 0, child is writer */
	    /* Direction = 1, child is reader */

	if ((context->direction != 0) && (context->write_iosb.status != 0))
	    start_child_watcher(context);

#ifdef VMS_BP_POISON
	memset(context->rbuf_active, 3, context->rbuf_active->size + sizeof(struct mbx_buffer_hdr));
#endif
	free(context->rbuf_active);
	context->rbuf_active = NULL;
    }

     /* If we have data to write, qio it to the mailbox */
    /*-------------------------------------------------*/
    context->write_ast_status = 0;
    if (REMQUEL
	((void *)context->rbuf_flink,(void *)&context->rbuf_active) >= 0) {
    unsigned long func;

	if (context->rbuf_active->flags == 1)
	   func = write_eof;
	else
	   func = write_it;
	context->write_ast_status = 1;

	status = SYS$QIO
	       (EFN$C_ENF,
		context->write_chan,
		func,
		&context->write_iosb,
		vms_write_mbx_ast,
		context,
		context->rbuf_active->buffer,
		context->rbuf_active->size,
		0,0,0,0);

	 /* This should only fail when the FIFO is in process of being
	  * shut down.
	  */
	if (!$VMS_STATUS_SUCCESS(status)) {
	struct mbx_buffer_hdr * rbuf;
	    context->write_ast_status = 0;
	    while(REMQUEL
		((void *)context->rbuf_flink, (void *)&rbuf) >= 0) {
#ifdef VMS_BP_POISON
	memset(rbuf, 4, rbuf->size + sizeof(struct mbx_buffer_hdr));
#endif
		  free(rbuf);
	    }

	     /* Also toss the active rbuf */
	    /*---------------------------*/
#ifdef VMS_BP_POISON
	memset(context->rbuf_active, 5, context->rbuf_active->size + sizeof(struct mbx_buffer_hdr));
#endif
	    free(context->rbuf_active);
	    context->rbuf_active = NULL;

	}
    }
    else {
	/* The REMQUEL above leaves the head of queue in the rbuf_active
	 * memory pointer.  That needs to be cleared or there will be
	 * some problems.
	 */
	context->rbuf_active = NULL;

	/* If the writing process is done, then time to clean up everything
	 * including the context structure
	 */
	if (context->read_ast_status < 0){
	    status = SYS$DASSGN(context->write_chan);

	     /* Kill the child watcher */
	    /*------------------------*/
	    if (context->child_watch_ast_status > 0) {
		/* Child watcher will free context eventually */
	    }
	    else {
#ifdef VMS_BP_POISON
		memset(context, 6, sizeof(struct vms_mbx));
#endif
		free(context);
	    }
	    return;
	}

   }
}

static void vms_read_mbx_ast(struct vms_mbx * context)
{
const unsigned long read_it = IO$_READVBLK | IO$M_WRITERCHECK;
int status;

     /* read_iosb.status == 0, means not yet an AST - set things up */
    /*-------------------------------------------------------------*/
    if (context->read_iosb.status == 0) {
    struct item_list_3 itemlist[2];
    unsigned short dvi_iosb[4];

	  /* Find out the buffer sizes of the read mailbox	        */
	 /*  Not yet an AST, so use the synchronous version on purpose */
	/*------------------------------------------------------------*/
	itemlist[0].len = 4;
	itemlist[0].code = DVI$_DEVBUFSIZ;
	itemlist[0].bufadr = &context->read_devbufsiz;
	itemlist[0].retlen = 0;
	itemlist[1].len = 0;
	itemlist[1].code = 0;

	status = SYS$GETDVIW
	       (EFN$C_ENF,
		context->read_chan,
		NULL,
		itemlist,
		dvi_iosb,
		NULL, 0, 0);
	if (!$VMS_STATUS_SUCCESS(status))
	    LIB$STOP(status);
	if (!$VMS_STATUS_SUCCESS(dvi_iosb[0]))
	    LIB$STOP(dvi_iosb[0]);

	   /* Find out the buffer sizes of the parent mailbox     */
	  /*  Should be same as child, but let's not be trusting */
	 /*   DECC$ features allow these to be different	*/
	/*-----------------------------------------------------*/
	itemlist[0].len = 4;
	itemlist[0].code = DVI$_DEVBUFSIZ;
	itemlist[0].bufadr = &context->write_devbufsiz;
	itemlist[0].retlen = 0;
	itemlist[1].len = 0;
	itemlist[1].code = 0;

	status = SYS$GETDVIW
	       (EFN$C_ENF,
		context->write_chan,
		NULL,
		itemlist,
		dvi_iosb,
		NULL, 0, 0);
	if (!$VMS_STATUS_SUCCESS(status))
	    LIB$STOP(status);
	if (!$VMS_STATUS_SUCCESS(dvi_iosb[0]))
	    LIB$STOP(dvi_iosb[0]);

	/*
	 * Check to see if parent buffer is large enough.  By default it
	 * should be the same, but someone could have changed something.
	 * Since it would have taken an OpenVMS specific hack to do this,
	 * instead of trying to compensate for it, which would mess up the
	 * record structure of the pipe, just die now.
	 */
	if (context->write_devbufsiz < context->read_devbufsiz)
	    LIB$STOP(SS$_MBTOOSML);

	 /* Set up the linked list */
	/*------------------------*/
	context->rbuf_flink = (struct mbx_buffer_hdr *)&context->rbuf_flink;
	context->rbuf_blink = (struct mbx_buffer_hdr *)&context->rbuf_flink;


	 /* Set up the receive buffer */
	/*---------------------------*/
	context->read_buffer = malloc(context->read_devbufsiz);
	if (context->read_buffer == NULL) {
	    /* Trouble - start bailout */
	    context->read_ast_status = -2;	/* AST in fatal error state */

	    /* Parent is out of memory, This means child is also.
	     * Choices are:
	     *  Dump process or dump data.
	     *
	     *  Dump data is silent data corruption.
	     */

	     /* stop application with diagnostic */
	    /*----------------------------------*/
	    LIB$STOP(SS$_INSFMEM);
	}
    }

     /* Handle any errors as best as possible in an AST */
    /*-------------------------------------------------*/
    switch (context->read_iosb.status) {
    case 0:
    case SS$_NORMAL:
    case SS$_ENDOFFILE:

	 /* These are expected */
	/*--------------------*/
	break;

    case SS$_NOWRITER:
    case SS$_CANCEL:
    case SS$_ABORT:
    case SS$_IVCHAN:

    /* Reading process done?, clean up and signal write_mbx AST.
     * write_mbx AST has the job of freeing the context structure
     * after draining it.
     */
	context->read_ast_status = -1;	/* AST is shutting down */
#ifdef VMS_BP_POISON
	memset(context->read_buffer, 7, context->read_devbufsiz);
#endif
	free(context->read_buffer);
	context->read_buffer = NULL;
	if (context->read_iosb.status != SS$_IVCHAN)
	    status = SYS$DASSGN(context->read_chan);
	break;
    default:

	 /* Die on fatal errors */
	/*---------------------*/
	if (!$VMS_STATUS_SUCCESS(context->read_iosb.status))
	   LIB$STOP(context->read_iosb.status);
    }


     /* If there is data? */
    /*-------------------*/
    if (context->read_iosb.status == SS$_ENDOFFILE) {
    struct mbx_buffer_hdr * rbuf;

	 /* Now that we have a PID, watch for the child to die */
	/*----------------------------------------------------*/
	    /* Direction = 0, child is writer */
	    /* Direction = 1, child is reader */

	if (context->direction == 0) {
	    start_child_watcher(context);
	}

	 /* Get a new rbuf */
	/*----------------*/
	rbuf = malloc(sizeof(struct mbx_buffer_hdr));
	if (rbuf == NULL) {
	    context->read_ast_status = -2;	/* AST in fatal error state */
	    LIB$STOP(SS$_INSFMEM);
	}

	 /* Mark it as an EOF */
	/*-------------------*/
	rbuf->flink = NULL;
	rbuf->blink = NULL;
	rbuf->flags = 1; /* EOF flag */
	rbuf->size = 0;
	rbuf->buffer = ((char *)rbuf + sizeof(struct mbx_buffer_hdr));

	 /* Put it on the end of the queue for a FIFO */
	/*-------------------------------------------*/
	INSQUEL((void *)context->rbuf_blink, rbuf);

	 /* Write out the buffer data */
	/*---------------------------*/
	if (context->write_ast_status == 0) {
	   context->write_iosb.status = 0;
	   vms_write_mbx_ast(context);
	}
    }
    else {
	if ($VMS_STATUS_SUCCESS(context->read_iosb.status)) {
	struct mbx_buffer_hdr * rbuf;

	     /* Now that we have a PID, watch for the child to die */
	    /*----------------------------------------------------*/
	    /* Direction = 0, child is writer */
	    /* Direction = 1, child is reader */

	    if (context->direction == 0)
	        start_child_watcher(context);

	     /* Add the data to the FIFO */
	    /*--------------------------*/
	    rbuf = malloc
	      (sizeof(struct mbx_buffer_hdr) + context->read_iosb.count);
	    if (rbuf == NULL) {
		context->read_ast_status = -2;	/* AST in fatal error state */
		LIB$STOP(SS$_INSFMEM);
	    }

	    rbuf->flink = NULL;
	    rbuf->blink = NULL;
	    rbuf->flags = 0; /* Normal data */
	    rbuf->size = context->read_iosb.count;
	    rbuf->buffer = ((char *)rbuf + sizeof(struct mbx_buffer_hdr));

	    if (context->read_iosb.count != 0) {
		memmove
		  (rbuf->buffer,
		   context->read_buffer,
		   context->read_iosb.count);
	    }

	     /* Put it on the end of the queue for a FIFO */
	    /*-------------------------------------------*/
	    INSQUEL((void *)context->rbuf_blink, rbuf);

            if (context->child_watch_ast_status == -1) {
                /* Child has already exited */
	        /* Add an EOF rbuf */
                rbuf = malloc(sizeof(struct mbx_buffer_hdr));
                if (rbuf == NULL) {
                    context->read_ast_status = -2; /* AST in fatal state */
                    LIB$STOP(SS$_INSFMEM);
                }

                rbuf->flink = NULL;
                rbuf->blink = NULL;
                rbuf->flags = 1; /* EOF flag */
                rbuf->size = 0;
                rbuf->buffer = ((char *)rbuf + sizeof(struct mbx_buffer_hdr));

	         /* Put it on the end of the queue for a FIFO */
	        /*-------------------------------------------*/
	        INSQUEL((void *)context->rbuf_blink, rbuf);

                /* Disable reading from child */
	        status = SYS$DASSGN(context->read_chan);
            }

	     /* Write out the buffer data */
	    /*---------------------------*/
	    if (context->write_ast_status == 0) {
		context->write_iosb.status = 0;
	        vms_write_mbx_ast(context);
	    }
	}
    }

     /* Set up the next read */
    /*----------------------*/
    if (context->read_ast_status >= 0) {
        status = SS$_IVCHAN;
        if (context->child_watch_ast_status >= 0) {
 	    status = SYS$QIO
	           (EFN$C_ENF,
		    context->read_chan,
		    read_it,
		    &context->read_iosb,
		    vms_read_mbx_ast,
		    context,
		    context->read_buffer,
		    context->read_devbufsiz,
		    0,0,0,0);
        }
	if (!$VMS_STATUS_SUCCESS(status)) {

	     /* Cleanup and exit time */
	    /*-----------------------*/
	    context->read_ast_status = -1;
#ifdef VMS_BP_POISON
	memset(context->read_buffer, 8, context->read_devbufsiz);
#endif
	    free(context->read_buffer);
	    context->read_buffer = NULL;
	    if (status != SS$_IVCHAN)
		status = SYS$DASSGN(context->read_chan);

	     /* Write AST does most of the cleanup */
	    /*-------------------------------------*/
	    if (context->write_ast_status == 0) {
		context->write_iosb.status = 0;
		vms_write_mbx_ast(context);
	    }
	}
    }

}

 /* Tell ASTs associated with mbx_ctx to clean up and self destruct */
/*=================================================================*/
static int cleanup_fifo(const int * pipe_fd, unsigned short chan)
{
int status;

     /* Return if nothing to do */
    /*-------------------------*/
    if (pipe_fd[1] == -1)
	return 0;

    close(pipe_fd[1]);

    /* Now send a cancel to the read mailbox.  Can not write an EOF
     * because the channel is read-only.
     * Note that the ASTs may have already closed their channels and cleaned up.
     * The channel should not have yet been re-used as BASH does not
     * use Posix Threads.
     */
    status = SYS$CANCEL(chan);
    return 0;
}


 /* Assign a VMS channel to the mailbox */
/*-------------------------------------*/
static int assign_fifo_chan(int fd, unsigned short *chan, int write_flag)
{
char mbxname[256];
char *namptr;
struct dsc$descriptor_s mbxname_dsc;
int status;
unsigned long mbx_flags;

    namptr = getname(fd, mbxname, 1);
    if (namptr == NULL) {
	return -1;
    }

     /* Assign a VMS channel to the mailbox for reading only */
    /*------------------------------------------------------*/
    mbxname_dsc.dsc$a_pointer = mbxname;
    mbxname_dsc.dsc$w_length = strlen(mbxname);
    mbxname_dsc.dsc$b_dtype = DSC$K_DTYPE_T;
    mbxname_dsc.dsc$b_class = DSC$K_CLASS_S;

    if (write_flag)
	mbx_flags = AGN$M_WRITEONLY;
    else
	mbx_flags = AGN$M_READONLY;
    status = SYS$ASSIGN(&mbxname_dsc, chan, 0, NULL, mbx_flags);
    if ($VMS_STATUS_SUCCESS(status))
	return 0;

     /* Assign failed */
    /*---------------*/
    errno = EVMSERR;
    vaxc$errno = status;
    return -1;
}


/* Install a FIFO buffer into a pipe
 *
 * pipe_fd is the fd the local process side of the FIFO pipe.
 * If it is not a pipe, this procedure does nothing.
 *
 * fifo_fd is the fd the other process side of the FIFO pipe.
 *
 * Direction is the direction of the pipe, 1 = parent is writer.
 *
 * chan is the VMS channel that a premature cleanup must use to
 * signal the ASTs that maintain the FIFO.
 *
 *===========================================================================*/

static int install_fifo
  (int pipe_fd, int * fifo_fd, int direction, unsigned short *chan)
{
int status;
int result;
struct vms_mbx *mbx_ctx;
int read_fd;
int write_fd;
int parent_fd;
char tbuf[5];
int sts;

     /* If this is not a pipe, we do not need a FIFO */
    /*----------------------------------------------*/
    if (!isapipe(pipe_fd)) {
	fifo_fd[1] = -1;
	return 0;
    }

     /* Need a file descriptor for a new pipe */
    /*---------------------------------------*/
    result = pipe(fifo_fd);
    if (result < 0)
	return result;

     /* Only need one of the descriptors */
    /*----------------------------------*/
    if (direction == 1) {

	 /* Stdin is reversed */
	/*-------------------*/
	close(fifo_fd[1]);
	fifo_fd[1] = -1;

	 /* Drain off the EOF just written by close */
	/*-----------------------------------------*/
	sts = read(fifo_fd[0], tbuf, 2);

	fifo_fd[1] = fifo_fd[0];
	fifo_fd[0] = -1;
	read_fd = pipe_fd;
	write_fd = fifo_fd[1];
    }
    else {
	close(fifo_fd[0]);
	fifo_fd[0] = -1;
	read_fd = fifo_fd[1];
	write_fd = pipe_fd;
    }

     /* get a mailbox context block for a new AST chain */
    /*-------------------------------------------------*/
    mbx_ctx = malloc(sizeof(struct vms_mbx));
    if (mbx_ctx == NULL) {
	if (fifo_fd[0] != -1)
	    close(fifo_fd[0]);
	if (fifo_fd[1] != -1)
	    close(fifo_fd[1]);
	return -1;
    }

      /* Set up the context to keep track of cleanup steps. When the */
     /*  AST's get started the ASTs have to handle the cleanup.     */
    /*-------------------------------------------------------------*/
    memset(mbx_ctx, 0, sizeof(struct vms_mbx));

     /* Get an read-only channel on the new mailbox */
    /*---------------------------------------------*/
    result = assign_fifo_chan(read_fd, &mbx_ctx->read_chan, 0);
    if (result < 0) {
	free(mbx_ctx);
	if (fifo_fd[0] != -1)
	    close(fifo_fd[0]);
	if (fifo_fd[1] != -1)
	    close(fifo_fd[1]);
	return -1;
    }

     /* Get an write-only channel on the old mailbox */
    /*----------------------------------------------*/
    result = assign_fifo_chan(write_fd, &mbx_ctx->write_chan, 1);
    if (result < 0) {
	SYS$DASSGN(mbx_ctx->read_chan);
	free(mbx_ctx);
	if (fifo_fd[0] != -1)
	    close(fifo_fd[0]);
	if (fifo_fd[1] != -1)
	    close(fifo_fd[1]);
	return -1;
    }

     /* Save the direction for cleanup */
    /*--------------------------------*/
    mbx_ctx->direction = direction;

     /* Start up the AST for the reader */
    /*---------------------------------*/
    *chan = mbx_ctx->read_chan;
    vms_read_mbx_ast(mbx_ctx);

     /* ASTs now are only ones that can touch mbx_ctx */
    /*-----------------------------------------------*/
    return 0;
}

/* wrapper for creating a FIFO for writing to a child
 *
 * In bash, sometimes a write is done to the child pipe before there is
 * a reader for the mailbox.  This FIFO buffers up the data until a
 * child appears, or all the channels are closed.
 *
 * Usage:
 *
 *	#include <unistd.h>
 *	int vms_fifo_write_pipe(int *pipe_fd);
 *	...
 *	#ifdef __VMS
 *	    sts = vms_fifo_write_pipe(pipe_fd);
 *	#else
 *	    sts = pipe(pipe_fd);
 *	#endif
 *
 *===============================================*/
int vms_fifo_write_pipe(int *pipe_fd)
{
int result;
int fifo_fd[2];
int tpipe_fd[2];
unsigned short temp_chan;

     /* Create the local end of the pipe */
    /*----------------------------------*/
    result = pipe(tpipe_fd);
    if (result < 0) {
	return result;
    }

    result = install_fifo(tpipe_fd[0], fifo_fd, 1, &temp_chan);
    if (result < 0) {
	return result;
    }

    /* Get rid of the input file descriptor */
    close(tpipe_fd[0]);


    /* The presumption is that the parent will dup2(pipe_fd[0],n)
     * before exec-ing a child, and this pipe is really one-way.
     */
    pipe_fd[0] = fifo_fd[1];
    pipe_fd[1] = tpipe_fd[1];

    return result;
}

/* wrapper for creating a FIFO for reading from a child
 *
 * In bash, sometimes the child starts writing while the parent is suspended
 * so that nothing is draining the mailbox.
 * This FIFO drains the outpuf from the child into the parent until the
 * parent decides to read it.
 *
 * Usage:
 *
 *	#include <unistd.h>
 *	int vms_fifo_read_pipe(int *pipe_fd);
 *	...
 *	#ifdef __VMS
 *	    sts = vms_fifo_read_pipe(pipe_fd);
 *	#else
 *	    sts = pipe(pipe_fd);
 *	#endif
 *
 *===============================================*/
int vms_fifo_read_pipe(int *pipe_fd)
{
int result;
int fifo_fd[2];
int tpipe_fd[2];
unsigned short temp_chan;
char tbuf[5];
int sts;

     /* Create the local end of the pipe */
    /*----------------------------------*/
    result = pipe(tpipe_fd);
    if (result < 0) {
	return result;
    }

    /* Get rid of the input which we will replace */
    close(tpipe_fd[1]);

     /* Drain off the EOF just written by close */
    /*-----------------------------------------*/
    sts = read(tpipe_fd[0], tbuf, 2);

    result = install_fifo(tpipe_fd[0], fifo_fd, 0, &temp_chan);
    if (result < 0) {
	return result;
    }

    /* The presumption is that the parent will dup2(pipe_fd[1],n)
     * before exec-ing a child, and this pipe is really one-way.
     */
    pipe_fd[0] = tpipe_fd[0];
    pipe_fd[1] = fifo_fd[1];

    /* The other 1/2 of tpipe_fd will be closed by the AST after
     * some data has been transferred
     */

    return result;
}
