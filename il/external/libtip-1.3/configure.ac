AC_INIT([LibTip], [1.3], [nbuckles@cisco.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_HEADERS(config.h)
AC_PROG_CC
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_FILES([
  Makefile
  lib/Makefile
  lib/common/Makefile
  lib/common/src/Makefile
  lib/common/test/Makefile
  lib/packet/Makefile
  lib/packet/src/Makefile
  lib/packet/test/Makefile
  lib/user/Makefile
  lib/user/src/Makefile
  lib/user/test/Makefile
  test/Makefile
  test/simple/Makefile
  test/pcap/Makefile
])

# add in compiler switches
AM_CXXFLAGS="$AM_CXXFLAGS -Wall -Werror"

# option to build or not build unit tests which require CPPUNIT
AC_ARG_ENABLE(unittest, [AS_HELP_STRING(--enable-unittest       Enable building of unit test programs, requires CppUnit)], [enable_unittest=yes])
AM_CONDITIONAL(UNITTEST, test "$enable_unittest" = yes)

# accept non-default CPPUNIT location
AC_ARG_WITH([CPPUNIT],
    [AS_HELP_STRING([--with-CPPUNIT=prefix],
        [path to CPPUNIT library])],
    [CPPUNIT_PATHSET=1],
    [CPPUNIT_PATHSET=0])

if test $CPPUNIT_PATHSET = 1; then
    AM_CXXFLAGS="$AM_CXXFLAGS -I$with_CPPUNIT/include"
    AM_LDFLAGS="$AM_LDFLAGS -L$with_CPPUNIT/lib"
fi

# option to build or not build pcap test program which requires libpcap
AC_ARG_ENABLE(pcaptest, [AS_HELP_STRING(--enable-pcaptest       Enable building of pcap parser program which requires LibPcap)], [enable_pcaptest=yes])
AM_CONDITIONAL(PCAPTEST, test "$enable_pcaptest" = yes)

AC_SUBST([AM_CXXFLAGS])
AC_SUBST([AM_LDFLAGS])

AC_OUTPUT

