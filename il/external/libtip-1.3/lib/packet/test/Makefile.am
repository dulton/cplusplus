bin_PROGRAMS = test_rtcp_packet test_rtcp_sdes_packet test_rtcp_rr_packet test_rtcp_tip_muxctrl_packet test_rtcp_tip_mediaopts_packet test_rtcp_tip_flowctrl_packet test_rtcp_tip_refresh_packet test_rtcp_packet_factory test_rtcp_tip_tlv test_rtcp_tip_packet_manager test_rtcp_tip_reqtosend_packet test_rtcp_tip_spimap_packet test_rtcp_tip_feedback_packet test_rtcp_tip_echo_packet test_rtcp_tip_notify_packet test_packet_buffer

TESTS = $(bin_PROGRAMS)

AM_CPPFLAGS = -I$(top_srcdir)/lib/common/src -I$(top_srcdir)/lib/packet/src

COMMON_SOURCES = $(top_srcdir)/lib/common/test/test_runner_main.cpp
COMMON_LDADD = $(top_srcdir)/lib/packet/src/libtippacket.la $(top_srcdir)/lib/common/src/libtipcommon.la -lcppunit

test_rtcp_packet_SOURCES = test_rtcp_packet.cpp test_packet_data.h $(COMMON_SOURCES)
test_rtcp_packet_LDADD = $(COMMON_LDADD)

test_rtcp_sdes_packet_SOURCES = test_rtcp_sdes_packet.cpp test_packet_data.h $(COMMON_SOURCES)
test_rtcp_sdes_packet_LDADD = $(COMMON_LDADD)

test_rtcp_rr_packet_SOURCES = test_rtcp_rr_packet.cpp test_packet_data.h $(COMMON_SOURCES)
test_rtcp_rr_packet_LDADD = $(COMMON_LDADD)

test_rtcp_tip_muxctrl_packet_SOURCES = test_rtcp_tip_muxctrl_packet.cpp test_packet_data.h $(COMMON_SOURCES)
test_rtcp_tip_muxctrl_packet_LDADD = $(COMMON_LDADD)

test_rtcp_tip_mediaopts_packet_SOURCES = test_rtcp_tip_mediaopts_packet.cpp test_packet_data.h $(COMMON_SOURCES)
test_rtcp_tip_mediaopts_packet_LDADD = $(COMMON_LDADD)

test_rtcp_tip_flowctrl_packet_SOURCES = test_rtcp_tip_flowctrl_packet.cpp test_packet_data.h $(COMMON_SOURCES)
test_rtcp_tip_flowctrl_packet_LDADD = $(COMMON_LDADD)

test_rtcp_tip_refresh_packet_SOURCES = test_rtcp_tip_refresh_packet.cpp test_packet_data.h $(COMMON_SOURCES)
test_rtcp_tip_refresh_packet_LDADD = $(COMMON_LDADD)

test_rtcp_packet_factory_SOURCES = test_rtcp_packet_factory.cpp test_packet_data.h $(COMMON_SOURCES)
test_rtcp_packet_factory_LDADD = $(COMMON_LDADD)

test_rtcp_tip_tlv_SOURCES = test_rtcp_tip_tlv.cpp $(COMMON_SOURCES)
test_rtcp_tip_tlv_LDADD = $(COMMON_LDADD)

test_rtcp_tip_packet_manager_SOURCES = test_rtcp_tip_packet_manager.cpp $(COMMON_SOURCES)
test_rtcp_tip_packet_manager_LDADD = $(COMMON_LDADD)

test_rtcp_tip_reqtosend_packet_SOURCES = test_rtcp_tip_reqtosend_packet.cpp test_packet_data.h $(COMMON_SOURCES)
test_rtcp_tip_reqtosend_packet_LDADD = $(COMMON_LDADD)

test_rtcp_tip_spimap_packet_SOURCES = test_rtcp_tip_spimap_packet.cpp test_packet_data.h $(COMMON_SOURCES)
test_rtcp_tip_spimap_packet_LDADD = $(COMMON_LDADD)

test_rtcp_tip_feedback_packet_SOURCES = test_rtcp_tip_feedback_packet.cpp test_packet_data.h $(COMMON_SOURCES)
test_rtcp_tip_feedback_packet_LDADD = $(COMMON_LDADD)

test_rtcp_tip_echo_packet_SOURCES = test_rtcp_tip_echo_packet.cpp test_packet_data.h $(COMMON_SOURCES)
test_rtcp_tip_echo_packet_LDADD = $(COMMON_LDADD)

test_rtcp_tip_notify_packet_SOURCES = test_rtcp_tip_notify_packet.cpp test_packet_data.h $(COMMON_SOURCES)
test_rtcp_tip_notify_packet_LDADD = $(COMMON_LDADD)

test_packet_buffer_SOURCES = test_packet_buffer.cpp test_packet_data.h $(COMMON_SOURCES)
test_packet_buffer_LDADD = $(COMMON_LDADD)
