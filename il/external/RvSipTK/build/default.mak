OS_NAME := $(shell uname)
OS_VER  := $(shell uname -r)

DEFAULT_MAK_DIR := common/examples

ifneq ($(RV_PSOSAPPL_COMPILATION),)
DEFAULT_MAK_DIR := ../../../../common/examples
endif

# -------------------------------- SunOS ------------------------------------------------
ifeq ($(OS_NAME), SunOS)
IS_X86_PROCESSOR := $(shell uname -i | grep 86)
ifneq ($(IS_X86_PROCESSOR), )
$(warning This is Solaris on x86 platform )
include $(DEFAULT_MAK_DIR)/solaris.x86.default.mak
else
include $(DEFAULT_MAK_DIR)/solaris.default.mak
endif
TARGET_OS_VERSION := $(shell echo $(OS_VER) | sed -n -e "s/5\.\([0-9]\)/\1/p")
COMPILER_TOOL_VERSION := $(shell gcc -v 2>&1 | sed -n -e "s/gcc version \([0-9]\.[0-9]\).*/\1/p")
ifeq ($(TARGET_OS_VERSION), 6)
TARGET_OS_VERSION := 2.6
endif
ifeq ($(SUNWS), on)
COMPILER_TOOL := sun
COMPILER_TOOL_VERSION := 5
endif
endif

# -------------------------------- Linux ------------------------------------------------
# Figure out the exact type of Linux system
# User may specify LINUX_TYPE variable to set this type
# If this variable isn't set then under RedHat or SUSE we try to figure it 
# out by ourselfes (by testing existence of /etc/SuSE-release file)
# Under MontaVista or ucLinux we also check existence of (deprecated) MVISTA or UCLINUX
# variables and set LINUX_TYPE accordingly

ifeq ($(OS_NAME), Linux)

SUPPORTED_LINUX_TYPES = MVISTA UCLINUX SUSE REDHAT

# LINUX_TYPE wasn't defined - try to figure it out
ifndef LINUX_TYPE

ifeq ($(MVISTA), on)
LINUX_TYPE=MVISTA
else

ifeq ($(UCLINUX), on)
LINUX_TYPE=UCLINUX
else
LINUX_TYPE := $(shell if ( test -e /etc/SuSE-release ) ; then echo SUSE ; else echo REDHAT ; fi)
endif # UCLINUX

endif # MVISTA

endif # LINUX_TYPE

ifeq ($(strip $(findstring $(LINUX_TYPE),$(SUPPORTED_LINUX_TYPES))) ,)
$(error Unknown or not set LINUX_TYPE: $(LINUX_TYPE), should be one of $(SUPPORTED_LINUX_TYPES))
endif


ifeq ($(LINUX_TYPE),SUSE)
include $(DEFAULT_MAK_DIR)/suse.default.mak
TARGET_OS_VERSION := SUSE_$(shell grep "VERSION = " /etc/SuSE-release | awk '{print $$3}' | awk '{printf("%1.0f", $$1)}')
endif

ifeq ($(LINUX_TYPE),REDHAT)
# ----------------------------------------------------------------------
#
# Set the KERNEL_INCLUDE variable, if it was not given through the environment variable
# or as a command line argument to make command
# First define the RPM name with kernel development files 
# and then define the name of the folder that ends with 'include'
KERNEL_RPM := $(shell rpm -q -a | grep -m 1 kernel-devel)
ifneq ("$(KERNEL_RPM)","")
KERNEL_INCLUDE := $(shell rpm -q -l $(KERNEL_RPM) | grep  '/include' -m 1)
$(warning KERNEL_INCLUDE is $(KERNEL_INCLUDE) )
endif

PLATFORM_PROCESSOR := $(shell uname -a | awk '{print $$12}')
ifeq ($(linux32), on)
$(warning Building for 32 bit architecture )
include $(DEFAULT_MAK_DIR)/redhat.default.mak
else 
ifeq ($(linux64), on)
$(warning Building for 64 bit architecture )
include $(DEFAULT_MAK_DIR)/redhat64.default.mak
else 
ifeq ($(PLATFORM_PROCESSOR), ia64)
$(warning Building for 64 bit architecture )
include $(DEFAULT_MAK_DIR)/redhat64.default.mak
else 
ifeq ($(PLATFORM_PROCESSOR), x86_64)
$(warning Building for 64 bit architecture )
include $(DEFAULT_MAK_DIR)/redhat64.default.mak
else
$(warning Building for 32 bit architecture )
include $(DEFAULT_MAK_DIR)/redhat.default.mak
endif #x86_64
endif #ia64
endif #linux64
endif #linux32


CFLAGS := $(CFLAGS) -I$(KERNEL_INCLUDE)

