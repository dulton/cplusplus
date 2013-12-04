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

#include <boost/algorithm/string.hpp>
#include <tag.h>
#include "CapabilitiesParser.h"

#include "XmppdLog.h"

USING_XMPP_NS;

///////////////////////////////////////////////////////////////////////////////

const std::string CapabilitiesParser::START_DELIMITER("<capability>");
const size_t CapabilitiesParser::START_DELIMITER_SIZE(START_DELIMITER.size());
const std::string CapabilitiesParser::END_DELIMITER("</capability>");
const size_t CapabilitiesParser::END_DELIMITER_SIZE(END_DELIMITER.size());

///////////////////////////////////////////////////////////////////////////////

CapabilitiesParser::CapabilitiesParser(std::vector<gloox::Tag*>& capabilities) : mCapabilities(capabilities)
{
    try
    {
       xercesc::XMLPlatformUtils::Initialize();  // Initialize Xerces infrastructure
    }
    catch( xercesc::XMLException& e )
    {
       char* message = xercesc::XMLString::transcode( e.getMessage() );
       TC_LOG_ERR(0,"XML toolkit initialization error: " + std::string(message));
       xercesc::XMLString::release( &message );
       // throw exception here to return ERROR_XERCES_INIT
    }
    
    // Tags and attributes used in XML file.
    // Can't call transcode till after Xerces Initialize()
    TAG_root = xercesc::XMLString::transcode("root");
    TAG_cec  = xercesc::XMLString::transcode("cloud-element-capabilities");
    TAG_pc   = xercesc::XMLString::transcode("performance-capabilities");
    TAG_nc   = xercesc::XMLString::transcode("network-capabilities");
    TAG_fc   = xercesc::XMLString::transcode("firewall-capabilities");
    TAG_lbc  = xercesc::XMLString::transcode("load-balancer-capabilities");
    TAG_cc   = xercesc::XMLString::transcode("compute-capabilities");
    TAG_sc   = xercesc::XMLString::transcode("storage-capabilities");
   
    m_ConfigFileParser = new xercesc::XercesDOMParser;
    
}

///////////////////////////////////////////////////////////////////////////////

CapabilitiesParser::~CapabilitiesParser()
{
   delete m_ConfigFileParser;

   try
   {
      xercesc::XMLString::release( &TAG_root );
      xercesc::XMLString::release( &TAG_cec  );
      xercesc::XMLString::release( &TAG_pc   );
      xercesc::XMLString::release( &TAG_nc   );
      xercesc::XMLString::release( &TAG_fc   );
      xercesc::XMLString::release( &TAG_lbc  );
      xercesc::XMLString::release( &TAG_cc   );
      xercesc::XMLString::release( &TAG_sc   );
   }
   catch( ... )
   {
      TC_LOG_ERR(0,"[CapabilitiesParser::~CapabilitiesParser] Unknown exception encountered");
   }

   // Terminate Xerces
   try
   {
       xercesc::XMLPlatformUtils::Terminate();  // Terminate after release of memory
   }
   catch( xercesc::XMLException& e )
   {
       char* message = xercesc::XMLString::transcode( e.getMessage() );
   
       TC_LOG_ERR(0,"[CapabilitiesParser::~CapabilitiesParser] XML ttolkit teardown error: " + std::string(message));
       xercesc::XMLString::release( &message );
   }
   
}

