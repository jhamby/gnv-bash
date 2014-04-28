$! Remove_old_bash.com
$!
$! $Id: remove_old_bash.com,v 1.2 2013/06/09 21:38:24 wb8tyw Exp $
$!
$! This is a procedure to remove the old bash images that were installed
$! by the GNV kits and replace them with links to the new image.
$!
$! 09-Dec-2012	J. Malmberg
$!
$!==========================================================================
$!
$vax = f$getsyi("HW_MODEL") .lt. 1024
$old_parse = ""
$if .not. VAX
$then
$   old_parse = f$getjpi("", "parse_style_perm")
$   set process/parse=extended
$endif
$!
$! First get the FID of the new bash image.
$! Don't remove anything that matches it.
$new_bash = f$search("GNV$GNU:[BIN]GNV$BASH.EXE")
$!
$new_bash_fid = "No_new_bash_fid"
$if new_bash .nes. ""
$then
$   new_bash_fid = f$file_attributes(new_bash, "FID")
$endif
$!
$! Next get the FIDs for bash.exe adn sh.exe.
$! Old GNV kits had them copies instead of links.
$!
$old_bash_exe_fid = "No_old_bash_exe_fid"
$old_bash_exe = f$search("gnv$gnu:[bin]bash.exe")
$if old_bash_exe .nes. ""
$then
$   fid = f$file_attributes(old_bash_exe, "FID")
$   if fid .nes. new_bash_fid then old_bash_exe_fid = fid
$endif
$!
$old_sh_exe_fid = "No_old_sh_exe_fid"
$old_sh_exe = f$search("gnv$gnu:[bin]sh.exe")
$if old_sh_exe .nes. ""
$then
$   fid = f$file_attributes(old_sh_exe, "FID")
$   if fid .nes. new_bash_fid then old_sh_exe_fid = fid
$endif
$!
$!
$! These were in the wrong locations, just remove them.
$!
$old_xpg4_bash = f$search("gnv$gnu:[xpg4.bin]sh.")
$if old_xpg4_bash .nes. ""
$then
$   set file/remove 'old_xpg4_bash'
$endif
$old_xpg4_bash = f$search("gnv$gnu:[xpg4.bin]sh.exe")
$if old_xpg4_bash .nes. ""
$then
$   set file/remove 'old_xpg4_bash'
$endif
$file = f$search("gnv$gnu:[xpg4]bin.dir")
if file .nes. ""
$then
$    delete 'file'
$endif
$file = f$search("gnv$gnu:[000000]xpg4.dir")
if file .nes. ""
$then
$    delete 'file'
$endif
$!
$!
$! Now get check the "sh." and "bash."
$! May be links or copies.
$! Ok to delete and replace.
$!
$old_sh_fid = "No_old_sh_fid"
$old_sh = f$search("gnv$gnu:[bin]sh.")
$if old_sh .nes. ""
$then
$   fid = f$file_attributes(old_sh, "FID")
$   if fid .nes. new_bash_fid
$   then
$	if fid .eqs. old_sh_exe_fid
$	then
$	    set file/remove 'old_sh'
$	else
$	    delete 'old_sh'
$	endif
$	if new_bash .nes. "" then set file/enter='old_sh' 'new_bash'
$   endif
$endif
$!
$!
$old_bash_fid = "No_old_bash_fid"
$old_bash = f$search("gnv$gnu:[bin]bash.")
$if old_bash .nes. ""
$then
$   fid = f$file_attributes(old_bash, "FID")
$   if fid .nes. new_bash_fid
$   then
$	if fid .eqs. old_bash_exe_fid
$	then
$	    set file/remove 'old_bash'
$	else
$	    delete 'old_bash'
$	endif
$	if new_bash .nes. "" then set file/enter='old_bash' 'new_bash'
$   endif
$endif
$!
$! Now we can remove/update the bash.exe and sh.exe
$if old_sh_exe .nes. ""
$then
$   if old_sh_fid .nes. new_bash_fid
$   then
$	if fid .eqs. old_bash_exe_fid
$	then
$	    set file/remove 'old_sh_exe'
$	else
$	    delete 'old_sh_exe'
$	endif
$	if new_bash .nes. "" then set file/enter='old_sh_exe' 'new_bash'
$   endif
$endif
$!
$if old_bash_exe .nes. ""
$then
$   if old_bash_fid .nes. new_bash_fid
$   then
$	delete 'old_bash_exe'
$	if new_bash .nes. "" then set file/enter='old_bash_exe' 'new_bash'
$   endif
$endif
$!
$if f$search("gnv$gnu:[usr]xpg4.dir") .eqs. ""
$then
$   create/dir/prot=w:re gnv$gnu:[usr.xpg4]
$endif
$if f$search("gnv$gnu:[usr.xpg4]bin.dir") .eqs. ""
$then
$   create/dir/prot=w:re gnv$gnu:[usr.xpg4.bin]
$endif
$!
$old_sh_fid = "No_old_sh_fid"
$new_sh = "gnv$gnu:[usr.xpg4.bin]sh."
$old_sh = f$search(new_sh)
$if old_sh .nes. ""
$then
$   fid = f$file_attributes(old_sh, "FID")
$   if fid .nes. new_bash_fid
$   then
$	if fid .eqs. old_sh_exe_fid
$	then
$	    set file/remove 'old_sh'
$	else
$	    delete 'old_sh'
$	endif
$	if new_bash .nes. "" then set file/enter='old_sh' 'new_bash'
$   endif
$else
$   set file/enter='new_sh' 'new_bash'
$endif
$!
$old_sh_fid = "No_old_sh_fid"
$new_sh = "gnv$gnu:[usr.xpg4.bin]sh.exe"
$old_sh = f$search(new_sh)
$if old_sh .nes. ""
$then
$   fid = f$file_attributes(old_sh, "FID")
$   if fid .nes. new_bash_fid
$   then
$	if fid .eqs. old_sh_exe_fid
$	then
$	    set file/remove 'old_sh'
$	else
$	    delete 'old_sh'
$	endif
$	if new_bash .nes. "" then set file/enter='old_sh' 'new_bash'
$   endif
$else
$   set file/enter='new_sh' 'new_bash'
$endif
$!
$if .not. VAX
$then
$   set process/parse='old_parse'
$endif
$!
$exit
