#include <memory>
#include <string>

#include <ace/OS.h>
#include <boost/foreach.hpp>

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "HlsAbrPlaylist.h"

USING_ABR_NS;
USING_ABR_UTILS_NS;

///////////////////////////////////////////////////////////////////////////////

class TestAbr : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestAbr);
    CPPUNIT_TEST(testParseUri);
    CPPUNIT_TEST(testHlsPlaylistMimeTypes);
    CPPUNIT_TEST(testHlsAbrVariantPlaylist);
    CPPUNIT_TEST(testHlsAbrVodBitratePlaylist);
    CPPUNIT_TEST(testHlsAbrLiveBitratePlaylist);
    CPPUNIT_TEST(testHlsAbrProgressiveVariantPlaylist);
    CPPUNIT_TEST(testHlsAbrProgressiveSimpleBitratePlaylist);
    CPPUNIT_TEST(testHlsAbrProgressiveSegmentedBitratePlaylist);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) 
    {
        // Configurations used for playlist creation
        serverVersion = http_1_port_server::HttpVideoServerVersionOptions_VERSION_1_0;
        targetDuration = 10;
        streamType = http_1_port_server::HttpVideoServerStreamTypeOptions_ADAPTIVE_BITRATE;

        windowType = 0;
        mediaSequenceNumber = 1;
        maxSlidingWindowSize = 10;
        videoLength = 62;
        startTime = ACE_OS::gettimeofday();
        programId = 1;

        bitrateList.push_back(http_1_port_server::HttpVideoServerBitrateOptions_BR_64);
        bitrateList.push_back(http_1_port_server::HttpVideoServerBitrateOptions_BR_150);
        bitrateList.push_back(http_1_port_server::HttpVideoServerBitrateOptions_BR_240);

        hlsPlaylist.reset(new HlsAbrPlaylist(  serverVersion,
                                               targetDuration,
                                               streamType,
                                               windowType,
                                               mediaSequenceNumber,
                                               maxSlidingWindowSize,
                                               videoLength,
                                               startTime,
                                               bitrateList
                                              ));
        }
    void tearDown(void) { }
    
protected:
    void testParseUri(void);
    void testHlsPlaylistMimeTypes(void);
    void testHlsAbrVariantPlaylist(void);
    void testHlsAbrVodBitratePlaylist(void);
    void testHlsAbrLiveBitratePlaylist(void);
    void testHlsAbrProgressiveVariantPlaylist(void);
    void testHlsAbrProgressiveSimpleBitratePlaylist(void);
    void testHlsAbrProgressiveSegmentedBitratePlaylist(void);

private:
    uint8_t serverVersion;
    uint8_t targetDuration;
    uint8_t streamType;
    uint8_t windowType;
    uint16_t mediaSequenceNumber;
    uint16_t maxSlidingWindowSize;
    uint16_t videoLength;
    ACE_Time_Value startTime;
    uint8_t bitrate;
    videoServerBitrateList_t bitrateList;
    // serverType_t serverType;
    requestType_t requestType;
    sessionType_t sessionType;
    uint16_t mediaSequence;
    size_t programId;

    std::auto_ptr<HlsAbrPlaylist> hlsPlaylist;
};

///////////////////////////////////////////////////////////////////////////////

