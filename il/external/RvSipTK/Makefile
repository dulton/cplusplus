#************************************************************************
#Filename   : Makefile
#Description: Top level makefile
#************************************************************************
#	Copyright (c) 2002 RADVISION Inc. and RADVISION Ltd.
#************************************************************************
#NOTICE:
#This document contains information that is confidential and proprietary
#to RADVISION Inc. and RADVISION Ltd.. No part of this document may be
#reproduced in any form whatsoever without written prior approval by
#RADVISION Inc. or RADVISION Ltd..
#
#RADVISION Inc. and RADVISION Ltd. reserve the right to revise this
#publication and make changes without obligation to notify any person of
#such revisions or changes.
#************************************************************************

# Note: This makefile must be run with GNU make version 3.78 or higher.
# Paramaters are read from the default.mak file.

SHELL = $(CYGWIN_ROOT)/bin/sh

# Set locale-related environment variables to "C"
# We're making decisions based on output of some commands, so it's important
# that this output will be in the form we can understand
LANG=C
LC_ALL=C

#-------------- Support functions -------------

# flagset - checks whether compilation flag is set
# usage:
# $(call flagset,$(SCTP))
# checks whether SCTP flag is set, e.g. whether SCTP is one of '1, on, ON, true or TRUE'
flagset = $(strip $(findstring $(1), 1 on ON true TRUE))


# Load the configuration file
include default.mak

###### UPDATED BY SPIRENT 
#
# BUILD_TYPE is passed to environment.mak from default.mak

# See if we specifically asked for a RELEASE build type
ifeq ($(RELEASE), on)
release := on
else
release := off
endif

ifeq ($(release), on)
BUILD_TYPE := release
else 
BUILD_TYPE := debug
endif

# Path of the project.mak file
PROJECT_MAKE_PATH := common/config/make

# Load validity checks for default.mak file
include $(PROJECT_MAKE_PATH)/environment.mak

# Master lists. Define starting values here. Sub-makes append their info
# to the end of the lists.

# list of directories containing configuration headers
CONFIG_DIRS := common/config common/config/arch common/config/os common/config/tool

# Global Makefile and sub-make list - used for dependencies
#--- MAKE_LIST := Makefile $(PROJECT_MAKE_PATH)/project.mak $(PROJECT_MAKE_PATH)/environment.mak $(PROJECT_MAKE_PATH)/project_list.mak default.mak

# List of external include directories (Current directory for rvbuildconfig.h)
INCLUDE_DIRS :=

# List of external libraries
LIBS_LIST :=

# List of external libraries needed for Tcl-Tk if used
LIBS_TK_LIST :=

# List of directories to look for libraries in (start with current directory)
LIB_DIRS := .

#  List of source file directories to search for header files (start with config headers)
SOURCE_DIRS := $(CONFIG_DIRS)

# List of all C++ objects
CPP_OBJECTS :=

# List of object files
OBJECT_LIST :=
PRJ_OBJECT_LIST :=

# List of targets to make for libs target
LIBS_TARGET_LIST :=

# List of execuatble applications to make in addition to libs for all target
EXE_TARGET_LIST :=

# List of other targets to make in addition to libs for all target
OTHER_TARGET_LIST :=

# List of all targets that are named by their project names for easier handling
NAMED_TARGET_LIST :=

# List of files that should be exported
INCLUDE_FILES_LIST := common/rvexternal.h 
INCLUDE_TARGETS_LIST :=

