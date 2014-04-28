$! File: gnv$bash_Startup.com
$!
$! $Id: gnv_bash_startup.com,v 1.1.1.1 2012/12/02 19:25:21 wb8tyw Exp $
$!
$! Procedure to setup the BASH images for use by programs from the
$! VMS SYSTARTUP*.COM procedure.
$!
$! Copyright 2011, John Malmberg
$!
$! Permission to use, copy, modify, and/or distribute this software for any
$! purpose with or without fee is hereby granted, provided that the above
$! copyright notice and this permission notice appear in all copies.
$!
$! THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
$! WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
$! MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
$! ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
$! WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
$! ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
$! OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
$!
$! 14-Mar-2011 J. Malmberg
$! 04-May-2011 J. Malmberg	Use GNV_PCSI_DESTINATION to find the
$!				value to assing GNV$GNU per suggestion
$!				by Martin Vorlander.
$!========================================================================
$!
$!
$! GNV$GNU if needed.
$if f$trnlnm("GNV$GNU") .eqs. ""
$then
$   x = f$trnlnm("GNU","LNM$SYSTEM_TABLE")
$   if x .nes. ""
$   then
$	write sys$output -
"Notice: logical name GNU: was found in the system table instead of GNV$GNU:"
$	write sys$output -
"This is a known bug in the GNV 2.1.3 and earlier kits."
$       define/system/exec/trans=conc GNV$GNU 'x'
$   else
$!
$!	File name per VMS standards
$!      ---------------------------
$	file1 = "sys$startup:gnv$destination_''f$getsyi("ARCH_NAME")'.com"
$!
$!      File name in GNV 2.1.3
$!	----------------------
$	file2 = "sys$startup:gnv_destination_''f$getsyi("ARCH_NAME")'.com"
$!
$!	File name before GNV 2.1.3
$!      ---------------------------
$	file3 = "sys$startup:gnv_destination''f$getsyi("ARCH_NAME")'.com"
$	arch_file = ""
$	if f$search(file1) .nes. ""
$	then
$	    arch_file = file1
$	else
$	    if f$search(file2) .nes. ""
$	    then
$		arch_file = file2
$	    else
$		if f$search("file3") .nes. "" then arch_file = file3
$	    endif
$	endif
$	if (arch_file) .nes. "" then @'arch_file'
$!
$!	Logical name per VMS standards
$!	-------------------------------
$	destination = f$trnlnm("GNV$PCSI_DESTINATION")
$!
$!	Logical name in GNV 2.1.3
$!	--------------------------
$	if destination .eqs. ""
$	then
$	    destination = f$trnlnm("GNV_PCSI_DESTINATION")
$	endif
$	if destination .eqs. ""
$	then
$           !Assume this procedure is on the same volume as the GNV install.
$           my_proc = f$environment("PROCEDURE")
$           my_dev = f$parse(my_proc,,,"DEVICE","NO_CONCEAL")
$	    destination = "''my_dev'[vms$common.gnv.]"
$	endif
$       define/system/exec/trans=conc gnv$gnu 'destination'
$   endif
$endif
$!
$!
$!
$! Install Bash as a known image.  This will allow it to launch faster
$! and with a smaller memory footprint.
$!----------------------------------------------------------------------
$file = "gnv$gnu:[bin]gnv$bash.exe"
$if .not. f$file_attributes(file, "known")
$then
$   install ADD 'file'/OPEN/SHARE/HEADER
$else
$   install REPLACE 'file'/OPEN/SHARE/HEADER
$endif
$!
$!
$all_exit:
$exit
