bin_PROGRAMS = 	tip_pcap

tip_pcap_SOURCES = tip_pcap.cpp
tip_pcap_CPPFLAGS = -I$(top_srcdir)/lib/common/src -I$(top_srcdir)/lib/packet/src -I$(top_srcdir)/lib/user/src 
tip_pcap_LDADD = $(top_srcdir)/lib/user/src/libtipuser.la $(top_srcdir)/lib/packet/src/libtippacket.la $(top_srcdir)/lib/common/src/libtipcommon.la -lpcap
