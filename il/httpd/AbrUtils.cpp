/// @file
/// @brief HTTP ABR Utils implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.

/// @file
/// @brief ABR Utils header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include "AbrUtils.h"

using namespace boost::spirit::classic;

///////////////////////////////////////////////////////////////////////////////

BEGIN_DECL_ABR_NS

namespace utils {

///////////////////////////////////////////////////////////////////////////////

const uint8_t IntegerToBitrate(uint16_t bitrate)
{
    switch(bitrate)
    {
    	case 64:
            return http_1_port_server::HttpVideoServerBitrateOptions_BR_64;
        case 96:
            return http_1_port_server::HttpVideoServerBitrateOptions_BR_96;
        case 150:
            return http_1_port_server::HttpVideoServerBitrateOptions_BR_150;
        case 240:
            return http_1_port_server::HttpVideoServerBitrateOptions_BR_240;
        case 256:
            return http_1_port_server::HttpVideoServerBitrateOptions_BR_256;
        case 440:
            return http_1_port_server::HttpVideoServerBitrateOptions_BR_440;
        case 640:
            return http_1_port_server::HttpVideoServerBitrateOptions_BR_640;
        case 800:
            return http_1_port_server::HttpVideoServerBitrateOptions_BR_800;
        case 840:
            return http_1_port_server::HttpVideoServerBitrateOptions_BR_840;
        case 1240:
            return http_1_port_server::HttpVideoServerBitrateOptions_BR_1240;
        default:
            return http_1_port_server::HttpVideoServerBitrateOptions_BR_64;
    }
}

/////////////////////////////////////////////////////////////////////

const std::string BitrateToString(uint8_t bitrate)
{
    std::string selectedBitrate;

    switch(bitrate) {
        case http_1_port_server::HttpVideoServerBitrateOptions_BR_64:
            selectedBitrate = "64k-audio";
            break;
        case http_1_port_server::HttpVideoServerBitrateOptions_BR_96:
            selectedBitrate = "96k";
            break;
        case http_1_port_server::HttpVideoServerBitrateOptions_BR_150:
            selectedBitrate = "150k";
            break;
        case http_1_port_server::HttpVideoServerBitrateOptions_BR_240:
            selectedBitrate = "240k";
            break;
        case http_1_port_server::HttpVideoServerBitrateOptions_BR_256:
            selectedBitrate = "256k";
            break;
        case http_1_port_server::HttpVideoServerBitrateOptions_BR_440:
            selectedBitrate = "440k";
            break;
        case http_1_port_server::HttpVideoServerBitrateOptions_BR_640:
            selectedBitrate = "640k";
            break;
        case http_1_port_server::HttpVideoServerBitrateOptions_BR_800:
            selectedBitrate = "800k";
            break;
        case http_1_port_server::HttpVideoServerBitrateOptions_BR_840:
            selectedBitrate = "840k";
            break;
        case http_1_port_server::HttpVideoServerBitrateOptions_BR_1240:
            selectedBitrate = "1240k";
            break;
        default:
            selectedBitrate = "64k-audio";
            break;
    }

    return selectedBitrate;
}

/////////////////////////////////////////////////////////////////////

const uint16_t BitrateToInteger(uint8_t bitrate)
{
    uint16_t selectedBitrate=0;

    switch(bitrate) {
        case http_1_port_server::HttpVideoServerBitrateOptions_BR_64:
            selectedBitrate = 64;
            break;
        case http_1_port_server::HttpVideoServerBitrateOptions_BR_96:
            selectedBitrate = 96;
            break;
        case http_1_port_server::HttpVideoServerBitrateOptions_BR_150:
            selectedBitrate = 150;
            break;
        case http_1_port_server::HttpVideoServerBitrateOptions_BR_240:
            selectedBitrate = 240;
            break;
        case http_1_port_server::HttpVideoServerBitrateOptions_BR_256:
            selectedBitrate = 256;
            break;
        case http_1_port_server::HttpVideoServerBitrateOptions_BR_440:
            selectedBitrate = 440;
            break;
        case http_1_port_server::HttpVideoServerBitrateOptions_BR_640:
            selectedBitrate = 640;
            break;
        case http_1_port_server::HttpVideoServerBitrateOptions_BR_800:
            selectedBitrate = 800;
            break;
        case http_1_port_server::HttpVideoServerBitrateOptions_BR_840:
            selectedBitrate = 840;
            break;
        case http_1_port_server::HttpVideoServerBitrateOptions_BR_1240:
            selectedBitrate = 1240;
            break;
        default:
            break;
    }

    return selectedBitrate;
}

/////////////////////////////////////////////////////////////////////

const size_t VersionToInteger(uint8_t serverVersion)
{
    switch(serverVersion) {
        case http_1_port_server::HttpVideoServerVersionOptions_VERSION_1_0:
            return 1;
        default:
            return 1;
    }
}

/////////////////////////////////////////////////////////////////////

const bool ParseUri(const std::string *uri, sessionType_t *sessionType, requestType_t *requestType, uint16_t *mediaSequence, uint8_t *bitrate)
{
    std::string filetype,
                session,
                progressive;

    uint16_t parsed_bitrate = 0;

    const bool success = parse(uri->begin(), uri->end(), 
                                    (
                                        (
                                            //!(str_p("http://spirent.") >> (+alpha_p)[assign_a(domain)] >> str_p(".com")) >> ch_p('/')
                                           !(
                                               ch_p('/')
                                               >> (+alpha_p)[assign_a(session)]    // VOD/LIVE?
                                           )
                                           >> ch_p('/')
                                           >> !(
                                                    int_p[assign_a(parsed_bitrate)]
                                                    >> ch_p('k')
                                                    >> (
                                                            (
                                                               (
                                                                   ch_p('-')
                                                                   >> !(
                                                                            str_p("audio")
                                                                            >> (ch_p('-') | ch_p('/'))
                                                                       )
                                                               )
                                                            )
                                                            |
                                                            ch_p('/')
                                                       )
                                                    >> !(
                                                            str_p("HLSvideo-")
                                                            >> ((int_p)[assign_a(*mediaSequence)] | str_p("entire")[assign_a(progressive)])
                                                            >> str_p(".ts")[assign_a(filetype)]
                                                        )
                                                )
                                            >> !(
                                                    +(alpha_p | digit_p)
                                                    >> ch_p('.')
                                                    >> (+(alpha_p | digit_p))[assign_a(filetype)]

                                                )
                                            >> *eol_p
                                        ) | *eol_p
                                    )).full;

                                    //std::cout << "uri: " << *uri << std::endl;
                                    //std::cout << "success: " << success << " sequenceNumber: " << mediaSequence << " parsedbitrate: " << parsed_bitrate << " session: " << session << std::endl;

    if (!success)
        return 0;

    // signifies received eol only, still valid.
    if (filetype.compare("") == 0 | filetype.compare("html") == 0)
    {
        *requestType = TEXT;
        return 1;
    }


    // Check if live or vod
    if (session.compare("VOD") == 0)
        *sessionType = VOD;
    else if (session.compare("LIVE") == 0)
        *sessionType = LIVE;


    if (parsed_bitrate == 0)    // no bitrate assigned, it is a variant playlist
           *requestType = VARIANT_PLAYLIST;
    else    // There is a bitrate, which means it can be a fragment or a bitrate playlist
    {
        if (*mediaSequence != 0 || !progressive.empty())    // If media sequence or progressive string not empty, then it is a fragment
            *requestType = FRAGMENT;
        else
        {
            // Check whether requesting html or m3u8 file
            if (filetype.compare("m3u8") == 0)
                   *requestType = MANIFEST;
               else
                *requestType = UNKNOWN;
        }

        *bitrate = IntegerToBitrate(parsed_bitrate);
    }

    //std::cout << "mediaSequence:" << *mediaSequence << ", requestType:" << *requestType << ", bitrate:" << (size_t)*bitrate << ", filetype:" << filetype << ", session:" << *sessionType << std::endl;
    return success;
}

///////////////////////////////////////////////////////////////////////////////
/*
static bool ParseHost(const std::string *host, sessionType_t *sessionType)
{
    std::string domain;

    const bool success = parse(host->begin(), host->end(), 
                                    (
                                        (
                                            (
                                                str_p("spirent.")
                                                >> (+alpha_p)[assign_a(domain)]
                                                >> str_p(".com")
                                                >> *eol_p
                                            )

                                            |

                                            (
                                                (+(digit_p | ch_p(':') | ch_p('.')))    // Cover IPv4 and IPv6 cases
                                                >> *eol_p
                                            )
                                        ) | *eol_p
                                    )).full;

    if (!success)
        return 0;

    if (domain.compare("VoDmedia") == 0)
        *sessionType = VOD;
    else if (domain.compare("Livemedia") == 0)
        *sessionType = LIVE;

    //std::cout << "success: " << success << ", host: " << *host << ", sessionType: " << *sessionType << std::endl;

    return success;
}
*/

///////////////////////////////////////////////////////////////////////////////

AbrStaticMessageBody::AbrStaticMessageBody(size_t size, std::string fill)
    : ACE_Data_Block(size, ACE_Message_Block::MB_DATA, 0, 0, 0, 0, 0)
{
    memcpy(base(), fill.c_str(), size);
}

///////////////////////////////////////////////////////////////////////////////

AbrStreamMessageBody::AbrStreamMessageBody(std::string *msg)
{
    // Chain message blocks with static body
    ACE_Message_Block *tail = 0;

    std::string playlist = *msg;
    std::string msgChunk;

    while (playlist.size())
    {
        const size_t mbSize = std::min(playlist.size(), (size_t)DEFAULT_MSGBODY_SIZE);

        msgChunk = playlist.substr(0, mbSize);

        // Update playlist to contain only remaining
        playlist = playlist.substr(mbSize);

        std::auto_ptr<ACE_Data_Block> newBody(new AbrStaticMessageBody(mbSize, msgChunk));

        if (!tail)
        {
            // First message block
            this->data_block(newBody.release());
            //this->set_self_flags(DONT_DELETE);
            this->wr_ptr(mbSize);

            tail = this;
        }
        else
        {
            std::auto_ptr<ACE_Message_Block> body(new ACE_Message_Block(newBody.release()));
            body->wr_ptr(mbSize);

            tail->cont(body.release());
            tail = tail->cont();
        }
    }
};

///////////////////////////////////////////////////////////////////////////////

} // utils namespace

END_DECL_ABR_NS
