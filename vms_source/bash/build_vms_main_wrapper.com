$! File: Build_vms_main_wrapper.com
$!
$! $Id: build_vms_main_wrapper.com,v 1.2 2013/06/09 23:08:38 wb8tyw Exp $
$!
$! This procedure builds the vms_main_wrapper.exe file and some aliases
$!
$! If the P1 is clean, then delete all files produced by the build.
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
$!
$!======================================================================
$!
$! Force the parameter to upper case so that we do not care about the
$! case of the parameter.  Also trim off Unixy Linuxy leading dashes.
$!
$P1 = f$edit(p1, "UPCASE,TRIM") - "-" - "-"
$!
$! Need both a clean and realclean option.  Normally clean removes the
$! intermediate build products and leaves the images and such, and
$! REALCLEAN will clean up all build files.
$!
$! For this demo/test program, both targets will do a realclean.
$!
$if p1 .eqs. "CLEAN" then goto clean
$if p1 .eqs. "REALCLEAN" then goto clean
$!
$! Compile the wrapper with some standard settings.
$! Any thing that is ported from UNIX needs _POSIX_EXIT=1
$!
$! The TEST_MAIN macro causes it to build as a self test in addition
$! to being a wrapper module.
$!
$CC/DEBUG/define=(_USE_STD_STAT=1, _POSIX_EXIT=1, TEST_MAIN=1) -
	vms_main_wrapper.c/object=vms_main_wrapper_test.obj
$!
$!
$! The vms_crtl_init.c is a module that presets some selected DECC$Feature
$! settings for better Unix compatibility.
$!
$! It only needs to be linked in with a module
$!
$CC/DEBUG vms_crtl_init.c
$!
$!
$! Now link the two modules together.
$!
$link vms_main_wrapper_test, vms_crtl_init
$!
$!
$! We need to test that alternate entry points work.
$! And also that an xxx$ prefix is also stripped.
$!
$if f$search("gnv$vms_main_link.exe") .nes. ""
$then
$   set file/remove gnv$vms_main_link.exe;*
$endif
$set file/enter=gnv$vms_main_link.exe vms_main_wrapper_test.exe
$!
$!
$! We need to test that null extensions also work.
$!
$if f$search("gnv$main_wrapper.") .nes. ""
$then
$   set file/remove gnv$main_wrapper.;*
$endif
$set file/enter=gnv$main_wrapper. vms_main_wrapper_test.exe
$!
$exit
$!
$!
$! General cleanup routine
$!
$clean:
$!
$if f$search("gnv$main_wrapper.") .nes. ""
$then
$    set file/remove gnv$main_wrapper.;*
$endif
$!
$if f$search("gnv$vms_main_link.exe") .nes. ""
$then
$    set file/remove gnv$vms_main_link.exe;*
$endif
$file = "vms_main_wrapper"
$if f$search("''file'.exe") .nes. "" then delete 'file'.exe;*
$if f$search("''file'.map") .nes. "" then delete 'file'.map;*
$if f$search("''file'.dsf") .nes. "" then delete 'file'.dsf;*
$file = "vms_main_wrapper_test"
$if f$search("''file'.exe") .nes. "" then delete 'file'.exe;*
$if f$search("''file'.lis") .nes. "" then delete 'file'.lis;*
$if f$search("''file'.obj") .nes. "" then delete 'file'.obj;*
$!
$file = "vms_crtl_init"
$if f$search("''file'.lis") .nes. "" then delete 'file'.lis;*
$if f$search("''file'.obj") .nes. "" then delete 'file'.obj;*
$!
$exit
