#include <cppunit/TestRunner.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/BriefTestProgressListener.h>

#include <ildaemon/ThreadSpecificStorage.h>
#include <base/LoadScheduler.h>
#include <boost/scoped_ptr.hpp>
#include <phxexception/PHXException.h>

#include "TestUtilities.h"
#include "VoIPCommon.h"
#include "CollectionOfEncodedFiles.h"
#include "MediaFilesCommon.h"
#include "EncodedMediaFile.h"
#include "MediaStreamInfo.h"
#include "MediaCodecs.h"
#include "MediaPayloads.h"
#include "MediaPacket.h"
#include "MediaCodecPreparator.h"

USING_MEDIA_NS;
USING_MEDIAFILES_NS;

///////////////////////////////////////////////////////////////////////////////

class TestMediaFileProcessor : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestMediaFileProcessor);
    CPPUNIT_TEST(testAMREncWithoutCSRC);
    CPPUNIT_TEST(testAMREncWithCSRC);
	CPPUNIT_TEST(testEncH264);
	CPPUNIT_TEST(testEncG711);
	CPPUNIT_TEST(testEncG723);
	CPPUNIT_TEST(testEncG726);
	CPPUNIT_TEST(testEncG711_1);	
	CPPUNIT_TEST(testEncG729);
    CPPUNIT_TEST_SUITE_END();


public:
    void generateUnitTestEncFile(uint8_t (&encDataArray)[30]);
	~TestMediaFileProcessor();
protected:
    void testAMREncWithoutCSRC(void);
    void testAMREncWithCSRC(void);
	void testEncH264(void);
	void testEncG711(void);
	void testEncG723(void);
	void testEncG711_1(void);
	void testEncG726(void);
	void testEncG729(void);
private:
	MediaStreamInfo streamInfo;
};

///////////////////////////////////////////////////////////////////////////////
TestMediaFileProcessor::~TestMediaFileProcessor()
{
    MediaFilePath rtpPath = streamInfo.getFilePath();
	FILE *fp = fopen(rtpPath.c_str(), "rb");
	if (fp)
	{
		fclose(fp);
		unlink(rtpPath.c_str());
	}
}

