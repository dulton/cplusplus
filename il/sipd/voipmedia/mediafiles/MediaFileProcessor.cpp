/// @file
/// @brief Media Files processor classes
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <map>

#include <boost/scoped_array.hpp>

#include "MediaFilesManager.h"

#include "MediaPacket.h"
#include "MediaStreamInfo.h"
#include "EncodedMediaFile.h"
#include "CollectionOfEncodedFiles.h"
#include "MediaCodecPreparator.h"
#include "MediaCodecs.h"

///////////////////////////////////////////////////////////////////////////////

DECL_MEDIAFILES_NS

/////////////////////////////////////////////////////////////

EncodedMediaFile::EncodedMediaFile(const MediaStreamInfo *streamInfo) {
  mStreamInfo.reset(new MediaStreamInfo(streamInfo));
  VoIPUtils::calculateCPUNumber(mNumberOfCpus);
  mPkts = 0;
}

EncodedMediaFile::~EncodedMediaFile() {
  ;
}

const MediaStreamInfo* EncodedMediaFile::getStreamInfo() const {
  return mStreamInfo.get();
}

void EncodedMediaFile::addPacket(MediaPacketHandler packet) {
  if(packet) {
    mFile.push_back(packet);
    mPkts++;
    if(mNumberOfCpus > 1){
        for(uint32_t i=0;i<mNumberOfCpus-1;i++){
            MediaPacketHandler pkt;
            pkt.reset( new MediaPacket(*(packet.get())));
            mFile.push_back(pkt);
        }
    }
  }
}

int EncodedMediaFile::getSize() const {
  return mPkts;
}

MediaPacket* EncodedMediaFile::getPacket(int index,int cpu) const {
  uint32_t fsize=mPkts;
  if(fsize>0 && index>=0 && index<fsize && cpu >= 0 && cpu < mNumberOfCpus) {
    return mFile[index*mNumberOfCpus + cpu].get();
  } else {
    static MediaPacketHandler packet;
    return packet.get();
  }
}

///////////////////////////////////////////////////////////////////////////////

class CollectionOfEncodedFilesImpl : public boost::noncopyable {
public:

  typedef std::map<EncodedMediaStreamIndex,EncodedMediaStreamHandler> IndexVsHandlerType;
  typedef std::map<MediaStreamInfo,EncodedMediaStreamIndex> InfoVsIndexType;

  CollectionOfEncodedFilesImpl() : mCurrentIndex(0) {
    ;
  }
  virtual ~CollectionOfEncodedFilesImpl() {
    ;
  }
  EncodedMediaStreamIndex getStreamIndex(const MediaStreamInfo *streamInfo) const {
    if(streamInfo) {
      InfoVsIndexType::const_iterator iter=mInfoVsIndex.find(*streamInfo);
      if(iter != mInfoVsIndex.end()) {
	return iter->second;
      }
    }
    return INVALID_STREAM_INDEX;
  }
  EncodedMediaStreamIndex addEncodedStream(EncodedMediaStreamHandler streamHandler) {
    if(streamHandler) {
      EncodedMediaStreamIndex newIndex=mCurrentIndex++;
      const MediaStreamInfo *si = streamHandler->getStreamInfo();
      if(si) {
	mInfoVsIndex[*si] = newIndex;
	mIndexVsHandler[newIndex]=streamHandler;
	return newIndex;
      }
    }
    return INVALID_STREAM_INDEX;
  }
  EncodedMediaStreamHandler getEncodedStreamByIndex(EncodedMediaStreamIndex index) const {
    EncodedMediaStreamHandler h;
    IndexVsHandlerType::const_iterator iter=mIndexVsHandler.find(index);
    if(iter!=mIndexVsHandler.end()) {
      h=iter->second;
    }
    return h;
  }
  void removeEncodedStream(EncodedMediaStreamIndex index) {
    IndexVsHandlerType::iterator iter=mIndexVsHandler.find(index);
    if(iter!=mIndexVsHandler.end()) {
      mIndexVsHandler.erase(iter);
    }
  }
  void removeAllEncodedStreams() {
    mInfoVsIndex.clear();
    mIndexVsHandler.clear();
  }

 private:
  EncodedMediaStreamIndex mCurrentIndex;
  IndexVsHandlerType mIndexVsHandler;
  InfoVsIndexType mInfoVsIndex;
};

CollectionOfEncodedFiles::CollectionOfEncodedFiles() {
  mPimpl.reset(new CollectionOfEncodedFilesImpl());
}
CollectionOfEncodedFiles::~CollectionOfEncodedFiles() {
  ;
}
EncodedMediaStreamIndex CollectionOfEncodedFiles::getStreamIndex(const MediaStreamInfo *streamInfo) const {
  return mPimpl->getStreamIndex(streamInfo);
}
EncodedMediaStreamHandler CollectionOfEncodedFiles::getEncodedStreamByIndex(EncodedMediaStreamIndex index) const {
  return mPimpl->getEncodedStreamByIndex(index);
}
EncodedMediaStreamIndex CollectionOfEncodedFiles::addEncodedStream(EncodedMediaStreamHandler streamHandler) {
  return mPimpl->addEncodedStream(streamHandler);
}
void CollectionOfEncodedFiles::removeEncodedStream(EncodedMediaStreamIndex index) {
  mPimpl->removeEncodedStream(index);
}
void CollectionOfEncodedFiles::removeAllEncodedStreams() {
  mPimpl->removeAllEncodedStreams();
}

///////////////////////////////////////////////////////////////////////////////

class MediaFilesManagerImpl : public boost::noncopyable {

public:

  typedef boost::shared_ptr<MediaCodecPreparator> MediaCodecPreparatorHandler;
  typedef std::map<CodecGroupType, int> CodecGroupVsIndex;

  MediaFilesManagerImpl() {
    mFiles.reset(new CollectionOfEncodedFiles);
    mCodecEncoders.clear();
    mCodecGroupVsIndex.clear();
    MediaCodecPreparatorHandler p0(new MediaCodecPreparator_G711());
    mCodecEncoders.push_back(p0);
    MediaCodecPreparatorHandler p1(new MediaCodecPreparator_G723());
    mCodecEncoders.push_back(p1);
    MediaCodecPreparatorHandler p2(new MediaCodecPreparator_G726());
    mCodecEncoders.push_back(p2);
    MediaCodecPreparatorHandler p3(new MediaCodecPreparator_G729());
    mCodecEncoders.push_back(p3);
    MediaCodecPreparatorHandler p4(new MediaCodecPreparator_G711_1());
    mCodecEncoders.push_back(p4);
    MediaCodecPreparatorHandler p5(new MediaCodecPreparator_CUSTOM());
    mCodecEncoders.push_back(p5);
    MediaCodecPreparatorHandler p6(new MediaCodecPreparator_H264());
    mCodecEncoders.push_back(p6);

    for(unsigned int i=0; i<mCodecEncoders.size(); i++)
        mCodecGroupVsIndex [mCodecEncoders[i]->getCodecGroup()] = i;
  }

