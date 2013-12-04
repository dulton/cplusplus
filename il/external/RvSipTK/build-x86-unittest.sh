#! /bin/sh

export MVISTA=on TARGET_CPU=i386_pentium3 release=off UNIT_TEST=on

make clean 
make all

