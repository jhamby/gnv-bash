# File: Bash.mms
#
# $Id: bash.mms,v 1.5 2013/06/18 04:22:47 wb8tyw Exp $
#
# Quick and dirty Make file for building Bash 5.1 on VMS
#
# This build procedure requires the following concealed rooted
# logical names to be set up.
# LCL_ROOT: This is a read/write directory for the build output.
# VMS_ROOT: This is a read only directory for VMS specific changes
#           that have not been checked into the official repository.
# SRC_ROOT: This is a read only directory containing the files in the
#           Offical repository.
# PRJ_ROOT: This is a search list of LCL_ROOT:,VMS_ROOT:,SRC_ROOT:
#
# The base directory is hard coded to be "BASH" to simplify things.
# Platform specific compiler or link options need to be set as
# dcl symbols CC and LINK before running this procedure.
#
# 12-Jun-2010  J. Malmberg - based on Makefile.com
# 03-May-2011  M Vorlander - fix for building error.c
# 01-Mar-2012  J. Malmberg - Bash 4.2.
# 04-Apr-2013  E. Robertson - Modifications to accommodate new
#			      tpu files for Bash bug fixes.
# 02-Jun-2013  J. Malmberg - Build on VAX/VMS 7.3 platform.
# 18-Jun-2022  J. Hamby    - Bash 5.1, removing VAX and subshell hacks.
#
##############################################################################


# CFLAGS
# All modules in all directories must share the same CXX repository.
#
# Always compile/debug, decide at link time if debug symbols are to
# be kept based on if an external "LINK" dcl symbol.
#
# Symbol names in base are case sensitive, bash will build wrong otherwise.
#
# For later debug, put only basic information in the listing files.
#
# Warnings should generaly be enabled globally and only suppressed if there
# is a specific issue that can not be addressed in the code.
# This old bash has several issues that require some messages to be
# suppressed.  Normally I would fix most of these issues instead of
# suppressing the messages, but I will save that for building a more
# current version of Bash.
#
# Normally the warning settings should be /warn=(enable=level4, questcode)
# for catching common programming errors.
#
# On Alpha/I64, the warnings would be fixed with /first_include file.
# DEC C 6.4 for VAX does not have that option.
#
# Much of the code in the 1.4.8 GNV port had a bug where config.h is not
# the first header file included.
#
#===================================================================
crepository = /repo=lcl_root:[bash.cxx_repository]
cnames = /name=(as_i,shor)$(crepository)/fl=ieee/ieee=denorm/stand=latest/nopure
clist = /list
cprefix = /pref=all/main=posix
cnowarn = missingreturn,conptrlosbit,dollarid
cwarn = /warnings=(disable=($(cnowarn)))/undef=(__HIDE_FORBIDDEN_NAMES)
#cinc1 = prj_root:[],prj_root:[.include],prj_root:[.lib.intl],prj_root:[.lib.sh]
cinc = /nested=none/Opt=(Lev=5)
#
# Force the status of the HAVE_REGEX_H, HAVE_REGCOMP, and HAVE_REGEXEC macros on
# VMS so that HAVE_POSIX_REGEXP is indirectly forced which will in turn set the
# macro COND_REGEXP. This will enable conditional regular expressions in Bash
# via [.lib.sh]shmatch.c. Note that this cannot be done via config_h.com because
# configure does not have tests for these macros; So they must be manually set.
#
cdefs1 = _USE_STD_STAT=1,_POSIX_EXIT=1,HAVE_CONFIG_H=1,__POSIX_TTYNAME=1,\
	 __DECC,__STDC_VERSION__=199901L
cdefs = /define=(VMS=1,$(cdefs1),HAVE_REGEX_H=1,HAVE_REGCOMP=1,HAVE_REGEXEC=1)
cflags = $(cnames)/debu$(clist)$(cprefix)$(cwarn)$(cinc)$(cdefs)
cflagsx = $(cnames)/debu$(clist)$(cprefix)$(cwarn)$(cinc)

#
# TPU symbols
#===================

UNIX_2_VMS = /COMM=prj_root:unix_c_to_vms_c.tpu

EVE = EDIT/TPU/SECT=EVE$SECTION/NODISP

# Use C++ compiler.
#==================

CC = CXX
LINK = CXXLINK

# Set up the rules for use.
#===========================================
.SUFFIXES
.SUFFIXES .exe .olb .obj .c .def

.obj.exe
   $(LINK)$(LFLAGS)/NODEBUG/EXE=$(MMS$TARGET)/DSF=$(MMS$TARGET_NAME)\
     /MAP=$(MMS$TARGET_NAME) $(MMS$SOURCE_LIST)

.c.obj
   $define/user readline prj_root:[.lib.readline]
   $define/user glob prj_root:[.lib.glob]
   $define/user tilde prj_root:[.lib.tilde]
   $define/user cxx$system_include prj_root:[],prj_root:[.include]
   $define/user cxx$user_include prj_root:[],prj_root:[.include],\
	prj_root:[.lib.readline],prj_root:[.lib.intl],prj_root:[.lib.sh],\
	prj_root:[.lib.glob]
   $(CC)$(CFLAGS)/OBJ=$(MMS$TARGET) $(MMS$SOURCE)

.obj.olb
   @ if f$search("$(MMS$TARGET)") .eqs. "" then \
	librarian/create/object $(MMS$TARGET)
   $ librarian/replace $(MMS$TARGET) $(MMS$SOURCE_LIST)

# MMS requires that the generated name be in upper case to generate an
# upper case module name for the librarian.
.def.c
   @ mkbuiltins :== $prj_root:[]mkbuiltins.exe
   $ set def prj_root:[.builtins]
   $ source = f$parse("$(MMS$SOURCE)",,,"NAME")
   $ source_u = f$edit(source, "UPCASE")
   $ mkbuiltins "-D" ./builtins/ 'source'.DEF
   $ set def prj_root:[-]

# Lists of source files for submodules
#=========================================

libsh_objs =	"cltck"=[.lib.sh]clktck.obj,\
                "getcwd"=[.lib.sh]getcwd.obj,\
		"getenv"=[.lib.sh]getenv.obj,\
                "oslib"=[.lib.sh]oslib.obj,\
		"setlinebuf"=[.lib.sh]setlinebuf.obj,\
                "strchrnul"=[.lib.sh]strchrnul.obj,\
		"strcasecmp"=[.lib.sh]strcasecmp.obj,\
                "strerror"=[.lib.sh]strerror.obj,\
		"strtod"=[.lib.sh]strtod.obj,\
                "strtol"=[.lib.sh]strtol.obj,\
		"strtoul"=[.lib.sh]strtoul.obj,\
                "vprint"=[.lib.sh]vprint.obj,\
		"itos"=[.lib.sh]itos.obj,\
                "rename"=[.lib.sh]rename.obj,\
		"zread"=[.lib.sh]zread.obj,\
                "zwrite"=[.lib.sh]zwrite.obj,\
		"shtty"=[.lib.sh]shtty.obj,\
                "inet_aton"=[.lib.sh]inet_aton.obj,\
		"netopen"=[.lib.sh]netopen.obj,\
                "strpbrk"=[.lib.sh]strpbrk.obj,\
		"timeval"=[.lib.sh]timeval.obj,\
                "clock"=[.lib.sh]clock.obj,\
		"makepath"=[.lib.sh]makepath.obj,\
                "pathcanon"=[.lib.sh]pathcanon.obj,\
		"pathphys"=[.lib.sh]pathphys.obj,\
                "stringlist"=[.lib.sh]stringlist.obj,\
		"stringvec"=[.lib.sh]stringvec.obj,\
                "tmpfile"=[.lib.sh]tmpfile.obj,\
		"spell"=[.lib.sh]spell.obj,\
                "strtrans"=[.lib.sh]strtrans.obj,\
		"strcasestr"=[.lib.sh]strcasestr.obj,\
                "shquote"=[.lib.sh]shquote.obj,\
		"snprintf"=[.lib.sh]snprintf.obj,\
                "mailstat"=[.lib.sh]mailstat.obj,\
		"fmtulong"=[.lib.sh]fmtulong.obj,\
                "fmtullong"=[.lib.sh]fmtullong.obj,\
		"strtoll"=[.lib.sh]strtoll.obj,\
                "strtoull"=[.lib.sh]strtoull.obj,\
		"fmtumax"=[.lib.sh]fmtumax.obj,\
                "netconn"=[.lib.sh]netconn.obj,\
                "memset"=[.lib.sh]memset.obj,\
                "mbschr"=[.lib.sh]mbschr.obj,\
		"zcatfd"=[.lib.sh]zcatfd.obj,\
                "shmatch"=[.lib.sh]shmatch.obj,\
		"strnlen"=[.lib.sh]strnlen.obj,\
                "winsize"=[.lib.sh]winsize.obj,\
		"eaccess"=[.lib.sh]eaccess.obj,\
                "wcsdup"=[.lib.sh]wcsdup.obj,\
		"zmapfd"=[.lib.sh]zmapfd.obj,\
                "fpurge"=[.lib.sh]fpurge.obj,\
		"zgetline"=[.lib.sh]zgetline.obj,\
                "mbscmp"=[.lib.sh]mbscmp.obj,\
		"casemod"=[.lib.sh]casemod.obj,\
                "uconvert"=[.lib.sh]uconvert.obj,\
		"ufuncs"=[.lib.sh]ufuncs.obj,\
                "dprintf"=[.lib.sh]dprintf.obj,\
		"input_avail"=[.lib.sh]input_avail.obj,\
                "mbscasecmp"=[.lib.sh]mbscasecmp.obj,\
		"fnxform"=[.lib.sh]fnxform.obj,\
                "unicode"=[.lib.sh]unicode.obj,\
		"wcswidth"=[.lib.sh]wcswidth.obj,\
		"wcsnwidth"=[.lib.sh]wcsnwidth.obj,\
                "shmbchar"=[.lib.sh]shmbchar.obj,\
                "utf8"=[.lib.sh]utf8.obj,\
                "random"=[.lib.sh]random.obj,\
                "gettimeofday"=[.lib.sh]gettimeofday.obj

libreadline_objs = "bind"=[.lib.readline]bind.obj,\
                   "callback"=[.lib.readline]callback.obj,\
                   "colors"=[.lib.readline]colors.obj,\
                   "compat"=[.lib.readline]compat.obj,\
                   "complete"=[.lib.readline]complete.obj,\
                   "display"=[.lib.readline]display.obj,\
                   "funmap"=[.lib.readline]funmap.obj,\
                   "histexpand"=[.lib.readline]histexpand.obj,\
                   "histfile"=[.lib.readline]histfile.obj,\
                   "history"=[.lib.readline]history.obj,\
                   "histsearch"=[.lib.readline]histsearch.obj,\
                   "input"=[.lib.readline]input.obj,\
                   "isearch"=[.lib.readline]isearch.obj,\
                   "keymaps"=[.lib.readline]keymaps.obj,\
                   "kill"=[.lib.readline]kill.obj,\
                   "macro"=[.lib.readline]macro.obj,\
                   "mbutil"=[.lib.readline]mbutil.obj,\
		   "misc"=[.lib.readline]misc.obj,\
                   "nls"=[.lib.readline]nls.obj,\
                   "parens"=[.lib.readline]parens.obj,\
                   "parse-colors"=[.lib.readline]parse-colors.obj,\
                   "readline"=[.lib.readline]readline.obj,\
                   "rltty"=[.lib.readline]rltty.obj,\
                   "savestring"=[.lib.readline]savestring.obj,\
                   "search"=[.lib.readline]search.obj,\
                   "signals"=[.lib.readline]signals.obj,\
                   "terminal"=[.lib.readline]terminal.obj,\
		   "text"=[.lib.readline]text.obj,\
                   "tilde"=[.lib.readline]tilde.obj,\
                   "undo"=[.lib.readline]undo.obj,\
                   "util"=[.lib.readline]util.obj,\
                   "vi_mode"=[.lib.readline]vi_mode.obj,\
		   "xfree"=[.lib.readline]xfree.obj,\
                   "xmalloc"=[.lib.readline]xmalloc.obj

libtermcap_objs = termcap=[.lib.termcap]termcap.obj

libglob_objs = "glob"=[.lib.glob]glob.obj,\
               "gmisc"=[.lib.glob]gmisc.obj,\
               "smatch"=[.lib.glob]smatch.obj,\
               "strmatch"=[.lib.glob]strmatch.obj,\
               "xmbsrtowcs"=[.lib.glob]xmbsrtowcs.obj


libtilde_objs = tilde=[.lib.tilde]tilde.obj


# libintl built locally
libintl_objs = "bindtextdom"=[.lib.intl]bindtextdom.obj,\
               "dcgettext"=[.lib.intl]dcgettext.obj,\
               "dcigettext"=[.lib.intl]dcigettext.obj,\
               "dcngettext"=[.lib.intl]dcngettext.obj,\
               "dgettext"=[.lib.intl]dgettext.obj,\
               "dngettext"=[.lib.intl]dngettext.obj,\
               "explodename"=[.lib.intl]explodename.obj,\
               "finddomain"=[.lib.intl]finddomain.obj,\
               "gettext"=[.lib.intl]gettext.obj,\
               "intl-compat"=[.lib.intl]intl-compat.obj,\
               "l10nflist"=[.lib.intl]l10nflist.obj,\
               "loadmsgcat"=[.lib.intl]loadmsgcat.obj,\
               "localcharset"=[.lib.intl]localcharset.obj,\
               "localealias"=[.lib.intl]localealias.obj,\
               "localename"=[.lib.intl]localename.obj,\
               "log"=[.lib.intl]log.obj,\
               "ngettext"=[.lib.intl]ngettext.obj,\
               "osdep"=[.lib.intl]osdep.obj,\
               "plural-exp"=[.lib.intl]plural-exp.obj,\
               "plural"=[.lib.intl]plural.obj,\
               "relocatable"=[.lib.intl]relocatable.obj,\
               "textdomain"=[.lib.intl]textdomain.obj