void TestAbr::testParseUri(void)
{
    std::string uri;
    bool success = false;
    bitrate = http_1_port_server::HttpVideoServerBitrateOptions_BR_64;
    mediaSequence = 0;

    // Test invalid request
    uri = "/non/existing/index.html";
    success = ParseUri(&uri, &sessionType, &requestType, &mediaSequence, &bitrate);

    CPPUNIT_ASSERT(success == false);

    // Test existing html request
    uri = "/1.html";
    success = ParseUri(&uri, &sessionType, &requestType, &mediaSequence, &bitrate);

    CPPUNIT_ASSERT(success == true);
    CPPUNIT_ASSERT(requestType == TEXT);
    CPPUNIT_ASSERT(mediaSequence == 0);
//    CPPUNIT_ASSERT(bitrate == BR_UNKNOWN);

    // Test VOD variant playlist request
    uri = "/VOD/spirentPlaylist.m3u8";
    success = ParseUri(&uri, &sessionType, &requestType, &mediaSequence, &bitrate);

    CPPUNIT_ASSERT(success == true);
    CPPUNIT_ASSERT(sessionType == VOD);
    CPPUNIT_ASSERT(requestType == VARIANT_PLAYLIST);
    CPPUNIT_ASSERT(mediaSequence == 0);
//    CPPUNIT_ASSERT(bitrate == BR_UNKNOWN);

    // Test LIVE variant playlist request
    uri = "/LIVE/spirentPlaylist.m3u8";
    success = ParseUri(&uri, &sessionType, &requestType, &mediaSequence, &bitrate);

    CPPUNIT_ASSERT(success == true);
    CPPUNIT_ASSERT(sessionType == LIVE);
    CPPUNIT_ASSERT(requestType == VARIANT_PLAYLIST);
    CPPUNIT_ASSERT(mediaSequence == 0);
//    CPPUNIT_ASSERT(bitrate == BR_UNKNOWN);

    // Test VOD bitrate playlist request
    uri = "/VOD/150k-video.m3u8";
    success = ParseUri(&uri, &sessionType, &requestType, &mediaSequence, &bitrate);

    CPPUNIT_ASSERT(success == true);
    CPPUNIT_ASSERT(sessionType == VOD);
    CPPUNIT_ASSERT(requestType == MANIFEST);
    CPPUNIT_ASSERT(mediaSequence == 0);
    CPPUNIT_ASSERT(bitrate == http_1_port_server::HttpVideoServerBitrateOptions_BR_150);

    // Test LIVE bitrate playlist request
    uri = "/LIVE/150k-video.m3u8";
    success = ParseUri(&uri, &sessionType, &requestType, &mediaSequence, &bitrate);

    CPPUNIT_ASSERT(success == true);
    CPPUNIT_ASSERT(sessionType == LIVE);
    CPPUNIT_ASSERT(requestType == MANIFEST);
    CPPUNIT_ASSERT(mediaSequence == 0);
    CPPUNIT_ASSERT(bitrate == http_1_port_server::HttpVideoServerBitrateOptions_BR_150);
    
    // Test VOD bitrate playlist audio-only request
    uri = "/VOD/64k-audio-only.m3u8";
    success = ParseUri(&uri, &sessionType, &requestType, &mediaSequence, &bitrate);

    CPPUNIT_ASSERT(success == true);
    CPPUNIT_ASSERT(sessionType == VOD);
    CPPUNIT_ASSERT(requestType == MANIFEST);
    CPPUNIT_ASSERT(mediaSequence == 0);
    CPPUNIT_ASSERT(bitrate == http_1_port_server::HttpVideoServerBitrateOptions_BR_64);

    // Test LIVE bitrate playlist audio-only request
    uri = "/LIVE/64k-audio-only.m3u8";
    success = ParseUri(&uri, &sessionType, &requestType, &mediaSequence, &bitrate);

    CPPUNIT_ASSERT(success == true);
    CPPUNIT_ASSERT(sessionType == LIVE);
    CPPUNIT_ASSERT(requestType == MANIFEST);
    CPPUNIT_ASSERT(mediaSequence == 0);
    CPPUNIT_ASSERT(bitrate == http_1_port_server::HttpVideoServerBitrateOptions_BR_64);

    // Test VOD fragment request
    uri = "/VOD/150k/HLSvideo-1.ts";
    success = ParseUri(&uri, &sessionType, &requestType, &mediaSequence, &bitrate);

    CPPUNIT_ASSERT(success == true);
    CPPUNIT_ASSERT(sessionType == VOD);
    CPPUNIT_ASSERT(requestType == FRAGMENT);
    CPPUNIT_ASSERT(mediaSequence == 1);
    CPPUNIT_ASSERT(bitrate == http_1_port_server::HttpVideoServerBitrateOptions_BR_150);

    // Test LIVE fragment request
    uri = "/LIVE/150k/HLSvideo-1.ts";
    success = ParseUri(&uri, &sessionType, &requestType, &mediaSequence, &bitrate);

    CPPUNIT_ASSERT(success == true);
    CPPUNIT_ASSERT(sessionType == LIVE);
    CPPUNIT_ASSERT(requestType == FRAGMENT);
    CPPUNIT_ASSERT(mediaSequence == 1);
    CPPUNIT_ASSERT(bitrate == http_1_port_server::HttpVideoServerBitrateOptions_BR_150);

    // Test VOD audio-only fragment request
    uri = "/VOD/64k-audio/HLSvideo-1.ts";
    success = ParseUri(&uri, &sessionType, &requestType, &mediaSequence, &bitrate);

    CPPUNIT_ASSERT(success == true);
    CPPUNIT_ASSERT(sessionType == VOD);
    CPPUNIT_ASSERT(requestType == FRAGMENT);
    CPPUNIT_ASSERT(mediaSequence == 1);
    CPPUNIT_ASSERT(bitrate == http_1_port_server::HttpVideoServerBitrateOptions_BR_64);

    // Test LIVE audio-only fragment request
    uri = "/LIVE/64k-audio/HLSvideo-1.ts";
    success = ParseUri(&uri, &sessionType, &requestType, &mediaSequence, &bitrate);

    CPPUNIT_ASSERT(success == true);
    CPPUNIT_ASSERT(sessionType == LIVE);
    CPPUNIT_ASSERT(requestType == FRAGMENT);
    CPPUNIT_ASSERT(mediaSequence == 1);
    CPPUNIT_ASSERT(bitrate == http_1_port_server::HttpVideoServerBitrateOptions_BR_64);

    // Test VOD progressive, simple playlist type, fragment request
    uri = "/VOD/150k/HLSvideo-entire.ts";
    mediaSequence = 0;                        // Reset media sequence to observe it won't change
    success = ParseUri(&uri, &sessionType, &requestType, &mediaSequence, &bitrate);

    CPPUNIT_ASSERT(success == true);
    CPPUNIT_ASSERT(sessionType == VOD);
    CPPUNIT_ASSERT(requestType == FRAGMENT);
    CPPUNIT_ASSERT(mediaSequence == 0);
    CPPUNIT_ASSERT(bitrate == http_1_port_server::HttpVideoServerBitrateOptions_BR_150);

    // Test LIVE progressive, simple playlist type, fragment request
    uri = "/LIVE/150k/HLSvideo-entire.ts";
    success = ParseUri(&uri, &sessionType, &requestType, &mediaSequence, &bitrate);

    CPPUNIT_ASSERT(success == true);
    CPPUNIT_ASSERT(sessionType == LIVE);
    CPPUNIT_ASSERT(requestType == FRAGMENT);
    CPPUNIT_ASSERT(mediaSequence == 0);
    CPPUNIT_ASSERT(bitrate == http_1_port_server::HttpVideoServerBitrateOptions_BR_150);

    // Test VOD progressive, simple playlist type, audio fragment request
    uri = "/VOD/64k-audio/HLSvideo-entire.ts";
    success = ParseUri(&uri, &sessionType, &requestType, &mediaSequence, &bitrate);

    CPPUNIT_ASSERT(success == true);
    CPPUNIT_ASSERT(sessionType == VOD);
    CPPUNIT_ASSERT(requestType == FRAGMENT);
    CPPUNIT_ASSERT(mediaSequence == 0);
    CPPUNIT_ASSERT(bitrate == http_1_port_server::HttpVideoServerBitrateOptions_BR_64);

    // Test VOD progressive, simple playlist type, audio fragment request
    uri = "/LIVE/64k-audio/HLSvideo-entire.ts";
    success = ParseUri(&uri, &sessionType, &requestType, &mediaSequence, &bitrate);

    CPPUNIT_ASSERT(success == true);
    CPPUNIT_ASSERT(sessionType == LIVE);
    CPPUNIT_ASSERT(requestType == FRAGMENT);
    CPPUNIT_ASSERT(mediaSequence == 0);
    CPPUNIT_ASSERT(bitrate == http_1_port_server::HttpVideoServerBitrateOptions_BR_64);
}

