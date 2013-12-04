#! /bin/sh

./configure --prefix=$PWD/binaries/MontaVista_4.0_i386_pentium3/debug CXX=/export/crosstools/mvl40/x86/pentium3/bin/pentium3-g++ --disable-shared CXXFLAGS="-DUPDATED_BY_SPIRENT" 

make clean
make
make install
rm -rf $PWD/binaries/MontaVista_4.0_i386_pentium3/debug/lib/*.la
