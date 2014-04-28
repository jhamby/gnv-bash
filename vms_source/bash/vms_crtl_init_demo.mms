# File: VMS_CRTL_INIT_DEMO.MMS
#
# $Id: vms_crtl_init_demo.mms,v 1.1.1.1 2012/12/02 19:25:22 wb8tyw Exp $
#
# This file is used to build and and demonstrate a vms_crtl_init* object
# file that can be used for the majority of Unix programs ported to VMS.
#
# Edit History
#
# 001	J. Malmberg	Initial version
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
cflags = $(cnames)/standard=portable/warn=enable=(level4, questcode)

all : vms_objs
	write sys$output "all target built"

test : crtl_report
	write sys$output "test target built"

crtl_report : crtl_report.exe, \
	      crtl_report_gnv.exe, \
	      crtl_report_gnv_unix.exe, \
	      crtl_report_gnv_bash.exe
	write sys$output "crtl_report target built"

vms_objs : vms_crtl_init.obj, \
	   vms_crtl_init_unix.obj, \
	   vms_crtl_init_bash.obj
	write sys$output "vms_objs target built"

vms_crtl_init.obj : vms_crtl_init.c
    $(CC)$(CFLAGS)/object=$(MMS$TARGET) $(MMS$SOURCE)

vms_crtl_init_unix.obj : vms_crtl_init.c
    $(CC)$(CFLAGS)/object=$(MMS$TARGET) $(MMS$SOURCE)/define=GNV_UNIX_TOOL=1

vms_crtl_init_bash.obj : vms_crtl_init.c
    $(CC)$(CFLAGS)/object=$(MMS$TARGET) $(MMS$SOURCE) \
	/define="GNV_UNIX_SHELL=""bash"""

crtl_report.exe : crtl_report.obj
	$(LINK)/exe=$(MMS$TARGET) crtl_report.obj

crtl_report_gnv.exe : crtl_report.obj, vms_crtl_init.obj
	$(LINK)/exe=$(MMS$TARGET) crtl_report.obj, vms_crtl_init.obj

crtl_report_gnv_unix.exe : crtl_report.obj, vms_crtl_init_unix.obj
	$(LINK)/exe=$(MMS$TARGET) crtl_report.obj, vms_crtl_init_unix.obj

crtl_report_gnv_bash.exe : crtl_report.obj, vms_crtl_init_bash.obj
	$(LINK)/exe=$(MMS$TARGET) crtl_report.obj, vms_crtl_init_bash.obj


realclean : clean
	if f$search("crtl_report.exe") .nes. "" then \
		delete crtl_report.exe;*
	if f$search("crtl_report_gnv.exe") .nes. "" then \
		delete crtl_report_gnv.exe;*
	if f$search("crtl_report_gnv_unix.exe") .nes. "" then \
		delete crtl_report_gnv_unix.exe;*
	if f$search("crtl_report.exe_gnv_bash") .nes. "" then \
		delete crtl_report_gnv_bash.exe;*
	if f$search("vms_crtl_init.obj") .nes. "" then \
		delete vms_crtl_init.obj;*
	if f$search("vms_crtl_init_unix.obj") .nes. "" then \
		delete vms_crtl_init_unix.obj;*
	if f$search("vms_crtl_init_bash.obj") .nes. "" then \
		delete vms_crtl_init_bash.obj;*

clean :
	if f$search("crtl_report.lis") .nes. "" then \
		delete crtl_report.lis;*
	if f$search("crtl_report.obj") .nes. "" then \
		delete crtl_report.obj;*
	if f$search("vms_crtl_init.lis") .nes. "" then \
		delete vms_crtl_init.lis;*
