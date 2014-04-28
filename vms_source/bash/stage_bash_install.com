$! File: stage_bash_install.com
$!
$! $Id: stage_bash_install.com,v 1.5 2013/07/18 00:26:57 wb8tyw Exp $
$!
$! This updates or removes the GNV$BASH.EXE and related files for the
$! new_gnu:[*...] directory tree for running the self tests.
$!
$! The files installed/removed are:
$!     [bin]gnv$bash.exe
$!     [bin]bash. hard link for [usr.bin]gnv$bash.exe
$!     [bin]sh.  link for [usr.bin]gnv$bash.exe
$!     [usr.bin]bashbug.  ! Future
$!     [xpg4.bin]sh. incorrect link for [usr.bin]gnv$bash.exe
$!     [usr.xpg4.bin]sh. incorrect link for [usr.bin]gnv$bash.exe
$!     [usr.share.man.man1]bash.1
$!     [usr.share.man.man1]bashbug.1  ! Future
$!     [usr.share.doc.bash]AUTHORS.
$!     [usr.share.doc.bash]CHANGES.
$!     [usr.share.doc.bash]COMPAT.
$!     [usr.share.doc.bash]COPYING.
$!     [usr.share.doc.bash]ChangeLog.
$!     [usr.share.doc.bash]NEWS.
$!     [usr.share.doc.bash]NOTES.
$!     [usr.share.doc.bash]POSIX.
$!     [usr.share.doc.bash]RBASH.
$!     [usr.share.doc.bash]README.
$!     [usr.share.doc.bash]article.pdf
$!     [usr.share.doc.bash]bash.pdf
$!     [usr.share.doc.bash]bashref.pdf
$!     [usr.share.doc.bash]rose94.pdf
$!     [usr.share.doc.bash]bash.html
$!     [usr.share.doc.bash]bashref.html
$!     [usr.bin]printenv.
$!     [usr.bin]recho.
$!     [usr.bin]xcase.
$!     [usr.bin]zedho.
$! Future: A symbolic link to the release notes?
$!
$!
$! Seen installed on Scientific Linux 6.1:
$!   /usr/share/man/man1/bashbug.1.gz
$!   /usr/share/man/man1/bash.1.gz
$!   /usr/share/kde4/apps/katepart/syntax/bash.xml
$!   /usr/share/locale/af/LC_MESSAGES/bash.mo
$!   /usr/share/locale/ro/LC_MESSAGES/bash.mo
$!   /usr/share/locale/fr/LC_MESSAGES/bash.mo
$!   /usr/share/locale/en@quot/LC_MESSAGES/bash.mo
$!   /usr/share/locale/vi/LC_MESSAGES/bash.mo
$!   /usr/share/locale/lt/LC_MESSAGES/bash.mo
$!   /usr/share/locale/pl/LC_MESSAGES/bash.mo
$!   /usr/share/locale/es/LC_MESSAGES/bash.mo
$!   /usr/share/locale/pt_BR/LC_MESSAGES/bash.mo
$!   /usr/share/locale/fi/LC_MESSAGES/bash.mo
$!   /usr/share/locale/eo/LC_MESSAGES/bash.mo
$!   /usr/share/locale/id/LC_MESSAGES/bash.mo
$!   /usr/share/locale/bg/LC_MESSAGES/bash.mo
$!   /usr/share/locale/ru/LC_MESSAGES/bash.mo
$!   /usr/share/locale/cs/LC_MESSAGES/bash.mo
$!   /usr/share/locale/sk/LC_MESSAGES/bash.mo
$!   /usr/share/locale/zh_TW/LC_MESSAGES/bash.mo
$!   /usr/share/locale/ga/LC_MESSAGES/bash.mo
$!   /usr/share/locale/en@boldquot/LC_MESSAGES/bash.mo
$!   /usr/share/locale/et/LC_MESSAGES/bash.mo
$!   /usr/share/locale/tr/LC_MESSAGES/bash.mo
$!   /usr/share/locale/de/LC_MESSAGES/bash.mo
$!   /usr/share/locale/ja/LC_MESSAGES/bash.mo
$!   /usr/share/locale/nl/LC_MESSAGES/bash.mo
$!   /usr/share/locale/sv/LC_MESSAGES/bash.mo
$!   /usr/share/locale/hu/LC_MESSAGES/bash.mo
$!   /usr/share/locale/ca/LC_MESSAGES/bash.mo
$!   /usr/share/doc/bash-4.1.2
$!   /usr/share/info/bash.info.gz
$!   /usr/bin/bashbug-64
$!   /usr/bin/sh
$!
$! We currently have not implemented building the *.mo files or the
$! bash.info files on VMS.  The /usr/share/doc/bash-4.1.2 file just
$! has the COPYING file, which we include in the VMS release note
$! file.
$!
$! We do not currently supply Bashbug.  Even if it were run, I doubt that anyone
$! at the bash bug site is interested in a bug that only occurs on VMS.
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
$! 20-Aug-2012	J. Malmberg
$!
$!===========================================================================
$!
$arch_type = f$getsyi("ARCH_NAME")
$arch_code = f$extract(0, 1, arch_type)
$!
$if arch_code .nes. "V"
$then
$   set proc/parse=extended
$endif
$!
$! If the first parameter begins with "r" or "R" then this is to
$! remove the files instead of installing them.
$remove_filesq = f$edit(p1, "upcase,trim")
$remove_filesq = f$extract(0, 1, remove_filesq)
$remove_files = 0
$if remove_filesq .eqs. "R" then remove_files = 1
$!
$!
$! Dest dirs
$!------------------
$dest_dirs = "[bin],[usr.xpg4.bin],[usr.share.doc.bash],[usr.share.man.man1]"
$dest_dirs = dest_dirs + ",[usr.bin]"
$!
$!
$!   Alias links needed.
$!-------------------------
$source_bash = "gnv$bash.exe"
$dest_bash = "[bin]gnv$bash.exe"
$bash_links = "[bin]bash.,[bin]sh.,[usr.xpg4.bin]sh."
$new_gnu = "new_gnu:"
$!
$!
$! Create the directories if they do not exist
$!---------------------------------------------
$ i = 0
$bash_dir_loop:
$       this_dir = f$element(i, ",", dest_dirs)
$       i = i + 1
$       if this_dir .eqs. "" then goto bash_dir_loop
$       if this_dir .eqs. "," then goto bash_dir_loop_end
$!	Just create the directories, do not delete them.
$!     --------------------------------------------------
$	if remove_files .eq. 0
$	then
$	    create/dir 'new_gnu''this_dir'/prot=(o:rwed)
$	endif
$	goto bash_dir_loop
$bash_dir_loop_end:
$!
$!
$! Need to add in the executable file
$!-----------------------------------
$if remove_files .eq. 0
$then
$    copy gnv$bash.exe 'new_gnu'[bin]gnv$bash.exe/prot=w:re
$!
$!   For running self tests
$    copy [.support]printenv.exe 'new_gnu'[usr.bin]printenv.exe/prot=w:re
$    copy [.support]recho.exe 'new_gnu'[usr.bin]recho.exe/prot=w:re
$    copy [.support]xcase.exe 'new_gnu'[usr.bin]xcase.exe/prot=w:re
$    copy [.support]zecho.exe 'new_gnu'[usr.bin]zecho.exe/prot=w:re
$endif
$!
$if remove_files .eq. 0
$then
$    set file/enter=sys$disk:[]bash. gnv$bash.exe
$    set file/enter='new_gnu'[bin]bash. 'new_gnu'[bin]gnv$bash.exe
$    set file/enter='new_gnu'[bin]bash.exe 'new_gnu'[bin]gnv$bash.exe
$    set file/enter='new_gnu'[bin]sh. 'new_gnu'[bin]gnv$bash.exe
$    set file/enter='new_gnu'[bin]sh.exe 'new_gnu'[bin]gnv$bash.exe
$    set file/enter='new_gnu'[usr.xpg4.bin]sh. 'new_gnu'[bin]gnv$bash.exe
$    set file/enter='new_gnu'[usr.xpg4.bin]sh.exe 'new_gnu'[bin]gnv$bash.exe
$!
$!   For running self tests
$    set file/enter='new_gnu'[usr.bin]printenv. 'new_gnu'[usr.bin]printenv.exe
$    set file/enter='new_gnu'[usr.bin]recho. 'new_gnu'[usr.bin]recho.exe
$    set file/enter='new_gnu'[usr.bin]xcase. 'new_gnu'[usr.bin]xcase.exe
$    set file/enter='new_gnu'[usr.bin]zecho. 'new_gnu'[usr.bin]zecho.exe
$else
$   file = "sys$disk:[]bash."
$   if f$search(file) .nes. "" then set file/remove 'file';*
$   file = "''new_gnu'[bin]bash."
$   if f$search(file) .nes. "" then set file/remove 'file';*
$   file = "''new_gnu'[bin]bash.exe"
$   if f$search(file) .nes. "" then set file/remove 'file';*
$   file = "''new_gnu'[bin]sh."
$   if f$search(file) .nes. "" then set file/remove 'file';*
$   file = "''new_gnu'[bin]sh.exe"
$   if f$search(file) .nes. "" then set file/remove 'file';*
$   file = "''new_gnu'[xpg4.bin]sh."
$   if f$search(file) .nes. "" then set file/remove 'file';*
$   file = "''new_gnu'[xpg4.bin]sh.exe"
$   if f$search(file) .nes. "" then set file/remove 'file';*
$   file = "''new_gnu'[usr.xpg4.bin]sh."
$   if f$search(file) .nes. "" then set file/remove 'file';*
$   file = "''new_gnu'[usr.xpg4.bin]sh.exe"
$   if f$search(file) .nes. "" then set file/remove 'file';*
$!
$!   For running self tests
$   file = "''new_gnu'[usr.bin]printenv."
$   if f$search(file) .nes. "" then set file/remove 'file';*
$   file = "''new_gnu'[usr.bin]recho."
$   if f$search(file) .nes. "" then set file/remove 'file';*
$   file = "''new_gnu'[usr.bin]zecho."
$   if f$search(file) .nes. "" then set file/remove 'file';*
$   file = "''new_gnu'[usr.bin]xcase."
$   if f$search(file) .nes. "" then set file/remove 'file';*
$   file = "''new_gnu'[usr.bin]zcase."
$   if f$search(file) .nes. "" then set file/remove 'file';*
$endif
$!
$!
$if remove_files .eq. 0
$then
$   copy [.doc]bash.1 'new_gnu'[usr.share.man.man1]bash.1
$!   copy [.doc]bashbug.1 'new_gnu'[usr.share.man.man1]bashbug.1
$   in_file = "AUTHORS."
$   out_file = in_file
$   if f$search(in_file) .eqs. ""
$   then
$	in_file = "$''in_file'"
$   endif
$   copy 'in_file' 'new_gnu'[usr.share.doc.bash]'out_file'
$   in_file = "CHANGES."
$   out_file = in_file
$   if f$search(in_file) .eqs. ""
$   then
$	in_file = "$''in_file'"
$   endif
$   copy 'in_file' 'new_gnu'[usr.share.doc.bash]'out_file'
$   in_file = "COMPAT."
$   out_file = in_file
$   if f$search(in_file) .eqs. ""
$   then
$	in_file = "$''in_file'"
$   endif
$   copy 'in_file' 'new_gnu'[usr.share.doc.bash]'out_file'
$   in_file = "COPYING."
$   out_file = in_file
$   if f$search(in_file) .eqs. ""
$   then
$	in_file = "$''in_file'"
$   endif
$   copy 'in_file' 'new_gnu'[usr.share.doc.bash]'out_file'
$!   copy ChangeLog. 'new_gnu'[usr.share.doc.bash]ChangeLog.
$   in_file = "NEWS."
$   out_file = in_file
$   if f$search(in_file) .eqs. ""
$   then
$	in_file = "$''in_file'"
$   endif
$   copy 'in_file' 'new_gnu'[usr.share.doc.bash]'out_file'
$   in_file = "NOTES."
$   out_file = in_file
$   if f$search(in_file) .eqs. ""
$   then
$	in_file = "$''in_file'"
$   endif
$   copy 'in_file' 'new_gnu'[usr.share.doc.bash]'out_file'
$   in_file = "POSIX."
$   out_file = in_file
$   if f$search(in_file) .eqs. ""
$   then
$	in_file = "$''in_file'"
$   endif
$   copy 'in_file' 'new_gnu'[usr.share.doc.bash]'out_file'
$   in_file = "RBASH."
$   out_file = in_file
$   if f$search(in_file) .eqs. ""
$   then
$	in_file = "$''in_file'"
$   endif
$   copy 'in_file' 'new_gnu'[usr.share.doc.bash]'out_file'
$   in_file = "README."
$   out_file = in_file
$   if f$search(in_file) .eqs. ""
$   then
$	in_file = "$''in_file'"
$   endif
$   copy 'in_file' 'new_gnu'[usr.share.doc.bash]'out_file'
$   copy [.doc]article.pdf 'new_gnu'[usr.share.doc.bash]article.pdf
$   copy [.doc]bash.pdf 'new_gnu'[usr.share.doc.bash]bash.pdf
$   copy [.doc]bashref.pdf 'new_gnu'[usr.share.doc.bash]bashref.pdf
$   copy [.doc]rose94.pdf 'new_gnu'[usr.share.doc.bash]rose94.pdf
$   copy [.doc]bash.html 'new_gnu'[usr.share.doc.bash]bash.html
$   copy [.doc]bashref.html 'new_gnu'[usr.share.doc.bash]bashref.html
$else
$   file = "''new_gnu'[bin]gnv$bash.exe
$   if f$search(file) .nes. "" then delete 'file';*
$   file = "''new_gnu'[usr.share.man.man1]bash.1"
$   if f$search(file) .nes. "" then delete 'file';*
$   file = "''new_gnu'[usr.share.man.man1]bashbug.1"
$   if f$search(file) .nes. "" then delete 'file';*
$   file = "''new_gnu'[usr.share.doc.bash]AUTHORS."
$   if f$search(file) .nes. "" then delete 'file';*
$   file = "''new_gnu'[usr.share.doc.bash]CHANGES."
$   if f$search(file) .nes. "" then delete 'file';*
$   file = "''new_gnu'[usr.share.doc.bash]COMPAT."
$   if f$search(file) .nes. "" then delete 'file';*
$   file = "''new_gnu'[usr.share.doc.bash]COPYING."
$   if f$search(file) .nes. "" then delete 'file';*
$!   file = "''new_gnu'[usr.share.doc.bash]ChangeLog."
$!   if f$search(file) .nes. "" then delete 'file';*
$   file = "''new_gnu'[usr.share.doc.bash]NEWS."
$   if f$search(file) .nes. "" then delete 'file';*
$   file = "''new_gnu'[usr.share.doc.bash]NOTES."
$   if f$search(file) .nes. "" then delete 'file';*
$   file = "''new_gnu'[usr.share.doc.bash]POSIX."
$   if f$search(file) .nes. "" then delete 'file';*
$   file = "''new_gnu'[usr.share.doc.bash]RBASH."
$   if f$search(file) .nes. "" then delete 'file';*
$   file = "''new_gnu'[usr.share.doc.bash]README."
$   if f$search(file) .nes. "" then delete 'file';*
$   file = "''new_gnu'[usr.share.doc.bash]article.pdf"
$   if f$search(file) .nes. "" then delete 'file';*
$   file = "''new_gnu'[usr.share.doc.bash]bash.pdf"
$   if f$search(file) .nes. "" then delete 'file';*
$   file = "''new_gnu'[usr.share.doc.bash]bashref.pdf"
$   if f$search(file) .nes. "" then delete 'file';*
$   file = "''new_gnu'[usr.share.doc.bash]rose94.pdf"
$   if f$search(file) .nes. "" then delete 'file';*
$   file = "''new_gnu'[usr.share.doc.bash]bash.html"
$   if f$search(file) .nes. "" then delete 'file';*
$   file = "''new_gnu'[usr.share.doc.bash]bashref.html"
$   if f$search(file) .nes. "" then delete 'file';*
$!
$!   For running self tests
$   file = "''new_gnu'[usr.bin]printenv.exe"
$   if f$search(file) .nes. "" then delete 'file';*
$   file = "''new_gnu'[usr.bin]recho.exe"
$   if f$search(file) .nes. "" then delete 'file';*
$   file = "''new_gnu'[usr.bin]xcase.exe"
$   if f$search(file) .nes. "" then delete 'file';*
$   file = "''new_gnu'[usr.bin]zecho.exe"
$   if f$search(file) .nes. "" then delete 'file';*
$endif
$!