  virtual ~MediaFilesManagerImpl() {
    ;
  }

  EncodedMediaStreamIndex prepareEncodedStream(const MediaStreamInfo *streamInfo) {

    EncodedMediaStreamIndex index = INVALID_STREAM_INDEX;
    if(streamInfo)
    {
        CodecGroupType codecGroup = streamInfo->getCodecGroup();
        if( ! mCodecGroupVsIndex.empty())
        {
            CodecGroupVsIndex::iterator it;
            it = mCodecGroupVsIndex.find(codecGroup);
            if( it != mCodecGroupVsIndex.end() )
                index = mCodecEncoders[it->second]->prepareEncodedStream(streamInfo,mFiles.get());
        }
    }
    return index;
  }

  EncodedMediaStreamHandler getEncodedStreamByIndex(EncodedMediaStreamIndex index) {
    return mFiles->getEncodedStreamByIndex(index);
  }

  EncodedMediaStreamIndex getStreamIndex(const MediaStreamInfo *streamInfo) const {
    return mFiles->getStreamIndex(streamInfo);
  }

  void removeEncodedStream(EncodedMediaStreamIndex index) {
    mFiles->removeEncodedStream(index);
  }

  void removeAllEncodedStreams() {
    mFiles->removeAllEncodedStreams();
  }

private:
  std::vector<MediaCodecPreparatorHandler> mCodecEncoders;
  boost::scoped_ptr<CollectionOfEncodedFiles> mFiles;
  CodecGroupVsIndex mCodecGroupVsIndex;
};

MediaFilesManager::MediaFilesManager() {
  mPimpl.reset(new MediaFilesManagerImpl);
}

MediaFilesManager::~MediaFilesManager() {
  ;
}

EncodedMediaStreamIndex MediaFilesManager::prepareEncodedStream(const MediaStreamInfo *streamInfo) {
  return mPimpl->prepareEncodedStream(streamInfo);
}

EncodedMediaStreamHandler MediaFilesManager::getEncodedStreamByIndex(EncodedMediaStreamIndex index) const {
  return mPimpl->getEncodedStreamByIndex(index);
}

EncodedMediaStreamIndex MediaFilesManager::getStreamIndex(const MediaStreamInfo *streamInfo) const {
  return mPimpl->getStreamIndex(streamInfo);
}

void MediaFilesManager::removeEncodedStream(EncodedMediaStreamIndex index) {
  mPimpl->removeEncodedStream(index);
}