# See if this builds with out the os2compat mode.
#              "os2compat"=[.lib.intl]os2compat.obj,\
#

# NOTE: crunch this down so they all fit in the `mkbuiltins' command line.
DEFSRC = alias.def bind.def break.def builtin.def caller.def cd.def colon.def \
command.def declare.def echo.def enable.def eval.def getopts.def exec.def \
exit.def fc.def fg_bg.def hash.def help.def history.def jobs.def kill.def \
let.def read.def return.def set.def setattr.def shift.def source.def \
suspend.def test.def times.def trap.def type.def ulimit.def umask.def wait.def \
reserved.def pushd.def shopt.def printf.def complete.def mapfile.def

#
# This macro defines the .def files for builtins which require changes for
# OpenVMS.
#
VMSDEFSRC = 


#		  "common"=[.builtins]common.obj,\
#
libbuiltins_objs = "alias"=[.builtins]alias.obj,\
                   "bashgetopt"=[.builtins]bashgetopt.obj,\
                   "bind"=[.builtins]bind.obj,\
                   "break"=[.builtins]break.obj,\
                   "builtin"=[.builtins]builtin.obj,\
                   "caller"=[.builtins]caller.obj,\
                   "cd"=[.builtins]cd.obj,\
                   "colon"=[.builtins]colon.obj,\
                   "command"=[.builtins]command.obj,\
		   "complete"=[.builtins]complete.obj,\
                   "declare"=[.builtins]declare.obj,\
                   "echo"=[.builtins]echo.obj,\
                   "enable"=[.builtins]enable.obj,\
                   "eval"=[.builtins]eval.obj,\
		   "evalfile"=[.builtins]evalfile.obj,\
		   "evalstring"=[.builtins]evalstring.obj,\
                   "exec"=[.builtins]exec.obj,\
                   "exit"=[.builtins]exit.obj,\
                   "fc"=[.builtins]fc.obj,\
                   "fg_bg"=[.builtins]fg_bg.obj,\
                   "getopts"=[.builtins]getopts.obj,\
		   "getopt"=[.builtins]getopt.obj,\
                   "hash"=[.builtins]hash.obj,\
                   "help"=[.builtins]help.obj,\
                   "history"=[.builtins]history.obj,\
		   "inlib"=[.builtins]inlib.obj,\
                   "jobs"=[.builtins]jobs.obj,\
                   "kill"=[.builtins]kill.obj,\
                   "let"=[.builtins]let.obj,\
		   "mapfile"=[.builtins]mapfile.obj,\
                   "printf"=[.builtins]printf.obj,\
                   "pushd"=[.builtins]pushd.obj,\
                   "read"=[.builtins]read.obj,\
                   "return"=[.builtins]return.obj,\
                   "set"=[.builtins]set.obj,\
                   "setattr"=[.builtins]setattr.obj,\
                   "shift"=[.builtins]shift.obj,\
                   "shopt"=[.builtins]shopt.obj,\
                   "source"=[.builtins]source.obj,\
                   "suspend"=[.builtins]suspend.obj,\
                   "test"=[.builtins]test.obj,\
                   "times"=[.builtins]times.obj,\
                   "trap"=[.builtins]trap.obj,\
                   "type"=[.builtins]type.obj,\
                   "ulimit"=[.builtins]ulimit.obj,\
                   "umask"=[.builtins]umask.obj,\
                   "wait"=[.builtins]wait.obj

libbuiltins_defs = [.builtins]alias.def \
                   [.builtins]bind.def \
                   [.builtins]break.def \
                   [.builtins]builtin.def \
                   [.builtins]caller.def \
                   [.builtins]cd.def \
                   [.builtins]colon.def \
                   [.builtins]command.def \
		   [.builtins]complete.def \
                   [.builtins]declare.def \
                   [.builtins]echo.def \
                   [.builtins]enable.def \
                   [.builtins]eval.def \
                   [.builtins]exec.def \
                   [.builtins]exit.def \
                   [.builtins]fc.def \
                   [.builtins]fg_bg.def \
                   [.builtins]getopts.def \
                   [.builtins]hash.def \
                   [.builtins]help.def \
                   [.builtins]history.def \
		   [.builtins]inlib.def \
                   [.builtins]jobs.obj \
                   [.builtins]kill.obj \
                   [.builtins]let.obj \
		   [.builtins]mapfile.def \
                   [.builtins]printf.def \
                   [.builtins]pushd.def \
                   [.builtins]read.def \
		   [.builtins]reserved.def \
                   [.builtins]return.def \
                   [.builtins]set.def \
                   [.builtins]setattr.def \
                   [.builtins]shift.def \
                   [.builtins]shopt.def \
                   [.builtins]source.def \
                   [.builtins]suspend.def \
                   [.builtins]test.def \
                   [.builtins]times.def \
                   [.builtins]trap.def \
                   [.builtins]type.def \
                   [.builtins]ulimit.def \
                   [.builtins]umask.def \
                   [.builtins]wait.def

bash_objs = alias.obj,\
	    array.obj,\
	    arrayfunc.obj,\
	    assoc.obj,\
            bashhist.obj,\
            bashline.obj,\
            bracecomp.obj,\
            braces.obj,\
            copy_cmd.obj,\
	    decw_showdisplay.obj,\
            dispose_cmd.obj,\
            error.obj,\
	    eval.obj,\
            execute_cmd.obj,\
            expr.obj,\
	    findcmd.obj,\
            flags.obj,\
            general.obj,\
	    hashcmd.obj,\
	    hashlib.obj,\
            input.obj,\
            nojobs.obj,\
            list.obj,\
            locale.obj,\
            mailcheck.obj,\
            make_cmd.obj,\
            pathexp.obj,\
            pcomplete.obj,\
            pcomplib.obj,\
            print_cmd.obj,\
            redir.obj,\
            shell.obj,\
            sig.obj,\
            siglist.obj,\
            stringlib.obj,\
            subst.obj,\
            syntax.obj,\
            test.obj,\
            trap.obj,\
            unwind_prot.obj,\
            variables.obj,\
            version.obj,\
            y_tab.obj,\
	    xmalloc.obj,\
            vms_get_foreign_cmd.obj,\
            vms_fname_to_unix.obj,\
	    vms_mailstat.obj,\
	    vms_terminal_io.obj,\
	    vms_term.obj

# Nested header files
#========================

# Add stdc.h to config.h since most things include both
config_h = config.h config-top.h config-bot.h [.include]stdc.h \
		vms_term.h vms_lstat_hack.h

alias_h = alias.h hashlib.h

assoc_h = assoc.h hashlib.h

bashansi_h = bashansi.h [.include]ansi_stdlib.h

bashintl_h = bashintl.h [.include]gettext.h

#Everything that includees builtins.h also includes config.h
builtins_h = builtins.h command.h $(general_h) $(alias_h)

xmalloc_h = xmalloc.h bashansi.h

general_h = general.h bashtypes.h [.include]chartypes.h $(xmalloc_h)

hashcmd_h = hashcmd.h hashlib.h

jobs_h = jobs.h quit.h siglist.h [.include]posixwait.h

parser_h = parser.h command.h input.h

pcomplete_h = pcomplete.h hashlib.h

trap_h = trap.h bashtypes.h

variables_h = variables.h array.h assoc.h hashlib.h conftypes.h

shell_h = shell.h bashjmp.h, [.include]posixjmp.h, command.h syntax.h \
	$(general_h) lcl_root:error.h $(variables_h) arrayfunc.h quit.h \
	[.include]maxpath.h unwind_prot.h dispose_cmd.h make_cmd.h \
        [.include]ocache.h subst.h sig.h pathnames.h externs.h

termios_h = vms_term.h, vms_terminal_io.h bits_termios.h

readline_history_h = [.lib.readline]history.h [.lib.readline]rlstdc.h \
	[.lib.readline]rltypedefs.h

readline_keymaps_h = [.lib.readline]keymaps.h [.lib.readline]rlstdc.h \
	[.lib.readline]chardefs.h [.lib.readline]rltypedefs.h

readline_readline_h = [.lib.readline]readline.h, \
		[.lib.tilde]tilde.h, $(readline_keymaps_h)

readline_rldefs_h = [.lib.readline]rldefs.h [.lib.readline]rlstdc.h \
                [.lib.readline]rlconf.h

readline_rlmbutil_h = [.lib.readline]rlmbutil.h [.lib.readline]rlstdc.h

readline_rlprivate_h = [.lib.readline]rlprivate.h [.lib.readline]rlconf.h \
	[.lib.readline]rlstdc.h [.include]posixjmp.h

readline_rlshell_h = [.lib.readline]rlshell.h [.lib.readline]rlstdc.h

readline_rltty_h = [.lib.readline]rltty.h [.lib.readline]rlwinsize.h \
	$(termios_h)

readline_tcap_h = [.lib.readline]tcap.h $(readline_rltty_h)
execute_cmd_h = execute_cmd.h

gnv_shell_c_first = gnv$shell.c_first []vms_pwd_hack.h
rl_gnv_shell_c_first = [.lib.readline]gnv$shell.c_first []vms_pwd_hack.h
rl_gnv_tilde_c_first = [.lib.readline]gnv$tilde.c_first []vms_pwd_hack.h
tl_gnv_tilde_c_first = [.lib.readline]gnv$tilde.c_first []vms_pwd_hack.h


# Definitions for additional directories. These directories are not
# necessarily needed for building bash using MMK but are required so
# that later builds using Bash with configure, make, and install will
# not complain about missing directories in LCL_ROOT:[bash...].
#====================================================================
VMS_ADDITIONAL_DIRS = lcl_root:[.lib]malloc.DIR, lcl_root:[.lib]termcap.DIR\
		      lcl_root:[.lib]tilde.DIR lcl_root:[]doc.DIR \
		      lcl_root:[]support.DIR lcl_root:[]po.DIR \
		      lcl_root:[]examples.DIR, \
		      lcl_root:[.examples]loadables.DIR \
		      lcl_root:[.examples.loadables]perl.DIR



# The first target should be the default build option.
#=======================================================
All : bash test_support $(VMS_ADDITIONAL_DIRS)
   @ $ write sys$output "Build is complete."

bash : [.builtins]mkbuiltins.exe, gnv$bash.exe, bashdebug.exe, \
	gnv$bash_startup.com
   @ $ write sys$output "Bash is built."

test_support : lcl_root:[bash]support.DIR [.support]printenv.exe \
		[.support]recho.exe [.support]xcase.exe \
		[.support]zecho.exe
   @ $ write sys$output "Bash test support routines have been built."


# NFS and repositories do not get along with source file with
# dollar signs in their names.
gnv$bash_startup.com : gnv_bash_startup.com
   @ $ copy/log gnv_bash_startup.com gnv$bash_startup.com

#bash_endian.h : endian.exe
#   $ run/out=bash_endian.h sys$disk:[]endian.exe

lsignames.h : mksignames.exe
	$if f$search("$(mms$target)") .nes. "" then delete $(mms$target);*
	$ mksignames :== $lcl_root:[]mksignames.exe
	$ mksignames $(mms$target)

signames.h : lsignames.h
	$copy lsignames.h $(mms$target)

# Generate the config.h files, and also set up some empty param.h files
# that the source modules are looking for.

lcl_root:[.sys]param.h : vms_sys_param.h
	$create/dir lcl_root:[.sys]/prot=o:rwed
	$copy vms_sys_param.h lcl_root:[.sys]param.h

lcl_root:[.builtins.sys]param.h : vms_sys_param.h
	$create/dir lcl_root:[.builtins.sys]/prot=o:rwed
	$copy vms_sys_param.h lcl_root:[.builtins.sys]param.h

lcl_root:[.lib.sh.sys]param.h : vms_sys_param.h
	$create/dir lcl_root:[.lib.sh.sys]/prot=o:rwed
	$copy vms_sys_param.h lcl_root:[.lib.sh.sys]param.h

config_h_in = config^.h.in
y_tab_obj = y.tab.obj
y_tab_c_in = y.tab.c
y_tab_h = y.tab.h
libgnuintl_h_in = libgnuintl.h.in

config.h : $(config_h_in) config_vms.h config_h.com \
	lcl_root:[.sys]param.h lcl_root:[.builtins.sys]param.h
   $ @config_h.com
   $ purge config.h

config_vms.h : config_vms.h_in
   $ type config_vms.h_in/out=config_vms.h
   $ arch = f$edit(f$getsyi("arch_name"),"LOWERCASE")
   $ machtype = arch + "-dec-vms"
   $ open/append cfv config_vms.h
   $ write cfv "#define CONF_MACHTYPE ""''machtype'"""
   $ write cfv "#define CONF_HOSTTYPE ""''arch'"""
   $ close cfv
   $ purge config_vms.h

#/bin/sh ./support/mkversion.sh -b -S . -s release -d 4.2 -o newversion.h \
#        && mv newversion.h version.h

# Generate a version file, read
version.h : $(config_h) patchlevel.h
   $ @version_h.com

# The following are used by the Bash test suite: printenv, recho, xcase, and
# zecho.
lcl_root:[bash]support.DIR :
   $ create/directory/prot=o:rwed lcl_root:[bash.support]

