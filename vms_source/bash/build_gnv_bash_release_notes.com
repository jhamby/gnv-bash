$! File: Build_GNV_bash_release_notes.com
$!
$! $Id: build_gnv_bash_release_notes.com,v 1.1 2012/12/09 23:54:36 wb8tyw Exp $
$!
$! Build the release note file from the three components:
$!    1. The bash_release_note_start.txt
$!    2. readme. file from the Bash distribution.
$!    3. The Bash_gnv-build_steps.txt.
$!
$! Set the name of the release notes from the GNV_PCSI_FILENAME_BASE
$! logical name.
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
$! 15-Mar-2011	J. Malmberg
$!
$!===========================================================================
$!
$base_file = f$trnlnm("GNV_PCSI_FILENAME_BASE")
$if base_file .eqs. ""
$then
$   write sys$output "@MAKE_PCSI_BASH_KIT_NAME.COM has not been run."
$   goto all_exit
$endif
$!
$copy bash_release_note_start.txt 'base_file'.release_notes
$!
$! If this is VAX and the file is on NFS, the name may be mangled.
$!-----------------------------------------------------------------
$readme_file = ""
$if f$search("readme.") .nes. ""
$then
$   readme_file = "readme."
$else
$   if f$search("$README.") .nes. ""
$   then
$	readme_file = "$README."
$   else
$	write sys$output "Can not find readme file."
$	goto all_exit
$   endif
$endif
$open/append rnf 'base_file'.release_notes
$open/read rf 'readme_file'
$!
$write rnf "------------- Start of BASH README file. -------------------"
$!
$rf_loop:
$   read/end=rf_loop_end rf line_in
$   write rnf line_in
$   goto rf_loop
$rf_loop_end:
$close rf
$!
$write rnf ""
$write rnf "--------------- End of BASH README file. -------------------"
$write rnf ""
$close rnf
$!
$!
$copy/concatenate 'base_file'.release_notes, bash_build_steps.txt -
                  'base_file'.release_notes
$purge 'base_file'.release_notes
$rename 'base_file.release_notes ;1
$!
$all_exit:
$   exit
