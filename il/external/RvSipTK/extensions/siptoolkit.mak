ifeq ($(jsr), on)
include extensions/jsr32toolkit.inc
else

NON_EMBEDDED_OS := solaris linux hpux freebsd mac netbsd
EMBEDDED_LINUXES:= MontaVista_5.0 MontaVista_4.0 MontaVista_3.1 MontaVista_3.0 uclinux-2.4

# Find out if Test Application should be built
ifeq ($(TARGET_OS), $(findstring $(TARGET_OS),$(NON_EMBEDDED_OS)))
# -- for linuxes check OS_VERSION in order to determine embedded systems
ifeq ($(findstring $(TARGET_OS_VERSION),$(EMBEDDED_LINUXES)), )
ifneq ($(noapp), on)
testapp := on
endif
endif
endif

# Find out if tester applications should be built
ifeq ($(testers), on)
ifneq ($(depcore), on)
testerapps := on
endif
endif

# Basic components
ifneq ($(nosip), on)
PROJECTS += sip/rvsip.mak
endif

PROJECTS += ads/rvads.mak

# PROJECTS += sdp/rvsdp.mak
# If h323toolkit.mak is used (e.g. for simpleSessionSip323), +=rvrtp.mak should be commented to avoid a collision
#
# compile regular rtp or advanced rtp
# ifeq ($(artp), on)
# PROJECTS += rtpRtcp/artp/rvrtp.mak
# else
# PROJECTS += rtpRtcp/rtp/rvrtp.mak
# endif

### RTP and SDP will be built with the UH application; so, we don't build them here
ifeq ($(build-sdp), on)
PROJECTS += sdp/rvsdp.mak
offeranswer := on
endif
ifeq ($(build-rtp), on)
PROJECTS += rtpRtcp/rtp/rvrtp.mak
ifeq ($(build-srtp), on)
PROJECTS    +=	rtpRtcp/srtp/srtp.mak 
USER_CFLAGS += -DSRTP_ADD_ON
endif
endif


ifeq ($(depcore), on)
PROJECTS += depcore/rvdepcore.mak
endif

ifneq ($(wildcard sigcomp/),)
PROJECTS += sigcomp/rvsigcomp.mak
endif

ifeq ($(stun), on)
PROJECTS += stun/rvstun.mak
endif

# SIMPLE Client libarary
ifeq ($(simple), on)
PROJECTS += simpleClient/rvsimpleclt.mak
PROJECTS += eventDispatcher/rveventdispatcher.mak

# Force usage of XML library, even user didn't specified it
# the checking of ifeq ($(xml), ) prevent double compilation in case user used also xml=on
ifeq ($(xmlall), )
ifeq ($(xml), )
XML := on
PROJECTS += xmlAddOns/common/rvxmlcommon.mak
PROJECTS += xmlAddOns/addons/SIMPLE/rvxmlsimple.mak
endif   # xml
endif   # xmlall
endif   # simple

# Offer-Answer library
ifeq ($(offeranswer), on)
PROJECTS += offerAnswer/rvofferanswer.mak
endif

# CommonApp library
# It should be compiled only if Test Application or Tester/AutoTester applications are built
ifeq ($(testapp),on)
PROJECTS += appl/sip/commonapp/commonapp.mak
else
ifeq ($(testerapps),on)
PROJECTS += appl/sip/commonapp/commonapp.mak
else
ifeq ($(autotester), on)
PROJECTS += appl/sip/commonapp/commonapp.mak
else
ifeq ($(depcore), on)
PROJECTS += appl/sip/commonapp/commonapp.mak
endif
endif
endif
endif

# LUA library
# It should be compiled only if Test Application or AutoTester applications are built
ifeq ($(testapp),on)
PROJECTS += appl/sip/lua/lua.mak
else
ifeq ($(autotester), on)
PROJECTS += appl/sip/lua/lua.mak
endif
endif

