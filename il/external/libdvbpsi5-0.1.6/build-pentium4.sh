TOOL_ROOT=/export/crosstools/mvl40/x86/pentium4

make distclean

./configure \
    CC=$TOOL_ROOT/bin/pentium4-gcc \
    CFLAGS='-fno-strict-aliasing' \
    CCFLAGS='-fno-strict-aliasing' \
    CXX=$TOOL_ROOT/bin/pentium4-g++ \
    CXXFLAGS='-fno-strict-aliasing -fpermissive' \
    LDFLAGS='-rdynamic' \
    --includedir=$TOOL_ROOT/usr/include \
    --host=i386-linux \
    --target=i686-linux

make
