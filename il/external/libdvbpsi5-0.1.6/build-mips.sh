TOOL_ROOT=/export/crosstools/mvl40/mips64/fp_be

make distclean

./configure \
    CC=$TOOL_ROOT/bin/mips64_fp_be-gcc \
    CFLAGS='-fno-strict-aliasing -mtune=sb1 -mabi=n32' \
    CCFLAGS='-fno-strict-aliasing -mtune=sb1 -mabi=n32' \
    CXX=$TOOL_ROOT/bin/mips64_fp_be-g++ \
    CXXFLAGS='-fno-strict-aliasing -mtune=sb1 -fpermissive -mabi=n32' \
    LDFLAGS='-rdynamic' \
    --includedir=$TOOL_ROOT/usr/include \
    --host=i386-linux \
    --target=mips-linux

make