# Test Application
ifeq ($(testapp),on)
PROJECTS += appl/sip/testapp/siptestapp.mak
endif

# Testers applications
ifeq ($(testerapps),on)
PROJECTS += testers/sip/performanceTest/tester.mak
PROJECTS += testers/sip/parserTest/tester.mak
PROJECTS += testers/sip/multiThreadTester/tester.mak
PROJECTS += testers/sip/simpleSessionSip323/tester.mak
endif
ifeq ($(depcore), on)
ifneq ($(wildcard /commonapp.mak),)
PROJECTS += appl/sip/commonapp/commonapp.mak
endif
PROJECTS += testers/sip/depcoreTester/tester.mak
endif

# autoTester applications
ifeq ($(autotester), on)
PROJECTS += autotester/client/basic_client/basic_client.mak
PROJECTS += autotester/client/sipPlugin/sipPlugin.mak
PROJECTS += autotester/client/sip_client/sipclient.mak
ifeq ($(xmlall), on)
PROJECTS += autotester/client/xmlPlugin/xmlPlugin.mak
PROJECTS += autotester/client/xml_client/xml_client.mak
endif
endif

# xmlAddOns applications
# --- global xml, add all xml projects
ifeq ($(xmlall), on)
ifeq ($(XML), )
PROJECTS += xmlAddOns/common/rvxmlcommon.mak
PROJECTS += xmlAddOns/addons/SIMPLE/rvxmlsimple.mak
endif
XML := on
PROJECTS += xmlAddOns/addons/XCAP/rvxmlxcap.mak
PROJECTS += xmlAddOns/addons/CONFERENCE/rvxmlconference.mak
PROJECTS += xmlAddOns/addons/CSTA/rvxmlcsta.mak
PROJECTS += xmlAddOns/addons/REG_EVENT/rvxmlregevent.mak

else
# --- user flag, add xml projects (no-MS)
ifeq ($(xml), on)
ifeq ($(XML), )
PROJECTS += xmlAddOns/common/rvxmlcommon.mak
PROJECTS += xmlAddOns/addons/SIMPLE/rvxmlsimple.mak
endif
XML := on
PROJECTS += xmlAddOns/addons/XCAP/rvxmlxcap.mak
PROJECTS += xmlAddOns/addons/CONFERENCE/rvxmlconference.mak
PROJECTS += xmlAddOns/addons/REG_EVENT/rvxmlregevent.mak
endif   # xml

# -- user flag, MS-xml projects
ifeq ($(xmlMs), on)
ifeq ($(XML), )
PROJECTS += xmlAddOns/common/rvxmlcommon.mak
endif
XML := on
PROJECTS += xmlAddOns/addons/CSTA/rvxmlcsta.mak
endif    # xmlMs
endif    # xmlall

# Sample applications
ifneq ($(nosamples), on)
PROJECTS += samples/sip/simpleProxy/sample.mak
PROJECTS += samples/sip/simpleServerAuth/sample.mak
PROJECTS += samples/sip/simpleTransaction/sample.mak
PROJECTS += samples/sip/simpleInvite/sample.mak

ifneq ($(primitives), on)
PROJECTS += samples/sip/simpleAuthentication/sample.mak
PROJECTS += samples/sip/simpleInfo/sample.mak
PROJECTS += samples/sip/simpleParserControl/sample.mak
PROJECTS += samples/sip/simplePersistentConnection/sample.mak
PROJECTS += samples/sip/simpleSession/sample.mak
PROJECTS += samples/sip/simpleSessionSDP/sample.mak
PROJECTS += samples/sip/simpleSIPT/sample.mak
PROJECTS += samples/sip/simpleSubscription/sample.mak
PROJECTS += samples/sip/simpleTransfer/sample.mak
PROJECTS += samples/sip/simpleRFC2833/sample.mak
PROJECTS += samples/sip/advancedTransfer/sample.mak
PROJECTS += samples/sip/simplePublish/sample.mak
PROJECTS += samples/sip/simpleRegistration/sample.mak
PROJECTS += samples/sip/simpleUpdate/sample.mak
PROJECTS += samples/sip/simpleTelSession/sample.mak
PROJECTS += samples/sip/simpleOfferAnswer/sample.mak
PROJECTS += samples/sip/simpleOfferAnswerPrecond/sample.mak