void TestMediaFileProcessor::testAMREncWithoutCSRC(void)
{   
   uint8_t armWithoutCsrcData[30]={0x00, 0x00, 0x00, 0x16, 0x0a, 0x0b, 0x0c, 0x0d, 0x80, 0xe2, 0xb2, 0x9e, 0x5f, 0xd5, 0xf1, 0x86, 0x1b, 0x27, 0x8c, 0xcb, 0xf0, 0x56, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
   streamInfo.setCodecAndPayloadNumber(AMRNB_0_OA_475, PAYLOAD_UNDEFINED);
   streamInfo.setFakeEncStreamGeneration(false);
   generateUnitTestEncFile(armWithoutCsrcData);

   CollectionOfEncodedFiles theFilesCollection;
   MediaCodecPreparator_CUSTOM custom;
   custom.prepareEncodedStream(&streamInfo, &theFilesCollection);
   EncodedMediaStreamIndex streamIndex = theFilesCollection.getStreamIndex(&streamInfo);
   
   _CPPUNIT_ASSERT(streamIndex != VOIP_NS::INVALID_STREAM_INDEX);
  
   EncodedMediaStreamHandler streamHandler = theFilesCollection.getEncodedStreamByIndex(streamIndex);
   MediaPacket* pkt = streamHandler->getPacket(0, 1);
   int payloadSize = pkt->getPayloadSize();

   // Verify RTP Header
   uint8_t* rtpHeader = pkt->getStartPtr();
   for (int i=0; i<12; i++) {
   		if (i==1) continue;
   		_CPPUNIT_ASSERT(rtpHeader[i] == armWithoutCsrcData[i+8]); 	
   	}
   
    // Verify body size
    _CPPUNIT_ASSERT(payloadSize == 10); 
}

///////////////////////////////////////////////////////////////////////////////

void TestMediaFileProcessor::testAMREncWithCSRC(void)
{
   uint8_t armWithCsrcData[30]={0x00, 0x00, 0x00, 0x16, 0x0a, 0x0b, 0x0c, 0x0d, 0x81, 0xe2, 0xfa, 0xf9, 0x3c, 0x7e, 0x3a, 0x9f, 0x01, 0x0f, 0xdd, 0xde, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xe6, 0x00, 0x00, 0x00, 0x00};
   streamInfo.setCodecAndPayloadNumber(AMRNB_0_OA_475, PAYLOAD_UNDEFINED);
   streamInfo.setFakeEncStreamGeneration(false);
   generateUnitTestEncFile(armWithCsrcData);

   CollectionOfEncodedFiles theFilesCollection;
   MediaCodecPreparator_CUSTOM custom;
   custom.prepareEncodedStream(&streamInfo, &theFilesCollection);
   EncodedMediaStreamIndex streamIndex = theFilesCollection.getStreamIndex(&streamInfo);
  
   _CPPUNIT_ASSERT(streamIndex != VOIP_NS::INVALID_STREAM_INDEX);

   EncodedMediaStreamHandler streamHandler = theFilesCollection.getEncodedStreamByIndex(streamIndex);
   MediaPacket* pkt = streamHandler->getPacket(0, 1);

   // Verify RTP Header
   uint8_t* rtpHeader = pkt->getStartPtr();
   for (int i=0; i<12; i++) {
	   	if (i==1) continue;
   		_CPPUNIT_ASSERT(rtpHeader[i] == armWithCsrcData[i+8]); 	
   	}
   
   // Verify body size
   int payloadSize = pkt->getPayloadSize();	    
    _CPPUNIT_ASSERT(payloadSize == 6); 	
}

///////////////////////////////////////////////////////////////////////////////

void TestMediaFileProcessor::testEncH264(void)
{
   uint8_t h264Data[30]={0x00, 0x00, 0x00, 0x16, 0x0a, 0x0b, 0x0c, 0x0d, 0x81, 0x72, 0xaf, 0x78, 0x01, 0x01, 0xf1, 0x3b, 0xd7, 0xcb, 0x70, 0x68, 0x27, 0x3f, 0xc0, 0x11, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00};
   streamInfo.setCodecAndPayloadNumber(VH264, PAYLOAD_711A);
   streamInfo.setFakeEncStreamGeneration(false);
   generateUnitTestEncFile(h264Data);

   CollectionOfEncodedFiles theFilesCollection;
   MediaCodecPreparator_H264 mediaCodecH264;
   mediaCodecH264.prepareEncodedStream(&streamInfo, &theFilesCollection);
   EncodedMediaStreamIndex streamIndex = theFilesCollection.getStreamIndex(&streamInfo);
  
   _CPPUNIT_ASSERT(streamIndex != VOIP_NS::INVALID_STREAM_INDEX);

   EncodedMediaStreamHandler streamHandler = theFilesCollection.getEncodedStreamByIndex(streamIndex);
   MediaPacket* pkt = streamHandler->getPacket(0, 1);

   // Verify RTP Header
   uint8_t* rtpHeader = pkt->getStartPtr();
   for (int i=0; i<12; i++) {
   		if (i==1) continue;
   		_CPPUNIT_ASSERT(rtpHeader[i] == h264Data[i+8]); 	
   	}
   
   // Verify body size
   int payloadSize = pkt->getPayloadSize();	    
    _CPPUNIT_ASSERT(payloadSize == 10); 	
}


///////////////////////////////////////////////////////////////////////////////

void TestMediaFileProcessor::testEncG711(void)
{   
   streamInfo.setCodecAndPayloadNumber(G711A, PAYLOAD_711A);

   CollectionOfEncodedFiles theFilesCollection;
   MediaCodecPreparator_G711 mediaCodecG711;
   mediaCodecG711.prepareEncodedStream(&streamInfo, &theFilesCollection);
   EncodedMediaStreamIndex streamIndex = theFilesCollection.getStreamIndex(&streamInfo);
  
   _CPPUNIT_ASSERT(streamIndex != VOIP_NS::INVALID_STREAM_INDEX);

   EncodedMediaStreamHandler streamHandler = theFilesCollection.getEncodedStreamByIndex(streamIndex);
   MediaPacket* pkt = streamHandler->getPacket(0, 1);
   
   // Verify body size
   int payloadSize = pkt->getPayloadSize();	

    _CPPUNIT_ASSERT(payloadSize > 0); 	
}

///////////////////////////////////////////////////////////////////////////////

void TestMediaFileProcessor::testEncG723(void)
{   
   streamInfo.setCodecAndPayloadNumber(G723_1, PAYLOAD_723);

   CollectionOfEncodedFiles theFilesCollection;
   MediaCodecPreparator_G723 mediaCodecG723;
   mediaCodecG723.prepareEncodedStream(&streamInfo, &theFilesCollection);
   EncodedMediaStreamIndex streamIndex = theFilesCollection.getStreamIndex(&streamInfo);
  
   _CPPUNIT_ASSERT(streamIndex != VOIP_NS::INVALID_STREAM_INDEX);

   EncodedMediaStreamHandler streamHandler = theFilesCollection.getEncodedStreamByIndex(streamIndex);
   MediaPacket* pkt = streamHandler->getPacket(0, 1);
   
   // Verify body size
   int payloadSize = pkt->getPayloadSize();	

    _CPPUNIT_ASSERT(payloadSize > 0); 	
}

///////////////////////////////////////////////////////////////////////////////

void TestMediaFileProcessor::testEncG726(void)
{   
   streamInfo.setCodecAndPayloadNumber(G726, PAYLOAD_726);

   CollectionOfEncodedFiles theFilesCollection;
   MediaCodecPreparator_G726 mediaCodecG726;
   mediaCodecG726.prepareEncodedStream(&streamInfo, &theFilesCollection);
   EncodedMediaStreamIndex streamIndex = theFilesCollection.getStreamIndex(&streamInfo);
  
   _CPPUNIT_ASSERT(streamIndex != VOIP_NS::INVALID_STREAM_INDEX);

   EncodedMediaStreamHandler streamHandler = theFilesCollection.getEncodedStreamByIndex(streamIndex);
   MediaPacket* pkt = streamHandler->getPacket(0, 1);
   
   // Verify body size
   int payloadSize = pkt->getPayloadSize();	

    _CPPUNIT_ASSERT(payloadSize > 0); 	
}

///////////////////////////////////////////////////////////////////////////////

void TestMediaFileProcessor::testEncG729(void)
{   
   streamInfo.setCodecAndPayloadNumber(G729, PAYLOAD_729);

   CollectionOfEncodedFiles theFilesCollection;
   MediaCodecPreparator_G729 mediaCodecG729;
   mediaCodecG729.prepareEncodedStream(&streamInfo, &theFilesCollection);
   EncodedMediaStreamIndex streamIndex = theFilesCollection.getStreamIndex(&streamInfo);
  
   _CPPUNIT_ASSERT(streamIndex != VOIP_NS::INVALID_STREAM_INDEX);

   EncodedMediaStreamHandler streamHandler = theFilesCollection.getEncodedStreamByIndex(streamIndex);
   MediaPacket* pkt = streamHandler->getPacket(0, 1);
   
   // Verify body size
   int payloadSize = pkt->getPayloadSize();	

    _CPPUNIT_ASSERT(payloadSize > 0); 	
}

///////////////////////////////////////////////////////////////////////////////

void TestMediaFileProcessor::testEncG711_1(void)
{   
   streamInfo.setCodecAndPayloadNumber(CODEC_TYPE_G711_1A, PAYLOAD_UNDEFINED);

   CollectionOfEncodedFiles theFilesCollection;
   MediaCodecPreparator_G711_1 mediaCodecG711_1;
   mediaCodecG711_1.prepareEncodedStream(&streamInfo, &theFilesCollection);
   EncodedMediaStreamIndex streamIndex = theFilesCollection.getStreamIndex(&streamInfo);
  
   _CPPUNIT_ASSERT(streamIndex != VOIP_NS::INVALID_STREAM_INDEX);

   EncodedMediaStreamHandler streamHandler = theFilesCollection.getEncodedStreamByIndex(streamIndex);
   MediaPacket* pkt = streamHandler->getPacket(0, 1);
   
   // Verify body size
   int payloadSize = pkt->getPayloadSize();	

    _CPPUNIT_ASSERT(payloadSize > 0); 	
}

///////////////////////////////////////////////////////////////////////////////

void TestMediaFileProcessor::generateUnitTestEncFile(uint8_t (&encDataArray)[30])
{		
	streamInfo.setFilePath("/tmp/");
    MediaFilePath rtpPath = streamInfo.getFilePath();

	FILE *fp = fopen(rtpPath.c_str(), "wb");
	
	_CPPUNIT_ASSERT(fp);
		
	fwrite(encDataArray, sizeof(encDataArray), 1, fp);			

	if (fp) fclose(fp);
}

///////////////////////////////////////////////////////////////////////////////

//CPPUNIT_TEST_SUITE_REGISTRATION(TestMediaFileProcessor);
CPPUNIT_TEST_SUITE_REGISTRATION(TestMediaFileProcessor);