[.support]printenv.obj : [.support]printenv.c $(config_h)

[.support]recho.obj : [.support]recho.c $(config_h)

[.support]xcase.obj : [.support]xcase.c $(config_h)

[.support]zecho.obj : [.support]zecho.c $(config_h)

[.support]printenv.exe : [.support]printenv.obj vms_crtl_init.obj vms_crtl_values.obj
   $(LINK)$(LFLAGS)/exec=$(MMS$TARGET)/DSF=$(MMS$TARGET_NAME) \
	/MAP=$(MMS$TARGET_NAME) [.support]printenv.obj, []vms_crtl_init.obj, []vms_crtl_values.obj

[.support]recho.exe : [.support]recho.obj vms_crtl_init.obj vms_crtl_values.obj
   $(LINK)$(LFLAGS)/exec=$(MMS$TARGET)/DSF=$(MMS$TARGET_NAME) \
	/MAP=$(MMS$TARGET_NAME) [.support]recho.obj, []vms_crtl_init.obj, []vms_crtl_values.obj

[.support]xcase.exe : [.support]xcase.obj vms_crtl_init.obj vms_crtl_values.obj
   $(LINK)$(LFLAGS)/exec=$(MMS$TARGET)/DSF=$(MMS$TARGET_NAME) \
	/MAP=$(MMS$TARGET_NAME) [.support]xcase.obj, []vms_crtl_init.obj, []vms_crtl_values.obj

[.support]zecho.exe : [.support]zecho.obj vms_crtl_init.obj vms_crtl_values.obj
   $(LINK)$(LFLAGS)/exec=$(MMS$TARGET)/DSF=$(MMS$TARGET_NAME) \
	/MAP=$(MMS$TARGET_NAME) [.support]zecho.obj, []vms_crtl_init.obj, []vms_crtl_values.obj

lcl_root:[.lib]malloc.DIR :
   $ create/directory/prot=o:rwed lcl_root:[.lib.malloc]

lcl_root:[.lib]termcap.DIR :
   $ create/directory/prot=o:rwed lcl_root:[.lib.termcap]

lcl_root:[]doc.DIR :
   $ create/directory/prot=o:rwed lcl_root:[.doc]

lcl_root:[]support.DIR :
   $ create/directory/prot=o:rwed lcl_root:[.support]

lcl_root:[]po.DIR :
   $ create/directory/prot=o:rwed lcl_root:[.po]

lcl_root:[]examples.DIR :
   $ create/directory/prot=o:rwed lcl_root:[.examples]

lcl_root:[.examples]loadables.DIR :
   $ create/directory/prot=o:rwed lcl_root:[.examples.loadables]

lcl_root:[.examples.loadables]perl.DIR :
   $ create/directory/prot=o:rwed lcl_root:[.examples.loadables.perl]


signames.obj : [.support]signames.c $(config_h)

mksignames.obj : [.support]mksignames.c $(config_h)

buildsignames.obj : [.support]signames.c $(config_h)
   $define/user cxx$system_include prj_root:[]
   $define/user cxx$user_include prj_root:[],prj_root:[.include],\
	prj_root:[.lib.readline],prj_root:[.lib.intl],prj_root:[.lib.sh],\
	prj_root:[.lib.glob]
   $(CC)$(CFLAGS)/OBJ=$(MMS$TARGET) $(MMS$SOURCE)

mksignames.exe : mksignames.obj, buildsignames.obj
    $ link/exe=$(mms$target) mksignames.obj, buildsignames.obj

mksyntax.exe : mksyntax.obj
    $ link mksyntax.obj

mksyntax.obj : mksyntax.c $(bashansi_h) syntax.h bashtypes.h $(config_h) \
	[.include]chartypes.h

syntax.c : mksyntax.exe
    $ mksyntax :== $lcl_root:[]mksyntax.exe
    $ mksyntax -o $(MMS$TARGET)

pathnames.h : pathnames.h_vms
    $ copy pathnames.h_vms pathnames.h


bashversion.obj : bashversion.c patchlevel.h conftypes.h version.h

bashversion.exe : bashversion.obj buildversion.obj


#gcc  -DPROGRAM='"bash"' -DCONF_HOSTTYPE='"alpha"' -DCONF_OSTYPE='"vms"'
#-DCONF_MACHTYPE='"alpha-dec-vms"' -DCONF_VENDOR='"dec"'
#-DLOCALEDIR='"/usr/share/locale"
#' -DPACKAGE='"bash"' -DSHELL -DHAVE_CONFIG_H   -I.  -I. -I./include -I./lib
#-I./
#lib/intl -I/PRJ_ROOT/bash-4.2/lib/intl  -g  -DBUILDTOOL -c -o buildversion.o
#./version.c
buildversion.obj : version.h conftypes.h patchlevel.h version.c


#gcc  -DPROGRAM='"bash"' -DCONF_HOSTTYPE='"alpha"' -DCONF_OSTYPE='"vms"'
#-DCONF_MACHTYPE='"alpha-dec-vms"' -DCONF_VENDOR='"dec"'
#-DLOCALEDIR='"/usr/share/locale"
#' -DPACKAGE='"bash"' -DSHELL -DHAVE_CONFIG_H
#-I.  -I. -I./include -I./lib -I./
#lib/intl -I/PRJ_ROOT/bash-4.2/lib/intl  -g  -o bashversion
#./support/bashversion
#.c buildversion.o



# We currently set DECC$PIPE_BUFFER_QUOTA to 65536 bytes, which means
# the writer can write that many bytes before blocking.
[.builtins]pipesize.h :
   $ open/write psh [.builtins]pipesize.h
   $ write psh "/* File: pipesize.h generated by bash.mms"
   $ write psh " * by ''f$user()' at ''f$cvtime(,"ABSOLUTE")"
   $ write psh " */"
   $ write psh "#define PIPESIZE 65536"
   $ close psh

#Bashversions

gnv$version.c_first : gnv_version.c_first
    $copy $(MMS$SOURCE) $(MMS$TARGET)

version.obj : version.c version.h gnv$version.c_first vms_eco_level.h
   $define/user cxx$system_include prj_root:[]
   $define/user cxx$user_include prj_root:[],prj_root:[.include],\
	prj_root:[.lib.readline],prj_root:[.lib.intl],prj_root:[.lib.sh],\
	prj_root:[.lib.glob]
   $(CC)$(CFLAGS)/OBJ=$(MMS$TARGET) \
   /first_include=gnv$version.c_first $(MMS$SOURCE)

# GNU Regular Expression Support
lcl_root:[.lib]regex.DIR :
	$create/directory/prot=o:rwed lcl_root:[.lib.regex]

[.lib.regex]regex.obj : vms_root:[.lib.regex]regex.c, \
                        $(config_h)
   $define/user cxx$system_include prj_root:[.lib.regex], prj_root:[bash]
   $define/user cxx$user_include prj_root:[.lib.regex], prj_root:[.include]
   $(CC)$(CFLAGS)/OBJ=$(MMS$TARGET) $(MMS$SOURCE)

# libsh
lcl_root:[.lib]sh.DIR :
	$create/directory/prot=o:rwed lcl_root:[.lib.sh]

libsh.olb : libsh($(libsh_objs))
    @ write sys$output "libsh is up to date"

[.lib.sh]casemod.obj : [.lib.sh]casemod.c, $(config_h) \
	$(bashansi_h) $(bashintl_h) bashtypes.h [.lib.glob]strmatch.h

[.lib.sh]clktck.obj : [.lib.sh]clktck.c $(config_h) \
	bashtypes.h

[.lib.sh]clock.obj : [.lib.sh]clock.c $(config_h)

[.lib.sh]dprintf.obj : [.lib.sh]dprintf.c $(config_h)

# moved to vms_lstat_hack.h
#[.lib.sh]gnv$eaccess.c_first : [.lib.sh]gnv_eaccess.c_first
#    $type $(MMS$SOURCE)/output=$(MMS$TARGET)

[.lib.sh]eaccess.obj : [.lib.sh]eaccess.c $(config_h) \
	bashtypes.h $(bashansi_h) [.include]posixstat.h
   $define/user cxx$system_include prj_root:[]
   $define/user cxx$user_include prj_root:[],prj_root:[.include],\
	prj_root:[.lib.readline],prj_root:[.lib.intl],prj_root:[.lib.sh],\
	prj_root:[.lib.glob]
   $(CC)$(CFLAGS)/OBJ=$(MMS$TARGET) $(MMS$SOURCE)

[.lib.sh]fmtullong.obj : [.lib.sh]fmtullong.c, [.lib.sh]fmtulong.c, \
	$(config_h), $(bashansi_h), $(bashintl_h)

[.lib.sh]fmtulong.obj : [.lib.sh]fmtulong.c $(config_h), \
	$(bashansi_h), $(bashintl_h)

[.lib.sh]fmtumax.obj : [.lib.sh]fmtumax.c, [.lib.sh]fmtulong.c, $(config_h), \
	$(bashansi_h), $(bashintl_h)

[.lib.sh]fnxform.obj : [.lib.sh]fnxform.c $(config_h) bashtypes.h \
	$(bashansi_h) $(bashintl_h)

lcl_root:[.lib.sh]fpurge.c : src_root:[.lib.sh]fpurge.c [.lib.sh]fpurge_c.tpu
    $(EVE) $(UNIX_2_VMS) $(MMS$SOURCE)/OUT=$(MMS$TARGET)\
	    /init='f$element(1, ",", "$(MMS$SOURCE_LIST)")'

[.lib.sh]fpurge.obj : lcl_root:[.lib.sh]fpurge.c, $(config_h)

[.lib.sh]getcwd.obj : [.lib.sh]getcwd.c $(config_h) \
	bashtypes.h $(bashansi_h) [.include]posixstat.h

[.lib.sh]getenv.obj : [.lib.sh]getenv.c, $(config_h), \
	$(bashansi_h)

[.lib.sh]inet_aton.obj : [.lib.sh]inet_aton.c, \
	$(config_h), $(bashansi_h)

lcl_root:[.lib.sh]input_avail.c : src_root:[.lib.sh]input_avail.c \
	[.lib.sh]input_avail_c.tpu
    $(EVE) $(UNIX_2_VMS) $(MMS$SOURCE)/OUT=$(MMS$TARGET)\
	    /init='f$element(1, ",", "$(MMS$SOURCE_LIST)")'

[.lib.sh]input_avail.obj : lcl_root:[.lib.sh]input_avail.c \
	$(config_h)

[.lib.sh]itos.ogj : [.lib.sh]itos.c $(config_h) \
	$(bashansi_h) $(shell_h)

[.lib.sh]mailstat.obj : [.lib.sh]mailstat.c $(config_h) \
	bashtypes.h $(bashansi_h) [.include]posixstat.h

[.lib.sh]makepath.obj : [.lib.sh]makepath.c \
	$(config_h) $(bashansi_h) $(shell_h) [.lib.tilde]tilde.h

[.lib.sh]mbscasecmp.obj : [.lib.sh]mbscasecmp.c \
	$(config_h)

[.lib.sh]mbschr.obj : [.lib.sh]mbschr.c $(config_h) \
	$(bashansi_h) [.include]shmbutil.h

[.lib.sh]mbscmp.obj : [.lib.sh]mbscmp.c $(config_h)

#[.lib.sh]memset.obj : [.lib.sh]memset.c

#[.lib.sh]mktime.obj : [.lib.sh]mktime.c $(config_h)

[.lib.sh]netconn.obj : [.lib.sh]netconn.c $(config_h) \
	bashtypes.h

[.lib.sh]netopen.obj : [.lib.sh]netopen.c $(config_h) \
	$(bashansi_h) $(bashintl_h) $(xmalloc_h)

[.lib.sh]gnv$oslib.c_first : gnv_lib_sh_oslib.c_first
		$type $(MMS$SOURCE) /output=$(MMS$TARGET)

[.lib.sh]oslib.obj : [.lib.sh]oslib.c $(config_h) \
		bashtypes.h $(bashansi_h) $(shell_h) [.include]posixstat.h \
		[.lib.sh]gnv$oslib.c_first
   $define/user cxx$system_include prj_root:[],prj_root:[.include]
   $define/user cxx$user_include prj_root:[],prj_root:[.include],\
	prj_root:[.lib.readline],prj_root:[.lib.intl],prj_root:[.lib.sh],\
	prj_root:[.lib.glob]
   $(CC)$(CFLAGS)/OBJ=$(MMS$TARGET) \
   /first_include=[.lib.sh]gnv$oslib.c_first $(MMS$SOURCE)

[.lib.sh]pathcanon.obj : [.lib.sh]pathcanon.c \
		$(config_h) bashtypes.h $(bashansi_h) $(shell_h)

[.lib.sh]pathphys.obj : [.lib.sh]pathphys.c $(config_h) \
		bashtypes.h $(bashansi_h) $(shell_h) [.include]posixstat.h

[.lib.sh]rename.obj : [.lib.sh]rename.c $(config_h) \
		bashtypes.h [.include]posixstat.h

[.lib.sh]setlinebuf.obj : [.lib.sh]setlinebuf.c \
		$(config_h) $(xmalloc_h)

[.lib.sh]shmatch.obj : [.lib.sh]shmatch.c $(config_h) \
		$(bashansi_h) regex.h $(shell_h) $(variables_h) externs.h

[.lib.sh]shmbchar.obj : [.lib.sh]shmbchar.c $(config_h) \
		[.include]shmbutil.h [.include]shmbchar.h

[.lib.sh]shquote.obj : [.lib.sh]shquote.c $(config_h) \
		syntax.h $(xmalloc_h)

