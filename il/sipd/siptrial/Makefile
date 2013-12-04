all: compile

RVSIP_ROOT = `pwd`/../../external/RvSipTK/
RVSIP_LIB_PATH = ${RVSIP_ROOT}/binaries/MontaVista_4.0_i386_pentium3/debug/lib/
RVSIP_INC_PATH = ${RVSIP_ROOT}/include/debug

MD5_PATH = `pwd`/../

STCSIP_PATH = `pwd`/../signaling/sip/rv/core/

clean:
		rm -f *.o siptrial
		@echo ======== Clean of siptrial done ======


compile:
	g++ -static -O2 -DUPDATED_BY_SPIRENT -DRVSIPPRINTF=printf -DRVSIPTRIAL \
	-I ${MD5_PATH} \
	-I ${RVSIP_INC_PATH}/common \
	-I ${RVSIP_INC_PATH}/sip \
	-I ${RVSIP_INC_PATH}/ads \
	-I ${RVSIP_INC_PATH}/sigcomp \
	-I ${RVSIP_INC_PATH}/common/ccore/netdrivers \
	-I ${RVSIP_INC_PATH}/depcore \
	-I ${RVSIP_INC_PATH}/sdp \
	-I ${STCSIP_PATH}/ \
	-o siptrial SipTrial.cpp \
	${STCSIP_PATH}/SipEngine.cpp ${STCSIP_PATH}/SipChannel.cpp \
	${STCSIP_PATH}/RvSipUtils.cpp ${STCSIP_PATH}/RvSipAka.cpp \
	${STCSIP_PATH}/SipCallSMBasic.cpp ${STCSIP_PATH}/SipCallSMTip.cpp \
	${MD5_PATH}/md5.c \
	${RVSIP_LIB_PATH}/librvsip.a \
	${RVSIP_LIB_PATH}/librvsigcomp.a \
	${RVSIP_LIB_PATH}/librvads.a \
	${RVSIP_LIB_PATH}/librvdepcore.a \
	${RVSIP_LIB_PATH}/librvrtp.a \
	${RVSIP_LIB_PATH}/librvsdp.a \
	${RVSIP_LIB_PATH}/librvcommon.a \
	-lssl -lcrypto -lz -lrt -ldl -lpthread
	@echo ======== Build of siptrial done =====

