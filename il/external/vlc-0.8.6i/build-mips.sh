ARCH_NAME=mips-fp_be
TOOL_ROOT=/export/crosstools/mvl40/mips64/fp_be
STAGING=`pwd`/staging
DVBPSI_ROOT=../libdvbpsi5-0.1.6

(cd $DVBPSI_ROOT; ./build-mips.sh)

make distclean

./configure \
    CC=$TOOL_ROOT/bin/mips64_fp_be-gcc \
    CFLAGS='-fno-strict-aliasing -mtune=sb1 -mabi=n32 -DSPIRENT_CHANGES' \
    CCFLAGS='-fno-strict-aliasing -mtune=sb1 -mabi=n32 -DSPIRENT_CHANGES' \
    CXX=$TOOL_ROOT/bin/mips64_fp_be-g++ \
    CXXFLAGS='-fno-strict-aliasing -mtune=sb1 -fpermissive -mabi=n32 -DSPIRENT_CHANGES' \
    LDFLAGS='-rdynamic' \
    --includedir=$TOOL_ROOT/usr/include \
    --prefix=$STAGING \
    --with-dvbpsi-tree=$DVBPSI_ROOT \
    --host=i386-linux \
    --target=mips-linux \
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
    --disable-gnutls

make install

[ ! -d $ARCH_NAME ] && mkdir $ARCH_NAME
[ ! -d $ARCH_NAME/plugins ] && mkdir $ARCH_NAME/plugins

(cd staging/bin; cp -v mips-linux-vlc ../../$ARCH_NAME/vlc)
(cd staging/lib/vlc; cp -v `find . -name \*.so` ../../../$ARCH_NAME/plugins)