[.lib.sh]shtty.obj : [.lib.sh]shtty.c $(config_h) \
		[.include]shtty.h $(termios_h)

[.lib.sh]snprintf.obj : [.lib.sh]snprintf.c $(config_h) \
		bashtypes.h $(bashansi_h) [.include]chartypes.h \
		[.include]shmbutil.h $(shell_h) $(xmalloc_h)

[.lib.sh]spell.obj : [.lib.sh]spell.c $(config_h) \
		bashtypes.h $(bashansi_h) [.include]posixstat.h

[.lib.sh]strcasecmp.obj : [.lib.sh]strcasecmp.c \
		$(config_h) $(bashansi_h) [.include]chartypes.h

[.lib.sh]strcasestr.obj : [.lib.sh]strcasestr.c \
		$(config_h) $(bashansi_h) [.include]chartypes.h

[.lib.sh]strchrnul.obj : [.lib.sh]strchrnul.c $(config_h)

[.lib.sh]strerror.obj : [.lib.sh]strerror.c $(config_h) \
		$(bashansi_h) $(shell_h)

#[.lib.sh]strftime.obj : [.lib.sh]strftime.c $(config_h)

[.lib.sh]stringlist.obj : [.lib.sh]stringlist.c \
		$(config_h) $(bashansi_h) $(shell_h)

[.lib.sh]stringvec.obj : [.lib.sh]stringvec.c \
		$(config_h) bashtypes.h [.include]chartypes.h $(shell_h)

[.lib.sh]strnlen.obj : [.lib.sh]strnlen.c $(config_h)

[.lib.sh]strpbrk.obj : [.lib.sh]strpbrk.c $(config_h)

[.lib.sh]strstr.obj : [.lib.sh]strstr.c $(config_h)

[.lib.sh]strtod.obj : [.lib.sh]strtod.c $(config_h) \
		$(bashansi_h) [.include]chartypes.h

strtol_c = [.lib.sh]strtol.c, $(config_h), $(bashansi_h) [.include]chartypes.h

[.lib.sh]strtol.obj : $(strtol_c)

[.lib.sh]strtoll.obj : [.lib.sh]strtoll.c $(strtol_c)

[.lib.sh]strtoul.obj : [.lib.sh]strtoul.c $(strtol_c)

[.lib.sh]strtoull.obj : [.lib.sh]strtoull.c $(strtol_c)

[.lib.sh]strtrans.obj : [.lib.sh]strtrans.c $(config_h) \
		$(bashansi_h) $(shell_h)

[.lib.sh]times.obj : [.lib.sh]times.c $(config_h)

[.lib.sh]timeval.obj : [.lib.sh]timeval.c $(config_h)

[.lib.sh]tmpfile.obj : [.lib.sh]tmpfile.c $(config_h) \
		bashtypes.h $(shell_h) [.include]posixstat.h

[.lib.sh]uconvert.obj : [.lib.sh]uconvert.c $(config_h) \
		bashtypes.h [.include]chartypes.h $(shell_h) $(builtins_h)

[.lib.sh]ufuncs.obj : [.lib.sh]ufuncs.c $(config_h) \
		bashtypes.h

lcl_root:[.lib.sh]unicode.c : src_root:[.lib.sh]unicode.c [.lib.sh]unicode_c.tpu
    $(EVE) $(UNIX_2_VMS) $(MMS$SOURCE)/OUT=$(MMS$TARGET)\
	    /init='f$element(1, ",", "$(MMS$SOURCE_LIST)")'

[.lib.sh]unicode.obj : lcl_root:[.lib.sh]unicode.c

[.lib.sh]vprint.obj : [.lib.sh]vprint.c $(config_h)

[.lib.sh]wcsdup.obj : [.lib.sh]wcsdup.c $(config_h) \
		$(bashansi_h) $(xmalloc_h)

[.lib.sh]wcswidth.obj : [.lib.sh]wcswidth.c $(config_h) \
		$(bashansi_h)

[.lib.sh]winsize.obj : [.lib.sh]winsize.c $(config_h) \
		bashtypes.h

[.lib.sh]zcatfd.obj : [.lib.sh]zcatfd.c $(config_h)

[.lib.sh]zgetline.obj : [.lib.sh]zgetline.c $(config_h) \
		$(xmalloc_h)

[.lib.sh]zmapfd.obj : [.lib.sh]zmapfd.c $(config_h) \
		$(bashansi_h) command.h $(general_h)

[.lib.sh]zread.obj : [.lib.sh]zread.c $(config_h)

[.lib.sh]zwrite.obj : [.lib.sh]zwrite.c $(config_h)


# libreadline
lcl_root:[]lib.DIR :
	$create/directory/prot=o:rwed lcl_root:[.lib]

lcl_root[.lib]readline.DIR :
	$create/directory/prot=o:rwed lcl_root:[.lib.readline]

libreadline.olb : libreadline($(libreadline_objs))
    @ write sys$output "libreadline is up to date"

# floss.h included, but not supplied?
[.lib.readline]bind.obj : [.lib.readline]bind.c \
		[.include]posixstat.h $(readline_rldefs_h) \
		$(readline_readline_h) \
		$(readline_history_h) $(readline_rlprivate_h) \
		$(readline_rlshell_h) $(xmalloc_h)

[.lib.readline]callback.obj : [.lib.readline]callback.c \
		$(config_h) , [.lib.readline]rlconf.h, $(readline_rldefs_h) \
		$(readline_readline_h), $(readline_rlprivate_h) \
		$(xmalloc_h)

[.lib.readline]compat.obj : [.lib.readline]compat.c $(config_h) \
		[.lib.readline]rlstdc.h [.lib.readline]rltypedefs.h

[.lib.readline]complete.obj : [.lib.readline]complete.c $(config_h) \
		[.include]ansi_stdlib.h [.include]posixdir.h \
		[.include]posixstat.h \
		$(readline_rldefs_h) $(readline_rldmbutil_h) \
		$(readline_readline_h) $(xmalloc_h) $(readline_rlprivate_h)

[.lib.readline]display.obj : [.lib.readline]display.c $(config_h) \
		[.include]posixstat.h [.include]ansi_stdlib.h \
		$(readline_rldefs_h) $(readline_rldmbutil_h) \
		$(readline_tcap_h) \
		$(readline_readline_h) $(readline_history_h) \
		$(readline_rlprivate_h) $(xmalloc_h)
#   $define/user readline prj_root:[.lib.readline]
#   $define/user glob prj_root:[.lib.glob]
#   $define/user tilde prj_root:[.lib.tilde]
#   $define/user cxx$user_include prj_root:[.lib.readline]

emacs_keymap_c = [.lib.readline]emacs_keymap.c $(readline_readline_h)
vi_keymap_c = [.lib.readline]vi_keymap.c $(readline_readline_h)


[.lib.readline]emacs_keymap.obj : $(emacs_keymap_c)

[.lib.readline]funmap.obj : [.lib.readline]funmap.c, \
		$(config_h) [.include]ansi_stdlib.h \
		[.lib.readline]rlconf.h $(readline_readline_h) $(xmalloc_h)


[.lib.readline]histexpand.obj : [.lib.readline]histexpand.c \
		$(config_h) [.include]ansi_stdlib.h $(readline_rldmbutil_h) \
		$(readline_history_h) \
		[.lib.readline]histlib.h \
		$(readline_rlshell_h) $(xmalloc_h)

[.lib.readline]histfile.obj : [.lib.readline]histfile.c \
		$(config_h) [.include]posixstat.h [.include]ansi_stdlib.h \
		$(readline_history_h) [.lib.readline]histlib.h \
		$(readline_rlshell_h) $(xmalloc_h)

[.lib.readline]history.obj : [.lib.readline]history.c, $(config_h) \
		[.include]ansi_stdlib.h, $(readline_history_h) \
		[.lib.readline]histlib.h $(xmalloc_h)

[.lib.readline]histsearch.obj : [.lib.readline]histsearch.c \
		$(config_h) [.include]ansi_stdlib.h \
		$(readline_history_h) [.lib.readline]histlib.h

[.lib.readline]histsearch.obj : [.lib.readline]histsearch.c \
		$(config_h) [.include]ansi_stdlib.h \
		$(readline_history_h) [.lib.readline]histlib.h

[.lib.readline]gnv$input.c_first : gnv_readline_input.c_first
		$type $(MMS$SOURCE) /output=$(MMS$TARGET)

[.lib.readline]input.obj : [.lib.readline]input.c $(config_h) \
		[.include]ansi_stdlib.h [.include]posixselect.h \
		$(readline_rldefs_h) $(readline_rlmbutil_h) \
		$(readline_readline_h) $(readline_rlprivate_h) \
		$(readline_rlshell_h) $(xmalloc_h) \
		[.lib.readline]gnv$input.c_first
   $define/user readline prj_root:[.lib.readline]
   $define/user cxx$system_include prj_root:[]
   $define/user cxx$user_include prj_root:[.lib.readline],prj_root:[.include]
   $(CC)$(CFLAGS)/OBJ=$(MMS$TARGET) \
   /first_include=gnv$input.c_first $(MMS$SOURCE)

[.lib.readline]isearch.obj : [.lib.readline]isearch.c $(config_h) \
		[.include]ansi_stdlib.h $(readline_rldefs_h) \
		$(readline_rldmbutil_h) $(readline_readline_h) \
		$(readline_history_h) \
		$(readline_rlprivate_h) $(xmalloc_h)

[.lib.readline]keymaps.obj : [.lib.readline]keymaps.c $(config_h) \
		[.include]ansi_stdlib.h $(readline_readline_h) \
		[.lib.readline]rlconf.h $(emacs_keymap_c) \
		$(vi_keymap_c) $(xmalloc_h)

[.lib.readline]kill.obj : [.lib.readline]kill.c $(config_h) \
		[.include]ansi_stdlib.h $(readline_rldefs_h) \
		$(readline_readline_h) $(readline_history_h) \
		$(readline_rlprivate_h) $(xmalloc_h)

[.lib.readline]macro.obj : [.lib.readline]macro.c $(config_h) \
		[.include]ansi_stdlib.h $(readline_rldefs_h) \
		$(readline_readline_h) $(readline_history_h) \
		$(readline_rlprivate_h) $(xmalloc_h)

[.lib.readline]mbutil.obj : [.lib.readline]mbutil.c $(config_h) \
		[.include]posixjmp.h [.include]ansi_stdlib.h \
		$(readline_rldefs_h) $(readline_rldmbutil_h) \
		$(readline_readline_h) \
		$(readline_rlprivate_h) $(xmalloc_h)

[.lib.readline]misc.obj : [.lib.readline]misc.c $(config_h) \
		[.include]ansi_stdlib.h $(readline_rldefs_h) \
		$(readline_rldmbutil_h) $(readline_readline_h) \
		$(readline_history_h) $(readline_rlprivate_h) \
		$(readline_rlshell_h) $(xmalloc_h)

[.lib.readline]nls.obj : [.lib.readline]nls.c $(config_h) \
		[.include]ansi_stdlib.h $(readline_rldefs_h) \
		$(readline_readline_h) $(readline_rlshell_h) \
		$(readline_rlprivate_h)

# floss.h included, but not supplied?
[.lib.readline]parens.obj : [.lib.readline]parens.c $(config_h) \
		[.lib.readline]rlconf.h [.include]posixselect.h \
		$(readline_readline_h) $(readline_rlprivate_h)

[.lib.readline]readline.obj : [.lib.readline]readline.c $(config_h) \
		[.include]posixstat.h [.include]posixjmp.h \
		[.include]ansi_stdlib.h $(readline_rldefs_h) \
		$(readline_rldmbutil_h) \
		$(readline_readline_h) $(readline_history_h) \
		$(readline_rlprivate_h) $(readline_rlshell_h) $(xmalloc_h)
   $define/user readline prj_root:[.lib.readline]
   $define/user glob prj_root:[.lib.glob]
   $define/user tilde prj_root:[.lib.tilde]
   $define/user cxx$system_include prj_root:[]
   $define/user cxx$user_include prj_root:[],prj_root:[.include],\
	prj_root:[.lib.readline],prj_root:[.lib.intl],prj_root:[.lib.sh]
   $(CC)$(cflagsx)/define=(MODULE_READLINE=1,$(cdefs1),SHELL=1)\
	/OBJ=$(MMS$TARGET) $(MMS$SOURCE)

[.lib.readline]rltty.obj : [.lib.readline]rltty.c  $(config_h) \
		$(readline_rldefs_h) $(readline_readline_h) \
		$(readline_rlprivate_h)
   $define/user readline prj_root:[.lib.readline]
   $define/user glob prj_root:[.lib.glob]
   $define/user tilde prj_root:[.lib.tilde]
   $define/user cxx$system_include prj_root:[]
   $define/user cxx$user_include prj_root:[],prj_root:[.include],\
	prj_root:[.lib.readline],prj_root:[.lib.intl],prj_root:[.lib.sh]
   $(CC)$(cflagsx)/define=(MODULE_RLTTY=1,$(cdefs1),SHELL=1)\
	/OBJ=$(MMS$TARGET) $(MMS$SOURCE)

[.lib.readline]savestring.obj : [.lib.readline]savestring.c $(config_h) \
		$(xmalloc_h)

[.lib.readline]search.obj : [.lib.readline]search.c $(config_h) \
		[.include]ansi_stdlib.h $(readline_rldefs_h) \
		$(readline_rldmbutil_h) $(readline_readline_h) \
		$(readline_history_h) \
		$(readline_rlprivate_h) $(xmalloc_h)

