#! /bin/sh

./configure --prefix=$PWD/binaries/MontaVista_4.0_i386_pentium4/debug CXX=/export/crosstools/mvl40/x86/pentium4/bin/pentium4-g++ --disable-shared CXXFLAGS="-DDEBUG_ON -DUPDATED_BY_SPIRENT"

make clean
make
make install
rm -rf $PWD/binaries/MontaVista_4.0_i386_pentium4/debug/lib/*.la
