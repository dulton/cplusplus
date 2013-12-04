# Global Configuration Parameters for RADVISION build system.

# Configuration Parameters ---------------------------------------------
# BUILD_TYE:= debug/release (build with debugging info or optimized)
# BUILD_TYPE := debug     (use the BUILD_TYPE specified in the makefile. don't override it here)

# Host Environment -----------------------------------------------------
# HOST_OS := osname (name of supported os)
HOST_OS := linux

# COMPILER_TOOL := toolname (name of supported compiler tool)
COMPILER_TOOL := gnu

# COMPILER_TOOL_VERSION := toolversion (name of supported compiler tool version)
COMPILER_TOOL_VERSION := 4.1

COMPILER_ROOT := /export/crosstools/mvl40/
# ----------------------------------------------------------------------

# Target information ---------------------------------------------------
# TARGET_OS := osname (name of supported os)
TARGET_OS := linux

# TARGET_OS_VERSION := osversion (name of supported os version)
TARGET_OS_VERSION := MontaVista_4.0

ifeq ($(TARGET_CPU), i386_yocto_i686)
  COMPILER_ROOT := /export/crosstools/yocto-1.0/changeling-vm-32/sysroots/i586-spirent-linux/
  TARGET_CPU_FAMILY := i586-spirent-linux
  COMPILER_PATH=$(COMPILER_ROOT)../i686-spirentsdk-linux/usr/bin/i586-spirent-linux

  #TARGET_CPU_ENDIAN := little/big (mode of CPU)
  TARGET_CPU_ENDIAN := little

  #TARGET_CPU_BITS := 32/64 (bit model to use)
  TARGET_CPU_BITS := 32

  LINK_TARGET=i386-linux

endif

ifeq ($(TARGET_CPU), i386_pentium4)
  TARGET_CPU_FAMILY := pentium4
  COMPILER_PATH=$(COMPILER_ROOT)x86/pentium4

  #TARGET_CPU_ENDIAN := little/big (mode of CPU)
  TARGET_CPU_ENDIAN := little

  #TARGET_CPU_BITS := 32/64 (bit model to use)
  TARGET_CPU_BITS := 32

  LINK_TARGET=i386-linux
endif

ifeq ($(TARGET_CPU), i386_pentium3)
  TARGET_CPU_FAMILY := pentium3
  COMPILER_PATH=$(COMPILER_ROOT)x86/pentium3

  #TARGET_CPU_ENDIAN := little/big (mode of CPU)
  TARGET_CPU_ENDIAN := little

  #TARGET_CPU_BITS := 32/64 (bit model to use)
  TARGET_CPU_BITS := 32

  LINK_TARGET=i386-linux
endif

ifeq ($(UNIT_TEST), on)
  CFLAGS  += -DUNIT_TEST
  CCFLAGS  += -DUNIT_TEST
endif

ifneq (,$(findstring mips,$(TARGET_CPU)))
#TARGET_CPU_ENDIAN := little/big (mode of CPU)
  TARGET_CPU_ENDIAN := big

#TARGET_CPU_BITS := 32/64 (bit model to use)
  TARGET_CPU_BITS := 32

  LINK_TARGET=mips-linux
  
 ifeq ($(TARGET_CPU), mips_fp_be)
  COMPILER_PATH := $(COMPILER_ROOT)mips64/fp_be

#TARGET_CPU_FAMILY := family (family id used to identify compiler)
  TARGET_CPU_FAMILY := mips64_fp_be

  CFLAGS  := -mtune=sb1 -mabi=n32 -D__MIPS__
  CCFLAGS := -mtune=sb1 -mabi=n32 -D__MIPS__
 endif

 ifeq ($(TARGET_CPU), mips_octeon)
  COMPILER_PATH := $(COMPILER_ROOT)mips64/octeon_v2_be

#TARGET_CPU_FAMILY := family (family id used to identify compiler)
  TARGET_CPU_FAMILY := mips64_octeon_v2_be

  CFLAGS  := -march=octeon -mabi=n32 -D__MIPS__
  CCFLAGS := -march=octeon -mabi=n32 -D__MIPS__ -fpermissive
 endif

endif

#TARGET_CPU_HEADERS := VALUE (value used to find and/or used by OS/compiler header files)
# **** NOT USED ****
#TARGET_CPU_HEADERS := 

# TARGET_CPU_ID := id (cpu identifier passed to the compiler)
# **** NOT USED ****
# TARGET_CPU_ID := 

# ----------------------------------------------------------------------

# OpenSSL folders variables --------------------------------------------
# Defines the OpenSSL include folder, if it was not defined in environment
ifeq ($(OPENSSL_INC),)
OPENSSL_INC :=
endif

# Defines the OpenSSL libraries folder, if it was not defined in environment
ifeq ($(OPENSSL_LIB),)
OPENSSL_LIB :=
endif
# ----------------------------------------------------------------------
CROSS_COMPILE := yes
# ----------------------------------------------------------------------

# Additional information -----------------------------------------------
# Special compiler flags
CFLAGS  += -fno-strict-aliasing -D_MONTAVISTA
CCFLAGS += -fno-strict-aliasing -D_MONTAVISTA

# Special linker flags
LDFLAGS := --includedir=$(COMPILER_PATH)/usr/include \
           --host=i386-linux --target=$(LINK_TARGET)

# ----------------------------------------------------------------------

# Install information --------------------------------------------------
# Destination root directory for install command (use '/', NOT '\" on Win32)
INSTALL_ROOT := /usr/local/rvcommon
# ----------------------------------------------------------------------

# The default TCL/TK version to be used
# may be overwritten through TCL_TK_VERSION environment variable or make command line argument (TCL_TK_VERSION = xyz)
DEFAULT_TCL_TK_VERSION := 8.4

export PATH:=$(COMPILER_PATH):$(COMPILER_PATH)/bin:$(PATH)