///////////////////////////////////////////////////////////////////////////////
/*
void CapabilitiesParser::createTag(const std::string& capability)
{
//    std::vector<gloox::Tag*> rootTags;
    std::vector<std::string> ancestors;
    std::string currTagName, attribute, value, cdata;
    std::string::const_iterator currPos = capability.begin();
    ParseState currState=IDLE;
    gloox::Tag *currTag;

    TC_LOG_DEBUG(0,"parsing capability string "<<capability);

    while (currPos < capability.end())
    {
        switch(currState)
        {
          case IDLE:            //looks for opening <, ignores any other chars before it
          {
              TC_LOG_DEBUG(0,"in idle for char "<<*currPos); 
              if (*currPos == '<')
              {
                  TC_LOG_DEBUG(0,"opening tag found in idle, changing to opening_tag");
                  currState = OPENING_TAG;
              }
              break;
          }
          case OPENING_TAG:     //gets the tag name surrounded by spaces, creates tag and adds to queue
          {
              TC_LOG_DEBUG(0,"in OPENING_TAG for char "<<*currPos);
              if ( (*currPos == ' ' || *currPos == '>') && !currTagName.empty())
              {   //we have the full name of the tag
                  //check if child
                  TC_LOG_DEBUG(0,"OPENING_TAG: full opening tag name found as: "<<currTagName);
                  if (!ancestors.empty())  //then there is a parent
                  {
                      TC_LOG_DEBUG(0,"OPENING_TAG: this is a child tag with parent: "<<currTag->name());
                      if (currTag)
                          currTag = new gloox::Tag(currTag,currTagName);
                  }
                  else
                  {   //new root node
                      TC_LOG_DEBUG(0,"OPENING_TAG: this is a root tag, now adding to vector");
                      currTag = new gloox::Tag(currTagName);
                      mCapabilities.push_back(currTag);
                  }
                  ancestors.push_back(currTagName);
                  TC_LOG_DEBUG(0,"OPENING_TAG: changing to ATTRIBUTE");
                  (*currPos == '>') ? (currState = CDATA) : (currState = ATTRIBUTE);
                  currTagName.clear();
                  break;
              }
              else if (*currPos == '/')
              {
                  TC_LOG_DEBUG(0,"OPENING_TAG: found a /");
                  std::string::const_iterator copy = currPos+1;
                  if (copy < capability.end() && *copy == '>') 
                  {
                      TC_LOG_DEBUG(0,"OPENING_TAG: this tag is an empty tag /> found");
                      ++currPos;
                      if (!ancestors.empty())  //then there is a parent
                      {
                          TC_LOG_DEBUG(0,"OPENING_TAG: this tag has a parent from ancestors: "<<currTag->name());
                          TC_LOG_DEBUG(0,"OPENING_TAG: giong to CDATA STATE");
                          if (currTag)
                          {
                              currTag = new gloox::Tag(currTag,currTagName);
                              currTag = currTag->parent();
                              currState = CDATA;
                          } //else there is an error
                      }
                      else
                      {   //new root node
                          TC_LOG_DEBUG(0,"OPENING_TAG: this tag is a root tagm adding to vector, going to IDLE state");
                          currTag = new gloox::Tag(currTagName);
                          mCapabilities.push_back(currTag);
                          currState = IDLE;
                      }
                      //ancestors.push_back(currTagName);
                      //currState = ATTRIBUTE;
                      currTagName.clear();
                  }
                  else
                  {
                      TC_LOG_DEBUG(0,"adding "<<*currPos<<" to tag name");
                      currTagName.append(currPos, currPos+1);
                  }
              }
              else if (*currPos != ' ')
              {
                  TC_LOG_DEBUG(0,"adding "<<*currPos<<" to tag name");
                  currTagName.append(currPos, currPos+1);
              }
              break;
          }
          case CLOSING_TAG:
          {
              TC_LOG_DEBUG(0,"CLOSING_TAG:");
              if(*currPos != ' ')
              {
                  std::string closingTag;
                  while (currPos < capability.end() && *currPos != '>')
                  {
                      closingTag.append(currPos, currPos+1);
                      ++currPos;
                  }
                  ++currPos;
                   TC_LOG_DEBUG(0,"CLOSING_TAG: closing tag name is "<<closingTag);
                  //make sure the closing tag name matches the one popped from ancestors
                  ancestors.pop_back();
                  if (!ancestors.empty())
                  {
                      TC_LOG_DEBUG(0,"CLOSING_TAG: "<<closingTag<<" had a parent "<<currTag->parent()->name());
                      currTag = currTag->parent();
                      currState = CDATA;
                  }
                  else
                  {
                      TC_LOG_DEBUG(0,"CLOSING_TAG: "<<closingTag<<" was a root tag");
                      currTag = NULL;
                      currState = IDLE;
                  }
                  continue;
              }
              break;
          }
          case ATTRIBUTE:
          {
              TC_LOG_DEBUG(0,"ATTRIBUTE: ");
              if (*currPos == '/')
              {
                  TC_LOG_DEBUG(0,"ATTRIBUTE: / found");
                  std::string::const_iterator copy = currPos+1;
                  if (copy < capability.end() && *copy == '>')
                  {
                      TC_LOG_DEBUG(0,"ATTRIBUTE: "<<currTag->name()<<" is a single tag. /> found");
                      ++currPos;
                      TC_LOG_DEBUG(0,"ATTRIBUTE: removing "<<ancestors.back()<<" from the ancestor queue");
                      ancestors.pop_back();
                      if (!ancestors.empty())  //then there is a parent
                      {
                          TC_LOG_DEBUG(0,"ATTRIBUTE: "<<currTag->name()<<" has a parent "<<currTag->parent()->name());
                          if (currTag)
                          {
                              currTag = currTag->parent();
                              currState = CDATA;
                          } //else there is an error
                      }
                      else
                      {   //end of root node
                          TC_LOG_DEBUG(0,"ATTRIBUTE: "<<currTag->name()<<" was a root tag, going into IDLE");
                          currState = IDLE;
                          currTag = NULL;
                      }
                  }
              }
              else if (*currPos == '>')
              {
                  TC_LOG_DEBUG(0,"ATTRIBUTE: tag "<<currTag->name()<<" entering cdata");
                  currState = CDATA;
              }
              else if (*currPos == '\'' && !attribute.empty())
              {
                  TC_LOG_DEBUG(0,"ATTRIBUTE: tag "<<currTag->name()<<" getting attribute value");
                  ++currPos;
                  while (currPos < capability.end() && *currPos != '\'')
                  {
                      value.append(currPos, currPos+1);
                      ++currPos;
                  }
                  if ( !value.empty())
                  {
                      TC_LOG_DEBUG(0,"ATTRIBUTE: tag "<<currTag->name()<<" now adding "<<attribute<<"='"<<value<<"' to the tag");
                      currTag->addAttribute(attribute,value);
                  }
                  attribute.clear();
                  value.clear();
                  //continue;
              }
              else if (*currPos != ' ' && *currPos != '=')
              {
                  TC_LOG_DEBUG(0,"ATTRIBUTE: beginning of attribute name found");
                  while (currPos < capability.end() && *currPos != '=')
                  {
                      attribute.append(currPos, currPos+1);
                      ++currPos;
                  }
                  TC_LOG_DEBUG(0,"ATTRIBUTE: found attribute name "<<attribute<<" for tag "<<currTag->name());
                  continue;
              }
              break;
          }
          case CDATA:
          {
              //if > do nothing
              //if < 
              //    check if next is /, if so then closing tag
              //                        if not then opening tag
              //else if any other character, add to tags.back() cdata
              TC_LOG_DEBUG(0,"CDATA: ");
              if (*currPos == '<')
              {
                  TC_LOG_DEBUG(0,"CDATA: found opening bracket <");
                  std::string::const_iterator copy = currPos+1;
                  if (copy < capability.end() && *copy == '/')
                  {
                      TC_LOG_DEBUG(0,"CDATA: CLOSING tag found, going to CLOSING_TAG");
                      ++currPos;
                      currState = CLOSING_TAG;
                  }
                  else
                  {
                      TC_LOG_DEBUG(0,"CDATA: bracket < was for opening tag. going to OPENING_TAG");
                      currState = OPENING_TAG;
                  }
              }
              else
              {
                  TC_LOG_DEBUG(0,"CDATA: tag:"<<currTag->name()<<" adding cData: "<<std::string(currPos, currPos+1));
                  currTag->addCData(std::string(currPos, currPos+1));
              }
              break;
          }
          default:
              break;
        } //end switch
        ++currPos;
    } //end while
    TC_LOG_DEBUG(0,"FINISHED WHILE LOOP");
}
*/

