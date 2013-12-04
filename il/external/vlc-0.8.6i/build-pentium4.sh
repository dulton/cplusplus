ARCH_NAME=i386-pentium4
TOOL_ROOT=/export/crosstools/mvl40/x86/pentium4
STAGING=`pwd`/staging
DVBPSI_ROOT=../libdvbpsi5-0.1.6

(cd $DVBPSI_ROOT; ./build-pentium4.sh)

make distclean

./configure \
    CC=$TOOL_ROOT/bin/pentium4-gcc \
    CFLAGS='-fno-strict-aliasing -DSPIRENT_CHANGES' \
    CCFLAGS='-fno-strict-aliasing -DSPIRENT_CHANGES' \
    CXX=$TOOL_ROOT/bin/pentium4-g++ \
    CXXFLAGS='-fno-strict-aliasing -fpermissive -DSPIRENT_CHANGES' \
    LDFLAGS='-rdynamic' \
    --includedir=$TOOL_ROOT/usr/include \
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
    --disable-gnutls

make install

[ ! -d $ARCH_NAME ] && mkdir $ARCH_NAME
[ ! -d $ARCH_NAME/plugins ] && mkdir $ARCH_NAME/plugins

(cd staging/bin; cp -v i686-linux-vlc ../../$ARCH_NAME/vlc)
(cd staging/lib/vlc; cp -v `find . -name \*.so` ../../../$ARCH_NAME/plugins)