ifeq ($(sigcomp), on)
PROJECTS += samples/sip/simpleSigComp/sample.mak
PROJECTS += samples/sip/advancedSigComp/sample.mak
endif

ifeq ($(stun), on)
ifeq ($(stunSrv), on)
BINARIES_DIR := $(BINARIES_DIR)Srv
USER_CFLAGS += -DRV_STUN_SRV
PROJECTS += tools/stun/stunServer/stunServer.mak
else
PROJECTS += samples/stun/advancedStunClient/advancedStunClient.mak
ifneq ($(wildcard rtpRtcp/artp),)
PROJECTS += samples/stun/simpleStunSipRtp/simpleStunSipRtp.mak
endif
PROJECTS += tools/stun/stunAdvTester/stunAdvTester.mak
endif
endif

ifeq ($(srv), on)
PROJECTS += samples/sip/advancedDNSSession/sample.mak
endif

ifeq ($(tls), on)
PROJECTS += samples/sip/simpleTlsSession/sample.mak
PROJECTS += samples/sip/advancedTlsSession/sample.mak
PROJECTS += samples/sip/simpleConnectionReuse/sample.mak
endif

ifeq ($(ims), on)
PROJECTS += samples/sip/simpleAkaAuth/sample.mak
PROJECTS += samples/sip/simpleServiceRoute/sample.mak
ifeq ($(ipsec), on)
PROJECTS += samples/sip/simpleIMS/sampleUtils.mak
PROJECTS += samples/sip/simpleIMS/client/sample.mak
PROJECTS += samples/sip/simpleIMS/server/sample.mak
PROJECTS += quickStart/sip/quickIms/sampleUE.mak
PROJECTS += quickStart/sip/quickIms/quickImsUE/sample.mak
PROJECTS += quickStart/sip/quickIms/demoImsServer/sample.mak
endif
endif

ifeq ($(simple), on)
PROJECTS += samples/simpleClient/simpleSIMPLE/demoSimpleServer/sample.mak
PROJECTS += samples/simpleClient/simpleSIMPLE/simpleSimpleClient/sample.mak
PROJECTS += samples/simpleClient/simpleIM/sample.mak
endif

ifeq ($(xmlall), on)
PROJECTS += samples/xmlAddOns/simplePIDF/sample.mak
PROJECTS += samples/xmlAddOns/simpleWINFO/sample.mak
PROJECTS += samples/xmlAddOns/simpleXCAP/sample.mak
PROJECTS += samples/xmlAddOns/simpleCONFERENCE/sample.mak
PROJECTS += samples/xmlAddOns/simpleCSTA/sample.mak
PROJECTS += samples/xmlAddOns/simpleREGEVENT/sample.mak

else

ifeq ($(xml), on)
PROJECTS += samples/xmlAddOns/simplePIDF/sample.mak
PROJECTS += samples/xmlAddOns/simpleWINFO/sample.mak
PROJECTS += samples/xmlAddOns/simpleXCAP/sample.mak
PROJECTS += samples/xmlAddOns/simpleCONFERENCE/sample.mak
PROJECTS += samples/xmlAddOns/simpleREGEVENT/sample.mak
endif

ifeq ($(xmlMs), on)
PROJECTS += samples/xmlAddOns/simpleCSTA/sample.mak
endif    # xmlMs

endif    # xmlall

endif    # primitives
endif    # nosamples

