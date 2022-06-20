$! File: Build_report_termcap.com
$!
$! $Id: build_report_termcap.com,v 1.1.1.1 2012/12/02 19:25:20 wb8tyw Exp $
$!
$! 07-Jul-2010	J. Malmberg	Original
$!
$!===========================================================================
$ cwarn = "/warn=(enable=level4)"
$ cstand = "/stand=port/names=(as_is,shortened)"
$ cdef = "/define=_POSIX_EXIT=1"
$ clist = "/list/show=(include,expansion)/machine
$ cflags = cwarn + cstand + clist + cdef
$ cc := cc'cflags'/debug
$!
$cc vms_term.c
$cc report_termcap.c
$!
$link report_termcap, vms_term
$!
$report_termcap :== $lcl_root:[bash]report_termcap.exe