[.lib.readline]gnv$shell.c_first : [.lib.readline]gnv_shell.c_first
		$type $(MMS$SOURCE) /output=$(MMS$TARGET)

[.lib.readline]shell.obj : [.lib.readline]shell.c $(config_h) \
		[.include]ansi_stdlib.h [.lib.readline]rlstdc.h \
		$(readline_rlshell_h) $(xmalloc_h) \
		$(rl_gnv_shell_c_first)

[.lib.readline]signals.obj : [.lib.readline]signals.c $(config_h) \
		$(readline_rldefs_h) $(readline_readline_h) \
		$(readline_history_h) $(readline_rlprivate_h)

[.lib.readline]terminal.obj : [.lib.readline]terminal.c $(config_h) \
		[.include]posixstat.h [.include]ansi_stdlib.h \
		$(readline_rldefs_h) $(readline_rltty_h) \
		$(readline_tcap_h) \
		$(readline_readline_h) $(readline_history_h) \
		$(readline_rlprivate_h) $(readline_rlshell_h) $(xmalloc_h)

[.lib.readline]text.obj : [.lib.readline]text.c $(config_h) \
		[.include]ansi_stdlib.h $(readline_rldefs_h) \
		$(readline_rldmbutil_h) $(readline_readline_h) \
		$(readline_history_h) $(readline_rlprivate_h) \
		$(shell_h) $(xmalloc_h)

[.lib.readline]gnv$tilde.c_first : [.lib.readline]gnv_tilde.c_first
		$type $(MMS$SOURCE) /output=$(MMS$TARGET)

[.lib.readline]tilde.obj : [.lib.readline]tilde.c [.lib.tilde]tilde.h \
		$(config_h) [.include]ansi_stdlib.h $(xmalloc_h) \
		$(rl_gnv_tilde_c_first)

[.lib.readline]undo.obj : [.lib.readline]undo.c $(config_h) \
		[.include]ansi_stdlib.h $(readline_rldefs_h) \
		$(readline_readline_h) $(readline_history_h) \
		$(readline_rlprivate_h) $(xmalloc_h)

[.lib.readline]util.obj : [.lib.readline]util.c $(config_h) \
		[.include]posixjmp.h [.include]ansi_stdlib.h \
		$(readline_rldefs_h) $(readline_rldmbutil_h) \
		$(readline_readline_h) $(readline_rlprivate_h) $(xmalloc_h)

[.lib.readline]vi_keymap.obj : $(vi_keymap_c)

[.lib.readline]vi_mode.obj : [.lib.readline]vi_mode.c $(config_h) \
		[.lib.readline]rlconf.h [.include]ansi_stdlib.h \
		$(readline_rldefs_h) $(readline_rldmbutil_h) \
		$(readline_readline_h) $(readline_history_h) \
		$(readline_rlprivate_h) $(xmalloc_h)

[.lib.readline]xfree.obj : [.lib.readline]xfree.c $(config_h) \
		[.include]ansi_stdlib.h $(xmalloc_h)

[.lib.readline]xmalloc.obj : [.lib.readline]xmalloc.c $(config_h) \
		[.include]ansi_stdlib.h $(xmalloc_h)

# libtermcap
lcl_root[.lib]termcap.DIR :
	$create/directory/prot=o:rwed lcl_root:[.lib.termcap]

[.lib.termcap]termcap.c : [.lib.termcap]termcap.c $(config_h) \
		[.lib.termcap]ltcap.h

[.lib.termcap]tparam.c : [.lib.termcap]tparam.c $(config_h) \
		[.lib.termcap]ltcap.h

[.lib.termcap]version.c : [.lib.termcap]version.c

# libglob
lcl_root:[.lib]glob.DIR :
	$create/directory/prot=o:rwed lcl_root:[.lib.glob]

libglob.olb : libglob($(libglob_objs))
    @ write sys$output "libglob is up to date"

[.lib.glob]glob.obj : [.lib.glob]glob.c $(config_h) \
		bashtypes.h $(bashansi_h) [.include]posixdir.h \
		[.include]posixstat.h [.include]shmbutil.h \
		$(xmalloc_h) $(shell_h) \
		[.lib.glob]glob.h [.lib.glob]glob_loop.c [.lib.glob]strmatch.h

[.lib.glob]gmisc.obj : [.lib.glob]gmisc.c $(config_h) bashtypes.h \
		$(bashansi_h) [.include]shmbutil.h [.lib.glob]gm_loop.c

[.lib.glob]smatch.obj : [.lib.glob]smatch.c $(config_h) [.lib.glob]strmatch.h \
		$(bashansi_h) [.include]shmbutil.h $(xmalloc_h) \
		[.lib.glob]collsyms.h [.lib.glob]sm_loop.c

[.lib.glob]strmatch.obj : [.lib.glob]strmatch.c $(config_h) \
		[.lib.glob]strmatch.h

[.lib.glob]xmbsrtowcs.obj : [.lib.glob]xmbsrtowcs.c $(config_h) \
		$(bashansi_h) [.include]shmbutil.h

#libtilde
lcl_root:[.lib]tilde.DIR :
	$create/directory/prot=o:rwed lcl_root:[.lib.tilde]

[.lib.tilde]gnv$tilde.c_first : [.lib.tilde]gnv_tilde.c_first
		$type $(MMS$SOURCE) /output=$(MMS$TARGET)

[.lib.tilde]tilde.obj : [.lib.tilde]tilde.c $(config_h) \
		[.include]ansi_stdlib.h \
		[.lib.tilde]tilde.h $(xmalloc_h) \
		$(tl_gnv_tilde_c_first)

#libintl
lcl_root:[.lib]intl.DIR :
	$create/directory/prot=o:rwed lcl_root:[.lib.intl]

libintl.olb : libintl($(libintl_objs))
    @ write sys$output "libintl is up to date"

[.lib.intl]libgnuintl.h : [.lib.intl]$(libgnuintl_h_in)
    @ type [.lib.intl]$(libgnuintl_h_in) /output=[.lib.intl]libgnuintl.h

gettextp_h = gettextP.h

[.lib.intl]bindtextdom.obj : [.lib.intl]bindtextdom.c $(config_h) \
		[.lib.intl]libgnuintl.h [.lib.intl]$(gettextp_h)

[.lib.intl]dcgettext.obj : [.lib.intl]dcgettext.c $(config_h) \
		[.lib.intl]libgnuintl.h [.lib.intl]gettextP.h

[.lib.intl]gnv$dcigettext.c_first : [.lib.intl]gnv_dcigettext.c_first
		$type $(MMS$SOURCE) /output=$(MMS$TARGET)

lcl_root:[.lib.intl]dcigettext.c : src_root:[.lib.intl]dcigettext.c \
	[.lib.intl]dcigettext_c.tpu
    $(EVE) $(UNIX_2_VMS) $(MMS$SOURCE)/OUT=$(MMS$TARGET)\
	    /init='f$element(1, ",", "$(MMS$SOURCE_LIST)")'

[.lib.intl]dcigettext.obj : lcl_root:[.lib.intl]dcigettext.c $(config_h) \
		[.lib.intl]gettextP.h [.lib.intl]plural-exp.h \
		[.lib.intl]hash-string.h [.lib.intl]libgnuintl.h \
		[.lib.intl]eval-plural.h, [.lib.intl]gnv$dcigettext.c_first
   $define/user cxx$system_include prj_root:[],prj_root:[.include]
   $define/user cxx$user_include prj_root:[],prj_root:[.include],\
	prj_root:[.lib.readline],prj_root:[.lib.intl],prj_root:[.lib.sh],\
	prj_root:[.lib.glob]
   $(CC)$(cflagsx)/define=(_USE_STD_STAT=1,$(cdefs1),IN_LIBINTL=1) \
	/OBJ=$(MMS$TARGET) \
	/first_include=[.lib.intl]gnv$dcigettext.c_first $(MMS$SOURCE)

[.lib.intl]dcngettext.obj : [.lib.intl]dcngettext.c $(config_h) \
		[.lib.intl]libgnuintl.h [.lib.intl]gettextP.h

[.lib.intl]dgettext.obj : [.lib.intl]dgettext.c $(config_h) \
		[.lib.intl]libgnuintl.h [.lib.intl]gettextP.h

[.lib.intl]dngettext.obj : [.lib.intl]dngettext.c $(config_h) \
		[.lib.intl]libgnuintl.h [.lib.intl]gettextP.h

[.lib.intl]explodename.obj : [.lib.intl]explodename.c $(config_h) \
		[.lib.intl]loadinfo.h

[.lib.intl]finddomain.obj : [.lib.intl]finddomain.c $(config_h) \
		[.lib.intl]libgnuintl.h [.lib.intl]gettextP.h

[.lib.intl]gettext.obj : [.lib.intl]gettext.c $(config_h) \
		[.lib.intl]libgnuintl.h [.lib.intl]gettextP.h

[.lib.intl]intl-compat.obj : [.lib.intl]intl-compat.c $(config_h) \
		[.lib.intl]gettextP.h

[.lib.intl]l10nflist.obj : [.lib.intl]l10nflist.c $(config_h) \
		[.lib.intl]loadinfo.h

[.lib.intl]gnv$loadmsgcat.c_first : [.lib.intl]gnv_loadmsgcat.c_first
		$type $(MMS$SOURCE) /output=$(MMS$TARGET)

[.lib.intl]loadmsgcat.obj : [.lib.intl]loadmsgcat.c $(config_h) \
		[.lib.intl]gmo.h [.lib.intl]gettextP.h \
		[.lib.intl]hash-string.h [.lib.intl]plural-exp.h \
		[.lib.intl]gnv$loadmsgcat.c_first
   $define/user cxx$system_include prj_root:[],prj_root:[.include]
   $define/user cxx$user_include prj_root:[],prj_root:[.include],\
	prj_root:[.lib.readline],prj_root:[.lib.intl],prj_root:[.lib.sh],\
	prj_root:[.lib.glob]
   $(CC)$(CFLAGS)/OBJ=$(MMS$TARGET) \
   /first_include=[.lib.intl]gnv$loadmsgcat.c_first $(MMS$SOURCE)

[.lib.intl]localcharset.obj : [.lib.intl]localcharset.c $(config_h) \
		[.lib.intl]localcharset.h [.lib.intl]relocatable.h

[.lib.intl]localealias.obj : [.lib.intl]localealias.c $(config_h) \
		[.lib.intl]gettextP.h [.lib.intl]relocatable.h

[.lib.intl]localename.obj : [.lib.intl]localename.c $(config_h)

[.lib.intl]log.obj : [.lib.intl]log.c $(config_h) \

[.lib.intl]ngettext.obj : [.lib.intl]ngettext.c $(config_h) \
		[.lib.intl]libgnuintl.h [.lib.intl]gettextP.h

[.lib.intl]os2compat.obj : [.lib.intl]os2compat.c $(config_h)

[.lib.intl]osdep.obj : [.lib.intl]osdep.c [.lib.intl]os2compat.c $(config_h) \

[.lib.intl]plural-exp.obj : [.lib.intl]plural-exp.c $(config_h) \
		[.lib.intl]plural-exp.h

[.lib.intl]plural.obj : [.lib.intl]plural.c $(config_h) \
		[.lib.intl]plural-exp.h

[.lib.intl]relocatable.obj : [.lib.intl]relocatable.c $(config_h) \
		[.lib.intl]relocatable.h $(xmalloc_h)

[.lib.intl]textdomain.obj : [.lib.intl]textdomain.c $(config_h) \
		[.lib.intl]libgnuintl.h [.lib.intl]gettextP.h

#libbuiltins

lcl_root:[]builtins.DIR :
	$create/directory/prot=o:rwed lcl_root:[.builtins]

libbuiltins.olb : libbuiltins($(libbuiltins_objs))
    @ write sys$output "libbuiltins is up to date"


[.builtins]bashgetopt.obj : [.builtins]bashgetopt.c, $(config_h) $(shell_h),\
	$(bashansi_h) [.builtins]common.h
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=bashgetopt.obj bashgetopt.c
   $set def prj_root:[-]

[.builtins]builtins.obj : [.builtins]builtins.c $(builtins_h), \
	[.builtins]builtext.h
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $(CC)$(CFLAGS)/OBJ=builtins.obj builtins.c
   $set def prj_root:[-]

[.builtins]builtins.c : [.builtins]builtext.h
    @ $continue

#  ./$(MKBUILTINS) -externfile builtext.h -structfile builtins.c \
#     -noproduction $(DIRECTDEFINE) $(HELPDIRDEFINE) $(HELPSTRINGS) $(DEFSRC)

[.builtins]builtext.h : [.builtins]mkbuiltins.exe $(VMSDEFSRC)
   $ create/dir lcl_root:[.builtins]/prot=o:rwed
   @ mkbuiltins :== $prj_root:[]mkbuiltins.exe
   $ set def prj_root:[.builtins]
   $ mkbuiltins -externfile builtext.h -structfile builtins.c \
	-noproduction "-D" . $(DEFSRC)
   $ set def prj_root:[-]

[.builtins]gnv$common.c_first : [.builtins]gnv_common.c_first
   $ create/dir lcl_root:[.builtins]/prot=o:rwed
   $ type $(MMS$SOURCE) /output=$(MMS$TARGET)

