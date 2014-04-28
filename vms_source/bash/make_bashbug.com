$! File: make_bashbug.com
$!
$! $Id: make_bashbug.com,v 1.1.1.1 2012/12/02 19:25:22 wb8tyw Exp $
$!
$! This procedure builds the bashbug file.
$!
$! The normal bash procedure is to execute a sed command.  On VMS
$! we can not expect that the sed command is present, so we need to
$! create a TPU program instead.
$!
$!        @sed -e "s%!MACHINE!%$(Machine)%" -e "s%!OS!%$(OS)%" \
$!             -e "s%!CFLAGS!%$(CCFLAGS)%" -e "s%!CC!%$(CC)%" \
$!             -e "s%!RELEASE!%$(Version)%" -e "s%!PATCHLEVEL!%$(PatchLevel)%" \
$!             -e "s%!MACHTYPE!%$(MACHTYPE)%" -e "s%!RELSTATUS!%$(RELSTATUS)%" \
$!             $(SUPPORT_SRC)bashbug.sh > $@
$!
$! Copyright 2012, John Malmberg
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
$! 28-Nov-2012 J. Malmberg - Original
$!
$!==================================================================
$!
$!
$hw_model = f$getsyi("HW_MODEL")
$if hw_model .lt. 1024
$then
$   arch_name = "vax"
$else
$   set process/parse=extended
$   arch_type = f$getsyi("ARCH_TYPE")
$   if arch_type .eq. 2
$   then
$       arch_name = "alpha"
$   else
$       if arch_type .eq. 3
$       then
$           arch_name = "ia64"
$       else
$           arch_name = "Unknown"
$           write sys$output "Something other than VAX, AXP or IA64 found."
$       endif
$   endif
$endif
$!
$DISTVERSION = ""
$RELSTATUS = ""
$open/read vf version.h
$version_loop:
$   if ((DISTVERSION .nes. "") .and. (RELSTATUS .nes. ""))
$   then
$	goto version_loop_end
$   endif
$   read/end=version_loop_end vf line_in
$   prefix = f$element(0, " ", line_in)
$   if prefix .nes. "#define" then goto version_loop
$   key = f$element(1, " ", line_in)
$   value = f$element(2, " ", line_in) - """" - """"
$   if key .eqs. "DISTVERSION"
$   then
$	DISTVERSION = value
$	goto version_loop
$   endif
$   if key .eqs. "BUILDVERSION"
$   then
$	BUILDVERSION = value
$	goto version_loop
$   endif
$   if key .eqs. "RELSTATUS"
$   then
$	RELSTATUS = value
$       goto version_loop
$   endif
$   goto version_loop
$version_loop_end:
$close vf
$!
$open/read pf patchlevel.h
$patchlevel_loop:
$   read/end=patchlevel_loop_end pf line_in
$   prefix = f$element(0, " ", line_in)
$   if prefix .nes. "#define" then goto patchlevel_loop
$   key = f$element(1, " ", line_in)
$   value = f$element(2, " ", line_in) - """" - """"
$   if key .eqs. "PATCHLEVEL"
$   then
$	PATCHLEVEL = value
$	goto patchlevel_loop_end
$   endif
$   goto patchlevel_loop
$patchlevel_loop_end:
$close pf
$!
$open/read mf bash.mms
$makefile_loop:
$   read/end=makefile_loop_end mf line_in
$   prefix = f$extract(0, 1, line_in)
$   if prefix .eqs. "#" then goto makefile_loop
$   key = f$element(0, "=", line_in)
$   key = f$edit(key, "trim,lowercase")
$   value = line_in - key - "="
$   value = f$edit(value, "trim")
$   if key .eqs. "crepository"
$   then
$	crepository = value
$	goto makefile_loop
$   endif
$   if key .eqs. "cnames"
$   then
$	cnames = f$element(0, "$", value) + crepository
$	goto makefile_loop
$   endif
$   if key .eqs. "cshow"
$   then
$	cshow = value
$	goto makefile_loop
$   endif
$   if key .eqs. "clist"
$   then
$	clist = f$element(0, "$", value) + cshow
$	goto makefile_loop
$   endif
$   if key .eqs. "cshow"
$   then
$	cshow = value
$	goto makefile_loop
$   endif
$   if key .eqs. "cprefix"
$   then
$	cprefix = value
$	goto makefile_loop
$   endif
$   if key .eqs. "cnowarn1"
$   then
$	cnowarn1 = value
$	goto makefile_loop
$   endif
$   if key .eqs. "cnowarn2"
$   then
$	cnowarn2 = value
$	goto makefile_loop
$   endif
$   if key .eqs. "cnowarn"
$   then
$	cnowarn = cnowarn1 + cnowarn2
$	goto makefile_loop
$   endif
$   if key .eqs. "cwarn"
$   then
$	cwarn = "/warnings=(disable=(" + cnowarn + "))"
$	goto makefile_loop
$   endif
$   if key .eqs. "cinc1"
$   then
$	cinc1 = value - "\"
$       read/end=makefile_loop_end mf line_in
$	line_in = f$edit(line_in, "trim")
$	cinc1 = cinc1 + line_in
$	goto makefile_loop
$   endif
$   if key .eqs. "cinc2"
$   then
$	cinc2 = value
$	goto makefile_loop
$   endif
$   if key .eqs. "cinc"
$   then
$	cinc = cinc1 + cinc2
$	goto makefile_loop
$   endif
$   if key .eqs. "cdefs"
$   then
$	cdefs = value
$	goto makefile_loop
$   endif
$   if key .eqs. "cflags"
$   then
$	value1 = value - "$(cnames)"
$	value1 = f$element(0, "$", value1)
$	cflags = cnames + value1 + clist + cprefix + cwarn + cinc + cdefs
$	goto makefile_loop_end
$   endif
$   goto makefile_loop
$makefile_loop_end:
$close mf
$!
$set nover
$REPLACE_CC = "DECC"
$REPLACE_OS = "vms"
$REPLACE_VENDOR = "dec"
$!
$REPLACE_Machine = arch_name
$REPLACE_MACHTYPE = arch_name + "-dec-vms"
$!
$REPLACE_PATCHLEVEL = PATCHLEVEL
$REPLACE_CCFLAGS = cflags
$!
$cflags_line1 = f$extract(0, 200, cflags)
$cflags_line2 = f$extract(200, 200, cflags)
$cflags_line3 = f$extract(400, 200, cflags)
$!
$REPLACE_RELSTATUS = RELSTATUS
$REPLACE_VERSION = DISTVERSION + "-" + RELSTATUS
$!
$!
$!        @sed -e "s%!MACHINE!%$(Machine)%" -e "s%!OS!%$(OS)%" \
$!             -e "s%!CFLAGS!%$(CCFLAGS)%" -e "s%!CC!%$(CC)%" \
$!             -e "s%!RELEASE!%$(Version)%" -e "s%!PATCHLEVEL!%$(PatchLevel)%" \
$!             -e "s%!MACHTYPE!%$(MACHTYPE)%" -e "s%!RELSTATUS!%$(RELSTATUS)%" \
$!
$! Put these in the same order as the input file for simplicity
$!
$bashbug_tpu = "lcl_root:bashbug.tpu"
$open/write tf 'bashbug_tpu'
$write tf "VMS_REPLACE('!MACHINE!', '" + REPLACE_Machine + "');"
$write tf "VMS_REPLACE('!OS!', '" + REPLACE_OS + "');"
$write tf "VMS_REPLACE('!CC!', '" + REPLACE_CC + "');"
$line_out = "VMS_REPLACE('!CFLAGS!', """");"
$write/symbol tf line_out
$line_out = "COPY_TEXT('" + cflags_line1 + "');"
$write/symbol tf line_out
$line_out = "COPY_TEXT('" + cflags_line2 + "');"
$write/symbol tf line_out
$line_out = "COPY_TEXT('" + cflags_line3 + "');"
$write/symbol tf line_out
$write tf "VMS_REPLACE('!RELEASE!', '" + REPLACE_VERSION + "');"
$write tf "VMS_REPLACE('!PATCHLEVEL!', '" + REPLACE_PATCHLEVEL + "');"
$write tf "VMS_REPLACE('!RELSTATUS!', '" + REPLACE_RELSTATUS + "');"
$write tf "VMS_REPLACE('!MACHTYPE!', '" + REPLACE_MACHTYPE + "');"
$!
$write tf "VMS_REPLACE('bbug.$$', 'bbug_$$');"
$!
$close tf
$!
$set ver
$!
$eve_command = "/command=prj_root:unix_c_to_vms_c.tpu"
$silent_eve := edit/tpu/section=eve$section/nodisplay'eve_command'
$!
$silent_eve [.support]bashbug.sh/out=lcl_root:bashbug./init='bashbug_tpu'
$!
$set nover
$!if f$search(bashbug_tpu) .nes. "" then delete 'bashbug_tpu';*
$!
$exit
