bin_PROGRAMS = test_tip_csrc

TESTS = $(bin_PROGRAMS)

AM_CPPFLAGS = -I$(top_srcdir)/lib/common/src

SOURCES_COMMON = test_runner_main.cpp
LDADD_COMMON = $(top_srcdir)/lib/common/src/libtipcommon.la -lcppunit

test_tip_csrc_SOURCES = test_tip_csrc.cpp $(SOURCES_COMMON)
test_tip_csrc_LDADD = $(LDADD_COMMON)
