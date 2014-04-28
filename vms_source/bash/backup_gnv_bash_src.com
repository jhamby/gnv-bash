$! Backup_gnv_bash_src.com
$!
$! $Id: backup_gnv_bash_src.com,v 1.2 2013/06/13 12:42:24 wb8tyw Exp $
$!
$! Skip a bunch of files that are not used or should not be
$! in the CVS repository
$!
$! This is not intended to be run on a VAX host.
$!
$! On some systems, this should not be run on NFS mounted source disks.
$! Backup tends to hang.
$! You need to copy the files from an NFS mounted drive.
$!
$! src_root1 and vms_root1 are used to designate an alternate directory
$! for the source modules.
$!
$! 18-Mar-2011	J. Malmberg
$!
$!==========================================================================
$!
$arch_type = f$getsyi("ARCH_NAME")
$arch_code = f$extract(0, 1, arch_type)
$!
$if arch_code .nes. "V"
$then
$   set proc/parse=extended
$endif
$!
$ss_abort = 44
$status = ss_abort
$!
$kit_name = f$trnlnm("GNV_PCSI_KITNAME")
$if kit_name .eqs. ""
$then
$   write sys$output "@MAKE_PCSI_BASH_KIT_NAME.COM has not been run."
$   goto all_exit
$endif
$producer = f$trnlnm("GNV_PCSI_PRODUCER")
$if producer .eqs. ""
$then
$   write sys$output "@MAKE_PCSI_BASH_KIT_NAME.COM has not been run."
$   goto all_exit
$endif
$filename_base = f$trnlnm("GNV_PCSI_FILENAME_BASE")
$if filename_base .eqs. ""
$then
$   write sys$output "@MAKE_PCSI_BASH_KIT_NAME.COM has not been run."
$   goto all_exit
$endif
$!
$node_swvers = f$getsyi("NODE_SWVERS")
$node_swvers_type = f$extract(0, 1, node_swvers)
$node_swvers_vers = f$extract(1, f$length(node_swvers), node_swvers)
$swvers_maj = f$element(0, ".", node_swvers_vers)
$node_swvers_min_update = f$element(1, ".", node_swvers_vers)
$swvers_min = f$element(0, "-", node_swvers_min_update)
$swvers_update = f$element(1, "-", node_swvers_min_update)
$!
$if swvers_update .eqs. "-" then swvers_update = ""
$!
$vms_vers = f$fao("!2ZB!2ZB!AS", 'swvers_maj', 'swvers_min', swvers_update)
$!
$!
$! If available make an interchange save set
$!-------------------------------------------
$interchange = ""
$if arch_code .eqs. "V"
$then
$   interchange = "/interchange"
$endif
$if (swvers_maj .ges. "8") .and. (swvers_min .ges. 4)
$then
$   interchange = "/interchange/noconvert"
$endif
$!
$src_root = "src_root:"
$if f$trnlnm("src_root1") .nes. "" then src_root = "src_root1:"
$backup'interchange' 'src_root'[bash...]*.*;0 -
           /exclude=(cvs.dir, [...cvs]*.*) -
	   'filename_base'_original_src.bck/sav
$status = $status
$!
$!
$vms_root = "vms_root:"
$if f$trnlnm("vms_root1") .nes. "" then vms_root = "vms_root1:"
$backup'interchange' 'vms_root'[bash...]*.*;0 -
	'filename_base'_vms_src.bck/sav
$status = $status
$!
$all_exit:
$exit 'status'
