$! File: version_h.com
$!
$! $Id: version_h.com,v 1.4 2013/07/28 12:14:55 robertsonericw Exp $
$!
$! This procedure builds the version.h file.
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
$! 06-Mar-2012 J. Malmberg - Original
$!
$ if f$search("version.h") .nes. "" then delete version.h;*
$ plv = 0
$ open/read plvl patchlevel.h
$ plvl_loop:
$ read/end=plvl_loop_end plvl line_in
$ line_in = f$edit(line_in, "TRIM, COMPRESS")
$ key1 = f$extract(0, 1, line_in)
$ if key1 .nes. "#" then goto plvl_loop
$ line = f$extract(1, f$length(line_in), line_in)
$ key2 = f$element(0, " ", line)
$ if key2 .nes. "define" then goto plvl_loop
$ key3 = f$element(1, " ", line)
$ if key3 .nes. "PATCHLEVEL" then goto plvl_loop
$ plv = f$element(2, " ", line)
$ plvl_loop_end:
$ close plvl
$!
$ maj_ver = "4"
$ min_ver = "2"
$ bv = "4"
$ rel = "unknown"
$ if f$search("configure.") .nes. ""
$ then
$    open/read tf configure.
$tver_loop:
$	read/end=tver_loop_end tf line_in
$	if line_in .eqs. "" then goto tver_loop
$	key1 = f$element(0, "=", line_in)
$	if key1 .nes. "PACKAGE_VERSION" then goto tver_loop
$	value1 = f$element(1, "=", line_in) - "'" - "'"
$	ver_str = f$element(0, "-", value1)
$	maj_ver = f$element(0, ".", ver_str)
$	min_ver = f$element(1, ".", ver_str)
$	rel = f$element(1, "-", value1)
$tver_loop_end:
$    close tf
$ endif
$ ver = maj_ver + "." + min_ver + "." + plv + "(" + bv + ")"
$!
$! Write it out
$ create version.h
$ open/append vers version.h
$ write vers -
"/* Version control for the shell.  This file gets changed when you say"
$ write vers -
" @version_h.com or mmk/descript=bash version.h."
$ write vers -
" It is created by version_h.com. */
$ write vers ""
$ write vers "/* The distribution version number of this shell. */
$ write vers "#define DISTVERSION ""''maj_ver'.''min_ver'"""
$ write vers ""
$ write vers "/* The last build version number of this shell. */
$ write vers "#define BUILDVERSION ''bv'"
$ write vers ""
$ write vers "#define RELSTATUS ""''rel'"""
$ write vers ""
$ write vers "#define DEFAULT_COMPAT_LEVEL ''maj_ver'''min_ver'"
$ write vers ""
$ write vers "#define SCCSVERSION ""@(#)Bash Version ''ver' ''rel' GNU/GNV"""
$ close vers
