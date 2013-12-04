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
COMPILER_TOOL_VERSION := 2.9
# ----------------------------------------------------------------------

# Target information ---------------------------------------------------
# TARGET_OS := osname (name of supported os)
TARGET_OS := linux

# TARGET_OS_VERSION := osversion (name of supported os version)
TARGET_OS_VERSION := redhat-7.3

#TARGET_CPU_FAMILY := family (family id used to identify compiler)
TARGET_CPU_FAMILY := i386

#TARGET_CPU_HEADERS := VALUE (value used to find and/or used by OS/compiler header files)
TARGET_CPU_HEADERS := i386

# TARGET_CPU_ID := id (cpu identifier passed to the compiler)
TARGET_CPU_ID := i386

#TARGET_CPU_ENDIAN := little/big (mode of CPU)
TARGET_CPU_ENDIAN := little

#TARGET_CPU_BITS := 32/64 (bit model to use)
TARGET_CPU_BITS := 32
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


# Additional information -----------------------------------------------
# Special compiler flags
CFLAGS :=

# Special linker flags
LDFLAGS :=
# ----------------------------------------------------------------------


# Install information --------------------------------------------------
# Destination root directory for install command (use '/', NOT '\" on Win32)
INSTALL_ROOT := /usr/local/rvcommon
# ----------------------------------------------------------------------

# The default TCL/TK version to be used
# may be overwritten through TCL_TK_VERSION environment variable or make command line argument (TCL_TK_VERSION = xyz)
DEFAULT_TCL_TK_VERSION := 8.3