[.builtins]common.obj : [.builtins]common.c, $(config_h) bashtypes.h \
		[.include]posixstat.h $(bashansi_h) $(bashintl_h) $(shell_h) \
		[.include]maxpath.h flags.h $(jobs_h) $(builtins_h) input.h \
		$(execute_cmd_h) unwind_prot.h $(trap_h) \
		[.builtins]bashgetopt.h  [.builtins]common.h \
		[.builtins]builtext.h [.lib.tilde]tilde.h bashhist.h \
		[.builtins]gnv$common.c_first
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(cflags)/first_include=gnv$common.c_first \
	/OBJ=common.obj common.c
   $set def prj_root:[-]

[.builtins]gnv$evalfile.c_first : [.builtins]gnv_evalfile.c_first
    $type $(MMS$SOURCE) /output=$(MMS$TARGET)

[.builtins]evalfile.obj : [.builtins]evalfile.c $(config_h) \
		bashtypes.h [.include]posixstat.h [.include]filecntl.h \
		$(bashansi_h) $(bashintl_h) $(shell_h) $(jobs_h) \
		$(builtins_h) flags.h input.h $(execute_cmd_h) $(trap_h) \
		bashhist.h [.builtins]common.h, [.builtins]gnv$evalfile.c_first
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=evalfile.obj evalfile.c \
	/first_include=gnv$evalfile.c_first
   $set def prj_root:[-]

[.builtins]evalstring.obj : [.builtins]evalstring.c $(config_h) \
		[.include]filecntl.h $(bashansi_h) $(shell_h) \
		$(jobs_h) $(builtins_h) flags.h input.h $(execute_cmd_h) \
		redir.h $(trap_h) $(bashintl_h) $(y_tab_h) bashhist.h \
		[.builtins]common.h [.builtins]builtext.h
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=evalstring.obj evalstring.c
   $set def prj_root:[-]

[.builtins]getopt.obj : [.builtins]getopt.c, $(config_h) [.include]memalloc.h \
		$(bashintl_h)  $(shell_h) [.builtins]getopt.h
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=getopt.obj getopt.c
   $set def prj_root:[-]

[.builtins]psize.obj : [.builtins]psize.c, $(config_h) bashtypes.h \
	command.h $(general_h) sig.h

[.builtins]psize.exe : [.builtins]psize.obj

# Build the builtins

[.builtins]mkbuiltins.exe : [.builtins]mkbuiltins.obj

lcl_root:[.builtins]mkbuiltins.c : src_root:[.builtins]mkbuiltins.c \
	[.builtins]mkbuiltins.tpu
    $(EVE) $(UNIX_2_VMS) $(MMS$SOURCE)/OUT=$(MMS$TARGET)\
	    /init='f$element(1, ",", "$(MMS$SOURCE_LIST)")'

[.builtins]mkbuiltins.obj : lcl_root:[.builtins]mkbuiltins.c, $(config_h) \
		bashtypes.h [.include]posixstat.h [.include]filecntl.h \
		$(bashansi_h) $(builtins_h)
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[],prj_root:[-]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $(CC)$(cflagsx) \
	/OBJ=mkbuiltins.obj mkbuiltins.c \
	/define=(_USE_STD_STAT=1,$(cdefs1),MODULE_MKBUILTINS=1)
   $set def prj_root:[-]

[.builtins]alias.c : [.builtins]alias.def, [.builtins]mkbuiltins.exe

[.builtins]alias.obj : [.builtins]alias.c
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=alias alias.c
   $set def prj_root:[-]

[.builtins]bind.c : [.builtins]bind.def, [.builtins]mkbuiltins.exe

[.builtins]bind.obj : [.builtins]bind.c, $(config_h) \
		$(readline_readline_h) $(readline_history_h) \
		$(bashintl_h) $(shell_h) \
		bashline.h [.builtins]bashgetopt.h [.builtins]common.h
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=bind.obj bind.c
   $set def prj_root:[-]

[.builtins]break.c : [.builtins]break.def, [.builtins]mkbuiltins.exe

[.builtins]break.obj : [.builtins]break.c $(config_h) $(bashintl_h) \
		$(shell_h) [.builtins]common.h
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=break.obj break.c
   $set def prj_root:[-]

[.builtins]builtin.c : [.builtins]builtin.def, [.builtins]mkbuiltins.exe

[.builtins]builtin.obj : [.builtins]builtin.c $(config_h) $(shell_h) \
		[.builtins]common.h [.builtins]bashgetopt.h
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=builtin.obj builtin.c
   $set def prj_root:[-]

[.builtins]caller.c : [.builtins]caller.def [.builtins]mkbuiltins.exe

[.builtins]caller.obj : [.builtins]caller.c $(config_h) [.include]chartypes.h \
		bashtypes.h $(bashintl_h) $(shell_h) \
		[.builtins]common.h [.builtins]builtext.h \
		[.builtins]bashgetopt.h $(builtins_h)
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=caller.obj caller.c
   $set def prj_root:[-]

[.builtins]cd.c : [.builtins]cd.def, [.builtins]mkbuiltins.exe

[.builtins]cd.obj : [.builtins]cd.c $(config_h) bashtypes.h \
		[.include]posixdir.h [.include]posixstat.h \
		$(bashansi_h) $(bashintl_h) \
		[.lib.tilde]tilde.h $(shell_h) flags.h \
		[.include]maxpath.h [.builtins]common.h \
		[.builtins]bashgetopt.h
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=cd.obj cd.c
   $set def prj_root:[-]

[.builtins]colon.c : [.builtins]colon.def [.builtins]mkbuiltins.exe

[.builtins]colon.obj : [.builtins]colon.c
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=colon.obj colon.c
   $set def prj_root:[-]

[.builtins]command.c : [.builtins]command.def, [.builtins]mkbuiltins.exe

[.builtins]command.obj : [.builtins]command.c $(config_h) \
		$(bashansi_h) $(shell_h) $(execute_cmd_h) flags.h \
		[.builtins]bashgetopt.h [.builtins]common.h
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=command.obj command.c
   $set def prj_root:[-]


[.builtins]complete.c : [.builtins]complete.def [.builtins]mkbuiltins.exe

[.builtins]complete.obj : [.builtins]complete.c $(config_h) \
		bashtypes.h $(bashansi_h) $(bashintl_h) $(shell_h) \
		$(builtins_h) $(pcomplete_h) bashline.h \
		[.builtins]common.h [.builtins]bashgetopt.h \
		$(readline_readline_h)
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=complete.obj complete.c
   $set def prj_root:[-]

[.builtins]declare.c : [.builtins]declare.def, [.builtins]mkbuiltins.exe

[.builtins]declare.obj : [.builtins]declare.c $(config_h) \
		$(bashansi_h) $(bashintl_h) $(shell_h) \
		[.builtins]common.h [.builtins]builtext.h \
		[.builtins]bashgetopt.h
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=declare.obj declare.c
   $set def prj_root:[-]

[.builtins]echo.c : [.builtins]echo.def, [.builtins]mkbuiltins.exe

[.builtins]echo.obj : [.builtins]echo.c $(config_h) \
		$(bashansi_h) $(shell_h) [.builtins]common.h
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=echo.obj echo.c
   $set def prj_root:[-]

[.builtins]enable.c : [.builtins]enable.def, [.builtins]mkbuiltins.exe

[.builtins]enable.obj : [.builtins]enable.c $(config_h) \
	$(bashansi_h) $(bashintl_h) $(shell_h), $(builtins_h), flags.h \
	[.builtins]common.h [.builtins]bashgetopt.h $(pcomplete_h)
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=enable.obj enable.c
   $set def prj_root:[-]

[.builtins]eval.c : [.builtins]eval.def, [.builtins]mkbuiltins.exe

[.builtins]eval.obj : [.builtins]eval.c $(config_h) $(shell_h) \
		[.builtins]bashgetopt.h [.builtins]common.h
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=eval.obj eval.c
   $set def prj_root:[-]

[.builtins]exec.c : [.builtins]exec.def, [.builtins]mkbuiltins.exe

[.builtins]exec.obj : [.builtins]exec.c $(config_h) bashtypes.h \
		[.include]posixstat.h $(bashansi_h) $(bashintl_h) $(shell_h) \
		$(execute_cmd_h) findcmd.h $(jobs_h) flags.h $(trap_h) \
		bashhist.h [.builtins]common.h [.builtins]bashgetopt.h
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=exec.obj exec.c
   $set def prj_root:[-]

[.builtins]exit.c : [.builtins]exit.def, [.builtins]mkbuiltins.exe

[.builtins]exit.obj : [.builtins]exit.c, $(config_h) bashtypes.h \
		$(bashintl_h) $(shell_h) $(jobs_h) [.builtins]common.h \
		[.builtins]builtext.h
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=exit.obj exit.c
   $set def prj_root:[-]

[.builtins]fc.c : [.builtins]fc.def, [.builtins]mkbuiltins.exe

[.builtins]fc.obj : [.builtins]fc.c $(config_h) bashtypes.h \
		[.include]posixstat.h [.include]chartypes.h \
		$(bashansi_h) $(bashintl_h) $(shell_h) \
		flags.h bashhist.h [.include]maxpath.h $(readline_history_h) \
		[.builtins]bashgetopt.h [.builtins]common.h
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=fc.obj fc.c
   $set def prj_root:[-]

[.builtins]fg_bg.c : [.builtins]fg_bg.def, [.builtins]mkbuiltins.exe

[.builtins]fg_bg.obj : [.builtins]fg_bg.c $(config_h) bashtypes.h \
		$(bashintl_h) $(shell_h) $(jobs_h) [.builtins]common.h \
		[.builtins]bashgetopt.h
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=fg_bg.obj fg_bg.c
   $set def prj_root:[-]

[.builtins]getopts.c : [.builtins]getopts.def, [.builtins]mkbuiltins.exe

[.builtins]getopts.obj : [.builtins]getopts.c $(config_h) $(bashansi_h) \
		$(shell_h) [.builtins]common.h [.builtins]bashgetopt.h \
		[.builtins]getopt.h
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=getopts.obj getopts.c
   $set def prj_root:[-]

[.builtins]hash.c : [.builtins]hash.def, [.builtins]mkbuiltins.exe

[.builtins]hash.obj : [.builtins]hash.c $(config_h) bashtypes.h \
		$(bashansi_h) $(bashintl_h) $(shell_h) \
		$(builtins_h) flags.h findcmd.h $(hashcmd_h) \
		[.builtins]common.h, \
		[.builtins]bashgetopt.h
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=hash.obj hash.c
   $set def prj_root:[-]

[.builtins]help.c : [.builtins]help.def, [.builtins]mkbuiltins.exe

[.builtins]help.obj : [.builtins]help.c, $(config_h) $(bashintl_h) \
		$(shell_h), $(builtins_h) pathexp.h [.builtins]common.h \
		[.builtins]bashgetopt.h [.lib.glob]strmatch.h [.lib.glob]glob.h
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=help.obj help.c
   $set def prj_root:[-]

[.builtins]history.c : [.builtins]history.def, [.builtins]mkbuiltins.exe

[.builtins]history.obj : [.builtins]history.c, $(config_h) bashtypes.h \
		[.include]posixstat.h [.include]filecntl.h $(bashansi_h) \
		$(bashintl_h) $(shell_h) bashhist.h $(readline_history_h) \
		[.builtins]bashgetopt.h [.builtins]common.h
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=history.obj history.c
   $set def prj_root:[-]

[.builtins]inlib.c : [.builtins]inlib.def [.builtins]mkbuiltins.exe

[.builtins]inlib.obj : [.builtins]inlib.c $(config_h) $(shell_h)
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=inlib.obj inlib.c
   $set def prj_root:[-]

[.builtins]jobs.c : [.builtins]jobs.def [.builtins]mkbuiltins.exe

[.builtins]jobs.obj : [.builtins]jobs.c, $(config_h) bashtypes.h \
		$(bashansi_h) $(bashintl_h) $(shell_h) $(jobs_h) \
		$(execute_cmd_h) [.builtins]bashgetopt.h [.builtins]common.h
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=jobs.obj jobs.c
   $set def prj_root:[-]

[.builtins]kill.c : [.builtins]kill.def [.builtins]mkbuiltins.exe

[.builtins]kill.obj : [.builtins]kill.c $(config_h) $(bashansi_h) \
		$(bashintl_h) $(shell_h) $(trap_h) $(jobs_h) \
		[.builtins]common.h
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=kill.obj kill.c
   $set def prj_root:[-]

[.builtins]let.c : [.builtins]let.def [.builtins]mkbuiltins.exe

[.builtins]let.obj : [.builtins]let.c $(config_h) $(bashintl_h) \
		$(shell_h) [.builtins]common.h
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=let.obj let.c
   $set def prj_root:[-]

[.builtins]mapfile.c : [.builtins]mapfile.def [.builtins]mkbuiltins.exe

[.builtins]mapfile.obj : [.builtins]mapfile.c $(config_h) $(builtins_h) \
		[.include]posixstat.h $(bashansi_h) $(bashintl_h) \
		$(readline_rlshell_h) [.builtins]common.h \
		[.builtins]bashgetopt.h
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=mapfile.obj mapfile.c
   $set def prj_root:[-]

[.builtins]printf.c : [.builtins]printf.def [.builtins]mkbuiltins.exe

[.builtins]printf.obj : [.builtins]printf.c $(config_h) bashtypes.h \
		[.include]chartypes.h [.include]posixtime.h $(bashansi_h) \
		$(bashintl_h) $(shell_h) [.include]shmbutil.h \
		[.builtins]bashgetopt.h [.builtins]common.h
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=printf.obj printf.c
   $set def prj_root:[-]

[.builtins]pushd.c : [.builtins]pushd.def [.builtins]mkbuiltins.exe