void CapabilitiesParser::addAttr( xercesc::DOMElement* currentElement, gloox::Tag* currTag )
{
    //add attributes
    xercesc::DOMNamedNodeMap* attributes = currentElement->getAttributes();  
    if (attributes)  //if the attributes is not null  
    {  
        for (XMLSize_t xx = 0; xx < attributes->getLength(); xx++)  
        {  
             xercesc::DOMNode* currentNode = attributes->item(xx);  
             if( currentNode->getNodeType() && //type is not NULL   
                 currentNode->getNodeType() == xercesc::DOMNode::ATTRIBUTE_NODE ) // is attribute  
             {  
                 // Found node which is an Attribute. Re-cast node as attribute  
                 xercesc::DOMAttr* currentAttribute = dynamic_cast<xercesc::DOMAttr*>( currentNode );  
                                                       
                 std::string name(xercesc::XMLString::transcode(currentAttribute->getName()));  
                 std::string value(xercesc::XMLString::transcode(currentAttribute->getValue()));
                 currTag->addAttribute(name,value);
             }  
        }  
    }  
}


void CapabilitiesParser::addChildren( gloox::Tag* parent, xercesc::DOMNodeList* children )
{
    //for each child, create tag connected to chile
    //check to see if they have childremn
        //if so, call addChildren with tag and children
    const XMLSize_t nodeCount = children->getLength();

    for(XMLSize_t i = 0; i < nodeCount; ++i)
    {
        //get child element
        xercesc::DOMNode* currentNode = children->item(i);
        
        if(currentNode->getNodeType())
        {
            switch(currentNode->getNodeType())
            {
              case xercesc::DOMNode::ELEMENT_NODE:
                  {
                  //cast node as an element
                  xercesc::DOMElement* currentElement = dynamic_cast< xercesc::DOMElement* >( currentNode );
    
                  //get tag name
                  const XMLCh* tagName = currentElement->getTagName();
                  std::string tagNameStr(xercesc::XMLString::transcode(tagName));
        TC_LOG_DEBUG(0,"parser [CapabilitiesParser::addChildren] adding child " + tagNameStr + " to parent " + parent->name());
                  gloox::Tag* currTag = new gloox::Tag(parent, tagNameStr);

                  addAttr(currentElement,currTag);
    
                  //add their children ass well
                  xercesc::DOMNodeList* grandchildren = currentElement->getChildNodes();
                  addChildren(currTag,grandchildren);
                  }
                  break;
              case xercesc::DOMNode::CDATA_SECTION_NODE:
              case xercesc::DOMNode::TEXT_NODE:
              case xercesc::DOMNode::COMMENT_NODE:
                  {
                  //cast node as an element
                  xercesc::DOMCharacterData* currentCDATA = dynamic_cast< xercesc::DOMCharacterData* >( currentNode );

                  //add cdata to tag
                  const XMLCh* cData = currentCDATA->getData();
                  std::string cDataStr(xercesc::XMLString::transcode(cData));
        TC_LOG_DEBUG(0,"parser [CapabilitiesParser::addChildren] adding CDATA " + cDataStr + " to parent " + parent->name());

                  parent->addCData(cDataStr);
                  }
                  break;
/*              case xercesc::DOMNode::ATTRIBUTE_NODE:
                  {
                  //cast node as attribute
                  xercesc::DOMAttr* currentAttr = dynamic_cast< xercesc::DOMAttr* >( currentNode );

                  //add attribute to parent
                  const XMLCh* attr = currentAttr->getName();
                  const XMLCh* val = currentAttr->getValue();
                  std::string attrStr(xercesc::XMLString::transcode(attr));
                  std::string valStr(xercesc::XMLString::transcode(val));
TC_LOG_DEBUG(0,"parser [CapabilitiesParser::addChildren] adding attribute " + attrStr + " ='" + valStr + "'");
                  parent->addAttribute(attrStr,valStr);
                  }
                  break;*/
              default:
/*                  TC_LOG_DEBUG(0,"parser  [CapabilitiesParser::addChildren] couldn't catch node type");
                  const XMLCh* attr = currentNode->getNodeName();
                  std::string stringname(xercesc::XMLString::transcode(attr));
                  TC_LOG_DEBUG(0,"parser  [CapabilitiesParser::addChildren] missed node name is " + stringname);*/
                  break;
            }
        }
    }
}