# List of include files to be copied into install directory (start with config headers)
INCLUDE_INSTALL_LIST := $(foreach dir,$(CONFIG_DIRS),$(wildcard $(dir)/*.h))

# List of demo files to be copied into install directory
DEMO_INSTALL_LIST :=

# Allow compilation for Unix (using RSH), VxWorks, Nucleus, etc. from within Visual C++.
# The CC_MESSAGE_CONVERSION defines the formula for converting the compiler's
# error/warning messages into VC++ format which enable jumping into code using double click
ifneq ($(_MSDEV_BLD_ENV_), )
ifneq ($(findstring $(COMPILER_TOOL),"gnu metrowerks"), )
CC_MESSAGE_CONVERSION := 2>&1 | sed 's=\(.*\):\([0-9]*\):\(.*\)=\1(\2):\3='
endif

ifneq ($(findstring $(COMPILER_TOOL),"ads ghs diab"), )
CC_MESSAGE_CONVERSION := 2>&1 | grep -v "before your license expires" | sed 's="\(.*\)", line \([0-9]*\)\(.*\)=\1(\2)\3='
# CC_MESSAGE_CONVERSION := 2>&1 | sed 's="\(.*\)", line \([0-9]*\)\(.*\)=\1(\2)\3='
endif
endif

ifneq ($(_MAKEAUTO_), )
ECHO_STDOUT := > /dev/null
endif

# List of all the projects we intend to support
PROJECTS := common/common.mak

# Handle make arguments (defined by the stack developers)
ifeq ($(BUILD_TYPE), debug)
BINARIES_DIR := Debug
nolog=off
else
BINARIES_DIR := Release
endif

ifeq ($(MWCC), on)
BINARIES_DIR := $(BINARIES_DIR)Emul
endif

##### UPDATED BY SPIRENT 
#
# add custom user settings here
#
ifeq ($(MAKECMDGOALS),all_nouc)
USER_CFLAGS := -DRV_NOUSRCONFIG
else
USER_CFLAGS := -DUPDATED_BY_SPIRENT -DRV_SELECT_MAX_DESCRIPTORS=32767
endif

-include extensions/*.mak

# Special directories necessary for the build procedure

# Include directory, where all include files will be placed during compilations
APPL_INCLUDE_DIR := include/$(BUILD_TYPE)

# Binaries root directory
# UPDATED_BY_SPIRENT
#
#BINARIES_ROOT_DIR := binaries/$(TARGET_OS)_$(TARGET_OS_VERSION)/$(BINARIES_DIR)
BINARIES_ROOT_DIR := binaries/$(TARGET_OS_VERSION)_$(TARGET_CPU)/$(BUILD_CONFIG_NAME)/$(BUILD_TYPE)

# Directory holding all .o object files
PROJECT_OBJ_DIR := $(BINARIES_ROOT_DIR)/obj

# Directory holding all .a library files
PROJECT_LIB_DIR := $(BINARIES_ROOT_DIR)/lib

# Directory holding all exeecutable files
PROJECT_BIN_DIR := $(BINARIES_ROOT_DIR)/bin


# NOTE: the following rule must be the FIRST one in the file
#
# if no goal (target) was specified then assume "all"
.PHONY: default
default: all
#	@echo "No target indicated. Please indicate target to build (ie 'make libs')."

# Stuff to save make some time
Makefile: ;
%.mak: ;
%.c: ;
%.h: ;

# Load OS specific definitions and add it to the dependency list
include $(PROJECT_MAKE_PATH)/os_$(TARGET_OS).mak
#--- MAKE_LIST += $(PROJECT_MAKE_PATH)/$(TARGET_OS).mak

# Check to make sure TARGET_OS_VERSION is valid (LEGAL_ set in os makefile)
ifndef RELAX_CHECKS

ifndef TARGET_OS_VERSION
$(error TARGET_OS_VERSION must be set in default.mak)
endif
ifneq ($(TARGET_OS_VERSION), $(findstring $(TARGET_OS_VERSION),$(LEGAL_TARGET_OS_VERSION)))
$(error The TARGET_OS_VERSION "$(TARGET_OS_VERSION)" is not supported. Legal values are: $(LEGAL_TARGET_OS_VERSION))
endif
ifeq ($(TARGET_OS_VERSION_LINE), )
$(error The TARGET_OS_VERSION "$(TARGET_OS_VERSION)" is not supported. Legal values are: $(LEGAL_TARGET_OS_VERSION))
endif

endif

# Load Compiler Tool specific definitions and add it to the dependency list
include $(PROJECT_MAKE_PATH)/tool_$(COMPILER_TOOL).mak
#--- MAKE_LIST += $(PROJECT_MAKE_PATH)/$(COMPILER_TOOL).mak

# Check to make sure TARGET_OS is supported by the COMPILER_TOOL
ifneq ($(TARGET_OS), $(findstring $(TARGET_OS),$(LEGAL_COMPILER_OS)))
$(error The TARGET_OS "$(TARGET_OS)" is not supported for $(COMPILER_TOOL). Legal values are: $(LEGAL_COMPILER_OS))
endif

# Check to make sure COMPILER_TOOL_VERSION is valid (LEGAL_ set in tool makefile)
ifndef RELAX_COMPILER_CHECK
	
ifndef COMPILER_TOOL_VERSION
$(error COMPILER_TOOL_VERSION must be set in default.mak)
endif
ifneq ($(COMPILER_TOOL_VERSION), $(findstring $(COMPILER_TOOL_VERSION),$(LEGAL_COMPILER_TOOL_VERSION)))
$(error The COMPILER_TOOL_VERSION "$(COMPILER_TOOL_VERSION)" is not supported. Legal values are: $(LEGAL_COMPILER_TOOL_VERSION))
endif
ifeq ($(COMPILER_TOOL_VERSION_LINE), )
$(error The COMPILER_TOOL_VERSION "$(COMPILER_TOOL_VERSION)" is not supported. Legal values are: $(LEGAL_COMPILER_TOOL_VERSION))
endif

endif


# Include all the projects
-include $(sort $(PROJECTS) )

# In case the "Make goal" is a project name (not "all" or "libs") then only
# the relevant project's object files are considered (to prevent dependencies
# calculation of un-necessary projects).
ifneq ($(PRJ_OBJECT_LIST), )
OBJECT_LIST = $(PRJ_OBJECT_LIST)
endif

# Set up compiler and linker flags for debug or optimization
ifeq ($(BUILD_TYPE), debug)
CFLAGS += $(C_DEBUGFLAG) -DRV_DEBUG
LDFLAGS += $(LD_DEBUGFLAG)
BUILD_DEBUG_LINE:= "\#define RV_DEBUG"
else
CFLAGS += $(C_OPTFLAG) -DRV_RELEASE
BUILD_DEBUG_LINE:= "\#define RV_RELEASE"
endif

# Set CFLAGS according to the variables defined in default.mak
CFLAGS += -DRV_TOOL_TYPE=$(COMPILER_TOOL_LINE)
CFLAGS += -DRV_TOOL_VERSION=$(COMPILER_TOOL_VERSION_LINE)
CFLAGS += -DRV_OS_TYPE=$(TARGET_OS_LINE)
CFLAGS += -DRV_OS_VERSION=$(TARGET_OS_VERSION_LINE)
CFLAGS += -DRV_ARCH_ENDIAN=$(TARGET_CPU_ENDIAN_LINE)
CFLAGS += -DRV_ARCH_BITS=$(TARGET_CPU_BITS_LINE)
CFLAGS += $(USER_CFLAGS)

# Set LIBXML folders
LIBXML2_INC_DIR := $(LIBXML2_INC)
LIBXML2_LIB_DIR := $(LIBXML2_LIB)
LIBXML2_LIBS_LIST := xml2
ifeq ($(TARGET_OS), solaris)
LIBXML2_LIBS_LIST += m
endif

# add libXml folders and libraries
ifeq ($(XML), on)
SOURCE_DIRS += $(LIBXML2_INC_DIR)
LIB_DIRS += $(LIBXML2_LIB_DIR)
LIBS_LIST += $(LIBXML2_LIBS_LIST)
endif

# Add OS/Tools specific include and library directories and libraries
CFLAGS += $(INCLUDE_DIRS:%=$(INCLUDE_FLAG)%)
#--- LDFLAGS += $(LIB_DIRS:%=$(LIBS_DIRFLAG)%) $(LIBS_LIST:%=$(LIBS_FLAG)%) $(LIBS_DIRFLAG)$(PROJECT_LIB_DIR)
LDFLAGS += $(LIB_DIRS:%=$(LIBS_DIRFLAG)%) $(LIBS_DIRFLAG)$(PROJECT_LIB_DIR)

# Save base CFLAGS and LDFLAGS info for creating stand-alone scripts
SCRIPT_CFLAGS := $(CFLAGS)
SCRIPT_LDFLAGS := $(LDFLAGS)

# Add build directory tree to include path
CFLAGS += $(SOURCE_DIRS:%=$(INCLUDE_FLAG)%)
# Handle the include exporting directory that will be used
INCLUDE_TARGETS_LIST := $(foreach h,$(INCLUDE_FILES_LIST),$(APPL_INCLUDE_DIR)/$(firstword $(subst /, ,$(h)))/$(notdir $(h)))
# The following line is to satisfy the patern "dir/*/FILE.h" used in the following 'cp' command
#   i.e. dir1/dir11/FILE.h as well as dir1/FILE.h
# UPDATED_BY_SPIRENT
#
# subst depth changed from $(word 2,...  to $(word 3,...) to acommodate a change in APPL_INCLUDE_DIR
#
INCLUDE_FILES_STRING = $(subst /,//,$(INCLUDE_FILES_LIST))
$(INCLUDE_TARGETS_LIST): $(INCLUDE_FILES_LIST)
	@[ -d $(@D) ] || mkdir -p $(@D)
	@cp -f $(subst //,/,$(filter $(word 3,$(subst /, ,$@))/%/$(notdir $@),$(INCLUDE_FILES_STRING))) $@ > /dev/null

# Specific rule for generating rvbuildconfig.h
# The _LINE variables are set up above and in the os and tool
# specific makefiles.
$(APPL_INCLUDE_DIR)/common/rvbuildconfig.h: $(MAKE_LIST)
	@echo Generating $@
	@[ -d $(@D) ] || mkdir -p $(@D)
	@rm -f $@
	@echo "/* Generated by Makefile, do not edit. */" > $@; \
	echo "$(BUILD_DEBUG_LINE)" >> $@; \
	echo "#define RV_TOOL_TYPE $(COMPILER_TOOL_LINE)" >> $@; \
	echo "#define RV_TOOL_VERSION $(COMPILER_TOOL_VERSION_LINE)" >> $@; \
	echo "#define RV_OS_TYPE $(TARGET_OS_LINE)" >> $@; \
	echo "#define RV_OS_VERSION $(TARGET_OS_VERSION_LINE)" >> $@; \
	echo "#define RV_ARCH_ENDIAN $(TARGET_CPU_ENDIAN_LINE)" >> $@; \
	echo "#define RV_ARCH_BITS $(TARGET_CPU_BITS_LINE)" >> $@

# Base build rules

#
# Creating dependencies for C++ files
#
$(CPP_OBJECTS:$(OBJ_EXTENSION)=$(DEPEND_EXTENSION)): $(PROJECT_OBJ_DIR)/%$(DEPEND_EXTENSION): %.cpp $(MAKE_LIST)
	@echo Creating dependencies for C++ file $<
	@[ -d $(@D) ] || mkdir -p $(@D)
	@$(MAKEDEPEND)

#
# Creating dependencies for C files
#
$(PROJECT_OBJ_DIR)/%$(DEPEND_EXTENSION): %.c $(MAKE_LIST)
	@echo Creating dependencies for C file $<
	@[ -d $(@D) ] || mkdir -p $(@D)
	@$(MAKEDEPEND)

#
# Compiling C++ files
#
$(CPP_OBJECTS): $(PROJECT_OBJ_DIR)/%$(OBJ_EXTENSION): %.cpp $(PROJECT_OBJ_DIR)/%$(DEPEND_EXTENSION) $(MAKE_LIST)
	@echo Compiling C++ file $<
	@$(CC) $(COMPILE_ONLY_FLAG) $(CFLAGS) $< $(CC_OBJ_OUTPUT) $(CC_MESSAGE_CONVERSION)

#
# Compiling C files
#
$(PROJECT_OBJ_DIR)/%$(OBJ_EXTENSION): %.c $(PROJECT_OBJ_DIR)/%$(DEPEND_EXTENSION) $(MAKE_LIST)
	@echo Compiling C file $<
	@$(CC) $(COMPILE_ONLY_FLAG) $(CFLAGS) $< $(CC_OBJ_OUTPUT) $(CC_MESSAGE_CONVERSION)


#
# Compiling C files for shared library
#
$(PROJECT_OBJ_DIR)/%$(SHARED_OBJ_EXTENSION): %.c $(PROJECT_OBJ_DIR)/%$(DEPEND_EXTENSION) $(MAKE_LIST)
	@echo Compiling C file $< for a shared library
	@$(CC) $(COMPILE_ONLY_FLAG) $(SHARED_LIB_C_FLAG) $(CFLAGS) $< $(CC_OBJ_OUTPUT) $(CC_MESSAGE_CONVERSION)

#
# Creating project library
#
# $(warning INCLUDE_TARGETS_LIST $(INCLUDE_TARGETS_LIST))
# $(warning OBJECT_LIST $(OBJECT_LIST) )

ifeq ($(findstring CYGWIN,$(OS_NAME)), CYGWIN)
TMP_OBJ_FILES := $(PROJECT_OBJ_DIR)/objs.txt
AR_CMD1 = echo $(filter %$(OBJ_EXTENSION), $^) > $(TMP_OBJ_FILES)
AR_CMD2 = cat $(TMP_OBJ_FILES) | gawk -f build/make_ar_cmd.awk prefix="$(AR) $(ARFLAGS) $(LIB_OUTPUT)" | $(SHELL);\
                      [ -s $(TMP_OBJ_FILES) ] || rm -f $(TMP_OBJ_FILES)
else
AR_CMD1 = $(AR) $(ARFLAGS) $(LIB_OUTPUT) $(filter %$(OBJ_EXTENSION), $^) 
AR_CMD2 = 
endif


$(PROJECT_LIB_DIR)/%$(LIB_EXTENSION): $(INCLUDE_TARGETS_LIST) $(MAKE_LIST)
	@echo Creating library $@ $(ECHO_STDOUT)
	$(_@_) [ -d $(@D) ] || mkdir -p $(@D)
	$(_@_) rm -f $@
	$(_@_) $(AR_CMD1)
	$(_@_) $(AR_CMD2)
	$(POST_AR_COMMAND)

#
# Creating project shared library
# SHARED_LINK_FLAGS defines flags for shared objects generation. 
#  Should be defined in per-tool .mak file (took_gnu.mak)
# SHARED_LIBS_LIST - potentially, os-dependent list of libraries needed. 
#  Defined in per-os .mak file (os_linux.mak)
#
$(PROJECT_LIB_DIR)/%$(SHARED_LIB_EXTENSION): $(INCLUDE_TARGETS_LIST) $(MAKE_LIST)
	@echo Creating shared library $@
	@if ( test ! -d $(dir $@) ) ; then mkdir -p $(dir $@); fi
	@rm -f $@; $(LINK) $(SHARED_LIB_C_FLAG) -shared $(filter %$(SHARED_OBJ_EXTENSION), $^) -o $(LIB_OUTPUT)


#
# Creating application
#
$(EXE_TARGET_LIST): $(MAKE_LIST) $(INCLUDE_TARGETS_LIST)
	@echo Creating application $@
	@[ -d $(@D) ] || mkdir -p $(@D)
ifeq ($(COMPILER_TOOL), msvc)
	@$(LINK) /out:"$@" $($(@)_EXE_OBJ_LIST) $(LDFLAGS) $($(@)_EXE_LIBS_NEEDED:%=$(LIBS_FLAG)$(PROJECT_LIB_DIR)/$(LIB_PREFIX)%$(LIB_EXTENSION)) $(LIBS_LIST:%=$(LIBS_FLAG)%) $(USER_LIBS_LIST)
else
ifeq ($(TARGET_OS), symbian)
ifeq ($(COMPILER_TOOL), metrowerks)
	$(LINK) -msgstyle gcc -stdlib -m "?_E32Startup@@YGXXZ" -subsystem windows -g -noimplib $(CC_OBJ_OUTPUT) $($(@)_EXE_OBJ_LIST) $($(@)_EXE_LIBS_NEEDED:%=$(PROJECT_LIB_DIR)/lib%$(LIB_EXTENSION)) $(LIBS_LIST:%=$(LIB_DIRS)/%$(LIB_EXTENSION)) $(USER_LIBS_LIST)
else
	$(LINK) -e _E32Startup -u _E32Startup --base-file $(@D)/appl.bas $(CC_OBJ_OUTPUT) $($(@)_EXE_OBJ_LIST) $($(@)_EXE_LIBS_NEEDED:%=$(PROJECT_LIB_DIR)/lib%$(LIB_EXTENSION)) $(LIBS_LIST:%=$(LIB_DIRS)/%$(LIB_EXTENSION)) $(USER_LIBS_LIST)
	@dlltool --as $(SYMBIAN_ROOT)/gcc/bin/as -m arm_interwork --base-file $(@D)/appl.bas --output-exp $(@D)/appl.exp
	@$(LINK) -e _E32Startup -u _E32Startup $(@D)/appl.exp $(CC_OBJ_OUTPUT) $($(@)_EXE_OBJ_LIST) $($(@)_EXE_LIBS_NEEDED:%=$(PROJECT_LIB_DIR)/lib%$(LIB_EXTENSION)) $(LIBS_LIST:%=$(LIB_DIRS)/%$(LIB_EXTENSION)) $(USER_LIBS_LIST)
	@$(SYMBIAN_ROOT)/tools/petran $@ $@ -nocall -uid1 0x1000007a -uid2 0x00000000 -uid3 0x00000000
endif
else
ifeq ($(TARGET_OS), integrity)
	@$(LINK) $(CC_OBJ_OUTPUT) $($(@)_EXE_OBJ_LIST) -T extensions/integrityApp.ld $(LDFLAGS) $($(@)_EXE_LIBS_NEEDED:%=$(LIBS_FLAG)%) $(LIBS_LIST:%=$(LIBS_FLAG)%) $(USER_LIBS_LIST) $(USER_LIBS_LIST)
	@gnm.exe -s $@ > $@.nm
	@dblink.exe -nm=gnm $@.nm -o $@.dnm -no_warn_for_dbo_not_found
	@sed 's=_APPLICATION_FILE_=$@=' extensions/integrityApp.int > $(@D)/integrityApp.int
	@intex -bspdir=$(INTEGRITY_ROOT)/$(TARGET_CPU_HEADERS) $(@D)/integrityApp.int $@.out
ifeq ($(shell awk '{ if ($$1=="Filename" && $$2=="DynamicDownload") print }' extensions/integrityApp.int), )
	@gmemfile $@.out -o $@.bin
endif
ifneq ($(BUILD_TYPE), debug)
	@rm -f $@ $@.dla $@.dnm $@.map $@.nm $(@D)/integrityApp.int
endif
else
	$(LINK) $(CC_OBJ_OUTPUT) $($(@)_EXE_OBJ_LIST) $(LDFLAGS) $($(@)_EXE_LIBS_NEEDED:%=$(LIBS_FLAG)%) $(LIBS_LIST:%=$(LIBS_FLAG)%) $(USER_LIBS_LIST) $(USER_LIBS_LIST)
endif
endif
endif

# Master Targets
libs: $(LIBS_TARGET_LIST)
	@echo $(shell date) > $@
	@echo All libs made.

all: config $(LIBS_TARGET_LIST) $(EXE_TARGET_LIST) $(OTHER_TARGET_LIST) $(INCLUDE_TARGETS_LIST)
	@echo $(shell date) > $@
	@echo All targets made.

# Ignoring rvusrconfig.h
.PHONY: all_nouc
all_nouc: all

###### UPDATED BY SPIRENT
#
# TARGET_OS subdir removed from the installation path
#

# Master install -- also create directories and scripts
install: all
	@echo Installing to $(INSTALL_ROOT)
#	@cp -pf common/config/rvbuildconfig.h $(INSTALL_ROOT)/include/$(TARGET_OS)
ifneq ($(words $(INCLUDE_INSTALL_LIST)), 0)
	@[ -d $(INSTALL_ROOT)/include ] || mkdir -p $(INSTALL_ROOT)/include
	@cp -pf $(INCLUDE_INSTALL_LIST) $(INSTALL_ROOT)/include
endif
ifneq ($(words $(LIBS_TARGET_LIST)), 0)
	@[ -d $(INSTALL_ROOT)/lib || mkdir -p $(INSTALL_ROOT)/lib
	@cp -pf $(LIBS_TARGET_LIST) $(INSTALL_ROOT)/lib
endif
ifneq ($(words $(DEMO_INSTALL_LIST)), 0)
	@[ -d $(INSTALL_ROOT)/demo/$(TARGET_OS) ] || mkdir -p $(INSTALL_ROOT)/demo/$(TARGET_OS)
	@cp -pf $(DEMO_INSTALL_LIST) $(INSTALL_ROOT)/demo/$(TARGET_OS)
endif
ifneq ($(noscriptinstall), on)
	@[ -d $(INSTALL_ROOT)/scripts/$(TARGET_OS) ] || mkdir -p $(INSTALL_ROOT)/scripts/$(TARGET_OS)
	@echo "#!/bin/sh" > $(INSTALL_ROOT)/scripts/$(TARGET_OS)/compile.sh; \
	echo "# Usage: compile filename" >> $(INSTALL_ROOT)/scripts/$(TARGET_OS)/compile.sh; \
	echo "# Will compile filename.c into filename.o" >> $(INSTALL_ROOT)/scripts/$(TARGET_OS)/compile.sh; \
	echo "$(CC) $(COMPILE_ONLY_FLAG) $(INCLUDE_FLAG)$(INSTALL_ROOT)/include $(SCRIPT_CFLAGS) \$$1.c $(CC_OBJ_OUTPUT) \$$1$(OBJ_EXTENSION)" >> $(INSTALL_ROOT)/scripts/$(TARGET_OS)/compile.sh
	@echo "#!/bin/sh" > $(INSTALL_ROOT)/scripts/$(TARGET_OS)/link.sh; \
	echo "# Usage: link filename \"objectfile1.o objectfile2.o ...\" \"$(LIBS_FLAG)lib1 $(LIBS_FLAG)lib2 ...\"" >> $(INSTALL_ROOT)/scripts/$(TARGET_OS)/link.sh; \
	echo "# Will link object files into filename" >> $(INSTALL_ROOT)/scripts/$(TARGET_OS)/link.sh; \
	echo "$(LINK) $(CC_OBJ_OUTPUT) \$$1 \$$2 $(LIBS_DIRFLAG)$(INSTALL_ROOT)/lib/$(TARGET_OS) $(SCRIPT_LDFLAGS) \$$3" >> $(INSTALL_ROOT)/scripts/$(TARGET_OS)/link.sh
	@echo done.
endif

# List of special targets that should not cause dependency generation
NODEPEND_TARGETS := clean config

# Clean up
.PHONY: clean
clean:
	@echo Cleaning all source directories and the top level directory.
	@rm -f all libs $(EXE_TARGET_LIST) $(OTHER_TARGET_LIST) *.[aiodt] *.dbo *.lib *.pdb $(foreach dir,$(SOURCE_DIRS), $(dir)/*.[odit] $(dir)/*.dbo $(dir)/*.obj)
	@rm -fR $(APPL_INCLUDE_DIR)/*
	@rm -fR $(BINARIES_ROOT_DIR)/*

# Generate dependencies only - which occurs as long as the include .d command occurs
.PHONY: depend
depend:
	@echo Only dependencies generated.

# Generate $(APPL_INCLUDE_DIR)/common/rvbuildconfig.h only
.PHONY: config
config: $(APPL_INCLUDE_DIR)/common/rvbuildconfig.h $(INCLUDE_TARGETS_LIST)
	@echo Only $(APPL_INCLUDE_DIR)/common/rvbuildconfig.h generated.

# Only allow building valid targets - eliminates strange errors with unknown targets
VALID_TARGETS := all all_nouc libs install clean depend config default $(LIBS_TARGET_LIST) $(EXE_TARGET_LIST) $(OTHER_TARGET_LIST) $(NAMED_TARGET_LIST)
INVALID_TARGETS := $(filter-out $(VALID_TARGETS), $(MAKECMDGOALS))
ifneq ($(words $(INVALID_TARGETS)), 0)
$(error Cannot build $(INVALID_TARGETS). Valid targets are: $(VALID_TARGETS))
endif

# Include module dependency files - don't do it for special targets
ifeq ($(findstring $(MAKECMDGOALS),$(NODEPEND_TARGETS)), )
DEPEND_LIST := $(OBJECT_LIST:$(OBJ_EXTENSION)=$(DEPEND_EXTENSION))
DEPEND_LIST := $(DEPEND_LIST:$(SHARED_OBJ_EXTENSION)=$(DEPEND_EXTENSION))
-include $(DEPEND_LIST)
endif