[.builtins]pushd.obj : [.builtins]pushd.c $(config_h) $(bashansi_h) \
		$(bashintl_h) [.lib.tilde]tilde.h $(shell_h) \
		[.include]maxpath.h [.builtins]common.h [.builtins]builtext.h \
		$(builtins_h)
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=pushd.obj pushd.c
   $set def prj_root:[-]

[.builtins]read.c : [.builtins]read.def [.builtins]mkbuiltins.exe

[.builtins]read.obj : [.builtins]read.c, $(config_h) bashtypes.h \
		[.include]posixstat.h $(bashansi_h) $(bashintl_h) $(shell_h) \
		[.builtins]common.h [.builtins]bashgetopt.h \
		[.include]shtty.h bashline.h \
		$(readline_readline_h) input.h $(termios_h)
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=read.obj read.c
   $set def prj_root:[-]

[.builtins]return.c : [.builtins]return.def [.builtins]mkbuiltins.exe

[.builtins]return.obj : [.builtins]return.c, $(config_h) \
		$(bashintl_h) $(shell_h) [.builtins]common.h \
		[.builtins]bashgetopt.h
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=return.obj return.c
   $set def prj_root:[-]

[.builtins]set.c : [.builtins]set.def [.builtins]mkbuiltins.exe

[.builtins]set.obj : [.builtins]set.c, $(config_h) $(bashansi_h) \
		$(bashintl_h) $(shell_h), flags.h, \
		[.builtins]common.h [.builtins]bashgetopt.h input.h bashline.h \
		$(readline_readline_h) bashhist.h
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=set.obj set.c
   $set def prj_root:[-]

[.builtins]setattr.c : [.builtins]setattr.def [.builtins]mkbuiltins.exe

[.builtins]setattr.obj : [.builtins]setattr.c, $(config_h) \
		$(bashansi_h) $(bashintl_h) $(shell_h) [.builtins]common.h \
		[.builtins]bashgetopt.h
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=setattr.obj setattr.c
   $set def prj_root:[-]

[.builtins]shift.c : [.builtins]shift.def [.builtins]mkbuiltins.exe,

[.builtins]shift.obj : [.builtins]shift.c, $(config_h) $(bashansi_h) \
		$(bashintl_h) $(shell_h) [.builtins]common.h
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=shift.obj shift.c
   $set def prj_root:[-]

[.builtins]shopt.c : [.builtins]shopt.def [.builtins]mkbuiltins.exe

[.builtins]shopt.obj : [.builtins]shopt.c $(config_h) version.h \
		$(bashintl_h) $(shell_h) flags.h [.builtins]common.h \
		[.builtins]bashgetopt.h bashhist.h
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=shopt.obj shopt.c
   $set def prj_root:[-]

[.builtins]source.c : [.builtins]source.def [.builtins]mkbuiltins.exe

[.builtins]source.obj : [.builtins]source.c, $(config_h) bashtypes.h \
		[.include]posixstat.h [.include]filecntl.h $(bashansi_h) \
		$(bashintl_h) $(shell_h) flags.h findcmd.h \
		[.builtins]common.h [.builtins]bashgetopt.h $(trap_h)
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=source.obj source.c
   $set def prj_root:[-]

[.builtins]suspend.c : [.builtins]suspend.def [.builtins]mkbuiltins.exe

[.builtins]suspend.obj : [.builtins]suspend.c, $(config_h) \
		bashtypes.h $(bashintl_h) $(shell_h) $(jobs_h) \
		[.builtins]common.h [.builtins]bashgetopt.h
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=suspend.obj suspend.c
   $set def prj_root:[-]

[.builtins]test.c : [.builtins]test.def [.builtins]mkbuiltins.exe

[.builtins]test.obj : [.builtins]test.c, $(config_h) $(bashansi_h) \
		$(bashintl_h) $(shell_h) test.h [.builtins]common.h
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=test.obj test.c
   $set def prj_root:[-]

[.builtins]times.c : [.builtins]times.def [.builtins]mkbuiltins.exe

[.builtins]times.obj : [.builtins]times.c, $(config_h) bashtypes.h \
		$(shell_h) [.include]posixtime.h [.builtins]common.h
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=times.obj times.c
   $set def prj_root:[-]

[.builtins]trap.c : [.builtins]trap.def [.builtins]mkbuiltins.exe

[.builtins]trap.obj : [.builtins]trap.c, $(config_h) bashtypes.h \
		$(bashansi_h) $(shell_h) $(trap_h) [.builtins]common.h \
		[.builtins]bashgetopt.h
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=trap.obj trap.c
   $set def prj_root:[-]

[.builtins]type.c : [.builtins]type.def [.builtins]mkbuiltins.exe

[.builtins]type.obj : [.builtins]type.c, $(config_h) [.include]posixstat.h \
		$(bashansi_h) $(bashintl_h) $(shell_h) \
		findcmd.h $(hashcmd_h) $(alias_h) [.builtins]common.h \
		[.builtins]bashgetopt.h
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=type.obj type.c
   $set def prj_root:[-]



[.builtins]ulimit.c : [.builtins]ulimit.def [.builtins]mkbuiltins.exe

[.builtins]gnv$ulimit.c_first : [.builtins]gnv_ulimit.c_first
    $type $(MMS$SOURCE) /output=$(MMS$TARGET)

[.builtins]ulimit.obj : [.builtins]ulimit.c, $(config_h) bashtypes.h \
		$(bashintl_h) $(shell_h) [.builtins]common.h \
		[.builtins]bashgetopt.h [.builtins]pipesize.h \
		[.builtins]gnv$ulimit.c_first
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=ulimit.obj ulimit.c \
	/first_include=gnv$ulimit.c_first
   $set def prj_root:[-]

[.builtins]umask.c : [.builtins]umask.def [.builtins]mkbuiltins.exe

[.builtins]umask.obj : [.builtins]umask.c $(config_h) bashtypes.h \
		[.include]filecntl.h [.include]chartypes.h $(bashintl_h) \
		$(shell_h) [.include]posixstat.h [.builtins]common.h \
		[.builtins]bashgetopt.h
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=umask.obj umask.c
   $set def prj_root:[-]

lcl_root:[.builtins]wait.def : src_root:[.builtins]wait.def [.builtins]wait_def.tpu
    $(EVE) $(UNIX_2_VMS) $(MMS$SOURCE)/OUT=$(MMS$TARGET)\
	    /init='f$element(1, ",", "$(MMS$SOURCE_LIST)")'

[.builtins]wait.c : lcl_root:[.builtins]wait.def [.builtins]mkbuiltins.exe

[.builtins]wait.obj : [.builtins]wait.c, $(config_h) bashtypes.h \
		[.include]chartypes.h $(bashansi_h) $(shell_h) $(jobs_h) \
		[.builtins]common.h [.builtins]bashgetopt.h
   $set def prj_root:[.builtins]
   $define/user cxx$user_include prj_root:[]
   $define/user cxx$system_include prj_root:[-],PRJ_ROOT:[-.include]
   $define/user readline prj_root:[-.lib.readline]
   $define/user glob prj_root:[-.lib.glob]
   $define/user tilde prj_root:[-.lib.tilde]
   $(CC)$(CFLAGS)/OBJ=wait.obj wait.c
   $set def prj_root:[-]

alias.obj : alias.c $(config_h) [.include]chartypes.h $(bashansi_h), command.h \
		$(general_h) externs.h $(alias_h) $(pcomplete_h)

array.obj : array.c $(config_h) $(bashansi_h) $(shell_h) array.h \
		[.builtins]common.h

arrayfunc.obj : arrayfunc.c $(config_h) $(bashintl_h) $(shell_h) pathexp.h \
		[.include]shmbutil.h [.builtins]common.h

assoc.obj : assoc.c $(config_h) $(bashansi_h) $(shell_h) array.h $(assoc_h) \
		[.builtins]common.h

bashhist.obj : bashhist.c, $(config_h) bashtypes.h $(bashansi_h) \
		[.include]posixstat.h [.include]filecntl.h $(bashintl_h) \
		$(shell_h) flags.h input.h $(parser_h) pathexp.h \
		bashhist.h [.builtins]common.h $(readline_history_h) \
		[.lib.glob]glob.h [.lib.glob]strmatch.h bashline.h

bashline.obj : bashline.c $(config_h) bashtypes.h \
		[.include]posixstat.h [.builtins]builtext.h \
		[.include]chartypes.h $(bashansi_h) $(bashintl_h) \
		$(shell_h) input.h $(builtins_h) \
		bashhist.h bashline.h $(execute_cmd_h) findcmd.h pathexp.h \
		[.builtins]common.h, [.lib.readline]rlconf.h \
		$(readline_readline_h) $(readline_history_h) \
		[.lib.glob]glob.h $(alias_h) $(pcomplete_h)

bracecomp.obj : bracecomp.c $(config_h) $(bashansi_h) [.include]shmbutil.h \
		$(shell_h)

braces.obj : braces.c, $(config_h) $(bashansi_h) $(shell_h) $(general_h) \
		[.include]shmbutil.h [.include]chartypes.h

copy_cmd.obj : copy_cmd.c, $(config_h) bashtypes.h $(shell_h)

dispose_cmd.obj : dispose_cmd.c, $(config_h) bashtypes.h $(bashansi_h) \
		$(shell_h)

lcl_root:error.c : src_root:error.c error_c.tpu
    $(EVE) $(UNIX_2_VMS) $(MMS$SOURCE)/OUT=$(MMS$TARGET)\
	    /init='f$element(1, ",", "$(MMS$SOURCE_LIST)")'

lcl_root:error.h : src_root:error.h error_h.tpu
    $(EVE) $(UNIX_2_VMS) $(MMS$SOURCE)/OUT=$(MMS$TARGET)\
	    /init='f$element(1, ",", "$(MMS$SOURCE_LIST)")'

error.obj : lcl_root:error.c $(config_h) bashtypes.h $(bashansi_h) $(bashintl_h) \
		 $(shell_h) flags.h input.h bashhist.h

eval.obj : eval.c $(config_h) $(bashansi_h) $(bashintl_h) $(shell_h) flags.h \
		$(trap_h) [.builtins]common.h input.h $(execute_cmd_h) \
		bashhist.h

lcl_root:execute_cmd.c : src_root:execute_cmd.c execute_cmd_c.tpu
    $(EVE) $(UNIX_2_VMS) $(MMS$SOURCE)/OUT=$(MMS$TARGET)\
	    /init='f$element(1, ",", "$(MMS$SOURCE_LIST)")'

execute_cmd.obj : lcl_root:execute_cmd.c $(config_h) \
		[.include]chartypes.h bashtypes.h \
		[.include]filecntl.h [.include]posixstat.h \
		[.include]posixtime.h $(bashansi_h) \
		$(bashintl_h) [.include]memalloc.h $(shell_h) \
		$(y_tab_h) flags.h $(builtins_h) hashlib.h \
		$(jobs_h) $(execute_cmd_h) findcmd.h \
		redir.h $(trap_h) pathexp.h $(hashcmd_h) test.h \
		[.builtins]common.h [.builtins]builtext.h \
		[.lib.glob]strmatch.h [.lib.tilde]tilde.h \
		input.h $(alias_h) bashhist.h
   $define/user glob prj_root:[.lib.glob]
   $define/user tilde prj_root:[.lib.tilde]
   $define/user cxx$system_include prj_root:[]
   $define/user cxx$user_include prj_root:[],prj_root:[.include],\
	prj_root:[.lib.readline],prj_root:[.lib.intl],prj_root:[.lib.sh]
   $(CC)$(cflagsx)/OBJ=$(MMS$TARGET) $(MMS$SOURCE) \
	/define=($(cdefs1),MODULE_EXECUTE_CMD=1)

expr.obj : expr.c $(config_h) $(bashansi_h) [.include]chartypes.h \
		$(bashintl_h)  $(shell_h)

#lcl_root:externs.h : src_root:externs.h externs_h.tpu
#    $(EVE) $(UNIX_2_VMS) $(MMS$SOURCE)/OUT=$(MMS$TARGET)\
#	    /init='f$element(1, ",", "$(MMS$SOURCE_LIST)")'

# moved to vms_lstat_hack.h
#gnv$findcmd.c_first : gnv_findcmd.c_first
#    $type $(MMS$SOURCE)/output=$(MMS$TARGET)

lcl_root:findcmd.c : src_root:findcmd.c findcmd.tpu
    $(EVE) $(UNIX_2_VMS) $(MMS$SOURCE)/OUT=$(MMS$TARGET)\
	    /init='f$element(1, ",", "$(MMS$SOURCE_LIST)")'

findcmd.obj : lcl_root:findcmd.c $(config_h) \
		[.include]chartypes.h bashtypes.h \
		[.include]filecntl.h [.include]posixstat.h $(bashansi_h) \
		[.include]memalloc.h $(shell_h) flags.h hashlib.h \
		pathexp.h $(hashcmd_h) findcmd.h
   $define/user readline prj_root:[.lib.readline]
   $define/user glob prj_root:[.lib.glob]
   $define/user tilde prj_root:[.lib.tilde]
   $define/user cxx$system_include prj_root:[],prj_root:[.include]
   $define/user cxx$user_include prj_root:[],prj_root:[.include],\
	prj_root:[.lib.readline],prj_root:[.lib.intl],prj_root:[.lib.sh],\
	prj_root:[.lib.glob]
   $(CC)$(CFLAGS)/OBJ=$(MMS$TARGET) \
	$(MMS$SOURCE)

flags.obj : flags.c $(config_h) $(shell_h) flags.h bashhist.h

