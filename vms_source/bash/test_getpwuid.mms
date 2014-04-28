# File: TEST_GETPWUID.MMS
#
# $Id: test_getpwuid.mms,v 1.1.1.1 2012/12/02 19:25:22 wb8tyw Exp $
#
# Builds a program to test a wrapper to getpwuid()
#
# Copyright 2012, John Malmberg
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
# OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
#============================================================================
cnames = /names=(shortened,as_is)
cdef = /define=(__USE_LONG_GID=1,VMS_PASSWORD_WRAPPER=1,_POSIX_EXIT=1)
cwarn = /standard=portable/warn=enable=(level4, questcode)
clist = /list
cflags = $(cnames)$(cdef)$(cwarn)$(clist)

all : test_getpwuid
	write sys$output "all targets built"

test_getenv.exe : test_getenv.obj, vms_crtl_init.obj,
	$(LINK)/exe=$(MMS$TARGET) test_getenv.obj, vms_crtl_init.obj

test_getpwuid : test_getpwuid.exe
	write sys$output "test_getpwuid target built"

test_getpwuid.exe : test_getpwuid.obj, vms_crtl_init_test_shell.obj,
	$(LINK)/exe=$(MMS$TARGET) test_getpwuid.obj, \
		vms_crtl_init_test_shell.obj

test_getpwuid.obj : test_getpwuid.c vms_pwd_hack.h


vms_crtl_init_test_shell.obj : vms_crtl_init.c
    $(CC)$(CFLAGS)/define="GNV_UNIX_SHELL=""test""" \
     /object=$(MMS$TARGET) $(MMS$SOURCE)

realclean : clean
	if f$search("test_getpwuid.exe") .nes. "" then \
		delete getpwuid.exe;*
	if f$search("vms_crtl_init_test_shell.obj") .nes. "" then \
		delete vms_crtl_init_test_shell.obj;*

clean :
	if f$search("vms_crtl_init.lis") .nes. "" then \
		delete vms_crtl_init.lis;*
	if f$search("test_getpwuid.lis") .nes. "" then \
		delete test_getpwuid.lis;*
	if f$search("test_getpwuid.obj") .nes. "" then \
		delete test_getpwuid.obj;*
	if f$search("vms_crtl_init.lis") .nes. "" then \
		delete vms_crtl_init.lis;*
