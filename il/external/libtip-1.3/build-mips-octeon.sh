#! /bin/sh

./configure --prefix=$PWD/binaries/MontaVista_4.0_mips_octeon/release CXX=/export/crosstools/mvl40/mips64/octeon_v2_be/bin/mips64_octeon_v2_be-g++ --disable-shared CXXFLAGS="-DUPDATED_BY_SPIRENT"

make clean
make
make install
rm -rf $PWD/binaries/MontaVista_4.0_mips_octeon/release/lib/*.la
