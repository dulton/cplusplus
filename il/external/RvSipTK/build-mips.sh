#! /bin/sh

export MVISTA=on TARGET_CPU=mips_fp_be RELEASE=on

make clean 
make all
