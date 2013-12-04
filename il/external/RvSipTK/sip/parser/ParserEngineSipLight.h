#if defined(RV_SIP_LIGHT) && defined(RV_SIP_PRIMITIVES) && !defined(RV_SIP_IMS_DCS_HEADER_SUPPORT) && !defined(RV_SIP_IMS_HEADER_SUPPORT) && !defined(RV_SIP_EXTENDED_HEADER_SUPPORT) && !defined(RV_SIP_JSR32_SUPPORT) && !defined(RV_SIP_TEL_URI_SUPPORT) && !defined(RV_SIP_AUTH_ON) && !defined(RV_SIP_SUBS_ON)
#ifndef PARSERENGINESIPLIGHT_H_1248033498
#define PARSERENGINESIPLIGHT_H_1248033498

typedef union {
  long alignment;
  char ag_vt_2[sizeof(int)];
  char ag_vt_4[sizeof(ParserBasicToken)];
  char ag_vt_5[sizeof(ParserQuotedString)];
  char ag_vt_6[sizeof(ParserMethod)];
  char ag_vt_7[sizeof(ParserContentLength)];
  char ag_vt_8[sizeof(ParserGenericParamDataVal)];
  char ag_vt_9[sizeof(ParserSipVersion)];
  char ag_vt_10[sizeof(ParserStatusCode)];
  char ag_vt_11[sizeof(ParserReasonPhrase)];
  char ag_vt_12[sizeof(ParserProtocolName)];
  char ag_vt_13[sizeof(ParserProtocolVersion)];
  char ag_vt_14[sizeof(ParserTransport)];
  char ag_vt_15[sizeof(ParserPort)];
  char ag_vt_16[sizeof(ParserViaParam)];
  char ag_vt_17[sizeof(ParserHidden)];
  char ag_vt_18[sizeof(ParserAlias)];
  char ag_vt_19[sizeof(ParserTTLParam)];
  char ag_vt_20[sizeof(ParserTtlParam)];
  char ag_vt_21[sizeof(ParserMaddrParam)];
  char ag_vt_22[sizeof(ParserReceived)];
  char ag_vt_23[sizeof(ParserBranch)];
  char ag_vt_24[sizeof(ParserCompParam)];
  char ag_vt_25[sizeof(ParserSigCompIdParam)];
  char ag_vt_26[sizeof(ParserAbsoluteUri)];
  char ag_vt_27[sizeof(ParserDisplayName)];
  char ag_vt_28[sizeof(ParserUserParam)];
  char ag_vt_29[sizeof(RvSipUserParam)];
  char ag_vt_30[sizeof(ParserOtherUser)];
  char ag_vt_31[sizeof(ParserLrParamType)];
  char ag_vt_32[sizeof(ParserTokenizedByParam)];
  char ag_vt_33[sizeof(RvBool)];
  char ag_vt_34[sizeof(ParserOptionalHeaders)];
  char ag_vt_35[sizeof(ParserTagParam)];
  char ag_vt_36[sizeof(ParserPartyParams)];
} SipParser_vs_type;