void MediaFilesManager::removeAllEncodedStreams() {
  mPimpl->removeAllEncodedStreams();
}
///////////////////////////////////////////////////////////////////////////////
void MediaCodecPreparator::addPacket(EncodedMediaFile *file, MediaPacketHandler packet) {
  if(file) file->addPacket(packet);
}
uint32_t MediaCodecPreparator :: getRawStreamSize(const MediaStreamInfo *streamInfo )
{
    uint32_t size = 0;
    FILE * fp;

    if( ( fp = fopen(streamInfo->getFilePath().c_str(), "rb") ) != NULL )
    {

        if ( fseek(fp,0L,SEEK_END) == 0 )
        {
            size = ftell(fp);
            if( size <= RAWDATA_HEADER_SIZE)
                size = 0;
            else
                size -= RAWDATA_HEADER_SIZE;
        }
        fclose(fp);
    }
    return size;
}
uint32_t MediaCodecPreparator :: getRawData(const MediaStreamInfo *streamInfo, uint8_t *pEnc,  uint32_t size )
{
    FILE * fp;

    if( ( fp = fopen(streamInfo->getFilePath().c_str(), "rb") ) != NULL )
    {
        if ( fseek(fp,RAWDATA_HEADER_SIZE,SEEK_SET) == 0 )
        {
            size =(uint32_t)fread(pEnc, sizeof(uint8_t), size, fp);
        }
        fclose(fp);
    }
    else
        size = 0;

    return size;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

EncodedMediaStreamIndex MediaCodecPreparator_CUSTOM::prepareEncodedStream(const MediaStreamInfo *theStreamInfo,
        CollectionOfEncodedFiles* theFilesCollection)
{ 
	if (theStreamInfo->getFakeEncStreamGeneration()){   
		bool OK;
		RtpHeaderParam	   aRtpHeader;
		uint32_t aNumPacket = 0;
		uint32_t aPacketLength		  = FAKE_RTP_SIZE;
		uint32_t aPacketTransmitTime  = 0;
		uint8_t aPayload = 0xFF;
	
		EncodedMediaStreamHandler anEncodedMediaFileSharedPtr (new EncodedMediaFile(theStreamInfo));
		while (aNumPacket < FAKE_PACKET_COUNT)
		{
		   MediaPacketHandler aMediaPacketSharedPtr(new MediaPacket(aPacketLength));
		   
		   memset(aMediaPacketSharedPtr->getStartPtr(), aPayload, aPacketLength);
		   
		   aMediaPacketSharedPtr->setPayloadSize(aPacketLength-12);
		   aMediaPacketSharedPtr->setTime(aPacketTransmitTime);
	
		   //place to manipulate with RTP Header  
		   aMediaPacketSharedPtr->getRtpHeaderParam (aRtpHeader);
		   aRtpHeader.rtp_pl=theStreamInfo->getPayloadNumber();
		   aMediaPacketSharedPtr->setRtpHeaderParam (aRtpHeader);
	
		   anEncodedMediaFileSharedPtr->addPacket(aMediaPacketSharedPtr);
		   
		   aPacketTransmitTime += FAKE_PACKET_TRANSMIT_TIME;
		   aNumPacket++;
		}
	
		OK = true;
		
		if(OK)
			return theFilesCollection->addEncodedStream(anEncodedMediaFileSharedPtr);
		else
			return INVALID_STREAM_INDEX;
	}
	else  {

	    FILE* 	 aFile;
	    uint8_t  aPacketHeader[8];
	    uint8_t  aPacketContent[MAXSIZE_OF_RTPPACKET];
	    uint32_t aPacketLength;
	    uint32_t aPacketTransmitTime;
	    uint32_t aNumberOfBytes;

	    bool      OK;


	    RtpHeaderParam     aRtpHeader;
	    EncodedMediaStreamHandler anEncodedMediaFileSharedPtr (new EncodedMediaFile(theStreamInfo));
				
	    aFile = fopen(theStreamInfo->getFilePath().c_str(), "rb");

	    OK = (aFile!=NULL);

	    if( OK )
	    {
	        aNumberOfBytes = fread(&aPacketHeader,1, 8, aFile);

	        aPacketLength        = (aPacketHeader[0]<<24) + (aPacketHeader[1]<<16) + (aPacketHeader[2]<<8) +(aPacketHeader[3]<<0);
	        aPacketTransmitTime  = (aPacketHeader[4]<<24) + (aPacketHeader[5]<<16) + (aPacketHeader[6]<<8) +(aPacketHeader[7]<<0);

	        OK = ( aNumberOfBytes == (uint32_t)8 ) && (aPacketLength <= (uint32_t)MAXSIZE_OF_RTPPACKET);			
	        while (!feof(aFile) && OK)
	        {
	            aNumberOfBytes = fread(aPacketContent, 1, aPacketLength, aFile);
	            OK = (aNumberOfBytes == aPacketLength);

	            if (OK)
	            {
	            	MediaPacketHandler aMediaPacketSharedPtr(new MediaPacket(aPacketLength));

	                memcpy(aMediaPacketSharedPtr->getStartPtr(), aPacketContent, 12);

					//place to manipulate with RTP Header
	                aMediaPacketSharedPtr->getRtpHeaderParam (aRtpHeader);
					uint8_t aCsrcc = aRtpHeader.rtp_csrcc * 4;
	                memcpy(aMediaPacketSharedPtr->getPayloadPtr(),aPacketContent+12+aCsrcc,aPacketLength-12-aCsrcc);

	                aMediaPacketSharedPtr->setPayloadSize(aPacketLength-12-aCsrcc);
	                aMediaPacketSharedPtr->setTime(aPacketTransmitTime);

	                //place to manipulate with RTP Header
	                //aMediaPacketSharedPtr->getRtpHeaderParam (aRtpHeader);
	                aRtpHeader.rtp_pl=theStreamInfo->getPayloadNumber();
	                aMediaPacketSharedPtr->setRtpHeaderParam (aRtpHeader);

	                anEncodedMediaFileSharedPtr->addPacket(aMediaPacketSharedPtr);

	                aNumberOfBytes = fread(&aPacketHeader, 1, 8, aFile);

	                aPacketLength        = (aPacketHeader[0]<<24) + (aPacketHeader[1]<<16) + (aPacketHeader[2]<<8) +(aPacketHeader[3]<<0);
	                aPacketTransmitTime  = (aPacketHeader[4]<<24) + (aPacketHeader[5]<<16) + (aPacketHeader[6]<<8) +(aPacketHeader[7]<<0);

	                OK = feof(aFile) || (( aNumberOfBytes == (uint32_t)8 ) && (aPacketLength <= (uint32_t)MAXSIZE_OF_RTPPACKET));
	            }
	        }
	        fclose(aFile);
	    }

	    if(OK)	
			return theFilesCollection->addEncodedStream(anEncodedMediaFileSharedPtr);
	    else
	        return INVALID_STREAM_INDEX;
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EncodedMediaStreamIndex MediaCodecPreparator_H264::prepareEncodedStream(const MediaStreamInfo *theStreamInfo,
        CollectionOfEncodedFiles* theFilesCollection)
{
if (theStreamInfo->getFakeEncStreamGeneration()){    
    bool      OK;
    RtpHeaderParam     aRtpHeader;
    uint32_t aNumPacket = 0;
    uint32_t aPacketLength        = FAKE_RTP_SIZE;
    uint32_t aPacketTransmitTime  = 0;
    uint8_t aPayload = 0xFF;

    EncodedMediaStreamHandler anEncodedMediaFileSharedPtr (new EncodedMediaFile(theStreamInfo));
    while (aNumPacket < FAKE_PACKET_COUNT)
    {
       MediaPacketHandler aMediaPacketSharedPtr(new MediaPacket(aPacketLength));
       
       memset(aMediaPacketSharedPtr->getStartPtr(), aPayload, aPacketLength);
       
       aMediaPacketSharedPtr->setPayloadSize(aPacketLength-12);
       aMediaPacketSharedPtr->setTime(aPacketTransmitTime);

       //place to manipulate with RTP Header  
       aMediaPacketSharedPtr->getRtpHeaderParam (aRtpHeader);
       aRtpHeader.rtp_pl=theStreamInfo->getPayloadNumber();
       aMediaPacketSharedPtr->setRtpHeaderParam (aRtpHeader);

       anEncodedMediaFileSharedPtr->addPacket(aMediaPacketSharedPtr);
       
       aPacketTransmitTime += FAKE_PACKET_TRANSMIT_TIME;
       aNumPacket++;
    }

    OK = true;

    if(OK)
        return theFilesCollection->addEncodedStream(anEncodedMediaFileSharedPtr);
    else
        return INVALID_STREAM_INDEX;
}
else {

    FILE* 	 aFile;
    uint8_t  aPacketHeader[8];
    uint8_t  aPacketContent[MAXSIZE_OF_RTPPACKET];
    uint32_t aPacketLength;
    uint32_t aPacketTransmitTime;
    uint32_t aNumberOfBytes;

    bool      OK;


    RtpHeaderParam     aRtpHeader;
    EncodedMediaStreamHandler anEncodedMediaFileSharedPtr (new EncodedMediaFile(theStreamInfo));
    aFile = fopen(theStreamInfo->getFilePath().c_str(), "rb");

    OK = (aFile!=NULL);

    if( OK )
    {
        aNumberOfBytes = fread(&aPacketHeader,1, 8, aFile);

        aPacketLength        = (aPacketHeader[0]<<24) + (aPacketHeader[1]<<16) + (aPacketHeader[2]<<8) +(aPacketHeader[3]<<0);
        aPacketTransmitTime  = (aPacketHeader[4]<<24) + (aPacketHeader[5]<<16) + (aPacketHeader[6]<<8) +(aPacketHeader[7]<<0);

        OK = ( aNumberOfBytes == (uint32_t)8 ) && (aPacketLength <= (uint32_t)MAXSIZE_OF_RTPPACKET);

        while (!feof(aFile) && OK)
        {
            aNumberOfBytes = fread(aPacketContent, 1, aPacketLength, aFile);
            OK = (aNumberOfBytes == aPacketLength);

            if (OK)
            {
            	MediaPacketHandler aMediaPacketSharedPtr(new MediaPacket(aPacketLength));

                memcpy(aMediaPacketSharedPtr->getStartPtr(), aPacketContent, 12);

                aMediaPacketSharedPtr->setPayloadSize(aPacketLength-12);
                memcpy(aMediaPacketSharedPtr->getPayloadPtr(), aPacketContent+12, aPacketLength-12);
                aMediaPacketSharedPtr->setTime(aPacketTransmitTime);

                //place to manipulate with RTP Header
                aMediaPacketSharedPtr->getRtpHeaderParam (aRtpHeader);
                aRtpHeader.rtp_pl=theStreamInfo->getPayloadNumber();
                aMediaPacketSharedPtr->setRtpHeaderParam (aRtpHeader);

                anEncodedMediaFileSharedPtr->addPacket(aMediaPacketSharedPtr);

                aNumberOfBytes = fread(&aPacketHeader, 1, 8, aFile);

                aPacketLength        = (aPacketHeader[0]<<24) + (aPacketHeader[1]<<16) + (aPacketHeader[2]<<8) +(aPacketHeader[3]<<0);
                aPacketTransmitTime  = (aPacketHeader[4]<<24) + (aPacketHeader[5]<<16) + (aPacketHeader[6]<<8) +(aPacketHeader[7]<<0);

                OK = feof(aFile) || (( aNumberOfBytes == (uint32_t)8 ) && (aPacketLength <= (uint32_t)MAXSIZE_OF_RTPPACKET));
        
            }
        }
        fclose(aFile);
    }
 
    if(OK)
        return theFilesCollection->addEncodedStream(anEncodedMediaFileSharedPtr);
    else
        return INVALID_STREAM_INDEX; 
}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////



EncodedMediaStreamIndex MediaCodecPreparator_G711 :: prepareEncodedStream(const MediaStreamInfo *streamInfo,
                                                                          CollectionOfEncodedFiles* filesCollection)
{

    EncodedMediaStreamIndex index = INVALID_STREAM_INDEX;

    uint32_t size;
    int smpl_rate = SMPL_RATE_G711uA;
    int pkt_len_smpl = streamInfo->getDefaultPacketLength() * smpl_rate;
    int nfr_in_pkt = pkt_len_smpl / VOICE_FRAME_LTH_SMPL_G711uA;
    uint8_t pl_type = streamInfo->getPayloadNumber();

    if( nfr_in_pkt <= 0 || nfr_in_pkt > MAX_FRAMES_IN_PACKET_G711uA )
        return INVALID_STREAM_INDEX;

    if( streamInfo->getFakeEncStreamGeneration())
    {
        size = (nfr_in_pkt)*(SPEECH_CODED_FRAME_LTH_G711uA + 1);
    }
    else
    {
        if( (size = getRawStreamSize(streamInfo)) == 0 )
             return INVALID_STREAM_INDEX;
    }

    boost::scoped_array<uint8_t> pEnc ( new uint8_t[size+1]);
    if(!pEnc.get())
        return INVALID_STREAM_INDEX;

    if( streamInfo->getFakeEncStreamGeneration())
    {
    	uint8_t *pG = (uint8_t *)pEnc.get();
    	int k;
    	uint8_t val = (streamInfo->getCodec() == G711A)?0xD5:0xFF;  // silence
    	uint8_t  frlth = SPEECH_CODED_FRAME_LTH_G711uA;
    	for(k=0; k<nfr_in_pkt; k++)
     	{
     	    *pG++ = frlth;
            memset(pG, val, frlth);
       	    pG += frlth;
     	}
    }
    else
    {
       if( getRawData(streamInfo,(uint8_t *)pEnc.get(), size ) != size )
           return INVALID_STREAM_INDEX;
    }

    uint8_t *pS = (uint8_t *)pEnc.get();
    uint8_t payload [MediaPacket::PAYLOAD_MAX_SIZE];
    uint8_t *pP = payload;
    uint8_t  lth = 0;
    uint32_t ts  = 0;
    uint32_t ts_rtp = 0;
    uint16_t sn = 0;
    bool isSilence = false;
    int iCount = 0;
    int pl_len = 0;
    int32_t mark = 0;
    uint32_t i;
    MediaPacketHandler packet;
    RtpHeaderParam hdrParam;

    EncodedMediaStreamHandler streamHandler (new EncodedMediaFile(streamInfo));
    index = filesCollection->addEncodedStream(streamHandler);

    for(i=0; i< size; )
    {
        if( iCount == 0 ) ts_rtp = ts;

        lth = *pS++;
        i += lth +1;
        if( i > size ) 	break;

        if(lth) memcpy(pP, pS, lth );
        pS += lth;
        pP += lth;
        pl_len += lth;
        iCount++;

        if( lth == SPEECH_CODED_FRAME_LTH_G711uA )
        {
            if( (iCount == nfr_in_pkt) || (( i + lth +1 ) > size) )
            {
                mark = 0;
                if( isSilence )
                {
                    isSilence = false;
                    mark = 1;
                }
                memset(&hdrParam, 0, sizeof(RtpHeaderParam));  // pad, ext, csrcc : 0
                hdrParam.rtp_pl = pl_type;
                hdrParam.rtp_v  = 2;
                hdrParam.rtp_mark = mark;
                hdrParam.rtp_sn   = sn;
                hdrParam.rtp_ts   = ts_rtp;

                packet.reset(new MediaPacket(pl_len + MediaPacket::HEADER_SIZE));
                packet->setRtpHeaderParam (hdrParam);
		if(pl_len>(int)sizeof(payload)) {
		  pl_len=sizeof(payload);
		}
                memcpy(packet->getPayloadPtr(), payload, pl_len);
                packet->setPayloadSize(pl_len);
                packet->setTime(ts_rtp/smpl_rate);
                addPacket(streamHandler.get(), packet);

                sn++;
                pP = payload;
                pl_len = 0;
                iCount = 0;
            }
        }
        else
        {
            isSilence = true;
            pP = payload;
            pl_len = 0;
            iCount = 0;
        }
        ts += VOICE_FRAME_LTH_SMPL_G711uA;
    }


    // dummy packet in the end of the file
    memset(&hdrParam, 0, sizeof(RtpHeaderParam));
    hdrParam.rtp_pl = MediaPacket::DUMMY_PAYLOAD_TYPE;
    hdrParam.rtp_v  = 2;
    hdrParam.rtp_mark = 0;
    hdrParam.rtp_sn   = static_cast<uint16_t>(-1);
    hdrParam.rtp_ts   = ts;
    packet.reset(new MediaPacket( MediaPacket::HEADER_SIZE)); // zero payload body
    packet->setRtpHeaderParam (hdrParam);
    packet->setTime(ts/smpl_rate);
    addPacket(streamHandler.get(), packet);

	return index;
}


EncodedMediaStreamIndex MediaCodecPreparator_G723 :: prepareEncodedStream(const MediaStreamInfo *streamInfo,
                                                                          CollectionOfEncodedFiles* filesCollection)
{
    EncodedMediaStreamIndex index = INVALID_STREAM_INDEX;

    uint32_t size;
    int smpl_rate = SMPL_RATE_G723;
    int pkt_len_smpl = streamInfo->getDefaultPacketLength() * smpl_rate;
    int nfr_in_pkt = pkt_len_smpl / VOICE_FRAME_LTH_SMPL_G723;
    uint8_t pl_type = streamInfo->getPayloadNumber();

    if( nfr_in_pkt <= 0 || nfr_in_pkt > MAX_FRAMES_IN_PACKET_G723 )
        return INVALID_STREAM_INDEX;

    if( streamInfo->getFakeEncStreamGeneration())
    {
        if(streamInfo->getBitRate() == 6300 )
    	   size = (nfr_in_pkt)*(SPEECH_CODED_FRAME_LTH_G723_6_3);
        else
     	   size = (nfr_in_pkt)*(SPEECH_CODED_FRAME_LTH_G723_5_3);
    }
    else
    {
        if( (size = getRawStreamSize(streamInfo)) == 0 )
            return INVALID_STREAM_INDEX;
    }

    boost::scoped_array<uint8_t> pEnc ( new uint8_t[size+1]);
    if(!pEnc.get())
        return INVALID_STREAM_INDEX;


    if( streamInfo->getFakeEncStreamGeneration())
    {
    	uint8_t *pG = (uint8_t *)pEnc.get();
    	int k;
    	uint8_t  frlth = (streamInfo->getBitRate() == 6300)?SPEECH_CODED_FRAME_LTH_G723_6_3:SPEECH_CODED_FRAME_LTH_G723_5_3;
    	uint8_t  info =  (streamInfo->getBitRate() == 6300)?0:1;
    	uint8_t  val = (streamInfo->getBitRate() == 6300)?0x63:0x53;  //fake:arbitrary
    	for(k=0; k<nfr_in_pkt; k++)
     	{
     	    *pG++ = info;
     	    memset(pG, val,frlth-1);
      	    pG += (frlth-1);
     	}
    }
    else
    {
       if( getRawData(streamInfo,(uint8_t *)pEnc.get(), size ) != size )
           return INVALID_STREAM_INDEX;
    }

    uint8_t *pS = (uint8_t *)pEnc.get();
    uint8_t payload [MediaPacket::PAYLOAD_MAX_SIZE];
    uint8_t *pP = payload;
    uint8_t  lth = 0;
    uint32_t ts  = 0;
    uint32_t ts_rtp = 0;
    uint16_t sn = 0;
    bool isSilence = false;
    int iCount = 0;
    int pl_len = 0;
    int32_t mark = 0;
    uint32_t i;
    MediaPacketHandler packet;
    RtpHeaderParam hdrParam;

    EncodedMediaStreamHandler streamHandler (new EncodedMediaFile(streamInfo));
    index = filesCollection->addEncodedStream(streamHandler);

    for(i=0; i< size; )
    {
        char info;
        if(iCount == 0 )ts_rtp = ts;
        info = (*pS) & 0x03;
        switch(info)
        {
        case 0:
            lth = SPEECH_CODED_FRAME_LTH_G723_6_3;
            break;
        case 1:
            lth = SPEECH_CODED_FRAME_LTH_G723_5_3;
            break;
        case 2:
            lth = SID_CODED_FRAME_LTH_G723;
            break;
        default:
            lth = 0;
            break;

        }
        i += lth;
        if( i > size ) 	break;
        if(lth) memcpy(pP, pS, lth );
        pS += lth;
        pP += lth;
        pl_len += lth;
        iCount++;

        switch( lth )
        {
        case SPEECH_CODED_FRAME_LTH_G723_5_3:
        case SPEECH_CODED_FRAME_LTH_G723_6_3:
            if( (iCount == nfr_in_pkt) || (( i + lth +1 ) > size) )
            {
                mark = 0;
                if( isSilence )
                {
                    isSilence = false;
                    mark = 1;
                }
                memset(&hdrParam, 0, sizeof(RtpHeaderParam));  // pad, ext, csrcc : 0
                hdrParam.rtp_pl = pl_type;
                hdrParam.rtp_v  = 2;
                hdrParam.rtp_mark = mark;
                hdrParam.rtp_sn   = sn;
                hdrParam.rtp_ts   = ts_rtp;

                packet.reset(new MediaPacket(pl_len + MediaPacket::HEADER_SIZE));
                packet->setRtpHeaderParam (hdrParam);
		if(pl_len>(int)sizeof(payload)) {
		  pl_len=sizeof(payload);
		}
                memcpy(packet->getPayloadPtr(), payload, pl_len);
                packet->setPayloadSize(pl_len);
                packet->setTime(ts_rtp/smpl_rate);
                addPacket(streamHandler.get(), packet);

                sn++;
                pP = payload;
                pl_len = 0;
                iCount = 0;
            }
            break;
        case SID_CODED_FRAME_LTH_G723:
            mark = 0;

            memset(&hdrParam, 0, sizeof(RtpHeaderParam));  // pad, ext, csrcc : 0
            hdrParam.rtp_pl = pl_type;
            hdrParam.rtp_v  = 2;
            hdrParam.rtp_mark = mark;
            hdrParam.rtp_sn   = sn;
            hdrParam.rtp_ts   = ts_rtp;

            packet.reset(new MediaPacket(pl_len + MediaPacket::HEADER_SIZE));
            packet->setRtpHeaderParam (hdrParam);
	    if(pl_len>(int)sizeof(payload)) {
	      pl_len=sizeof(payload);
	    }
            memcpy(packet->getPayloadPtr(), payload, pl_len);
            packet->setPayloadSize(pl_len);
            packet->setTime(ts_rtp/smpl_rate);
            addPacket(streamHandler.get(), packet);

            sn++;
            pP = payload;
            pl_len = 0;
            iCount = 0;
            break;
        case 0:
        default:
            isSilence = true;
            pP = payload;
            pl_len = 0;
            iCount = 0;
            break;
        }
        ts += VOICE_FRAME_LTH_SMPL_G723;
    }


    // dummy packet in the end of the file
    memset(&hdrParam, 0, sizeof(RtpHeaderParam));
    hdrParam.rtp_pl = MediaPacket::DUMMY_PAYLOAD_TYPE;
    hdrParam.rtp_v  = 2;
    hdrParam.rtp_mark = 0;
    hdrParam.rtp_sn   = static_cast<uint16_t>(-1);
    hdrParam.rtp_ts   = ts;
    packet.reset(new MediaPacket( MediaPacket::HEADER_SIZE)); // zero payload body
    packet->setRtpHeaderParam (hdrParam);
    packet->setTime(ts/smpl_rate);
    addPacket(streamHandler.get(), packet);

    return index;


}

EncodedMediaStreamIndex MediaCodecPreparator_G726 :: prepareEncodedStream(const MediaStreamInfo *streamInfo,
                                                                          CollectionOfEncodedFiles* filesCollection)
{

    EncodedMediaStreamIndex index = INVALID_STREAM_INDEX;
    uint32_t size;
    int smpl_rate = SMPL_RATE_G726;
    int pkt_len_smpl = streamInfo->getDefaultPacketLength() * smpl_rate;
    int nfr_in_pkt = pkt_len_smpl / VOICE_FRAME_LTH_SMPL_G726;
    uint8_t pl_type = streamInfo->getPayloadNumber();

    if( nfr_in_pkt <= 0 || nfr_in_pkt > MAX_FRAMES_IN_PACKET_G726 )
        return INVALID_STREAM_INDEX;

    if( streamInfo->getFakeEncStreamGeneration())
    {
        size = (nfr_in_pkt)*(SPEECH_CODED_FRAME_LTH_G726_32 + 1);
    }
    else
    {
        if( (size = getRawStreamSize(streamInfo)) == 0 )
            return INVALID_STREAM_INDEX;
    }

    boost::scoped_array<uint8_t> pEnc ( new uint8_t[size+1]);
    if(!pEnc.get())
        return INVALID_STREAM_INDEX;

    if( streamInfo->getFakeEncStreamGeneration())
    {
    	uint8_t *pG = (uint8_t *)pEnc.get();
    	int k;
    	uint8_t val = 0x26;//fake:arbitrary
    	uint8_t  frlth = SPEECH_CODED_FRAME_LTH_G726_32;
    	for(k=0; k<nfr_in_pkt; k++)
     	{
     	    *pG++ = frlth;
            memset(pG, val, frlth);
       	    pG += frlth;
     	}
    }
    else
    {
       if( getRawData(streamInfo,(uint8_t *)pEnc.get(), size ) != size )
          return INVALID_STREAM_INDEX;
    }

    uint8_t *pS = (uint8_t *)pEnc.get();
    uint8_t payload [MediaPacket::PAYLOAD_MAX_SIZE];
    uint8_t *pP = payload;
    uint8_t  lth = 0;
    uint32_t ts  = 0;
    uint32_t ts_rtp = 0;
    uint16_t sn = 0;
    bool isSilence = false;
    int iCount = 0;
    int pl_len = 0;
    int32_t mark = 0;
    uint32_t i;
    MediaPacketHandler packet;
    RtpHeaderParam hdrParam;

    EncodedMediaStreamHandler streamHandler (new EncodedMediaFile(streamInfo));
    index = filesCollection->addEncodedStream(streamHandler);

    for(i=0; i< size; )
    {
        if( iCount == 0 ) ts_rtp = ts;

        lth = *pS++;
        i += lth +1;
        if( i > size ) 	break;

        if(lth) memcpy(pP, pS, lth );
        pS += lth;
        pP += lth;
        pl_len += lth;
        iCount++;

        switch( lth )
        {
        case SPEECH_CODED_FRAME_LTH_G726_16:
        case SPEECH_CODED_FRAME_LTH_G726_24:
        case SPEECH_CODED_FRAME_LTH_G726_32:
        case SPEECH_CODED_FRAME_LTH_G726_40:
            if( (iCount == nfr_in_pkt) || (( i + lth +1 ) > size) )
            {
                mark = 0;
                if( isSilence )
                {
                    isSilence = false;
                    mark = 1;
                }
                memset(&hdrParam, 0, sizeof(RtpHeaderParam));  // pad, ext, csrcc : 0
                hdrParam.rtp_pl = pl_type;
                hdrParam.rtp_v  = 2;
                hdrParam.rtp_mark = mark;
                hdrParam.rtp_sn   = sn;
                hdrParam.rtp_ts   = ts_rtp;

                packet.reset(new MediaPacket(pl_len + MediaPacket::HEADER_SIZE));
                packet->setRtpHeaderParam (hdrParam);
		if(pl_len>(int)sizeof(payload)) {
		  pl_len=sizeof(payload);
		}
                memcpy(packet->getPayloadPtr(), payload, pl_len);
                packet->setPayloadSize(pl_len);
                packet->setTime(ts_rtp/smpl_rate);
                addPacket(streamHandler.get(), packet);

                sn++;
                pP = payload;
                pl_len = 0;
                iCount = 0;
            }
            break;
        case 0:
        default:
            isSilence = true;
            pP = payload;
            pl_len = 0;
            iCount = 0;
            break;
        }
        ts += VOICE_FRAME_LTH_SMPL_G726;
    }


    // dummy packet in the end of the file
    memset(&hdrParam, 0, sizeof(RtpHeaderParam));
    hdrParam.rtp_pl = MediaPacket::DUMMY_PAYLOAD_TYPE;
    hdrParam.rtp_v  = 2;
    hdrParam.rtp_mark = 0;
    hdrParam.rtp_sn   = static_cast<uint16_t>(-1);
    hdrParam.rtp_ts   = ts;
    packet.reset(new MediaPacket( MediaPacket::HEADER_SIZE)); // zero payload body
    packet->setRtpHeaderParam (hdrParam);
    packet->setTime(ts/smpl_rate);
    addPacket(streamHandler.get(), packet);

    return index;

}
EncodedMediaStreamIndex MediaCodecPreparator_G729 :: prepareEncodedStream(const MediaStreamInfo *streamInfo,
                                                                            CollectionOfEncodedFiles* filesCollection)
{

    EncodedMediaStreamIndex index = INVALID_STREAM_INDEX;
    uint32_t size;
    int smpl_rate = SMPL_RATE_G729;
    int pkt_len_smpl = streamInfo->getDefaultPacketLength() * smpl_rate;
    int nfr_in_pkt = pkt_len_smpl / VOICE_FRAME_LTH_SMPL_G729;
    uint8_t pl_type = streamInfo->getPayloadNumber();

    if( nfr_in_pkt <= 0 || nfr_in_pkt > MAX_FRAMES_IN_PACKET_G729 )
        return INVALID_STREAM_INDEX;


    if( streamInfo->getFakeEncStreamGeneration())
    {
        size = (nfr_in_pkt)*(SPEECH_CODED_FRAME_LTH_G729 + 1);
        if(streamInfo->getCodec() == G729B || streamInfo->getCodec() == G729AB )
        {
        	size += SID_CODED_FRAME_LTH_G729 + 1;  // SID
        	size += 8; // DTX frames
            size += (nfr_in_pkt)*(SPEECH_CODED_FRAME_LTH_G729 + 1); // for marker
        }
    }
    else
    {
        if( (size = getRawStreamSize(streamInfo)) == 0 )
           return INVALID_STREAM_INDEX;
    }

    boost::scoped_array<uint8_t> pEnc ( new uint8_t[size+1]);
    if(!pEnc.get())
        return INVALID_STREAM_INDEX;

    if( streamInfo->getFakeEncStreamGeneration())
    {
     	uint8_t *pG = (uint8_t *)pEnc.get();
     	int k;
     	uint8_t val = 0x29;//fake:arbitrary
     	uint8_t  frlth = SPEECH_CODED_FRAME_LTH_G729;
     	for(k=0; k<nfr_in_pkt; k++)
      	{
      	    *pG++ = frlth;
            memset(pG, val, frlth);
        	pG += frlth;
      	}
        if(streamInfo->getCodec() == G729B || streamInfo->getCodec() == G729AB)
        {
            *pG++ = SID_CODED_FRAME_LTH_G729;
            memset(pG, 0xBB, SID_CODED_FRAME_LTH_G729); //SID fake
         	pG += SID_CODED_FRAME_LTH_G729;
            memset(pG, 0, 8);  // DTX
            pG += 8;
          	for(k=0; k<nfr_in_pkt; k++)
          	{
          	    *pG++ = frlth;
                memset(pG, val, frlth);
            	pG += frlth;
          	}

        }
    }
    else
    {
        if( getRawData(streamInfo,(uint8_t *)pEnc.get(), size ) != size )
            return INVALID_STREAM_INDEX;
    }

    uint8_t *pS = (uint8_t *)pEnc.get();
    uint8_t payload [MediaPacket::PAYLOAD_MAX_SIZE];
    uint8_t *pP = payload;
    uint8_t  lth = 0;
    uint32_t ts  = 0;
    uint32_t ts_rtp = 0;
    uint16_t sn = 0;
    bool isSilence = false;
    int iCount = 0;
    int pl_len = 0;
    int32_t mark = 0;
    uint32_t i;
    MediaPacketHandler packet;
    RtpHeaderParam hdrParam;

    EncodedMediaStreamHandler streamHandler (new EncodedMediaFile(streamInfo));
    index = filesCollection->addEncodedStream(streamHandler);

    for(i=0; i< size; )
    {
        if( iCount == 0 ) ts_rtp = ts;

        lth = *pS++;
        i += lth +1;
        if( i > size ) 	break;

        if(lth) memcpy(pP, pS, lth );
        pS += lth;
        pP += lth;
        pl_len += lth;
        iCount++;

        switch( lth )
        {
        case SPEECH_CODED_FRAME_LTH_G729:
                if( (iCount == nfr_in_pkt) || (( i + lth +1 ) > size) )
                {
                    mark = 0;
                    if( isSilence )
                    {
                        isSilence = false;
                        mark = 1;
                    }
                    memset(&hdrParam, 0, sizeof(RtpHeaderParam));  // pad, ext, csrcc : 0
                    hdrParam.rtp_pl = pl_type;
                    hdrParam.rtp_v  = 2;
                    hdrParam.rtp_mark = mark;
                    hdrParam.rtp_sn   = sn;
                    hdrParam.rtp_ts   = ts_rtp;

                    packet.reset(new MediaPacket(pl_len + MediaPacket::HEADER_SIZE));
                    packet->setRtpHeaderParam (hdrParam);
		    if(pl_len>(int)sizeof(payload)) {
		      pl_len=sizeof(payload);
		    }
                    memcpy(packet->getPayloadPtr(), payload, pl_len);
                    packet->setPayloadSize(pl_len);
                    packet->setTime(ts_rtp/smpl_rate);
                    addPacket(streamHandler.get(), packet);

                    sn++;
                    pP = payload;
                    pl_len = 0;
                    iCount = 0;
                }
             break;
        case SID_CODED_FRAME_LTH_G729:
                    mark = 0;

                    memset(&hdrParam, 0, sizeof(RtpHeaderParam));  // pad, ext, csrcc : 0
                    hdrParam.rtp_pl = pl_type;
                    hdrParam.rtp_v  = 2;
                    hdrParam.rtp_mark = mark;
                    hdrParam.rtp_sn   = sn;
                    hdrParam.rtp_ts   = ts_rtp;

                    packet.reset(new MediaPacket(pl_len + MediaPacket::HEADER_SIZE));
                    packet->setRtpHeaderParam (hdrParam);
		    if(pl_len>(int)sizeof(payload)) {
		      pl_len=sizeof(payload);
		    }
                    memcpy(packet->getPayloadPtr(), payload, pl_len);
                    packet->setPayloadSize(pl_len);
                    packet->setTime(ts_rtp/smpl_rate);
                    addPacket(streamHandler.get(), packet);

                    sn++;
                    pP = payload;
                    pl_len = 0;
                    iCount = 0;
              break;
        case 0:
        default:
                isSilence = true;
                pP = payload;
                pl_len = 0;
                iCount = 0;
            break;
        }
        ts += VOICE_FRAME_LTH_SMPL_G729;
    }


    // dummy packet in the end of the file
    memset(&hdrParam, 0, sizeof(RtpHeaderParam));
    hdrParam.rtp_pl = MediaPacket::DUMMY_PAYLOAD_TYPE;
    hdrParam.rtp_v  = 2;
    hdrParam.rtp_mark = 0;
    hdrParam.rtp_sn   = static_cast<uint16_t>(-1);
    hdrParam.rtp_ts   = ts;
    packet.reset(new MediaPacket( MediaPacket::HEADER_SIZE)); // zero payload body
    packet->setRtpHeaderParam (hdrParam);
    packet->setTime(ts/smpl_rate);
    addPacket(streamHandler.get(), packet);

    return index;

}

EncodedMediaStreamIndex MediaCodecPreparator_G711_1 :: prepareEncodedStream(const MediaStreamInfo *streamInfo,
                                                                          CollectionOfEncodedFiles* filesCollection)
{

    EncodedMediaStreamIndex index = INVALID_STREAM_INDEX;

    uint32_t size;
    int smpl_rate = SMPL_RATE_G711_1uA;
    int pkt_len_smpl = streamInfo->getDefaultPacketLength() * smpl_rate;
    int nfr_in_pkt = pkt_len_smpl / VOICE_FRAME_LTH_SMPL_G711_1uA;
    uint8_t pl_type = streamInfo->getPayloadNumber();
    uint8_t mode_index =  G711_1uA_MODE_INDEX_R3;
    bool bFakeStream = true;  // streamInfo->getFakeEncStreamGeneration()

    if( nfr_in_pkt <= 0 || nfr_in_pkt > MAX_FRAMES_IN_PACKET_G711_1uA )
        return INVALID_STREAM_INDEX;

    if( bFakeStream )
    {
        size = (nfr_in_pkt)*(G711_1uA_FRAME_LTH_L0 +G711_1uA_FRAME_LTH_L1 + G711_1uA_FRAME_LTH_L2 + 1);
    }
    else
    {
        if( (size = getRawStreamSize(streamInfo)) == 0 )
             return INVALID_STREAM_INDEX;
    }

    boost::scoped_array<uint8_t> pEnc ( new uint8_t[size+1]);
    if(!pEnc.get())
        return INVALID_STREAM_INDEX;

    if( bFakeStream )
    {
    	uint8_t *pG = (uint8_t *)pEnc.get();
    	int k;
    	uint8_t val_L0 = (streamInfo->getCodec() == CODEC_TYPE_G711_1A)?0xD5:0xFF;  // silence
       	uint8_t val_L1 = 0xAA;
      	uint8_t val_L2 = 0x3d;

       	for(k=0; k<nfr_in_pkt; k++)
     	{
           *pG++ = (G711_1uA_FRAME_LTH_L0 +G711_1uA_FRAME_LTH_L1 + G711_1uA_FRAME_LTH_L2);
           memset(pG, val_L0, G711_1uA_FRAME_LTH_L0);
           pG += G711_1uA_FRAME_LTH_L0;
           memset(pG, val_L1, G711_1uA_FRAME_LTH_L1);
           pG += G711_1uA_FRAME_LTH_L1;
           memset(pG, val_L2, G711_1uA_FRAME_LTH_L2);
           pG += G711_1uA_FRAME_LTH_L2;
     	}
    }
    else
    {
       if( getRawData(streamInfo,(uint8_t *)pEnc.get(), size ) != size )
           return INVALID_STREAM_INDEX;
    }

    uint8_t *pS = (uint8_t *)pEnc.get();
    uint8_t payload [MediaPacket::PAYLOAD_MAX_SIZE];
    uint8_t *pP = payload;
    uint8_t  lth = 0;
    uint32_t ts  = 0;
    uint32_t ts_rtp = 0;
    uint16_t sn = 0;
    bool isSilence = false;
    int iCount = 0;
    int pl_len = 0;
    int32_t mark = 0;
    uint32_t i;
    MediaPacketHandler packet;
    RtpHeaderParam hdrParam;

    EncodedMediaStreamHandler streamHandler (new EncodedMediaFile(streamInfo));
    index = filesCollection->addEncodedStream(streamHandler);

    for(i=0; i< size; )
    {
        if( iCount == 0 ) ts_rtp = ts;

        lth = *pS++;
        i += lth +1;
        if( i > size ) 	break;

        if(lth) memcpy(pP, pS, lth );
        pS += lth;
        pP += lth;
        pl_len += lth;
        iCount++;

        if( lth >= G711_1uA_FRAME_LTH_L0 )
        {
            if( (iCount == nfr_in_pkt) || (( i + lth +1 ) > size) )
            {

            	mark = 0;
                if( isSilence )
                {
                    isSilence = false;
                    mark = 1;
                }
                memset(&hdrParam, 0, sizeof(RtpHeaderParam));  // pad, ext, csrcc : 0
                hdrParam.rtp_pl = pl_type;
                hdrParam.rtp_v  = 2;
                hdrParam.rtp_mark = mark;
                hdrParam.rtp_sn   = sn;
                hdrParam.rtp_ts   = ts_rtp;
                pl_len += 1; // Mode Index;
                packet.reset(new MediaPacket(pl_len + MediaPacket::HEADER_SIZE));
                packet->setRtpHeaderParam (hdrParam);
                if(pl_len>(int)sizeof(payload)) {
                   pl_len=sizeof(payload);
                }
                uint8_t *pPayloadData = packet->getPayloadPtr();
                *pPayloadData++ = mode_index;
                if(pl_len > 1 )
                   memcpy(pPayloadData, payload, pl_len-1);

                packet->setPayloadSize(pl_len);
                packet->setTime(ts_rtp/smpl_rate);
                addPacket(streamHandler.get(), packet);

                sn++;
                pP = payload;
                pl_len = 0;
                iCount = 0;
            }
        }
        else
        {
            isSilence = true;
            pP = payload;
            pl_len = 0;
            iCount = 0;
        }
        ts += VOICE_FRAME_LTH_SMPL_G711_1uA;
    }


    // dummy packet in the end of the file
    memset(&hdrParam, 0, sizeof(RtpHeaderParam));
    hdrParam.rtp_pl = MediaPacket::DUMMY_PAYLOAD_TYPE;
    hdrParam.rtp_v  = 2;
    hdrParam.rtp_mark = 0;
    hdrParam.rtp_sn   = static_cast<uint16_t>(-1);
    hdrParam.rtp_ts   = ts;
    packet.reset(new MediaPacket( MediaPacket::HEADER_SIZE)); // zero payload body
    packet->setRtpHeaderParam (hdrParam);
    packet->setTime(ts/smpl_rate);
    addPacket(streamHandler.get(), packet);

	return index;
}

///////////////////////////////////////////////////////////////////////////////

END_DECL_MEDIAFILES_NS

////////////////////////////////////////////////////////////////////////////////