ifeq ($(nolog), on)
# BINARIES_DIR := $(BINARIES_DIR)Nolog
USER_CFLAGS += -DRV_CFLAG_NOLOG
endif

ifeq ($(release), on)
USER_CFLAGS += -DRV_RELEASE
else
USER_CFLAGS += -DRV_DEBUG
endif

ifeq ($(primitives), on)
# BINARIES_DIR := $(BINARIES_DIR)Primitives
USER_CFLAGS += -DRV_SIP_PRIMITIVES
endif


ifeq ($(nothreads), on)
# BINARIES_DIR := $(BINARIES_DIR)Nothreads
USER_CFLAGS += -DRV_CFLAG_NOTHREADS
endif

ifeq ($(ipv6), on)
# BINARIES_DIR := $(BINARIES_DIR)Ipv6
USER_CFLAGS += -DRV_CFLAG_IPV6
endif


ifeq ($(srv), on)
# BINARIES_DIR := $(BINARIES_DIR)Dns
USER_CFLAGS += -DRV_DNS_ENHANCED_FEATURES_SUPPORT
endif

ifeq ($(tls), on)
TLS := on
# BINARIES_DIR := $(BINARIES_DIR)Tls
USER_CFLAGS += -DRV_CFLAG_TLS
endif

ifeq ($(ipsec), on)
ims := on
# BINARIES_DIR := $(BINARIES_DIR)IPsec
USER_CFLAGS += -DRV_CFLAG_IMS_IPSEC
SBR_FLAGS   += -DRV_CFLAG_IMS_IPSEC
endif

ifeq ($(ims), on)
#TLS := on
# BINARIES_DIR := $(BINARIES_DIR)Ims
USER_CFLAGS += -DRV_SIP_IMS_ON -DRV_SIGCOMP_ON
endif

ifeq ($(sctp), on)
SCTP := on
# BINARIES_DIR := $(BINARIES_DIR)Sctp
USER_CFLAGS += -DRV_CFLAG_SCTP
endif

ifeq ($(sigcomp), on)
USER_CFLAGS += -DRV_SIGCOMP_ON
# BINARIES_DIR := $(BINARIES_DIR)SigComp
endif

ifeq ($(simple), on)
XML := on
# BINARIES_DIR := $(BINARIES_DIR)Simple
USER_CFLAGS += -DCOMMONAPP_SIMPLE_ON -DRV_SIP_SUBS_ON
endif

#ifeq ($(offeranswer), on)
ifneq ($(primitives), on)
USER_CFLAGS += -DCOMMONAPP_OFFERANSWER_ON
endif
#endif

ifeq ($(depcore), on)
USER_CFLAGS += -DRV_DEPRECATED_CORE
endif

ifeq ($(BUILD_TYPE), debug)
USER_CFLAGS += -DSIP_DEBUG
endif

ifeq ($(poll), on)
USER_CFLAGS += -DRV_CFLAG_POLL
endif

ifeq ($(epoll), on)
USER_CFLAGS += -DRV_CFLAG_EPOLL
endif

ifeq ($(devpoll), on)
USER_CFLAGS += -DRV_CFLAG_DEVPOLL
endif

ifeq ($(rtp), on)
BINARIES_DIR := $(BINARIES_DIR)Rtp
USER_CFLAGS += -DRV_USE_RTP
endif


# The following flags are for internal usage
ifeq ($(transenh), on)
USER_CFLAGS += -DRV_SIP_TRANSPORT_ENHANCEMENTS
endif

ifeq ($(h323), on)
USER_CFLAGS += -DRV_USE_H323
endif


ifneq ($(SUNWS), on)
ifneq ($(TARGET_OS), integrity)
ifneq ($(MWCC), on)
USER_CFLAGS += -Wno-unused
endif
endif
endif

ifeq ($(testers), on)
USER_CFLAGS += -DRV_SIP_AUTH_ON -DRV_SIP_SUBS_ON
endif

# jsr=on
endif