///////////////////////////////////////////////////////////////////////////////

void TestAbr::testHlsPlaylistMimeTypes(void)
{
    CPPUNIT_ASSERT(hlsPlaylist->GetPlaylistMimeType() == "application/x-mpegURL");
    CPPUNIT_ASSERT(hlsPlaylist->GetFragmentMimeType() == "video/MP2T");
}

///////////////////////////////////////////////////////////////////////////////

void TestAbr::testHlsAbrVariantPlaylist(void)
{
    std::stringstream oss;

    oss << "#EXTM3U\n"
        << "#EXT-X-STREAM-INF:PROGRAM-ID=" << programId << ",BANDWIDTH=64000\n"
        << "64k-audio-only.m3u8\n"
        << "#EXT-X-STREAM-INF:PROGRAM-ID=" << programId << ",BANDWIDTH=150000\n"
        << "150k-video.m3u8\n"
        << "#EXT-X-STREAM-INF:PROGRAM-ID=" << programId << ",BANDWIDTH=240000\n"
        << "240k-video.m3u8\n";

    CPPUNIT_ASSERT(*(hlsPlaylist->GetVodVariantPlaylist()) == oss.str());
    CPPUNIT_ASSERT(*(hlsPlaylist->GetLiveVariantPlaylist()) == oss.str());
}