typedef enum {
  SipParser_START_token = 1, SipParser_FIRST_COLON_token = 3,
  SipParser_VIA_HEADER_token, SipParser_NULL_token,
  SipParser_ROUTE_HOP_HEADER_token = 7, SipParser_STATUS_LINE_token,
  SipParser_REQUEST_LINE_token, SipParser_ABSOLUTE_URI_HEADER_token = 11,
  SipParser_PRES_URI_HEADER_token, SipParser_IM_URI_HEADER_token,
  SipParser_SIP_URL_HEADER_token, SipParser_OTHER_HEADER_token,
  SipParser_WHITE_token = 17, SipParser_SP_token = 19,
  SipParser_COMMA_token = 21, SipParser_COMMA_BETWEEN_HEADERS_token = 23,
  SipParser_EQUAL_token = 26, SipParser_AT_token = 28,
  SipParser_COLON_token = 30, SipParser_SEMI_COLON_token,
  SipParser_MARK_SIGN_token = 33, SipParser_SLASH_token = 35,
  SipParser_LEFT_ANGLE_BRACKET_token = 37,
  SipParser_RIGHT_ANGLE_BRACKET_token = 39,
  SipParser_LEFT_QUOTATION_token = 41, SipParser_TOKEN_token = 43,
  SipParser_TOKEN_CHAR_token, SipParser_CALL_ID_WORD_token = 46,
  SipParser_CALL_ID_WORD_CHAR_token,
  SipParser_TOKEN_FOR_CONTENT_TYPE_token = 49,
  SipParser_TOKEN_CHAR_FOR_CONTENT_TYPE_token,
  SipParser_FIELD_VALUE_token = 52, SipParser_QUOTED_STRING_token = 58,
  SipParser_QUOTED_CHAR_token, SipParser_METHOD_token = 66,
  SipParser_CONTENT_LENGTH_token = 72, SipParser_CONTENT_LENGTH_VAL_token,
  SipParser_DIGIT_token, SipParser_GENERIC_PARAM_LIST_token = 76,
  SipParser_GENERIC_PARAM_DATA_token,
  SipParser_GENERIC_PARAM_DATA_VAL_token, SipParser_SIP_VERSION_token,
  SipParser_STATUS_CODE_token = 81, SipParser_REASON_PHRASE_token,
  SipParser_EXTENSION_CODE_token = 84, SipParser_REASONE_TEXT_UTF8_token,
  SipParser_VIA_LIST_token = 89, SipParser_SINGLE_VIA_token,
  SipParser_SENT_PROTOCOL_token, SipParser_SENT_BY_token,
  SipParser_VIA_PARAM_LIST_token, SipParser_VIA_PARAM_token,
  SipParser_PROTOCOL_NAME_token, SipParser_PROTOCOL_VERSION_token,
  SipParser_TRANSPORT_token, SipParser_TRANSPORT_TYPE_token = 99,
  SipParser_PORT_token = 104, SipParser_HOST_NAME_token,
  SipParser_VIA_HIDDEN_token, SipParser_VIA_ALIAS_token,
  SipParser_VIA_TTL_token, SipParser_VIA_MADDR_token,
  SipParser_VIA_RECEIVED_token, SipParser_VIA_BRANCH_token,
  SipParser_RESPONSE_PORT_token, SipParser_VIA_COMPRESSION_token,
  SipParser_VIA_SIP_SIGCOMP_ID_token, SipParser_OTHER_PARAM_token,
  SipParser_OTHER_PARAM_EXPR_token, SipParser_IPV_6_REFERENCE_token,
  SipParser_TTL_PARAM_token = 120, SipParser_TTL_token = 122,
  SipParser_RPORT_NAME_token, SipParser_RPORT_VAL_token,
  SipParser_MADDR_PARAM_token = 126,
  SipParser_HOST_NAME_FOR_RECEIVED_token = 129,
  SipParser_COMPRESSION_TYPE_token = 132,
  SipParser_SIGCOMP_ID_TOKEN_token = 135, SipParser_IPV_6_ADDRESS_token,
  SipParser_REQUEST_URI_token, SipParser_ADDR_SPEC_token,
  SipParser_ABSOLUTE_URI_token, SipParser_ABS_URI_SCHEME_token,
  SipParser_URI_IDENTIFIER_token, SipParser_ABS_URI_SCHEME_CHAR_token,
  SipParser_URI_SCHEME_OR_DISPLAY_NAME_token = 144,
  SipParser_DISPLAY_NAME_token, SipParser_URI_IDENTIFIER_CHAR_token,
  SipParser_URI_IDENTIFIER_WITHOUT_SEPARATORS_token = 148,
  SipParser_SIP_URL_token = 151, SipParser_PRES_URI_token,
  SipParser_IM_URI_token, SipParser_SIP_PREFIX_token, SipParser_URL_token,
  SipParser_PRES_PREFIX_token = 158, SipParser_RV_ABS_PREFIX_token,
  SipParser_IM_PREFIX_token = 162, SipParser_URL_USERINFO_token = 164,
  SipParser_OPTIONAL_URL_PARAMETERS_token, SipParser_OPTIONAL_HEADERS_token,
  SipParser_URL_HOST_token, SipParser_GENERAL_URL_WITHOUT_PARAM_token,
  SipParser_SIP_URL_WITHOUT_PARAMS_token,
  SipParser_PRES_URL_WITHOUT_PARAMS_token,
  SipParser_IM_URL_WITHOUT_PARAMS_token,
  SipParser_ABS_URI_WITHOUT_PARAMS_token,
  SipParser_URL_USERINFO_PREFIX_token, SipParser_URL_USERINFO_VAL_token,
  SipParser_HOSTPORT_token, SipParser_USER_INFO_TOKEN_token = 177,
  SipParser_PASSWORD_token, SipParser_TOKEN_USER_INFO_token,
  SipParser_TOKEN_USER_INFO_CHAR_token,
  SipParser_PASSWORD_STRING_token = 182,
  SipParser_PASSWORD_STRING_CHAR_token,
  SipParser_URL_HOST_PREFIX_token = 187, SipParser_TOKEN_HOST_token = 189,
  SipParser_ALPHANUM_token, SipParser_TOKEN_HOST_CHAR_token,
  SipParser_IPV_4_ADDRESS_token = 195, SipParser_IPV_4_PART_token,
  SipParser_HEXPART_token = 198, SipParser_HEXSEQ_token,
  SipParser_HEX4_token = 202, SipParser_HEX_token = 206,
  SipParser_URL_PARAMETERS_token, SipParser_URL_PARAMETER_token,
  SipParser_TRANSPORT_PARAM_token, SipParser_USER_PARAM_token,
  SipParser_TTL_PARAM_URL_token, SipParser_MADDR_PARAM_URL_token,
  SipParser_METHOD_PARAM_URL_token, SipParser_LR_PARAM_URL_token,
  SipParser_COMPRESSION_PARAM_URL_token,
  SipParser_SIGCOMPID_PARAM_URL_token,
  SipParser_TOKENIZED_BY_PARAM_URL_token, SipParser_ORIG_PARAM_URL_token,
  SipParser_URL_GENERIC_PARAM_DATA_token,
  SipParser_USER_PARAM_TYPE_token = 222, SipParser_OTHER_USER_token,
  SipParser_PARAMCHAR_token = 233,
  SipParser_URL_OTHER_PARAM_VAL_token = 239, SipParser_PVALUE_token,
  SipParser_HEADERS_LIST_token, SipParser_HEADER_token,
  SipParser_HNAME_token = 244, SipParser_HVALUE_token,
  SipParser_REQUEST_LINE_VALUE_token = 252, SipParser_REQUEST_METHOD_token,
  SipParser_NAME_ADDR_token = 267, SipParser_MORE_THAN_ONE_TOKEN_token,
  SipParser_TAG_PARAM_token = 272, SipParser_PARTY_PARAMS_LIST_token = 274,
  SipParser_PARTY_PARAM_DATA_token, SipParser_ROUTE_LIST_token,
  SipParser_SINGLE_ROUTE_token
} SipParser_token_type;

