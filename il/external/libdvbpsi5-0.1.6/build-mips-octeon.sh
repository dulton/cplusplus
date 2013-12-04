TOOL_ROOT=/export/crosstools/mvl40/mips64/octeon_v2_be

make distclean

./configure \
    CC=$TOOL_ROOT/bin/mips64_octeon_v2_be-gcc \
    CFLAGS='-fno-strict-aliasing -march=octeon -mabi=n32 -D__OCTEON__' \
    CCFLAGS='-fno-strict-aliasing -march=octeon -mabi=n32 -D__OCTEON__' \
    CXX=$TOOL_ROOT/bin/mips64_octeon_v2_be-g++ \
    CXXFLAGS='-fno-strict-aliasing -march=octeon -fpermissive -mabi=n32 -D__OCTEON__' \
    LDFLAGS='-rdynamic' \
    --includedir=$TOOL_ROOT/usr/include \
    --host=i386-linux \
    --target=mips-linux

make
