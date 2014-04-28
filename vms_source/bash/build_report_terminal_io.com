$! File: Build_report_terminal_io.com
$!
$! $Id: build_report_terminal_io.com,v 1.1.1.1 2012/12/02 19:25:20 wb8tyw Exp $
$!
$! 26-Mar-2012	J. Malmberg	Original
$!
$!===========================================================================
$ cwarn = "/warn=(enable=level4)"
$ cstand = "/stand=port/names=(as_is,shortened)"
$ cdef = "/define=_POSIX_EXIT=1"
$ clist = "/list/show=(include,expansion)/machine
$ cinc = "/include=[]"
$ cflags = cwarn + cstand + clist + cdef + cinc
$ cc := cc'cflags'/debug
$!
$cc vms_terminal_io.c
$cc report_terminal_io.c
$!
$link report_terminal_io, vms_terminal_io
$!
$report_terminal_io :== $lcl_root:[bash-4^.2]report_terminal_io.exe
