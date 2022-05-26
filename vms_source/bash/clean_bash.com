$! File: Clean_bash.com
$!
$! $Id: clean_bash.com,v 1.3 2013/06/12 04:21:47 wb8tyw Exp $
$!
$! This procedure cleans up the Bash project of unneeded files.
$!
$! This was originally part of the file MAKEFILE.COM and will still remove
$! any files that MAKEFILE.COM may have generated.
$!
$! Since Bash 1.4.8 can run configure scripts, we also need to clean up
$! the files that the configure scripts can leave behind
$!
$! See the HELP section below for the parameters.
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
$! 12-Jun-2010 J. Malmberg - Original based on MAKEFILE.COM
$! 01-Mar-2012 J. Malmberg - Update for Bash 4.2.
$!
$!============================================================================
$!
$p1 = f$edit(p1,"UPCASE,TRIM") - "-" - "-"
$if p1 .nes. "REALCLEAN" then goto clean
$!
$if ((p1 .eqs. "HELP") .or. (P1 .eqs. "?"))
$then
$   write sys$output "Clean_bash.com - Remove build files from bash project"
$   write sys$output ""
$   write sys$putout " The default action is to remove everything except the"
$   write sys$output " final bash binaries and debug files.
$   write sys$output ""
$   write sys$output " P1 = REALCLEAN, Also causes the Bash binaries to be"
$   write sys$output " removed."
$   write sys$output ""
$   write sys$output " P1 = HELP or ?, Prints out this help message.
$   write sys$output ""
$   goto all_exit
$endif
$!
$realclean:
$   file = "BASH"
$   if f$search("''file'.EXE") .nes. "" then delete 'file'.EXE;*
$   if f$search("''file'.DSF") .nes. "" then delete 'file'.DSF;*
$   if f$search("''file'.STB") .nes. "" then delete 'file'.STB;*
$   if f$search("''file'.MAP") .nes. "" then delete 'file'.MAP;*
$   file = "GNV$BASH"
$   if f$search("''file'.EXE") .nes. "" then delete 'file'.EXE;*
$   if f$search("''file'.DSF") .nes. "" then delete 'file'.DSF;*
$   if f$search("''file'.STB") .nes. "" then delete 'file'.STB;*
$   if f$search("''file'.MAP") .nes. "" then delete 'file'.MAP;*
$   file = "BASHDEBUG"
$   if f$search("''file'.EXE") .nes. "" then delete 'file'.EXE;*
$   if f$search("''file'.DSF") .nes. "" then delete 'file'.DSF;*
$   if f$search("''file'.STB") .nes. "" then delete 'file'.STB;*
$   if f$search("''file'.MAP") .nes. "" then delete 'file'.MAP;*
$   file = "lcl_root:gnv$bash_startup.com"
$   if f$search(file) .nes. "" then delete 'file';*
$   file = "lcl_root:*.pcsi$desc"
$   if f$search(file) .nes. "" then delete 'file';*
$   file = "lcl_root:*.pcsi$text"
$   if f$search(file) .nes. "" then delete 'file';*
$   file = "lcl_root:*.release_notes"
$   if f$search(file) .nes. "" then delete 'file';*
$   file = "lcl_root:*.bck"
$   if f$search(file) .nes. "" then delete 'file';*
$   file = "lcl_root:config.log"
$   if f$search(file) .nes. "" then delete 'file';*
$   file = "lcl_root:config.status"
$   if f$search(file) .nes. "" then delete 'file';*
$   file = "lcl_root:[.sys]param.h"
$   if f$search(file) .nes. "" then delete 'file';*
$   file = "lcl_root:[.builtins.sys]param.h"
$   if f$search(file) .nes. "" then delete 'file';*
$   file = "lcl_root:[.lib.sh.sys]param.h"
$   if f$search(file) .nes. "" then delete 'file';*
$   file = "report_termcap"
$   if f$search("''file'.EXE") .nes. "" then delete 'file'.EXE;*
$   if f$search("''file'.DSF") .nes. "" then delete 'file'.DSF;*
$   if f$search("''file'.STB") .nes. "" then delete 'file'.STB;*
$   if f$search("''file'.MAP") .nes. "" then delete 'file'.MAP;*
$   file = "report_terminal_io"
$   if f$search("''file'.EXE") .nes. "" then delete 'file'.EXE;*
$   if f$search("''file'.DSF") .nes. "" then delete 'file'.DSF;*
$   if f$search("''file'.STB") .nes. "" then delete 'file'.STB;*
$   if f$search("''file'.MAP") .nes. "" then delete 'file'.MAP;*
$   file = "[.support]printenv"
$   if f$search("''file'.EXE") .nes. "" then delete 'file'.EXE;*
$   if f$search("''file'.DSF") .nes. "" then delete 'file'.DSF;*
$   if f$search("''file'.STB") .nes. "" then delete 'file'.STB;*
$   if f$search("''file'.MAP") .nes. "" then delete 'file'.MAP;*
$   file = "[.support]recho"
$   if f$search("''file'.EXE") .nes. "" then delete 'file'.EXE;*
$   if f$search("''file'.DSF") .nes. "" then delete 'file'.DSF;*
$   if f$search("''file'.STB") .nes. "" then delete 'file'.STB;*
$   if f$search("''file'.MAP") .nes. "" then delete 'file'.MAP;*
$   file = "[.support]xcase"
$   if f$search("''file'.EXE") .nes. "" then delete 'file'.EXE;*
$   if f$search("''file'.DSF") .nes. "" then delete 'file'.DSF;*
$   if f$search("''file'.STB") .nes. "" then delete 'file'.STB;*
$   if f$search("''file'.MAP") .nes. "" then delete 'file'.MAP;*
$   file = "[.support]zecho"
$   if f$search("''file'.EXE") .nes. "" then delete 'file'.EXE;*
$   if f$search("''file'.DSF") .nes. "" then delete 'file'.DSF;*
$   if f$search("''file'.STB") .nes. "" then delete 'file'.STB;*
$   if f$search("''file'.MAP") .nes. "" then delete 'file'.MAP;*
$!
$clean:
$   file = "conf*.file"
$   if f$search("lcl_root:[.conf*]''file'") .nes. ""
$   then
$	delete lcl_root:[.conf*]'file';*
$   endif
$   file = "*.awk"
$   if f$search("lcl_root:[.conf*]''file'") .nes. ""
$   then
$	delete lcl_root:[.conf*]'file';*
$   endif
$   if f$search("lcl_root:[]conf*.dir") .nes. ""
$   then
$	set prot=o:rwed lcl_root:[]conf*.dir
$       delete lcl_root:[]conf*.dir;
$   endif
$   file = "stamp-h"
$   if f$search("lcl_root:''file'.") .nes. "" then delete lcl_root:'file'.;*
$   file = "confdefs"
$   if f$search("lcl_root:''file'.h") .nes. "" then delete lcl_root:'file'.h;*
$   file = "conftest"
$   if f$search("lcl_root:''file'.dsf") .nes. ""
$   then
$       delete lcl_root:'file'.dsf;*
$   endif
$   file = "gnv$version"
$   if f$search("lcl_root:''file'.c_first") .nes. ""
$   then
$	delete lcl_root:'file'.c_first;*
$   endif
$   file = "temp_stub"
$   if f$search("lcl_root:''file'.*") .nes. ""
$   then
$	delete lcl_root:'file'.*;*
$   endif
$   file = "PIPESIZE"
$   if f$search("lcl_root:''file'.h") .nes. "" then delete lcl_root:'file'.h;*
$   file = "CONFIG"
$   if f$search("lcl_root:''file'.h") .nes. "" then delete lcl_root:'file'.h;*
$   file = "ALIAS"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "ARRAY"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "ARRAYFUNC"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "ASSOC"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "BASHHIST"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "BASHLINE"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   if f$search("lcl_root:''file'.c") .nes. "" then delete lcl_root:'file'.c;*
$   file = "BIND"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "BINDTEXTDOM"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "BRACECOMP"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "BRACES"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "BUILDSIGNAMES"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "CALLBACK"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "CASEMOD"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "CLKTCK"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "CLOCK"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "COMPAT"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "COMPLETE"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "LCL_ROOT:CONFIG.H"
$   if f$search("''file'") .nes. "" then delete 'file';*
$   file = "LCL_ROOT:CONFIG_VMS.H"
$   if f$search("''file'") .nes. "" then delete 'file';*
$   file = "COPY_CMD"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "DCGETTEXT"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "DCIGETTEXT"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "DCNGETTEXT"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "DGETTEXT"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "DISPLAY"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "DISPOSE_CMD"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "DNGETTEXT"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "DPRINT"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "DPRINTF"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "EACCESS"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.lib.sh]gnv$eaccess.c_first"
$   if f$search("''file'") .nes. "" then delete 'file';*
$   file = "ERROR"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "EVAL"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "EXECUTE_CMD"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   if f$search("lcl_root:''file'.c") .nes. "" then delete lcl_root:'file'.c;*
$   if f$search("lcl_root:''file'.h") .nes. "" then delete lcl_root:'file'.h;*
$   file = "EXPLODENAME"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "EXPR"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "FINDCMD"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "FINDDOMAIN"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "FLAGS"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "FMTULONG"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "FMTULLONG"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "FMTUMAX"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "FNXFORM"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "FPURGE"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "FUNMAP"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "GENERAL"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "GETCWD"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "GETENV"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "GETTEXT"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "GLOB"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "GMISC"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "HASHCMD"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "HASHLIB"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "HISTEXPAND"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "HISTFILE"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "HISTORY"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "HISTSEARCH"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "INET_ATON"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "INPUT"
$   if f$search("lcl_root:''file'.c") .nes. ""
$   then
$	delete lcl_root:'file'.c;*
$   endif
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "INPUT_AVAIL"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "INTL-COMPAT"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "ISEARCH"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "ITOS"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "JOBS"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "KEYMAPS"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "KILL"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "L10NFLIST"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "LIST"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "LOADMSGCAT"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "LOCALCHARSET"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "LOCALE"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "LOCALEALIAS"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "LOCALENAME"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "LOG"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "LCL_ROOT:LSIGNAMES.H"
$   if f$search("''file'") .nes. "" then delete 'file';*
$   file = "MACRO"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "MAILCHECK"
$   if f$search("lcl_root:''file'.c") .nes. ""
$   then
$	delete lcl_root:'file'.c;*
$   endif
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "gnv$mailcheck"
$   if f$search("lcl_root:''file'.c_first") .nes. ""
$   then
$	delete lcl_root:'file'.c_first;*
$   endif
$   file = "MAILSTAT"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "MAKEPATH"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "MAKE_CMD"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "MBSCASECMP"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "MBSCHR"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "MBSCMP"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "MBUTIL"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "MEMSET"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "MISC"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "MKSIGNAMES"
$   if f$search("''file'.EXE") .nes. "" then delete 'file'.EXE;*
$   if f$search("''file'.DBG") .nes. "" then delete 'file'.DBG;*
$   if f$search("''file'.MAP") .nes. "" then delete 'file'.MAP;*
$   if f$search("''file'.STB") .nes. "" then delete 'file'.STB;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "MKSYNTAX"
$   if f$search("''file'.EXE") .nes. "" then delete 'file'.EXE;*
$   if f$search("''file'.DBG") .nes. "" then delete 'file'.DBG;*
$   if f$search("''file'.MAP") .nes. "" then delete 'file'.MAP;*
$   if f$search("''file'.STB") .nes. "" then delete 'file'.STB;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "MKTIME"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "NETCONN"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "NETOPEN"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "NGETTEXT"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "NLS"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "NOJOBS"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   if f$search("lcl_root:''file'.C") .nes. "" then delete lcl_root:'file'.C;*
$   file = "OSDEP"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "OSLIB"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.lib.sh]gnv$oslib.c_first"
$   if f$search("''file'") .nes. "" then delete 'file';*
$   file = "PARENS"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "PATHCANON"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "PATHEXP"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "LCL_ROOT:PATHNAMES.H"
$   if f$search("''file'") .nes. "" then delete 'file';*
$   file = "PATHNAMES"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "PATHPHYS"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "PCOMPLETE"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "PCOMPLIB"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "PIPESIZE"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.H;*
$   file = "PLURAL-EXP"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "PLURAL"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "PRINT_CMD"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   if f$search("lcl_root:''file'.c") .nes. "" then delete lcl_root:'file'.c;*
$   file = "READLINE"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "REDIR"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   if f$search("lcl_root:''file'.C") .nes. "" then delete lcl_root:'file'.C;*
$   file = "RELOCATABLE"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "RENAME"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "RLTTY"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "SAVESTRING"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "SEARCH"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "SETLINEBUF"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "SHELL"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   if f$search("lcl_root:''file'.c") .nes. "" then delete lcl_root:'file'.c;*
$   if f$search("lcl_root:''file'.c1") .nes. ""
$   then
$	delete lcl_root:'file'.c1;*
$   endif
$   file = "lcl_root:[]GNV$SHELL.C_FIRST"
$   if f$search("''file'") .nes. "" then delete 'file';*
$   file = "SHMATCH"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "SHMBCHAR"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "SHQUOTE"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "SHTTY"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "SIG"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "SIGLIST"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "SIGNALS"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "SIGNAMES"
$   if f$search("''file'.H") .nes. "" then delete 'file'.H;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "SMATCH"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "SNPRINTF"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "SPELL"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "STRCASECMP"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "STRCASESTR"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "STRCHRNUL"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "STRERROR"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "STRFTIME"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "STRINGLIB"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "STRINGLIST"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "STRINGVEC"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "STRMATCH"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "STRNLEN"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "STRPBRK"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "STRTOD"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "STRTOIMAX"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "STRTOL"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "STRTOLL"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "STRTOUL"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "STRTOUMAX"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "STRTRANS"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "SUBST"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   if f$search("lcl_root:''file'.c") .nes. "" then delete lcl_root:'file'.c;*
$   file = "lcl_root:SYNTAX"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "TERMINAL"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "TEST"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "TEXT"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "TEXTDOMAIN"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "TILDE"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "TIMEVAL"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "TMPFILE"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "TRAP"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   if f$search("lcl_root:''file'.C") .nes. "" then delete lcl_root:'file'.C;*
$   if f$search("lcl_root:''file'.h") .nes. "" then delete lcl_root:'file'.h;*
$   file = "UCONVERT"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "UFUNCS"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "UNDO"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "UNICODE"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "UNWIND_PROT"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   if f$search("lcl_root:''file'.c") .nes. "" then delete lcl_root:'file'.c;*
$   file = "UTIL"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "VARIABLES"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   if f$search("lcl_root:''file'.c") .nes. "" then delete lcl_root:'file'.c;*
$   file = "LCL_ROOT:VERSION"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.H") .nes. "" then delete 'file'.H;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "VI_MODE"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "VMS_CRTL_INIT"
$   if f$search("''file'_BASH.OBJ") .nes. "" then delete 'file'_BASH.OBJ;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "VMS_FAKEFORK"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "VMS_MAILSTAT"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "VPRINT"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "WCSDUP"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "WCSWIDTH"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "WINSIZE"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "XFREE"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "XMALLOC"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "XMBSRTOWCS"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "Y^.TAB"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "Y_TAB"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   if f$search("lcl_root:''file'.c") .nes. "" then delete lcl_root:'file'.c;*
$   if f$search("lcl_root:''file'.h") .nes. "" then delete lcl_root:'file'.h;*
$   file = "ZCATFD"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "ZGETLINE"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "ZMAPFD"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "ZREAD"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "ZWRITE"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$!
$   file = "DECW_SHOWDISPLAY"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "VMS_TERM"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "VMS_VM_PIPE"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "VMS_TERMINAL_IO"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "REPORT_TERMCAP"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "REPORT_TERMINAL_IO"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$!
$   !BUILTINS
$   file = "LIBBUILTINS"
$   if f$search("''file'.OLB") .nes. "" then delete 'file'.OLB;*
$   file = "lcl_root:[.builtins]GNV$COMMON.C_FIRST"
$   if f$search("''file'") .nes. "" then delete 'file';*
$   file = "lcl_root:[.builtins]GNV$PRINTF.C_FIRST"
$   if f$search("''file'") .nes. "" then delete 'file';*
$   file = "[.BUILTINS]ALIAS"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]BASHGETOPT"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]BIND"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]BREAK"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "lcl_root:[.BUILTINS]BUILTEXT"
$   if f$search("''file'.H") .nes. "" then delete 'file'.H;*
$   file = "[.BUILTINS]BUILTIN"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]BUILTINS"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]CALLER"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]CD"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]COLON"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]COMMAND"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]COMMON"
$   if f$search("lcl_root:''file'.c") .nes. ""
$   then
$	delete lcl_root:'file'.c;*
$   endif
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]COMPLETE"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]DECLARE"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]ECHO"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]ENABLE"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]EVAL"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "LCL_ROOT:[.BUILTINS]EVALFILE"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "lcl_root:[.builtins]GNV$EVALFILE.C_FIRST"
$   if f$search("''file'") .nes. "" then delete 'file';*
$   file = "LCL_ROOT:[.BUILTINS]EVALSTRING"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]EXEC"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   if f$search("lcl_root:''file'.def") .nes. ""
$   then
$	delete lcl_root:'file'.def;*
$   endif
$   file = "[.BUILTINS]EXIT"
$   if f$search("lcl_root:''file'.def") .nes. ""
$   then
$	delete lcl_root:'file'.def;*
$   endif
$   if f$search("''file'") .nes. "" then delete 'file';*
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]FC"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]FG_BG"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]GETOPT"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]GETOPTS"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]HASH"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]HELP"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]HISTORY"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]INLIB"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]JOBS"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]KILL"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]LET"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]MAPFILE"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]MKBUILTINS"
$   if f$search("lcl_root:''file'.c") .nes. "" then delete lcl_root:'file'.c;*
$   if f$search("''file'.EXE") .nes. "" then delete 'file'.EXE;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   if f$search("''file'.MAP") .nes. "" then delete 'file'.MAP;*
$   if f$search("''file'.STB") .nes. "" then delete 'file'.STB;*
$   if f$search("''file'.DSF") .nes. "" then delete 'file'.DSF;*
$   file = "[.BUILTINS]PRINTF"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]PUSHD"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]READ"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]RETURN"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]SET"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]SETATTR"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]SHIFT"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]SHOPT"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]SOURCE"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]SUSPEND"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]TEST"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]TIMES"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]TRAP"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]TYPE"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]ULIMIT"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.C1") .nes. "" then delete 'file'.C1;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "lcl_root:[.builtins]GNV$ULIMIT.C_FIRST"
$   if f$search("''file'") .nes. "" then delete 'file';*
$   file = "[.BUILTINS]UMASK"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.BUILTINS]WAIT"
$   if f$search("''file'.C") .nes. "" then delete 'file'.C;*
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$!
$!  ![.BUILTINS]CXX_REPOSITORY.DIR
$   file = "lcl_root:[.BUILTINS.CXX_REPOSITORY]CXX$DEMANGLER_DB"
$   if f$search("''file'") .nes. "" then delete 'file';*
$   file = "lcl_root:[.BUILTINS]CXX_REPOSITORY.DIR"
$   if f$search("''file'") .nes. "" then set file/prot=o:rwed 'file';*
$   if f$search("''file'") .nes. "" then delete 'file';*
$!
$!  !CXX_REPOSITORY.DIR
$   file = "lcl_root:[.CXX_REPOSITORY]CXX$DEMANGLER_DB"
$   if f$search("''file'") .nes. "" then delete 'file';*
$   file = "lcl_root:CXX_REPOSITORY.DIR"
$   if f$search("''file'") .nes. "" then set file/prot=o:rwed 'file';*
$   if f$search("''file'") .nes. "" then delete 'file';*
$!
$   ![LIB.GLOB]
$   file = "LIBGLOB"
$   if f$search("''file'.OLB") .nes. "" then delete 'file'.OLB;*
$   file = "[.LIB.GLOB]GLOB"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.GLOB]GMISC"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.GLOB]SMATCH"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   if f$search("''file'.OLB") .nes. "" then delete 'file'.OLB;*
$   file = "[.LIB.GLOB]STRMATCH"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.GLOB]XMBSRTOWCS"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$!
$!  ![.LIB.GLOB]CXX_REPOSITORY.DIR
$   file = "lcl_root:[.LIB.GLOB.CXX_REPOSITORY]CXX$DEMANGLER_DB"
$   if f$search("''file'") .nes. "" then delete 'file';*
$   file = "lcl_root:[.LIB.GLOB]CXX_REPOSITORY.DIR"
$   if f$search("''file'") .nes. "" then set file/prot=o:rwed 'file';*
$   if f$search("''file'") .nes. "" then delete 'file';*
$!
$   ![LIB.INTL]
$   file = "LIBINTL"
$   if f$search("''file'.OLB") .nes. "" then delete 'file'.OLB;*
$   file = "[.LIB.INTL]BINDTEXTDOM"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.INTL]DCGETTEXT"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.INTL]DCIGETTEXT"
$   if f$search("lcl_root:''file'.c") .nes. ""
$   then
$	delete lcl_root:'file'.c;*
$   endif
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "lcl_root:[.lib.intl]GNV$DCIGETTEXT.C_FIRST"
$   if f$search("''file'") .nes. "" then delete 'file';*
$   file = "lcl_root:[.lib.intl]gettextp.h"
$   if f$search("''file'") .nes. "" then delete 'file';*
$   file = "[.LIB.INTL]DCNGETTEXT"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.INTL]DGETTEXT"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.INTL]DNGETTEXT"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.INTL]EXPLODENAME"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.INTL]FINDDOMAIN"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.INTL]GETTEXT"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.INTL]INTL-COMPAT"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.INTL]L10NFLIST"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "LCL_ROOT:[.LIB.INTL]libgnuintl.h"
$   if f$search("''file'") .nes. "" then delete 'file';*
$   file = "[.LIB.INTL]LOADMSGCAT"
$   if f$search("lcl_root:''file'.c") .nes. ""
$   then
$	delete lcl_root:'file'.c;*
$   endif
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "lcl_root:[.lib.intl]GNV$LOADMSGCAT.C_FIRST"
$   if f$search("''file'") .nes. "" then delete 'file';*
$   file = "[.LIB.INTL]LOCALCHARSET"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.INTL]LOCALEALIAS"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.INTL]LOCALENAME"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.INTL]LOG"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.INTL]NGETTEXT"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.INTL]OSDEP"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.INTL]PLURAL-EXP"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.INTL]PLURAL"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.INTL]RELOCATABLE"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.INTL]TEXTDOMAIN"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$!
$!  ![.LIB.MALLOC]CXX_REPOSITORY.DIR
$   file = "lcl_root:[.LIB.INTL.CXX_REPOSITORY]CXX$DEMANGLER_DB"
$   if f$search("''file'") .nes. "" then delete 'file';*
$   file = "lcl_root:[.LIB.INTL]CXX_REPOSITORY.DIR"
$   if f$search("''file'") .nes. "" then set file/prot=o:rwed 'file';*
$   if f$search("''file'") .nes. "" then delete 'file';*
$!
$   ![LIB.READLINE]
$   file = "LIBREADLINE"
$   if f$search("''file'.OLB") .nes. "" then delete 'file'.OLB;*
$   file = "[.LIB.READLINE]BIND"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.READLINE]CALLBACK"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.READLINE]COLORS"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.READLINE]COMPAT"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.READLINE]COMPLETE"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.READLINE]DISPLAY"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.READLINE]FUNMAP"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.READLINE]HISTEXPAND"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.READLINE]HISTFILE"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.READLINE]HISTORY"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.READLINE]HISTSEARCH"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.READLINE]INPUT"
$   if f$search("lcl_root:''file'.c") .nes. ""
$   then
$	delete lcl_root:'file'.c;*
$   endif
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "lcl_root:[.lib.readline]GNV$INPUT.C_FIRST"
$   if f$search("''file'") .nes. "" then delete 'file';*
$   file = "[.LIB.READLINE]ISEARCH"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.READLINE]KEYMAPS"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.READLINE]KILL"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.READLINE]MACRO"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.READLINE]MBUTIL"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.READLINE]MISC"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.READLINE]NLS"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.READLINE]PARENS"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.READLINE]PARSE-COLORS"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.READLINE]READLINE"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.READLINE]RLTTY"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.READLINE]SAVESTRING"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.READLINE]SEARCH"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.READLINE]SHELL"
$   if f$search("lcl_root:''file'.c") .nes. ""
$   then
$	delete lcl_root:'file'.c;*
$   endif
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "lcl_root:[.lib.readline]GNV$SHELL.C_FIRST"
$   if f$search("''file'") .nes. "" then delete 'file';*
$   file = "[.LIB.READLINE]SIGNALS"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   if f$search("lcl_root:''file'.c") .nes. ""
$   then
$	delete lcl_root:'file'.c;*
$   endif
$   file = "[.LIB.READLINE]TERMINAL"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.READLINE]TEXT"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.READLINE]TILDE"
$   if f$search("lcl_root:''file'.c") .nes. ""
$   then
$	delete lcl_root:'file'.c;*
$   endif
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "lcl_root:[.lib.readline]GNV$TILDE.C_FIRST"
$   if f$search("''file'") .nes. "" then delete 'file';*
$   file = "[.LIB.READLINE]UNDO"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.READLINE]UTIL"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.READLINE]VI_MODE"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.READLINE]XFREE"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.READLINE]XMALLOC"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$!
$!  ![.LIB.READLINE]CXX_REPOSITORY.DIR
$   file = "lcl_root:[.LIB.READLINE.CXX_REPOSITORY]CXX$DEMANGLER_DB"
$   if f$search("''file'") .nes. "" then delete 'file';*
$   file = "lcl_root:[.LIB.READLINE]CXX_REPOSITORY.DIR"
$   if f$search("''file'") .nes. "" then set file/prot=o:rwed 'file';*
$   if f$search("''file'") .nes. "" then delete 'file';*
$!
$   ![LIB.REGEX]
$   file = "[.LIB.REGEX]REGEX"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$!
$   ![LIB.TILDE]
$   file = "[.LIB.TILDE]TILDE"
$   if f$search("lcl_root:''file'.c") .nes. ""
$   then
$	delete lcl_root:'file'.c;*
$   endif
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   if f$search("''file'.OLB") .nes. "" then delete 'file'.OLB;*
$   file = "lcl_root:[.lib.tilde]GNV$TILDE.C_FIRST"
$   if f$search("''file'") .nes. "" then delete 'file';*
$!
$!  ![.LIB.TILDE]CXX_REPOSITORY.DIR
$   file = "lcl_root:[.LIB.TILDE.CXX_REPOSITORY]CXX$DEMANGLER_DB"
$   if f$search("''file'") .nes. "" then delete 'file';*
$   file = "lcl_root:[.LIB.TILDE]CXX_REPOSITORY.DIR"
$   if f$search("''file'") .nes. "" then set file/prot=o:rwed 'file';*
$   if f$search("''file'") .nes. "" then delete 'file';*
$
$   ![LIB.SH]
$   file = "shlib"
$   if f$search("''file'.OLB") .nes. "" then delete 'file'.OLB;*
$   file = "libsh"
$   if f$search("''file'.OLB") .nes. "" then delete 'file'.OLB;*
$   file = "[.LIB.SH]CASEMOD"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]CLKTCK"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]CLOCK"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]DPRINTF"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]EACCESS"
$   if f$search("lcl_root:''file'.c") .nes. ""
$   then
$	delete lcl_root:'file'.c;*
$   endif
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]FMTULLONG"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]FMTULONG"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]FMTUMAX"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]FNXFORM"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]FPURGE"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]GETCWD"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]GETENV"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]INET_ATON"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]INPUT_AVAIL"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]ITOS"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]MAILSTAT"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]MAKEPATH"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]MBSCASECMP"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]MBSCHR"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]MBSCMP"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]MEMSET"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]MKTIME"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]NETCONN"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]NETOPEN"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]OSLIB"
$   if f$search("lcl_root:''file'.c") .nes. ""
$   then
$	delete lcl_root:'file'.c;*
$   endif
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]PATHCANON"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]PATHPHYS"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]RENAME"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]SETLINEBUF"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]SHMATCH"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]SHMBCHAR"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]SHQUOTE"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]SHTTY"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]SNPRINTF"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]SPELL"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]STRCASECMP"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]STRCASESTR"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]STRCHRNUL"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]STRERROR"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]STRFTIME"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]STRINGLIST"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]STRINGVEC"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]STRNLEN"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]STRPBRK"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]STRTOD"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]STRTOIMAX"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]STRTOL"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]STRTOLL"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]STRTOUL"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]STRTOUMAX"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]STRTRANS"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]TIMEVAL"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]TMPFILE"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]UCONVERT"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]UFUNCS"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]UNICODE"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]VPRINT"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]WCSDUP"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]WCSNWIDTH"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]WCSWIDTH"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]WINSIZE"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]ZCATFD"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]ZGETLINE"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]ZMAPFD"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]ZREAD"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.LIB.SH]ZWRITE"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$!
$   ![.support]
$   file = "[.support]printenv"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   file = "printenv"
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.support]recho"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   file = "recho"
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.support]xcase"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   file = "xcase"
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$   file = "[.support]zecho"
$   if f$search("''file'.OBJ") .nes. "" then delete 'file'.OBJ;*
$   file = "zecho"
$   if f$search("''file'.LIS") .nes. "" then delete 'file'.LIS;*
$!
$   file = "[.po]Makefile"
$   if f$search("lcl_root:''file'.in") .nes. "" then delete lcl_root:'file'.in;*
$   file = "[.po]potfiles"
$   if f$search("lcl_root:''file'.") .nes. "" then delete lcl_root:'file'.;*
$!
$all_exit:
$  exit
