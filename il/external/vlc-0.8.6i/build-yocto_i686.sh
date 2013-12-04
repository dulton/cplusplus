ARCH_NAME=i386-yocto_i686
SDK_ROOT=/export/crosstools/yocto-1.0/changeling-vm-32/sysroots/i686-spirentsdk-linux
TOOL_ROOT=$SDK_ROOT/usr/bin/i586-spirent-linux/i586-spirent-linux
# Stolen from build/il/configs/environs/arch/yocto_i686.py
COMPILER_SYSROOT=/export/crosstools/yocto-1.0/changeling-vm-32/sysroots/i586-spirent-linux
ARCH_LINKFLAGS="-m32 -lgcc_s --sysroot=$COMPILER_SYSROOT"
ARCH_CXXFLAGS="-m32 -DYOCTO -DARCH_I686 -DARCH_X86 -DYOCTO_I686"
ARCH_CPPFLAGS="--sysroot=$COMPILER_SYSROOT -I/export/crosstools/yocto-1.0/changeling-vm-32/sysroots/i586-spirent-linux/usr/include/c++ -I/export/crosstools/yocto-1.0/changeling-vm-32/sysroots/i586-spirent-linux/usr/include/c++/backward -I/export/crosstools/yocto-1.0/changeling-vm-32/sysroots/i586-spirent-linux/usr/include/c++/i586-spirent-linux -m32 -ggdb -DYOCTO -DARCH_I686 -DI686 -DARCH_X86"
ARCH_CFLAGS="--sysroot=$COMPILER_SYSROOT"
ARCH_CCFLAGS="--sysroot=$COMPILER_SYSROOT -include string.h -include unistd.h -include stdlib.h -include stdio.h"
STAGING=`pwd`/staging
DVBPSI_ROOT=../libdvbpsi5-0.1.6

(cd $DVBPSI_ROOT; ./build-yocto_i686.sh)

make distclean

./configure \
    CC=$TOOL_ROOT-gcc \
    CFLAGS="-fno-strict-aliasing -DSPIRENT_CHANGES $ARCH_CFLAGS" \
    CCFLAGS="-fno-strict-aliasing -DSPIRENT_CHANGES $ARCH_CCFLAGS" \
    CXX=$TOOL_ROOT-g++ \
    CXXFLAGS="-fno-strict-aliasing -fpermissive -DSPIRENT_CHANGES $ARCH_CXXFLAGS" \
    LDFLAGS="-rdynamic $ARCH_LINKFLAGS" \
    --includedir=$SDK_ROOT/usr/include \
    --prefix=$STAGING \
    --with-dvbpsi-tree=$DVBPSI_ROOT \
    --host=i386-linux \
    --target=i686-linux \
    --enable-mostly-builtin \
    --disable-xvideo \
    --disable-mad \
    --disable-ffmpeg \
    --disable-libmpeg2 \
    --disable-hal \
    --disable-cdda \
    --disable-screen \
    --disable-vcd \
    --disable-fb \
    --disable-x11 \
    --disable-growl \
    --disable-httpd \
    --disable-cmml \
    --disable-png \
    --disable-oss \
    --disable-alsa \
    --disable-visual \
    --disable-wxwidgets \
    --disable-skins2 \
    --disable-bonjour \
    --disable-gnutls \
    --disable-sdl \
    --disable-gnomevfs \
    --disable-freetype \
    --enable-libtool

make install

mkdir -p $ARCH_NAME/plugins

(cd staging/bin; cp -v i686-linux-vlc ../../$ARCH_NAME/vlc)
(cd staging/lib/vlc; cp -v `find . -name \*.so` ../../../$ARCH_NAME/plugins)
