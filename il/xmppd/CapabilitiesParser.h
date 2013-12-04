/// @file
/// @brief Object to parse capability file contents into component capabilities
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _CAPABILITIES_PARSER_H_
#define _CAPABILITIES_PARSER_H_

#include <vector>
#include <string>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMDocumentType.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMNodeIterator.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMText.hpp>

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/XMLUni.hpp>

#include "XmppCommon.h"

//forward declarations
namespace gloox
{
    class Tag;
}


DECL_XMPP_NS

///////////////////////////////////////////////////////////////////////////////

class CapabilitiesParser
{
public:
/*    enum ParseState
    {
        IDLE,
        OPENING_TAG,
        CLOSING_TAG,
        ATTRIBUTE,
        CDATA
    };*/
    CapabilitiesParser(std::vector<gloox::Tag*>& capabilities); // : mCapabilities(capabilities) {}
    ~CapabilitiesParser();
    void Parse(const std::string& filepath); //data);
private:
    static const std::string START_DELIMITER;
    static const size_t START_DELIMITER_SIZE;
    static const std::string END_DELIMITER;
    static const size_t END_DELIMITER_SIZE;
//    void createTag(const std::string& capability);
    void addAttr( xercesc::DOMElement* currentElement, gloox::Tag* currTag );
    void addChildren(gloox::Tag* parent, xercesc::DOMNodeList* children );

    //xerces
    xercesc::XercesDOMParser *m_ConfigFileParser;
    XMLCh* TAG_root;
    XMLCh* TAG_cec;
    XMLCh* TAG_pc;
    XMLCh* TAG_nc;
    XMLCh* TAG_fc;
    XMLCh* TAG_lbc;
    XMLCh* TAG_cc;
    XMLCh* TAG_sc;

    std::vector<gloox::Tag*>& mCapabilities;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_XMPP_NS

#endif