///////////////////////////////////////////////////////////////////////////////

void TestAbr::testHlsAbrVodBitratePlaylist(void)
{
    size_t fragmentCount = videoLength / targetDuration;
    size_t remainingSeconds = videoLength % targetDuration;
    size_t count;

    BOOST_FOREACH(uint8_t bitrate, bitrateList)
    {
        std::stringstream oss;

        oss << "#EXTM3U\n"
            << "#EXT-X-VERSION:" << VersionToInteger(serverVersion) << "\n"
            << "#EXT-X-TARGETDURATION:" << (uint16_t)targetDuration << "\n"
            << "#EXT-X-MEDIA-SEQUENCE:" << mediaSequenceNumber << "\n";

        for(count = 0; count < fragmentCount; count++)
        {
            oss << "#EXTINF:" << (uint16_t)targetDuration << ",\n"
                << BitrateToString(bitrate) << "/HLSvideo-" << mediaSequenceNumber + count << ".ts\n";
        }

        if(remainingSeconds)
        {
            oss << "#EXTINF:" << remainingSeconds<< ",\n"
                << BitrateToString(bitrate) << "/HLSvideo-" << mediaSequenceNumber + count << ".ts\n";  
        }

        oss << "#EXT-X-ENDLIST\n"; 
       
        CPPUNIT_ASSERT(*(hlsPlaylist->GetPlaylistFromBitrateMapAt(bitrate)) == oss.str());  
    }  
}

///////////////////////////////////////////////////////////////////////////////

void TestAbr::testHlsAbrLiveBitratePlaylist(void)
{
    size_t timeToAdd;
    ACE_Time_Value *nowTime = new ACE_Time_Value(ACE_OS::gettimeofday());


    BOOST_FOREACH(uint8_t bitrate, bitrateList)
    {
        std::stringstream oss;

        /// Test initial playlist, there should be three entries
        oss << "#EXTM3U\n"
            << "#EXT-X-VERSION:" << VersionToInteger(serverVersion) << "\n"
            << "#EXT-X-TARGETDURATION:" << (uint16_t)targetDuration << "\n"
            << "#EXT-X-MEDIA-SEQUENCE:" << mediaSequenceNumber << "\n"
            << "#EXTINF:" << (uint16_t)targetDuration << ",\n"
            << BitrateToString(bitrate) << "/HLSvideo-" << mediaSequenceNumber << ".ts\n"
            << "#EXTINF:" << (uint16_t)targetDuration << ",\n"
            << BitrateToString(bitrate) << "/HLSvideo-" << mediaSequenceNumber + 1 << ".ts\n"
            << "#EXTINF:" << (uint16_t)targetDuration << ",\n"
            << BitrateToString(bitrate) << "/HLSvideo-" << mediaSequenceNumber + 2 << ".ts\n"; 
       
        CPPUNIT_ASSERT(hlsPlaylist->CreateAbrLiveBitratePlaylist(bitrate, nowTime) == oss.str());  

        /// After five target durations, there should be additional five fragments
        // increase current time by five target durations
        timeToAdd = 5 * targetDuration;
        ACE_Time_Value newTime(nowTime->sec() + timeToAdd);

        oss << "#EXTINF:" << (uint16_t)targetDuration << ",\n"
            << BitrateToString(bitrate) << "/HLSvideo-" << mediaSequenceNumber + 3 << ".ts\n" 
            << "#EXTINF:" << (uint16_t)targetDuration << ",\n"
            << BitrateToString(bitrate) << "/HLSvideo-" << mediaSequenceNumber + 4 << ".ts\n" 
            << "#EXTINF:" << (uint16_t)targetDuration << ",\n"
            << BitrateToString(bitrate) << "/HLSvideo-" << mediaSequenceNumber + 5 << ".ts\n" 
            << "#EXTINF:" << (uint16_t)targetDuration << ",\n"
            << BitrateToString(bitrate) << "/HLSvideo-" << mediaSequenceNumber + 6 << ".ts\n"
            << "#EXTINF:" << (uint16_t)targetDuration << ",\n"
            << BitrateToString(bitrate) << "/HLSvideo-" << mediaSequenceNumber + 7 << ".ts\n"; 

        CPPUNIT_ASSERT(hlsPlaylist->CreateAbrLiveBitratePlaylist(bitrate, &newTime) == oss.str()); 
    }  
}