lcl_root:general.c : src_root:general.c []general.tpu
    $(EVE) $(UNIX_2_VMS) $(MMS$SOURCE)/OUT=$(MMS$TARGET)\
	    /init='f$element(1, ",", "$(MMS$SOURCE_LIST)")'

general.obj : lcl_root:general.c $(config_h) bashtypes.h [.include]posixstat.h \
		[.include]filecntl.h $(bashansi_h) [.include]chartypes.h \
		$(bashintl_h) $(shell_h) test.h [.lib.tilde]tilde.h

hashcmd.obj : hashcmd.c $(config_h) bashtypes.h [.include]posixstat.h \
		$(bashansi_h) $(shell_h) findcmd.h $(hashcmd_h)

hashlib.obj : hashlib.c $(config_h) $(bashansi_h) $(shell_h) hashlib.h

input.obj : input.c $(config_h) bashtypes.h [.include]filecntl.h \
		[.include]posixstat.h $(bashansi_h) $(bashintl_h) \
		$(general_h) input.h lcl_root:error.h externs.h quit.h

lcl_root:nojobs.c : src_root:nojobs.c nojobs_c.tpu
    $(EVE) $(UNIX_2_VMS) $(MMS$SOURCE)/OUT=$(MMS$TARGET)\
	    /init='f$element(1, ",", "$(MMS$SOURCE_LIST)")'

nojobs.obj : lcl_root:nojobs.c $(config_h) bashtypes.h $(trap_h) \
		[.include]posixtime.h \
		[.include]filecntl.h input.h [.include]shtty.h \
		$(bashansi_h) $(bashintl_h) $(shell_h) $(jobs_h) \
		$(execute_cmd_h) flags.h [.builtins]builtext.h \
		[.builtins]common.h $(termios_h)

list.obj : list.c $(config_h) $(shell_h)

locale.obj : locale.c $(config_h) bashtypes.h $(bashintl_h) $(bashansi_h) \
		[.include]chartypes.h $(shell_h) input.h

gnv$mailcheck.c_first : gnv_mailcheck.c_first
    $type $(MMS$SOURCE) /output=$(MMS$TARGET)

mailcheck.obj : mailcheck.c $(config_h) bashtypes.h [.include]posixstat.h \
		[.include]posixtime.h $(bashansi_h) $(bashintl_h) $(shell_h) \
		$(execute_cmd_h) mailcheck.h [.lib.tilde]tilde.h \
		gnv$mailcheck.c_first
   $define/user readline prj_root:[.lib.readline]
   $define/user tilde prj_root:[.lib.tilde]
   $define/user glob prj_root:[.lib.glob]
   $define/user cxx$system_include prj_root:[]
   $define/user cxx$user_include prj_root:[],prj_root:[.include],\
	prj_root:[.lib.readline],prj_root:[.lib.intl],prj_root:[.lib.sh],\
	prj_root:[.lib.glob]
   $(CC)$(cflags)/first_include=gnv$mailcheck.c_first \
	/OBJ=mailcheck.obj mailcheck.c

make_cmd.obj : make_cmd.c $(config_h) bashtypes.h [.include]filecntl.h \
		$(bashansi_h) $(bashintl_h) $(parser_h) syntax.h \
		command.h $(general_h) lcl_root:error.h flags.h make_cmd.h \
		dispose_cmd.h $(variables_h) subst.h input.h \
		[.include]ocache.h externs.h $(jobs_h) \
		[.include]shmbutil.h

pathexp.obj : pathexp.c $(config_h) bashtypes.h $(bashansi_h) \
		$(shell_h) pathexp.h flags.h [.include]shmbutil.h \
		$(bashintl_h) [.lib.glob]strmatch.h [.lib.glob]glob.h

pcomplete.obj : pcomplete.c $(config_h) bashtypes.h [.include]posixstat.h \
		$(bashansi_h) $(bashintl_h) $(shell_h) $(pcomplete_h) \
		$(alias_h) bashline.h $(execute_cmd_h) pathexp.h $(jobs_h) \
		$(trap_h) $(builtins_h) [.builtins]common.h [.lib.glob]glob.h \
		[.lib.glob]strmatch.h [.lib.readline]rlconf.h \
		$(readline_readline_h) $(readline_history_h)

pcomplib.obj : pcomplib.c $(config_h) $(bashintl_h) $(shell_h) $(pcomplete_h)

print_cmd.obj : print_cmd.c $(config_h) $(bashansi_h) $(bashintl_h) \
		$(shell_h) flags.h $(y_tab_h) [.include]shmbutil.h \
		[.builtins]common.h
   $define/user glob prj_root:[.lib.glob]
   $define/user tilde prj_root:[.lib.tilde]
   $define/user cxx$system_include prj_root:[]
   $define/user cxx$user_include prj_root:[],prj_root:[.include],\
	prj_root:[.lib.readline],prj_root:[.lib.intl],prj_root:[.lib.sh]
   $(CC)$(cflagsx)/OBJ=$(MMS$TARGET) $(MMS$SOURCE) \
	/define=($(cdefs1),MODULE_PRINT_CMD=1)

redir.obj : redir.c $(config_h) bashtypes.h [.include]filecntl.h \
		[.include]posixstat.h $(bashansi_h) $(bashintl_h) \
		[.include]memalloc.h $(shell_h) flags.h \
		$(execute_cmd_h) redir.h input.h [.builtins]pipesize.h

gnv$shell.c_first : gnv_shell.c_first
    $type $(MMS$SOURCE) /output=$(MMS$TARGET)

lcl_root:shell.c : src_root:shell.c []shell_c.tpu
    $(EVE) $(UNIX_2_VMS) $(MMS$SOURCE)/OUT=$(MMS$TARGET)\
	    /init='f$element(1, ",", "$(MMS$SOURCE_LIST)")'

shell.obj : lcl_root:shell.c $(config_h) bashtypes.h [.include]posixstat.h \
		[.include]posixtime.h $(bashansi_h) \
		[.include]filecntl.h $(bashintl_h) $(shell_h) \
		flags.h $(trap_h) $(jobs_h) input.h $(execute_cmd_h) \
		findcmd.h [.lib.malloc]shmalloc.h bashhist.h \
		$(readline_history_h) bashline.h \
		[.lib.tilde]tilde.h [.lib.glob]strmatch.h \
		$(gnv_shell_c_first)
   $define/user readline prj_root:[.lib.readline]
   $define/user tilde prj_root:[.lib.tilde]
   $define/user glob prj_root:[.lib.glob]
   $define/user cxx$system_include prj_root:[]
   $define/user cxx$user_include prj_root:[],prj_root:[.include],\
	prj_root:[.lib.readline],prj_root:[.lib.intl],prj_root:[.lib.sh],\
	prj_root:[.lib.glob]
   $(CC)$(cflags)/first_include=gnv$shell.c_first \
	/OBJ=shell.obj lcl_root:shell.c

#lcl_root:sig.h : src_root:sig.h sig_h.tpu
#    $(EVE) $(UNIX_2_VMS) $(MMS$SOURCE)/OUT=$(MMS$TARGET)\
#	    /init='f$element(1, ",", "$(MMS$SOURCE_LIST)")'

sig.obj : sig.c $(config_h) bashtypes.h $(bashintl_h) $(shell_h) $(jobs_h) \
		siglist.h sig.h $(trap_h) [.builtins]common.h \
		bashline.h bashhist.h

siglist.obj : siglist.c $(config_h) bashtypes.h siglist.h $(trap_h) \
		$(bashintl_h) $(xmalloc_h)

stringlib.obj : stringlib.c $(config_h) bashtypes.h $(bashansi_h) \
		[.include]chartypes.h $(shell_h) pathexp.h [.lib.glob]glob.h \
		[.lib.glob]strmatch.h

subst.obj : subst.c $(config_h) bashtypes.h \
		[.include]chartypes.h \
		$(bashansi_h) [.include]posixstat.h $(bashintl_h) \
		$(shell_h) $(parser_h) flags.h $(jobs_h) $(execute_cmd_h) \
		[.include]filecntl.h $(trap_h) pathexp.h mailcheck.h \
		[.include]shmbutil.h [.include]typemax.h \
		[.builtins]getopt.h [.builtins]common.h \
		[.builtins]builtext.h [.lib.tilde]tilde.h \
		[.lib.glob]strmatch.h

syntax.obj : syntax.c $(config_h) syntax.h

test.obj : test.c $(config_h) \
		bashtypes.h [.include]posixstat.h \
		[.include]filecntl.h $(bashintl_h) $(shell_h) \
		pathexp.h test.h [.builtins]common.h [.lib.glob]strmatch.h

trap.obj : trap.c $(config_h) bashtypes.h $(bashansi_h) $(bashintl_h) \
		$(trap_h) $(shell_h) flags.h input.h $(jobs_h) signames.h \
		$(builtins_h) [.builtins]common.h [.builtins]builtext.h

unwind_prot.obj : unwind_prot.c $(config_h) bashtypes.h $(bashansi_h) \
		command.h $(general_h) unwind_prot.h quit.h sig.h \
		lcl_root:error.h

lcl_root:variables.c : src_root:variables.c variables.tpu version.h
    $(EVE) $(UNIX_2_VMS) $(MMS$SOURCE)/OUT=$(MMS$TARGET)\
	    /init='f$element(1, ",", "$(MMS$SOURCE_LIST)")'

variables.obj : lcl_root:variables.c $(config_h) bashtypes.h \
		[.include]posixstat.h \
		[.include]posixtime.h [.include]chartypes.h \
		$(bashansi_h) $(bashintl_h) $(shell_h) flags.h \
		$(execute_cmd_h) findcmd.h mailcheck.h input.h $(hashcmd_h) \
		pathexp.h $(alias_h) [.builtins]getopt.h [.builtins]common.h \
		bashline.h $(readline_readline_h) [.lib.tilde]tilde.h \
		bashhist.h $(readline_history_h) $(pcomplete_h)

lcl_root:y_tab.c : src_root:$(y_tab_c_in) y_tab.tpu
    $(EVE) $(UNIX_2_VMS) $(MMS$SOURCE)/OUT=$(MMS$TARGET)\
	    /init='f$element(1, ",", "$(MMS$SOURCE_LIST)")'

y_tab.obj : lcl_root:y_tab.c $(config_h) bashtypes.h $(bashansi.h)\
		[.include]filecntl.h [.include]chartypes.h \
		[.include]memalloc.h $(bashintl_h) $(shell_h) \
		$(trap_h) flags.h parser.h mailcheck.h test.h \
		builtins.h [.builtins]common.h [.builtins]builtext.h \
		[.include]shmbutil.h bashline.h $(readline_readline_h) \
		bashhist.h $(readline_history_h) jobs.h alias.h \
		[.include]maxpath.h

vms_get_foreign_cmd.obj : vms_get_foreign_cmd.c

vms_term.obj : vms_term.c vms_term.h

vms_terminal_io.obj : vms_terminal_io.c vms_terminal_io.h

decw_showdisplay.obj : decw_showdisplay.c

#y.tab.obj : y.tab.c bashtypes.h $(bashansi_h) $(shell_h) flags.h \
#	input.h bashhist.h $(jobs_h) $(alias_h) [.include]maxpath.h \
#	[.builtins]common.h [.builtins]builtext.h $(trap_h)

vms_crtl_init.obj : vms_crtl_init.c
	$(CC)$(cflags) \
	/object=$(MMS$TARGET) $(MMS$SOURCE)

vms_crtl_values.obj : vms_crtl_values.c
	$(CC)$(cflags) \
	/object=$(MMS$TARGET) $(MMS$SOURCE)

gnv$bash.exe : $(bash_objs) \
	   [.builtins]builtins.obj,\
	   [.builtins]common.obj,\
           vms_crtl_init.obj,\
           vms_crtl_values.obj \
           libbuiltins.olb \
           libreadline.olb \
           libglob.olb \
	   libintl.olb \
	   [.lib.regex]regex.obj \
	   libsh.olb
   $(LINK)$(LFLAGS)/exec=$(MMS$TARGET)/DSF=$(MMS$TARGET_NAME) \
	/MAP=$(MMS$TARGET_NAME) \
	 $(bash_objs),vms_crtl_init.obj,vms_crtl_values.obj,\
	[.builtins]builtins.obj,\
	[.builtins]common.obj,\
        []libglob.olb/lib, \
	[]libintl.olb/lib, \
        []libreadline.olb/lib, \
	[]libbuiltins.olb/lib,\
	[]libsh.olb/lib,\
	[.lib.regex]regex.obj

bashdebug.exe : $(bash_objs), \
	   [.builtins]builtins.obj,\
	   [.builtins]common.obj,\
           vms_crtl_init.obj,\
           vms_crtl_values.obj,\
           libbuiltins.olb \
           libreadline.olb \
           libglob.olb \
	   libintl.olb \
	   libsh.olb
   $(LINK)$(LFLAGS)/debug/exec=$(MMS$TARGET)/DSF=$(MMS$TARGET_NAME) \
	/MAP=$(MMS$TARGET_NAME) \
	 $(bash_objs),vms_crtl_init.obj,vms_crtl_values.obj,\
	[.builtins]builtins.obj,\
	[.builtins]common.obj,\
        []libglob.olb/lib, \
	[]libintl.olb/lib, \
        []libreadline.olb/lib, \
	[]libbuiltins.olb/lib,\
	[]libsh.olb/lib,\
	[.lib.regex]regex.obj

# Always provide these two cleanup targets.
# In this case DCL will do the cleanup.
#==============================================
CLEAN :
   $ @prj_root:[bash]clean_bash.com

REALCLEAN :
   $ @prj_root:[bash]clean_bash.com REALCLEAN
