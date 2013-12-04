bin_PROGRAMS = test_tip_media_option test_tip_system test_map_tip_system test_tip_profile test_tip_packet_receiver test_tip_timer test_tip test_tip_relay test_tip_media

TESTS = $(bin_PROGRAMS)

AM_CPPFLAGS = -I$(top_srcdir)/lib/common/src -I$(top_srcdir)/lib/packet/src -I$(top_srcdir)/lib/user/src

SOURCES_COMMON = $(top_srcdir)/lib/common/test/test_runner_main.cpp
LDADD_COMMON = $(top_srcdir)/lib/user/src/libtipuser.la $(top_srcdir)/lib/packet/src/libtippacket.la $(top_srcdir)/lib/common/src/libtipcommon.la -lcppunit

test_tip_media_option_SOURCES = test_tip_media_option.cpp $(SOURCES_COMMON)
test_tip_media_option_LDADD = $(LDADD_COMMON)

test_tip_system_SOURCES = test_tip_system.cpp $(SOURCES_COMMON)
test_tip_system_LDADD = $(LDADD_COMMON)

test_map_tip_system_SOURCES = test_map_tip_system.cpp $(SOURCES_COMMON)
test_map_tip_system_LDADD = $(LDADD_COMMON)

test_tip_profile_SOURCES = test_tip_profile.cpp $(SOURCES_COMMON)
test_tip_profile_LDADD = $(LDADD_COMMON)

test_tip_packet_receiver_SOURCES = test_tip_packet_receiver.cpp $(SOURCES_COMMON)
test_tip_packet_receiver_LDADD = $(LDADD_COMMON)

test_tip_timer_SOURCES = test_tip_timer.cpp $(SOURCES_COMMON)
test_tip_timer_LDADD = $(LDADD_COMMON)

test_tip_SOURCES = test_tip.cpp $(SOURCES_COMMON)
test_tip_LDADD = $(LDADD_COMMON)

test_tip_relay_SOURCES = test_tip_relay.cpp $(SOURCES_COMMON)
test_tip_relay_LDADD = $(LDADD_COMMON)

test_tip_media_SOURCES = test_tip_media.cpp $(SOURCES_COMMON)
test_tip_media_LDADD = $(LDADD_COMMON)

memcheck:
	TESTS_ENVIRONMENT="libtool --mode=execute valgrind --tool=memcheck --leak-check=yes --num-callers=12 -q" $(MAKE) $(AM_MAKEFLAGS) check-TESTS
