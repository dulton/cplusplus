SDK_ROOT=/export/crosstools/yocto-1.0/changeling-vm-32/sysroots/i686-spirentsdk-linux
TOOL_ROOT=$SDK_ROOT/usr/bin/i586-spirent-linux/i586-spirent-linux
# Stolen from build/il/configs/environs/arch/yocto_i686.py
COMPILER_SYSROOT=/export/crosstools/yocto-1.0/changeling-vm-32/sysroots/i586-spirent-linux
ARCH_LINKFLAGS="-m32 -lgcc_s --sysroot=$COMPILER_SYSROOT"
ARCH_CXXFLAGS="-m32 -DYOCTO -DARCH_I686 -DARCH_X86 -DYOCTO_I686"
ARCH_CPPFLAGS="--sysroot=$COMPILER_SYSROOT -I/export/crosstools/yocto-1.0/changeling-vm-32/sysroots/i586-spirent-linux/usr/include/c++ -I/export/crosstools/yocto-1.0/changeling-vm-32/sysroots/i586-spirent-linux/usr/include/c++/backward -I/export/crosstools/yocto-1.0/changeling-vm-32/sysroots/i586-spirent-linux/usr/include/c++/i586-spirent-linux -m32 -ggdb -DYOCTO -DARCH_I686 -DI686 -DARCH_X86"
ARCH_CFLAGS="--sysroot=$COMPILER_SYSROOT"
ARCH_CCFLAGS="--sysroot=$COMPILER_SYSROOT -include string.h -include unistd.h -include stdlib.h -include stdio.h"

make distclean

./configure \
    CC=$TOOL_ROOT-gcc \
    CFLAGS="-fno-strict-aliasing $ARCH_CFLAGS" \
    CCFLAGS="-fno-strict-aliasing $ARCH_CCFLAGS" \
    CXX=$TOOL_ROOT-g++ \
    CXXFLAGS="-fno-strict-aliasing -fpermissive $ARCH_CXXFLAGS" \
    LDFLAGS="-rdynamic" \
    --includedir=$SDK_ROOT/usr/include \
    --host=i386-linux \
    --target=i686-linux

make