void CapabilitiesParser::Parse(const std::string& filepath)
{
    if (filepath.empty())
        return;
    TC_LOG_DEBUG(0,"parser [CapabilitiesParser::Parse] starting");
//    size_t currPos = 0;

    //setup tags XMLCh* in the header file
    //add xercesc::XercesDOMParser* in header file
    //initialize the tags in the constructor

    //Configure DOM Parser
    m_ConfigFileParser->setValidationScheme( xercesc::XercesDOMParser::Val_Never );
    m_ConfigFileParser->setDoNamespaces( false );
    m_ConfigFileParser->setDoSchema( false );
    m_ConfigFileParser->setLoadExternalDTD( false );
    m_ConfigFileParser->setIncludeIgnorableWhitespace( false );

    try
    {
        m_ConfigFileParser->parse( filepath.c_str() );
        xercesc::DOMDocument* xmlDoc = m_ConfigFileParser->getDocument();
        xercesc::DOMElement* elementRoot = xmlDoc->getDocumentElement();   //get the main document tag?
        if ( !elementRoot )
            TC_LOG_ERR(0,"parser [CapabilitiesParser::Parse] Capability file is empty");
        //Get children
        xercesc::DOMNodeList* children = elementRoot->getChildNodes();
        const XMLSize_t nodeCount = children->getLength();

        //for each child in main XML document, create gloox tag and add to vector
        //the for each of these tags, check to see if they have children and pass to addChildren
        for( XMLSize_t i = 0; i < nodeCount; ++i)
        {
            //get child element
            xercesc::DOMNode* currentNode = children->item(i);
            
            if( currentNode->getNodeType() && currentNode->getNodeType() == xercesc::DOMNode::ELEMENT_NODE )
            {
                //cast node as an element
                xercesc::DOMElement* currentElement = dynamic_cast< xercesc::DOMElement* >( currentNode );

                //get tag name
                const XMLCh* tagName = currentElement->getTagName();
                std::string tagNameStr(xercesc::XMLString::transcode(tagName));
TC_LOG_DEBUG(0,"parser [CapabilitiesParser::Parse] adding tag to mCapabilities named: " + tagNameStr);
                gloox::Tag* currTag = new gloox::Tag(tagNameStr);
                mCapabilities.push_back(currTag);

                addAttr(currentElement,currTag);

                //get children added as well
                xercesc::DOMNodeList* grandchildren = currentElement->getChildNodes();
                addChildren(currTag,grandchildren);
 
            }
            //check for cData
            //check for attributes
            
        }


    }
    catch ( xercesc::XMLException& e )
    {
        char* message = xercesc::XMLString::transcode( e.getMessage() );
        std::ostringstream errBuf;
        TC_LOG_DEBUG(0,"[CapabilitiesParser::Parse] Error parsing file: " + std::string(message));
        xercesc::XMLString::release( &message );
    }

    //setup parser
    //get document
    //get document element (main tag? or does it have to be root)
    //get child nodes into node list
    //for each child in the list add to vector
    //if child has child, add to parent
    

    //parse document and fill vector

    //tear down xerces


/*    while (currPos != std::string::npos)
    {
        currPos = data.find(START_DELIMITER, currPos);
        if (currPos != std::string::npos)
        {
            size_t capabilityStart = currPos + START_DELIMITER_SIZE;
            if ((currPos = data.find(END_DELIMITER, capabilityStart)) != std::string::npos)
            {
                std::string capability(&data[capabilityStart], &data[currPos]);
                boost::trim(capability);
                boost::erase_all(capability, "\t");
                boost::erase_all(capability, "\r");
                boost::erase_all(capability, "\n");
                if (!capability.empty())
                    createTag(capability);
                    //mCapabilities.push_back(capability);
                
                currPos += END_DELIMITER_SIZE;
            }
        }
    }*/
}

///////////////////////////////////////////////////////////////////////////////
