$! File: MAKE_PCSI_BASH_KIT_NAME.COM
$!
$! $Id: make_pcsi_bash_kit_name.com,v 1.2 2012/12/11 02:35:53 wb8tyw Exp $
$!
$! Calculates the PCSI kit name for use in building an installation kit.
$! PCSI is HP's PolyCenter Software Installation Utility.
$!
$! The results are stored in as logical names so that other procedures
$! can use them.
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
$! 14-Mar-2011	J. Malmberg
$!========================================================================
$!
$! Save default
$default_dir = f$environment("DEFAULT")
$!
$! Put things back on error.
$on warning then goto all_exit
$!
$! The producer is the name or common abbreviation for the entity that is
$! making the kit.  It must be set as a logical name before running this
$! procedure.
$!
$! HP documents the producer as the legal owner of the software, but for
$! open source work, it should document who is creating the package for
$! distribution.
$!
$producer = f$trnlnm("GNV_PCSI_PRODUCER")
$if producer .eqs. ""
$then
$   write sys$output "The logical name GNV_PCSI_PRODUCER needs to be defined."
$   write sys$output "This should be set to the common abbreviation or name of"
$   write sys$output "the entity creating this kit.  If you are an individual"
$   write sys$output "then use your initials as long as they do not match"
$   write sys$output "a different well known producer prefix."
$   goto all_exit
$endif
$producer_full_name = f$trnlnm("GNV_PCSI_PRODUCER_FULL_NAME")
$if producer_full_name .eqs. ""
$then
$   write sys$output "The logical name GNV_PCSI_PRODUCER_FULL_NAME needs to"
$   write sys$output "be defined.  This should be set to the full name of"
$   write sys$output "the entity creating this kit.  If you are an individual"
$   write sys$output "then use your name."
$   write sys$output "EX: DEFINE GNV_PCSI_PRODUCER_FULL_NAME ""First M. Last"""
$   goto all_exit
$endif
$!
$write sys$output "*****"
$write sys$output "***** Producer = ''producer'"
$write sys$output "*****"
$!
$!
$! Base is one of 'VMS', 'AXPVMS', 'I64VMS', 'VAXVMS' and indicates what
$! binaries are in the kit.  A kit with just 'VMS' can be installed on all
$! architectures.
$!
$base = "VMS"
$arch_type = f$getsyi("ARCH_NAME")
$code = f$extract(0, 1, arch_type)
$if (code .eqs. "I") then base = "I64VMS"
$if (code .eqs. "V") then base = "VAXVMS"
$if (code .eqs. "A") then base = "AXPVMS"
$!
$!
$product = "bash"
$!
$!
$! We need to get the version from version.h.  It will have a lines like
$! #define DISTVERSION "1.14"
$! #define PATCHLEVEL 8
$!
$update = ""
$open/read pf patchlevel.h
$patchlevel_loop:
$   read/end=patchlevel_loop_end pf line_in
$   prefix = f$element(0, " ", line_in)
$   if prefix .nes. "#define" then goto patchlevel_loop
$   key = f$element(1, " ", line_in)
$   if key .eqs. "PATCHLEVEL"
$   then
$	value = f$element(2, " ", line_in) - """" - """"
$       update = value
$       goto patchlevel_loop_end
$   endif
$   goto patchlevel_loop
$patchlevel_loop_end:
$close pf
$!
$open/read/error=version_loop_end verf version.h
$version_loop:
$   read/end=version_loop_end verf line_in
$   if line_in .eqs. "" then goto version_loop
$   if f$locate("#define", line_in) .ne. 0 then goto version_loop
$   tag = f$element(1, " ", line_in)
$   value = f$element(2, " ", line_in) - """" - """"
$   if tag .eqs. "DISTVERSION"
$   then
$	distversion = value
$   else
$	if tag .eqs. "PATCHLEVEL"
$	then
$	    update = value
$	endif
$   endif
$   goto version_loop
$version_loop_end:
$close verf
$!
$if update .eqs. "0" then update = ""
$!
$open/read ef vms_eco_level.h
$ecolevel_loop:
$   read/end=ecolevel_loop_end ef line_in
$   prefix = f$element(0, " ", line_in)
$   if prefix .nes. "#define" then goto ecolevel_loop
$   key = f$element(1, " ", line_in)
$   value = f$element(2, " ", line_in) - """" - """"
$   if key .eqs. "VMS_BASH_ECO"
$   then
$       ECO_LEVEL = value
$	if ECO_LEVEL .eq. 0
$	then
$	    ECO_LEVEL = ""
$	else
$	    ECO_LEVEL = "E" + ECO_LEVEL
$	endif
$       goto ecolevel_loop_end
$   endif
$   goto ecolevel_loop
$ecolevel_loop_end:
$close ef
$!
$raw_version = distversion + "." + update
$!
$!
$! This translates to V0114-08 or D0115-01
$! We can not encode the snapshot date into the version due to the way that
$! the Polycenter Software Installation Utility evaluates the name.
$!
$! version_type = 'V' for a production release, and 'D' for a build from a
$! daiy CVS snapshot.
$majorver = f$element(0, ".", raw_version)
$minorver = f$element(1, ".", raw_version)
$vtype = "V"
$daily_tag = ""
$!
$!
$version_fao = "!2ZB!2ZB"
$mmversion = f$fao(version_fao, 'majorver', 'minorver')
$version = vtype + "''mmversion'"
$if update .nes. "" .or. ECO_LEVEL .nes. ""
$then
$!  The presence of an ECO implies an update
$   if update .eqs. "" .and. ECO_LEVEL .nes. "" then update = "0"
$   version = version + "-" + update + ECO_LEVEL
$else
$   version = version + "--"
$endif
$!
$! Kit type 1 is complete kit, the only type that this procedure will make.
$Kittype = 1
$!
$! Write out a logical name for the resulting base kit name.
$name = "''producer'-''base'-''product'-''version'-''kittype'"
$define GNV_PCSI_KITNAME "''name'"
$fname = "''product'-''version'"
$if daily_tag .nes. ""
$then
$   fname = fname + "-" + daily_tag
$endif
$define GNV_PCSI_FILENAME_BASE 'fname'
$write sys$output "*****"
$write sys$output "***** GNV_PCSI_KITNAME = ''name'."
$write sys$output "***** GNV_PCSI_FILENAME_BASE = ''fname'."
$write sys$output "*****"
$!
$all_exit:
$set def 'default_dir'
$exit '$status'