typedef struct SipParser_pcb_struct{
  SipParser_token_type token_number, reduction_token, error_frame_token;
  int input_code;
  int input_value;
  int line, column;
  int ssx, sn, error_frame_ssx;
  int drt, dssx, dsn;
  int ss[26];
  SipParser_vs_type vs[26];
  int ag_ap;
  char *error_message;
  char read_flag;
  char exit_flag;
  unsigned char * pointer;
  unsigned char * la_ptr;
  const unsigned char *key_sp;
  int save_index, key_state;
/*  Line 67, RV SIP Stack */
        ParserMgr              *pParserMgr;     
        void                   *pSipObject;     


        SipParseType            eHeaderType;    



        SipParseType            eWhichHeader;   



        RvChar                 *pBuffer;        


                  
        RvStatus                eStat;          
        RvChar                 *pCurToken;      
        void                   *pExtParams;     

        void                   *pUrlExtParams;  
                                              
        RvChar                 *pCurLine;       
		RvInt32					lineNumber;     
       
        RvBool                 isCompactForm;  

        RvBool                 isNewChallenge; 

        RvUint32               parenCount;     
        RvBool                 isWithinAngleBrackets; 



        












        ParserRequestLine       requestLine;    
        ParserRouteHeader       route;          
        ParserNameAddr          nameAddr;       
        ParserExUri             exUri;          
        ParserSipUrl            sipUrl;         
		ParserSingleVia         singleVia;      
        ParserContentType       contentType;    
        ParserUrlParameter      urlParameter;   









		ParserSentProtocol      sentProtocol;   






















        ParserSentByInVia          viaSentBy;       
































	} SipParser_pcb_type;

#ifndef PRULE_CONTEXT
#define PRULE_CONTEXT(pcb)  (&((pcb).cs[(pcb).ssx]))
#define PERROR_CONTEXT(pcb) ((pcb).cs[(pcb).error_frame_ssx])
#define PCONTEXT(pcb)       ((pcb).cs[(pcb).ssx])
#endif

#ifndef AG_RUNNING_CODE_CODE
/* PCB.exit_flag values */
#define AG_RUNNING_CODE         0
#define AG_SUCCESS_CODE         1
#define AG_SYNTAX_ERROR_CODE    2
#define AG_REDUCTION_ERROR_CODE 3
#define AG_STACK_ERROR_CODE     4
#define AG_SEMANTIC_ERROR_CODE  5
#endif
void init_SipParser(SipParser_pcb_type *);
void SipParser(SipParser_pcb_type *);
#endif

#endif
