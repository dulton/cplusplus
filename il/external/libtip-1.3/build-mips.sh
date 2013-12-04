#! /bin/sh

./configure --prefix=$PWD/binaries/MontaVista_4.0_mips_fp_be/release --disable-shared CXX=/export/crosstools/mvl40/mips64/fp_be/bin/mips64_fp_be-g++ LDFLAGS="-L/export/crosstools/mvl40/mips64/fp_be/bin/../lib/gcc/mips64-montavista-linux/3.4.3 -L/export/crosstools/mvl40/mips64/fp_be/bin/../lib/gcc/mips64-montavista-linux/3.4.3/../../../../mips64-montavista-linux/lib/../lib32 -L/export/crosstools/mvl40/mips64/fp_be/bin/../target/lib/../lib32 -L/export/crosstools/mvl40/mips64/fp_be/bin/../target/usr/lib/../lib32" CXXFLAGS="-DUPDATED_BY_SPIRENT" 

make clean
make
make install
rm -rf $PWD/binaries/MontaVista_4.0_mips_fp_be/release/lib/*.la