TARGET_OS_VERSION := redhat-$(shell cat /etc/redhat-release | awk '{print $$5}')

endif

ifeq ($(LINUX_TYPE),MVISTA)
include $(DEFAULT_MAK_DIR)/mvista.default.mak
endif

ifeq ($(LINUX_TYPE),UCLINUX)
include $(DEFAULT_MAK_DIR)/uclinux.default.mak
endif


COMPILER_TOOL_VERSION := $(shell gcc -v 2>&1 | sed -n -e "s/gcc version \([0-9]\.[0-9]\).*/\1/p")

ifeq ($(icc),on)
COMPILER_TOOL := icc
endif # ifeq ($(icc),on)


endif

# -------------------------------- UnixWare ---------------------------------------------
ifeq ($(OS_NAME), UnixWare)
include $(DEFAULT_MAK_DIR)/unixware.default.mak
endif

# -------------------------------- OSF1 ------------------------------------------------
ifeq ($(OS_NAME), OSF1)
include $(DEFAULT_MAK_DIR)/tru64.default.mak
endif

# -------------------------------- HP-UX ------------------------------------------------
ifeq ($(OS_NAME), HP-UX)
include $(DEFAULT_MAK_DIR)/hpux.default.mak
endif

# -------------------------------- MAC OS -----------------------------------------------
ifeq ($(OS_NAME), Darwin)
include $(DEFAULT_MAK_DIR)/mac.default.mak
endif
# -------------------------------- FreeBSD -----------------------------------------------
ifeq ($(OS_NAME), FreeBSD)
include $(DEFAULT_MAK_DIR)/freebsd.default.mak
endif
# -------------------------------- NetBSD -----------------------------------------------
ifeq ($(OS_NAME), NetBSD)
include $(DEFAULT_MAK_DIR)/netbsd.default.mak
endif


# --------------- CygWin: VxWorks, Symbian, Mopi, Nucleus, pSOS, OSE, Win32 -------------

ifeq ($(findstring CYGWIN,$(OS_NAME)), CYGWIN)
ifneq ($(OSA_ROOT), )
include $(DEFAULT_MAK_DIR)/osa.ads.arm.default.mak # ARM
endif

# VxWorks
ifneq ($(TORNADO_VERSION), )
TARGET_OS_VERSION := $(TORNADO_VERSION)
ifneq ($(DIABLIB), )
include $(DEFAULT_MAK_DIR)/vxworks.diab.default.mak
else
include $(DEFAULT_MAK_DIR)/vxworks.gnu.default.mak
endif
endif

# Symbian
ifneq ($(SYMBIAN_ROOT), )
include $(DEFAULT_MAK_DIR)/symbian.default.mak
ifeq ($(MWCC), on)
COMPILER_TOOL := metrowerks
COMPILER_TOOL_VERSION := 2.4.7
endif
endif

# MOPI
# note that MOPI section MUST come before Nucleus section, since MOPI sets up nucleus environment too
ifneq ($(MOPI_ROOT), )
include $(DEFAULT_MAK_DIR)/mopi.ads.arm.default.mak
endif

# Nucleus
ifneq ($(NUCLEUS_ROOT), )
ifneq ($(DIABLIB), )
include $(DEFAULT_MAK_DIR)/nucleus.diab.ppc.default.mak # PPC
else
include $(DEFAULT_MAK_DIR)/nucleus.ads.arm.default.mak # ARM
endif
endif

# pSOS
ifneq ($(PSS_ROOT), )
include $(DEFAULT_MAK_DIR)/psos.diab.ppc.default.mak
endif

# OSE
ifneq ($(OSE_ROOT), )
include $(DEFAULT_MAK_DIR)/ose.ghs.ppc.default.mak
endif

# INTEGRITY
ifneq ($(INTEGRITY_ROOT), )
include $(DEFAULT_MAK_DIR)/integrity.ghs.default.mak
endif

# Microsoft Visual C
ifneq ($(MSVC_ROOT), )
include $(DEFAULT_MAK_DIR)/win32.default.mak
endif

endif # CYGWIN (EMBEDDED)

ifeq ($(RV_PSOSAPPL_COMPILATION),)
$(warning *** TARGET_OS: $(TARGET_OS)($(TARGET_OS_VERSION)), COMPILER_TOOL: $(COMPILER_TOOL)($(COMPILER_TOOL_VERSION)) ***)
endif

ifeq ($(TCL_TK_VERSION), )
TCL_TK_VERSION := $(DEFAULT_TCL_TK_VERSION)
endif
$(warning Tcl/Tk version is $(TCL_TK_VERSION) )

BASIC_TCL_TK_LIBS := tcl$(TCL_TK_VERSION) tk$(TCL_TK_VERSION)
