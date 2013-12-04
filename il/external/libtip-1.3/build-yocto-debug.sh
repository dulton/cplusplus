#! /bin/sh

./configure \
    --prefix=$PWD/binaries/i386_yocto_i686/debug \
    CXX=/export/crosstools/yocto-1.0/changeling-vm-32/sysroots/i686-spirentsdk-linux/usr/bin/i586-spirent-linux/i586-spirent-linux-g++ \
    --disable-shared \
    CXXFLAGS="-DDEBUG_ON -DUPDATED_BY_SPIRENT --sysroot=/export/crosstools/yocto-1.0/changeling-vm-32/sysroots/i586-spirent-linux -I/export/crosstools/yocto-1.0/changeling-vm-32/sysroots/i586-spirent-linux/usr/include/c++ -I/export/crosstools/yocto-1.0/changeling-vm-32/sysroots/i586-spirent-linux/usr/include/c++/backward -I/export/crosstools/yocto-1.0/changeling-vm-32/sysroots/i586-spirent-linux/usr/include/c++/i586-spirent-linux" \
    LDFLAGS="-lgcc_s --sysroot=/export/crosstools/yocto-1.0/changeling-vm-32/sysroots/i586-spirent-linux"

make clean
make
make install
rm -rf $PWD/binaries/i386_yocto_i686/debug/lib/*.la