///////////////////////////////////////////////////////////////////////////////

void TestAbr::testHlsAbrProgressiveVariantPlaylist(void)
{
    std::stringstream oss;

    HlsAbrPlaylist progressivePlaylist( serverVersion,
                                        videoLength,        // Set target duration to match videoLength
                                        http_1_port_server::HttpVideoServerStreamTypeOptions_PROGRESSIVE,        // Set as progressive
                                        windowType,
                                        mediaSequenceNumber,
                                        maxSlidingWindowSize,
                                        videoLength,
                                        startTime,
                                        bitrateList         // There are three bitrates, but only first will be used
                                      );
   
    oss << "#EXTM3U\n"
        << "#EXT-X-STREAM-INF:PROGRAM-ID=" << programId << ",BANDWIDTH=64000\n"
        << "64k-audio-only.m3u8\n";


    CPPUNIT_ASSERT(*(progressivePlaylist.GetVodVariantPlaylist()) == oss.str());
    CPPUNIT_ASSERT(*(progressivePlaylist.GetLiveVariantPlaylist()) == "");
}

///////////////////////////////////////////////////////////////////////////////

///TODO: MAY NEED TO MODIFY:  How will progressive simple be identified in future?  When target duration is not set?
void TestAbr::testHlsAbrProgressiveSimpleBitratePlaylist(void)
{
    std::stringstream oss;

    HlsAbrPlaylist progressivePlaylist( serverVersion,
                                        videoLength,        // Set target duration to match videoLength
                                        http_1_port_server::HttpVideoServerStreamTypeOptions_PROGRESSIVE,        // Set as progressive
                                        windowType,
                                        mediaSequenceNumber,
                                        maxSlidingWindowSize,
                                        videoLength,
                                        startTime,
                                        bitrateList         // There are three bitrates, but only first will be used
                                      );
   
    oss << "#EXTM3U\n"
        << "#EXT-X-VERSION:" << VersionToInteger(serverVersion) << "\n"
        << "#EXT-X-TARGETDURATION:" << videoLength << "\n"
        << "#EXTINF:" << videoLength << ",\n"
        << BitrateToString(bitrateList.front()) << "/HLSvideo-entire.ts\n"
        << "#EXT-X-ENDLIST\n"; 

    CPPUNIT_ASSERT(*(progressivePlaylist.GetPlaylistFromBitrateMapAt(bitrateList.front())) == oss.str());  
}

///////////////////////////////////////////////////////////////////////////////

void TestAbr::testHlsAbrProgressiveSegmentedBitratePlaylist(void)
{
    std::stringstream oss;
    size_t fragmentCount = videoLength / targetDuration;
    size_t remainingSeconds = videoLength % targetDuration;
    size_t count;


    HlsAbrPlaylist progressivePlaylist( serverVersion,
                                        targetDuration,     // Set target duration to match videoLength
                                        http_1_port_server::HttpVideoServerStreamTypeOptions_PROGRESSIVE,        // Set as progressive
                                        windowType,
                                        mediaSequenceNumber,
                                        maxSlidingWindowSize,
                                        videoLength,
                                        startTime,
                                        bitrateList         // There are three bitrates, but only first will be used
                                      );

    oss << "#EXTM3U\n"
        << "#EXT-X-VERSION:" << VersionToInteger(serverVersion) << "\n"
        << "#EXT-X-TARGETDURATION:" << (uint16_t)targetDuration << "\n"
        << "#EXT-X-MEDIA-SEQUENCE:" << mediaSequenceNumber << "\n";

    for(count = 0; count < fragmentCount; count++)
    {
        oss << "#EXTINF:" << (uint16_t)targetDuration << ",\n"
            << BitrateToString(bitrateList.front()) << "/HLSvideo-" << mediaSequenceNumber + count << ".ts\n";
    }

    if(remainingSeconds)
    {
        oss << "#EXTINF:" << remainingSeconds<< ",\n"
            << BitrateToString(bitrateList.front()) << "/HLSvideo-" << mediaSequenceNumber + count << ".ts\n";  
    }

    oss << "#EXT-X-ENDLIST\n"; 

    CPPUNIT_ASSERT(*(progressivePlaylist.GetPlaylistFromBitrateMapAt(bitrateList.front())) == oss.str());  
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestAbr);
