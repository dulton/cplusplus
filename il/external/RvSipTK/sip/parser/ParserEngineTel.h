#if defined(RV_SIP_TEL_URI_SUPPORT) && !defined(RV_SIP_IMS_DCS_HEADER_SUPPORT) && !defined(RV_SIP_IMS_HEADER_SUPPORT) && !defined(RV_SIP_EXTENDED_HEADER_SUPPORT) && !defined(RV_SIP_JSR32_SUPPORT) && !defined(RV_SIP_LIGHT) && !defined(RV_SIP_PRIMITIVES) && defined(RV_SIP_AUTH_ON) && !defined(RV_SIP_SUBS_ON)
#ifndef PARSERENGINETEL_H_1248033518
#define PARSERENGINETEL_H_1248033518

typedef union {
  long alignment;
  char ag_vt_2[sizeof(int)];
  char ag_vt_4[sizeof(ParserBasicToken)];
  char ag_vt_5[sizeof(ParserQuotedString)];
  char ag_vt_6[sizeof(ParserMethod)];
  char ag_vt_7[sizeof(ParserCSeq)];
  char ag_vt_8[sizeof(ParserSequenceNumber)];
  char ag_vt_9[sizeof(ParserContentLength)];
  char ag_vt_10[sizeof(ParserCallId)];
  char ag_vt_11[sizeof(ParserMediaType)];
  char ag_vt_12[sizeof(ParserMediaSubType)];
  char ag_vt_13[sizeof(ParserBoundary)];
  char ag_vt_14[sizeof(ParserVersion)];
  char ag_vt_15[sizeof(ParserBase)];
  char ag_vt_16[sizeof(ParserGenericParamDataVal)];
  char ag_vt_17[sizeof(ParserSipVersion)];
  char ag_vt_18[sizeof(ParserStatusCode)];
  char ag_vt_19[sizeof(ParserReasonPhrase)];
  char ag_vt_20[sizeof(ParserProtocolName)];
  char ag_vt_21[sizeof(ParserProtocolVersion)];
  char ag_vt_22[sizeof(ParserTransport)];
  char ag_vt_23[sizeof(ParserPort)];
  char ag_vt_24[sizeof(ParserViaParam)];
  char ag_vt_25[sizeof(ParserHidden)];
  char ag_vt_26[sizeof(ParserAlias)];
  char ag_vt_27[sizeof(ParserTTLParam)];
  char ag_vt_28[sizeof(ParserTtlParam)];
  char ag_vt_29[sizeof(ParserMaddrParam)];
  char ag_vt_30[sizeof(ParserReceived)];
  char ag_vt_31[sizeof(ParserBranch)];
  char ag_vt_32[sizeof(ParserCompParam)];
  char ag_vt_33[sizeof(ParserSigCompIdParam)];
  char ag_vt_34[sizeof(ParserAbsoluteUri)];
  char ag_vt_35[sizeof(ParserDisplayName)];
  char ag_vt_36[sizeof(ParserEnumdiParamType)];
  char ag_vt_37[sizeof(ParserUserParam)];
  char ag_vt_38[sizeof(RvSipUserParam)];
  char ag_vt_39[sizeof(ParserOtherUser)];
  char ag_vt_40[sizeof(ParserLrParamType)];
  char ag_vt_41[sizeof(ParserTokenizedByParam)];
  char ag_vt_42[sizeof(RvBool)];
  char ag_vt_43[sizeof(ParserOptionalHeaders)];
  char ag_vt_44[sizeof(ParserTagParam)];
  char ag_vt_45[sizeof(ParserPartyParams)];
  char ag_vt_46[sizeof(ParserQVal)];
  char ag_vt_47[sizeof(ParserContactActionType)];
  char ag_vt_48[sizeof(ParserDeltaSeconds)];
  char ag_vt_49[sizeof(ParserSipDate)];
  char ag_vt_50[sizeof(RvSipDateWeekDay)];
  char ag_vt_51[sizeof(ParserDDMMYY)];
  char ag_vt_52[sizeof(ParserInt32)];
  char ag_vt_53[sizeof(RvSipDateMonth)];
  char ag_vt_54[sizeof(ParserTime)];
  char ag_vt_55[sizeof(ParserTimeZone)];
  char ag_vt_56[sizeof(RvSipAuthStale)];
  char ag_vt_57[sizeof(ParserAlgorithm)];
  char ag_vt_58[sizeof(ParserQopValue)];
  char ag_vt_59[sizeof(ParserAuthQopOptions)];
  char ag_vt_60[sizeof(RvSipAuthQopOption)];
  char ag_vt_61[sizeof(RvSipAuthIntegrityProtected)];
  char ag_vt_62[sizeof(ParserRetryParam)];
  char ag_vt_63[sizeof(ParserRSeq)];
  char ag_vt_64[sizeof(ParserResponseNum)];
  char ag_vt_65[sizeof(ParserEventType)];
  char ag_vt_66[sizeof(ParserDisposition)];
  char ag_vt_67[sizeof(ParserDispositionParams)];
  char ag_vt_68[sizeof(ParserDispositionParam)];
  char ag_vt_69[sizeof(ParserSessionExpiresHeader)];
  char ag_vt_70[sizeof(RvSipSessionExpiresRefresherType)];
  char ag_vt_71[sizeof(ParserMinSEHeader)];
  char ag_vt_72[sizeof(ParserReplacesEarlyFlagParamType)];
} SipParser_vs_type;

typedef enum {
  SipParser_START_token = 1, SipParser_FIRST_COLON_token = 3,
  SipParser_CSEQ_HEADER_token, SipParser_NULL_token,
  SipParser_PARTY_HEADER_token = 7, SipParser_EXPIRES_HEADER_token = 9,
  SipParser_SIP_DATE_HEADER_token = 11, SipParser_CONTACT_HEADER_token = 13,
  SipParser_CALL_ID_HEADER_token = 15, SipParser_CONTENT_LENGTH_token = 17,
  SipParser_VIA_HEADER_token = 19, SipParser_ROUTE_HOP_HEADER_token = 21,
  SipParser_AUTHENTICATION_HEADER_token = 23,
  SipParser_AUTHORIZATION_HEADER_token = 25,
  SipParser_ALLOW_HEADER_token = 27, SipParser_RACK_HEADER_token = 29,
  SipParser_RSEQ_HEADER_token = 31, SipParser_SUPPORTED_HEADER_token = 33,
  SipParser_OPTION_TAG_BASE_HEADER_token = 35,
  SipParser_CONTENT_DISPOSITION_HEADER_token = 37,
  SipParser_CONTENT_TYPE_HEADER_token = 39,
  SipParser_CONTENT_ID_HEADER_token = 41,
  SipParser_RETRY_AFTER_HEADER_token = 43,
  SipParser_SESSION_EXPIRES_HEADER_token = 45,
  SipParser_MIN_SE_HEADER_token = 47, SipParser_REPLACES_HEADER_token = 49,
  SipParser_EVENT_HEADER_token = 51,
  SipParser_AUTHENTICATION_INFO_HEADER_token = 53,
  SipParser_STATUS_LINE_token, SipParser_REQUEST_LINE_token,
  SipParser_ABSOLUTE_URI_HEADER_token = 57, SipParser_PRES_URI_HEADER_token,
  SipParser_IM_URI_HEADER_token, SipParser_SIP_URL_HEADER_token,
  SipParser_TEL_URI_HEADER_token, SipParser_OTHER_HEADER_token,
  SipParser_WHITE_token = 64, SipParser_SP_token = 66,
  SipParser_COMMA_token = 68, SipParser_COMMA_BETWEEN_HEADERS_token = 70,
  SipParser_DOT_token, SipParser_EQUAL_token = 75, SipParser_AT_token = 77,
  SipParser_COLON_token = 79, SipParser_SEMI_COLON_token,
  SipParser_MARK_SIGN_token = 82, SipParser_SLASH_token = 84,
  SipParser_LEFT_PARENTHESIS_token = 86,
  SipParser_LEFT_ANGLE_BRACKET_token = 88,
  SipParser_RIGHT_ANGLE_BRACKET_token = 90,
  SipParser_LEFT_QUOTATION_token = 92, SipParser_HYPHEN_token = 94,
  SipParser_TOKEN_token = 96, SipParser_TOKEN_CHAR_token,
  SipParser_CALL_ID_WORD_token = 99, SipParser_CALL_ID_WORD_CHAR_token,
  SipParser_TOKEN_FOR_CONTENT_TYPE_token = 102,
  SipParser_TOKEN_CHAR_FOR_CONTENT_TYPE_token,
  SipParser_TOKEN_NO_DOT_token = 105, SipParser_TOKEN_CHAR_NO_DOT_token,
  SipParser_FIELD_VALUE_token = 108, SipParser_QUOTED_STRING_token = 114,
  SipParser_QUOTED_CHAR_token, SipParser_HEX_VALUE_token = 122,
  SipParser_HEX_token, SipParser_METHOD_token = 125,
  SipParser_CSEQ_VAL_token = 132, SipParser_SEQUNCE_NUMBER_token,
  SipParser_DIGIT_token, SipParser_CONTENT_LENGTH_VAL_token = 136,
  SipParser_CALL_ID_token, SipParser_MEDIA_TYPE_token,
  SipParser_MEDIA_TYPE_TYPE_token, SipParser_MEDIA_TYPE_SUB_TYPE_token,
  SipParser_OPTIONAL_CONTENT_TYPE_PARAM_LIST_token,
  SipParser_CONTENT_TYPE_PARAM_LIST_token, SipParser_BOUNDARY_PARAM_token,
  SipParser_VERSION_PARAM_token, SipParser_BASE_PARAM_token,
  SipParser_START_PARAM_token, SipParser_TYPE_PARAM_token,
  SipParser_CONTENT_TYPE_GENERIC_PARAM_DATA_token,
  SipParser_START_PARAM_NAME_token = 173,
  SipParser_OLD_OR_NEW_ADDR_SPEC_token,
  SipParser_NEW_ADDR_SPEC_PREFIX_token = 178, SipParser_ADDR_SPEC_token,
  SipParser_OLD_ADDR_SPEC_PREFIX_token, SipParser_USER_INFO_TOKEN_token,
  SipParser_HOST_NAME_token, SipParser_TYPE_PARAM_MEDIA_token = 186,
  SipParser_CONTENT_TYPE_GENERIC_PARAM_DATA_VAL_token,
  SipParser_CONTENT_ID_HEADER_VALUE_token,
  SipParser_CONTENT_ID_HEADER_LEFT_BRACKET_token,
  SipParser_CONTENT_ID_OLD_OR_NEW_ADDR_SPEC_token,
  SipParser_GENERIC_PARAM_LIST_token, SipParser_GENERIC_PARAM_DATA_token,
  SipParser_GENERIC_PARAM_DATA_VAL_token, SipParser_SIP_VERSION_token,
  SipParser_STATUS_CODE_token = 196, SipParser_REASON_PHRASE_token,
  SipParser_EXTENSION_CODE_token = 199, SipParser_REASONE_TEXT_UTF8_token,
  SipParser_ALLOW_VAL_token = 204, SipParser_VIA_LIST_token,
  SipParser_SINGLE_VIA_token, SipParser_SENT_PROTOCOL_token,
  SipParser_SENT_BY_token, SipParser_VIA_PARAM_LIST_token,
  SipParser_VIA_PARAM_token, SipParser_PROTOCOL_NAME_token,
  SipParser_PROTOCOL_VERSION_token, SipParser_TRANSPORT_token,
  SipParser_TRANSPORT_TYPE_token = 215, SipParser_PORT_token = 220,
  SipParser_VIA_HIDDEN_token, SipParser_VIA_ALIAS_token,
  SipParser_VIA_TTL_token, SipParser_VIA_MADDR_token,
  SipParser_VIA_RECEIVED_token, SipParser_VIA_BRANCH_token,
  SipParser_RESPONSE_PORT_token, SipParser_VIA_COMPRESSION_token,
  SipParser_VIA_SIP_SIGCOMP_ID_token, SipParser_OTHER_PARAM_token,
  SipParser_OTHER_PARAM_EXPR_token, SipParser_IPV_6_REFERENCE_token,
  SipParser_TTL_PARAM_token = 235, SipParser_TTL_token = 237,
  SipParser_RPORT_NAME_token, SipParser_RPORT_VAL_token,
  SipParser_MADDR_PARAM_token = 241,
  SipParser_HOST_NAME_FOR_RECEIVED_token = 244,
  SipParser_COMPRESSION_TYPE_token = 247,
  SipParser_SIGCOMP_ID_TOKEN_token = 250, SipParser_IPV_6_ADDRESS_token,
  SipParser_TEST_COMMENT_token, SipParser_COMMENT_TEXT_token,
  SipParser_END_COMMENT_token = 255, SipParser_REQUEST_URI_token = 258,
  SipParser_ABSOLUTE_URI_token, SipParser_ABS_URI_SCHEME_token,
  SipParser_URI_IDENTIFIER_token, SipParser_ABS_URI_SCHEME_CHAR_token,
  SipParser_URI_SCHEME_OR_DISPLAY_NAME_token = 264,
  SipParser_DISPLAY_NAME_token, SipParser_URI_IDENTIFIER_CHAR_token,
  SipParser_URI_IDENTIFIER_WITHOUT_SEPARATORS_token = 268,
  SipParser_TEL_URI_token = 271, SipParser_TEL_PREFIX_token,
  SipParser_TEL_token, SipParser_GLOBAL_OR_LOCAL_NUMBER_token = 275,
  SipParser_OPTIONAL_TEL_URI_PARAMETERS_token,
  SipParser_GLOBAL_NUMBER_token, SipParser_LOCAL_NUMBER_token,
  SipParser_GLOBAL_NUMBER_DIGITS_token, SipParser_PHONEDIGIT_token = 281,
  SipParser_VISUAL_SEPERATOR_token = 285,
  SipParser_LOCAL_NUMBER_DIGITS_token = 287,
  SipParser_HEX_OR_PHONECHAR_token, SipParser_PHONEDIGIT_HEX_token,
  SipParser_PHONECHAR_token = 293, SipParser_TEL_URI_PARAMETERS_token,
  SipParser_TEL_URI_PARAMETER_token,
  SipParser_TEL_URI_EXTENSION_PARAM_token,
  SipParser_TEL_URI_POST_DIAL_PARAM_token,
  SipParser_TEL_URI_ISDN_SUB_ADDR_PARAM_URL_token,
  SipParser_TEL_URI_CONTEXT_PARAM_token,
  SipParser_TEL_URI_ENUMDI_PARAM_token,
  SipParser_URL_GENERIC_PARAM_DATA_token,
  SipParser_TEL_URI_EXTENTION_VALUE_token = 303,
  SipParser_TEL_URI_POST_DIAL_VALUE_token = 306,
  SipParser_TEL_URI_POST_DIAL_CHAR_token, SipParser_DTMFDIGIT_token = 309,
  SipParser_PAUSE_CHAR_token,
  SipParser_TEL_URI_ISDN_SUB_ADDR_VALUE_token = 312, SipParser_URIC_token,
  SipParser_RESERVED_FOR_URIC_token = 315, SipParser_UNRESERVED_token,
  SipParser_PCT_ENCODED_token, SipParser_TEL_URI_CONTEXT_VALUE_token = 320,
  SipParser_DESCRIPTOR_token, SipParser_DOMAINNAME_token,
  SipParser_LABEL_token, SipParser_NEXT_LABEL_token,
  SipParser_ALPHA_token = 328, SipParser_ALPHANUM_OR_HYPHEN_token,
  SipParser_ALPHANUM_token = 333, SipParser_SIP_URL_token = 335,
  SipParser_PRES_URI_token, SipParser_IM_URI_token,
  SipParser_SIP_PREFIX_token, SipParser_URL_token,
  SipParser_PRES_PREFIX_token = 342, SipParser_RV_ABS_PREFIX_token,
  SipParser_IM_PREFIX_token = 346, SipParser_URL_USERINFO_token = 348,
  SipParser_OPTIONAL_URL_PARAMETERS_token, SipParser_OPTIONAL_HEADERS_token,
  SipParser_URL_HOST_token, SipParser_GENERAL_URL_WITHOUT_PARAM_token,
  SipParser_SIP_URL_WITHOUT_PARAMS_token,
  SipParser_TEL_URL_WITHOUT_PARAMS_token,
  SipParser_PRES_URL_WITHOUT_PARAMS_token,
  SipParser_IM_URL_WITHOUT_PARAMS_token,
  SipParser_ABS_URI_WITHOUT_PARAMS_token,
  SipParser_URL_USERINFO_PREFIX_token, SipParser_URL_USERINFO_VAL_token,
  SipParser_HOSTPORT_token, SipParser_PASSWORD_token = 362,
  SipParser_TOKEN_USER_INFO_token, SipParser_TOKEN_USER_INFO_CHAR_token,
  SipParser_PASSWORD_STRING_token = 366,
  SipParser_PASSWORD_STRING_CHAR_token,
  SipParser_URL_HOST_PREFIX_token = 371, SipParser_TOKEN_HOST_token = 373,
  SipParser_TOKEN_HOST_CHAR_token, SipParser_IPV_4_ADDRESS_token = 378,
  SipParser_IPV_4_PART_token, SipParser_HEXPART_token,
  SipParser_HEXSEQ_token, SipParser_HEX4_token = 384,
  SipParser_URL_PARAMETERS_token = 388, SipParser_URL_PARAMETER_token,
  SipParser_TRANSPORT_PARAM_token, SipParser_USER_PARAM_token,
  SipParser_TTL_PARAM_URL_token, SipParser_MADDR_PARAM_URL_token,
  SipParser_METHOD_PARAM_URL_token, SipParser_LR_PARAM_URL_token,
  SipParser_COMPRESSION_PARAM_URL_token,
  SipParser_SIGCOMPID_PARAM_URL_token,
  SipParser_TOKENIZED_BY_PARAM_URL_token, SipParser_ORIG_PARAM_URL_token,
  SipParser_USER_PARAM_TYPE_token = 402, SipParser_OTHER_USER_token,
  SipParser_PARAMCHAR_token = 413,
  SipParser_URL_OTHER_PARAM_VAL_token = 419, SipParser_PVALUE_token,
  SipParser_HEADERS_LIST_token, SipParser_HEADER_token,
  SipParser_HNAME_token = 424, SipParser_HVALUE_token,
  SipParser_REQUEST_LINE_VALUE_token = 432, SipParser_REQUEST_METHOD_token,
  SipParser_PARTY_VALUE_token = 447, SipParser_NAME_ADDR_token,
  SipParser_PARTY_PARAMS_LIST_token, SipParser_MORE_THAN_ONE_TOKEN_token,
  SipParser_TAG_PARAM_token = 454, SipParser_PARTY_PARAM_DATA_token = 456,
  SipParser_CONTACT_HEADER_VALUES_token,
  SipParser_CONTACT_HEADER_OTHER_token,
  SipParser_CONTACT_HEADER_VALUE_LIST_token,
  SipParser_CONTACT_HEADER_VALUE_token = 461,
  SipParser_CONTACT_PARAMS_token, SipParser_CONTACT_PARAM_token,
  SipParser_Q_token, SipParser_ACTION_token,
  SipParser_CONTACT_EXPIRES_token, SipParser_Q_VALUE_token = 468,
  SipParser_Q_VALUE_OPTIONAL_token = 470,
  SipParser_ACTION_VALUE_token = 474, SipParser_EXPIRES_VALUE_token = 477,
  SipParser_EXPIRES_VALUE_FOR_CONTACT_token = 479,
  SipParser_DELTA_SECONDS_token, SipParser_SIP_DATE_token,
  SipParser_SIP_DATE_HEADER_VAL_token, SipParser_DATE_token,
  SipParser_TIME_token, SipParser_DAY_token, SipParser_DD_token = 493,
  SipParser_MM_token, SipParser_YY_token, SipParser_HOUR_token = 508,
  SipParser_MINUTE_token, SipParser_SECOND_token, SipParser_TIME_ZONE_token,
  SipParser_AUTHENTICATION_VAL_token = 515, SipParser_CHALLENGE_VALUE_token,
  SipParser_DIGEST_CHALLENGE_LIST_token, SipParser_AUTH_PARAM_LIST_token,
  SipParser_DIGEST_CHALLENGE_token, SipParser_REALM_token,
  SipParser_DOMAIN_token, SipParser_NONCE_token, SipParser_OPAQUE_token,
  SipParser_INTEGRITY_KEY_token, SipParser_CIPHER_KEY_token,
  SipParser_AUTH_PARAM_token, SipParser_STALE_token,
  SipParser_ALGORITHM_token, SipParser_QOP_OPTIONS_token,
  SipParser_PARAM_IN_NEXT_CHALLENGE_token,
  SipParser_REALM_VALUE_token = 532, SipParser_DOMAIN_EQUAL_token = 534,
  SipParser_DOMAIN_WHITE_token, SipParser_DOMAIN_VAL_token = 537,
  SipParser_URI_LIST_token, SipParser_STRING_DOMAIN_PATCH_token,
  SipParser_TOKEN_FOR_DOMAIN_token, SipParser_TOKEN_CHAR_FOR_DOMAIN_token,
  SipParser_DOMAIN_SPACE_token = 543, SipParser_NONCE_VALUE_token = 546,
  SipParser_OPAQUE_VALUE_token = 548,
  SipParser_ALGORITHM_DECLARATION_token = 551,
  SipParser_ALGORITHM_AND_AKAV_token, SipParser_AKAV_PREFIX_token = 554,
  SipParser_AKAV_token, SipParser_AKA_VERSION_VAL_token,
  SipParser_ALGORITHM_VAL_token, SipParser_ALGORITHM_PREFIX_token,
  SipParser_QOP_VALUE_LIST_token = 566, SipParser_QOP_VALUE_token,
  SipParser_QUOTED_HEX_VALUE_token = 571,
  SipParser_AUTH_PARAM_DATA_token = 574, SipParser_AUTHORIZATION_VAL_token,
  SipParser_CREDENTIALS_VALUE_token, SipParser_DIGEST_RESPONSE_LIST_token,
  SipParser_DIGEST_RESPONSE_token, SipParser_USERNAME_token,
  SipParser_RESPONSE_token, SipParser_CNONCE_token,
  SipParser_NONCE_COUNT_token, SipParser_AUTS_PARAM_token,
  SipParser_INTEGRITY_PROTECTED_token, SipParser_MESSAGE_QOP_token,
  SipParser_DIGEST_URI_token, SipParser_USERNAME_VALUE_token = 588,
  SipParser_DIGEST_URI_VALUE_token = 590,
  SipParser_LEFT_URI_QUOTATION_token, SipParser_CNONCE_VALUE_token = 593,
  SipParser_NC_VALUE_token = 595, SipParser_LHEX_token,
  SipParser_REQUEST_DIGEST_token = 598, SipParser_THIRTY_TWO_LHEX_token,
  SipParser_SIXTY_FOUR_BASE_ENCODING_token = 603,
  SipParser_SIXTY_FOUR_BASE_ENCODING_CHAR_token,
  SipParser_ONE_OR_TWO_EQUAL_SIGNS_token = 606,
  SipParser_ROUTE_LIST_token = 613, SipParser_SINGLE_ROUTE_token,
  SipParser_RETRY_AFTER_VAL_token, SipParser_RETRY_AFTER_COMMENT_token,
  SipParser_RETRY_PARAM_token = 618, SipParser_DURATION_PARAM_token,
  SipParser_RSEQ_VAL_token = 621, SipParser_RESPONSE_NUM_token,
  SipParser_RACK_VAL_token, SipParser_OPTION_TAG_token,
  SipParser_OPTION_TAG_LIST_token, SipParser_SUPPORTED_OPTION_TAG_token,
  SipParser_EVENT_HEADER_VAL_token, SipParser_EVENT_TYPE_token,
  SipParser_EVENT_PARAMS_token, SipParser_EVENT_PACKAGE_token,
  SipParser_EVENT_TEMPLATE_token, SipParser_EVENT_PARAM_token,
  SipParser_EVENT_ID_token, SipParser_DISPOSITION_VAL_token = 635,
  SipParser_DISPOSITION_TYPE_token, SipParser_DISPOSITION_PARAMS_token,
  SipParser_HANDLING_PARAM_token = 643,
  SipParser_SESSION_EXPIRES_VALUE_token = 647,
  SipParser_REFRESHER_TYPE_token, SipParser_REFRESHER_TYPE_NAME_token = 650,
  SipParser_REFRESHER_TYPE_VALUE_token, SipParser_MIN_SE_VALUE_token = 655,
  SipParser_REPLACES_HEADER_VALUE_token, SipParser_REPLACES_PARAMS_token,
  SipParser_REPLACES_PARAM_token, SipParser_TO_TAG_PARAM_token,
  SipParser_FROM_TAG_PARAM_token, SipParser_EARLY_FLAG_PARAM_token,
  SipParser_AUTHENTICATION_INFO_HEADER_LIST_token = 665,
  SipParser_AUTHENTICATION_INFO_HEADER_PARAM_token,
  SipParser_NEXT_NONCE_token, SipParser_RESPONSE_AUTH_token,
  SipParser_MSG_QOP_token, SipParser_RESPONSE_AUTH_VAL_token = 672
} SipParser_token_type;

typedef struct SipParser_pcb_struct{
  SipParser_token_type token_number, reduction_token, error_frame_token;
  int input_code;
  int input_value;
  int line, column;
  int ssx, sn, error_frame_ssx;
  int drt, dssx, dsn;
  int ss[43];
  SipParser_vs_type vs[43];
  int ag_ap;
  char *error_message;
  char read_flag;
  char exit_flag;
  unsigned char * pointer;
  unsigned char * la_ptr;
  const unsigned char *key_sp;
  int save_index, key_state;
  const  int *ag_dtl;
  int ag_dsn;
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



        








        ParserContactHeader     contact;        
        ParserExpiresHeader     expires;        
        ParserPartyHeader       party;          

        ParserRequestLine       requestLine;    
        ParserRouteHeader       route;          
        ParserNameAddr          nameAddr;       
        ParserExUri             exUri;          
        ParserSipUrl            sipUrl;         
		ParserSingleVia         singleVia;      
        ParserContentType       contentType;    
        ParserUrlParameter      urlParameter;   

		ParserTelUri            telUri;         
        ParserTelUriParameter   telUriParameter;
		RvBool                  isTopLabel;     





		ParserSentProtocol      sentProtocol;   

		ParserAuthorizationHeader  authorization; 
        ParserAuthenticationHeader authentication;
        ParserDigestChallenge      digestChallenge;
        ParserAuthenticationInfoHeader auth_info;		



        ParserEvent				   event;       






        ParserContentID            contentID;      
        ParserRetryAfter           retryAfter;  
        ParserContentDisposition   contentDisposition; 
        ParserRack                 rack;        
        ParserReplacesParams       replacesParams; 
        ParserReplacesParam        replacesParam;  

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

#elif defined(RV_SIP_TEL_URI_SUPPORT) && !defined(RV_SIP_IMS_DCS_HEADER_SUPPORT) && !defined(RV_SIP_IMS_HEADER_SUPPORT) && !defined(RV_SIP_EXTENDED_HEADER_SUPPORT) && !defined(RV_SIP_JSR32_SUPPORT) && !defined(RV_SIP_LIGHT) && !defined(RV_SIP_PRIMITIVES) && !defined(RV_SIP_AUTH_ON) && defined(RV_SIP_SUBS_ON)
#ifndef PARSERENGINETEL_H_1248033524
#define PARSERENGINETEL_H_1248033524

typedef union {
  long alignment;
  char ag_vt_2[sizeof(int)];
  char ag_vt_4[sizeof(ParserBasicToken)];
  char ag_vt_5[sizeof(ParserQuotedString)];
  char ag_vt_6[sizeof(ParserMethod)];
  char ag_vt_7[sizeof(ParserCSeq)];
  char ag_vt_8[sizeof(ParserSequenceNumber)];
  char ag_vt_9[sizeof(ParserContentLength)];
  char ag_vt_10[sizeof(ParserCallId)];
  char ag_vt_11[sizeof(ParserMediaType)];
  char ag_vt_12[sizeof(ParserMediaSubType)];
  char ag_vt_13[sizeof(ParserBoundary)];
  char ag_vt_14[sizeof(ParserVersion)];
  char ag_vt_15[sizeof(ParserBase)];
  char ag_vt_16[sizeof(ParserGenericParamDataVal)];
  char ag_vt_17[sizeof(ParserSipVersion)];
  char ag_vt_18[sizeof(ParserStatusCode)];
  char ag_vt_19[sizeof(ParserReasonPhrase)];
  char ag_vt_20[sizeof(ParserProtocolName)];
  char ag_vt_21[sizeof(ParserProtocolVersion)];
  char ag_vt_22[sizeof(ParserTransport)];
  char ag_vt_23[sizeof(ParserPort)];
  char ag_vt_24[sizeof(ParserViaParam)];
  char ag_vt_25[sizeof(ParserHidden)];
  char ag_vt_26[sizeof(ParserAlias)];
  char ag_vt_27[sizeof(ParserTTLParam)];
  char ag_vt_28[sizeof(ParserTtlParam)];
  char ag_vt_29[sizeof(ParserMaddrParam)];
  char ag_vt_30[sizeof(ParserReceived)];
  char ag_vt_31[sizeof(ParserBranch)];
  char ag_vt_32[sizeof(ParserCompParam)];
  char ag_vt_33[sizeof(ParserSigCompIdParam)];
  char ag_vt_34[sizeof(ParserAbsoluteUri)];
  char ag_vt_35[sizeof(ParserDisplayName)];
  char ag_vt_36[sizeof(ParserEnumdiParamType)];
  char ag_vt_37[sizeof(ParserUserParam)];
  char ag_vt_38[sizeof(RvSipUserParam)];
  char ag_vt_39[sizeof(ParserOtherUser)];
  char ag_vt_40[sizeof(ParserLrParamType)];
  char ag_vt_41[sizeof(ParserTokenizedByParam)];
  char ag_vt_42[sizeof(RvBool)];
  char ag_vt_43[sizeof(ParserOptionalHeaders)];
  char ag_vt_44[sizeof(ParserTagParam)];
  char ag_vt_45[sizeof(ParserPartyParams)];
  char ag_vt_46[sizeof(ParserQVal)];
  char ag_vt_47[sizeof(ParserContactActionType)];
  char ag_vt_48[sizeof(ParserDeltaSeconds)];
  char ag_vt_49[sizeof(ParserSipDate)];
  char ag_vt_50[sizeof(RvSipDateWeekDay)];
  char ag_vt_51[sizeof(ParserDDMMYY)];
  char ag_vt_52[sizeof(ParserInt32)];
  char ag_vt_53[sizeof(RvSipDateMonth)];
  char ag_vt_54[sizeof(ParserTime)];
  char ag_vt_55[sizeof(ParserTimeZone)];
  char ag_vt_56[sizeof(ParserRetryParam)];
  char ag_vt_57[sizeof(ParserRSeq)];
  char ag_vt_58[sizeof(ParserResponseNum)];
  char ag_vt_59[sizeof(ParserEventType)];
  char ag_vt_60[sizeof(ParserSubsStateValue)];
  char ag_vt_61[sizeof(ParserSubsStateReason)];
  char ag_vt_62[sizeof(ParserDisposition)];
  char ag_vt_63[sizeof(ParserDispositionParams)];
  char ag_vt_64[sizeof(ParserDispositionParam)];
  char ag_vt_65[sizeof(ParserSessionExpiresHeader)];
  char ag_vt_66[sizeof(RvSipSessionExpiresRefresherType)];
  char ag_vt_67[sizeof(ParserMinSEHeader)];
  char ag_vt_68[sizeof(ParserReplacesEarlyFlagParamType)];
} SipParser_vs_type;

typedef enum {
  SipParser_START_token = 1, SipParser_FIRST_COLON_token = 3,
  SipParser_CSEQ_HEADER_token, SipParser_NULL_token,
  SipParser_PARTY_HEADER_token = 7, SipParser_EXPIRES_HEADER_token = 9,
  SipParser_SIP_DATE_HEADER_token = 11, SipParser_CONTACT_HEADER_token = 13,
  SipParser_CALL_ID_HEADER_token = 15, SipParser_CONTENT_LENGTH_token = 17,
  SipParser_VIA_HEADER_token = 19, SipParser_ROUTE_HOP_HEADER_token = 21,
  SipParser_ALLOW_HEADER_token = 23, SipParser_RACK_HEADER_token = 25,
  SipParser_RSEQ_HEADER_token = 27, SipParser_SUPPORTED_HEADER_token = 29,
  SipParser_OPTION_TAG_BASE_HEADER_token = 31,
  SipParser_CONTENT_DISPOSITION_HEADER_token = 33,
  SipParser_CONTENT_TYPE_HEADER_token = 35,
  SipParser_CONTENT_ID_HEADER_token = 37,
  SipParser_RETRY_AFTER_HEADER_token = 39,
  SipParser_SESSION_EXPIRES_HEADER_token = 41,
  SipParser_MIN_SE_HEADER_token = 43, SipParser_REPLACES_HEADER_token = 45,
  SipParser_EVENT_HEADER_token = 47, SipParser_REFER_TO_HEADER_token = 49,
  SipParser_REFERRED_BY_HEADER_token = 51,
  SipParser_ALLOW_EVENTS_HEADER_token = 53,
  SipParser_SUBSCRIPTION_STATE_HEADER_token = 55,
  SipParser_STATUS_LINE_token, SipParser_REQUEST_LINE_token,
  SipParser_ABSOLUTE_URI_HEADER_token = 59, SipParser_PRES_URI_HEADER_token,
  SipParser_IM_URI_HEADER_token, SipParser_SIP_URL_HEADER_token,
  SipParser_TEL_URI_HEADER_token, SipParser_OTHER_HEADER_token,
  SipParser_WHITE_token = 66, SipParser_SP_token = 68,
  SipParser_COMMA_token = 70, SipParser_COMMA_BETWEEN_HEADERS_token = 72,
  SipParser_DOT_token, SipParser_EQUAL_token = 77, SipParser_AT_token = 79,
  SipParser_COLON_token = 81, SipParser_SEMI_COLON_token,
  SipParser_MARK_SIGN_token = 84, SipParser_SLASH_token = 86,
  SipParser_LEFT_PARENTHESIS_token = 88,
  SipParser_LEFT_ANGLE_BRACKET_token = 90,
  SipParser_RIGHT_ANGLE_BRACKET_token = 92,
  SipParser_LEFT_QUOTATION_token = 94, SipParser_TOKEN_token = 96,
  SipParser_TOKEN_CHAR_token, SipParser_CALL_ID_WORD_token = 99,
  SipParser_CALL_ID_WORD_CHAR_token,
  SipParser_TOKEN_FOR_CONTENT_TYPE_token = 102,
  SipParser_TOKEN_CHAR_FOR_CONTENT_TYPE_token,
  SipParser_TOKEN_NO_DOT_token = 105, SipParser_TOKEN_CHAR_NO_DOT_token,
  SipParser_FIELD_VALUE_token = 108, SipParser_QUOTED_STRING_token = 114,
  SipParser_QUOTED_CHAR_token, SipParser_METHOD_token = 122,
  SipParser_CSEQ_VAL_token = 132, SipParser_SEQUNCE_NUMBER_token,
  SipParser_DIGIT_token, SipParser_CONTENT_LENGTH_VAL_token = 136,
  SipParser_CALL_ID_token, SipParser_MEDIA_TYPE_token,
  SipParser_MEDIA_TYPE_TYPE_token, SipParser_MEDIA_TYPE_SUB_TYPE_token,
  SipParser_OPTIONAL_CONTENT_TYPE_PARAM_LIST_token,
  SipParser_CONTENT_TYPE_PARAM_LIST_token, SipParser_BOUNDARY_PARAM_token,
  SipParser_VERSION_PARAM_token, SipParser_BASE_PARAM_token,
  SipParser_START_PARAM_token, SipParser_TYPE_PARAM_token,
  SipParser_CONTENT_TYPE_GENERIC_PARAM_DATA_token,
  SipParser_START_PARAM_NAME_token = 173,
  SipParser_OLD_OR_NEW_ADDR_SPEC_token,
  SipParser_NEW_ADDR_SPEC_PREFIX_token = 178, SipParser_ADDR_SPEC_token,
  SipParser_OLD_ADDR_SPEC_PREFIX_token, SipParser_USER_INFO_TOKEN_token,
  SipParser_HOST_NAME_token, SipParser_TYPE_PARAM_MEDIA_token = 186,
  SipParser_CONTENT_TYPE_GENERIC_PARAM_DATA_VAL_token,
  SipParser_CONTENT_ID_HEADER_VALUE_token,
  SipParser_CONTENT_ID_HEADER_LEFT_BRACKET_token,
  SipParser_CONTENT_ID_OLD_OR_NEW_ADDR_SPEC_token,
  SipParser_GENERIC_PARAM_LIST_token, SipParser_GENERIC_PARAM_DATA_token,
  SipParser_GENERIC_PARAM_DATA_VAL_token, SipParser_SIP_VERSION_token,
  SipParser_STATUS_CODE_token = 196, SipParser_REASON_PHRASE_token,
  SipParser_EXTENSION_CODE_token = 199, SipParser_REASONE_TEXT_UTF8_token,
  SipParser_ALLOW_VAL_token = 204, SipParser_VIA_LIST_token,
  SipParser_SINGLE_VIA_token, SipParser_SENT_PROTOCOL_token,
  SipParser_SENT_BY_token, SipParser_VIA_PARAM_LIST_token,
  SipParser_VIA_PARAM_token, SipParser_PROTOCOL_NAME_token,
  SipParser_PROTOCOL_VERSION_token, SipParser_TRANSPORT_token,
  SipParser_TRANSPORT_TYPE_token = 215, SipParser_PORT_token = 220,
  SipParser_VIA_HIDDEN_token, SipParser_VIA_ALIAS_token,
  SipParser_VIA_TTL_token, SipParser_VIA_MADDR_token,
  SipParser_VIA_RECEIVED_token, SipParser_VIA_BRANCH_token,
  SipParser_RESPONSE_PORT_token, SipParser_VIA_COMPRESSION_token,
  SipParser_VIA_SIP_SIGCOMP_ID_token, SipParser_OTHER_PARAM_token,
  SipParser_OTHER_PARAM_EXPR_token, SipParser_IPV_6_REFERENCE_token,
  SipParser_TTL_PARAM_token = 235, SipParser_TTL_token = 237,
  SipParser_RPORT_NAME_token, SipParser_RPORT_VAL_token,
  SipParser_MADDR_PARAM_token = 241,
  SipParser_HOST_NAME_FOR_RECEIVED_token = 244,
  SipParser_COMPRESSION_TYPE_token = 247,
  SipParser_SIGCOMP_ID_TOKEN_token = 250, SipParser_IPV_6_ADDRESS_token,
  SipParser_TEST_COMMENT_token, SipParser_COMMENT_TEXT_token,
  SipParser_END_COMMENT_token = 255, SipParser_REQUEST_URI_token = 258,
  SipParser_ABSOLUTE_URI_token, SipParser_ABS_URI_SCHEME_token,
  SipParser_URI_IDENTIFIER_token, SipParser_ABS_URI_SCHEME_CHAR_token,
  SipParser_URI_SCHEME_OR_DISPLAY_NAME_token = 264,
  SipParser_DISPLAY_NAME_token, SipParser_URI_IDENTIFIER_CHAR_token,
  SipParser_URI_IDENTIFIER_WITHOUT_SEPARATORS_token = 268,
  SipParser_TEL_URI_token = 271, SipParser_TEL_PREFIX_token,
  SipParser_TEL_token, SipParser_GLOBAL_OR_LOCAL_NUMBER_token = 275,
  SipParser_OPTIONAL_TEL_URI_PARAMETERS_token,
  SipParser_GLOBAL_NUMBER_token, SipParser_LOCAL_NUMBER_token,
  SipParser_GLOBAL_NUMBER_DIGITS_token, SipParser_PHONEDIGIT_token = 281,
  SipParser_VISUAL_SEPERATOR_token = 285,
  SipParser_LOCAL_NUMBER_DIGITS_token = 287,
  SipParser_HEX_OR_PHONECHAR_token, SipParser_PHONEDIGIT_HEX_token,
  SipParser_HEX_token = 293, SipParser_PHONECHAR_token,
  SipParser_TEL_URI_PARAMETERS_token, SipParser_TEL_URI_PARAMETER_token,
  SipParser_TEL_URI_EXTENSION_PARAM_token,
  SipParser_TEL_URI_POST_DIAL_PARAM_token,
  SipParser_TEL_URI_ISDN_SUB_ADDR_PARAM_URL_token,
  SipParser_TEL_URI_CONTEXT_PARAM_token,
  SipParser_TEL_URI_ENUMDI_PARAM_token,
  SipParser_URL_GENERIC_PARAM_DATA_token,
  SipParser_TEL_URI_EXTENTION_VALUE_token = 304,
  SipParser_TEL_URI_POST_DIAL_VALUE_token = 307,
  SipParser_TEL_URI_POST_DIAL_CHAR_token, SipParser_DTMFDIGIT_token = 310,
  SipParser_PAUSE_CHAR_token,
  SipParser_TEL_URI_ISDN_SUB_ADDR_VALUE_token = 313, SipParser_URIC_token,
  SipParser_RESERVED_FOR_URIC_token = 316, SipParser_UNRESERVED_token,
  SipParser_PCT_ENCODED_token, SipParser_TEL_URI_CONTEXT_VALUE_token = 321,
  SipParser_DESCRIPTOR_token, SipParser_DOMAINNAME_token,
  SipParser_LABEL_token, SipParser_NEXT_LABEL_token,
  SipParser_ALPHA_token = 329, SipParser_ALPHANUM_OR_HYPHEN_token,
  SipParser_ALPHANUM_token = 335, SipParser_SIP_URL_token = 337,
  SipParser_PRES_URI_token, SipParser_IM_URI_token,
  SipParser_SIP_PREFIX_token, SipParser_URL_token,
  SipParser_PRES_PREFIX_token = 344, SipParser_RV_ABS_PREFIX_token,
  SipParser_IM_PREFIX_token = 348, SipParser_URL_USERINFO_token = 350,
  SipParser_OPTIONAL_URL_PARAMETERS_token, SipParser_OPTIONAL_HEADERS_token,
  SipParser_URL_HOST_token, SipParser_GENERAL_URL_WITHOUT_PARAM_token,
  SipParser_SIP_URL_WITHOUT_PARAMS_token,
  SipParser_TEL_URL_WITHOUT_PARAMS_token,
  SipParser_PRES_URL_WITHOUT_PARAMS_token,
  SipParser_IM_URL_WITHOUT_PARAMS_token,
  SipParser_ABS_URI_WITHOUT_PARAMS_token,
  SipParser_URL_USERINFO_PREFIX_token, SipParser_URL_USERINFO_VAL_token,
  SipParser_HOSTPORT_token, SipParser_PASSWORD_token = 364,
  SipParser_TOKEN_USER_INFO_token, SipParser_TOKEN_USER_INFO_CHAR_token,
  SipParser_PASSWORD_STRING_token = 368,
  SipParser_PASSWORD_STRING_CHAR_token,
  SipParser_URL_HOST_PREFIX_token = 373, SipParser_TOKEN_HOST_token = 375,
  SipParser_TOKEN_HOST_CHAR_token, SipParser_IPV_4_ADDRESS_token = 380,
  SipParser_IPV_4_PART_token, SipParser_HEXPART_token,
  SipParser_HEXSEQ_token, SipParser_HEX4_token = 386,
  SipParser_URL_PARAMETERS_token = 390, SipParser_URL_PARAMETER_token,
  SipParser_TRANSPORT_PARAM_token, SipParser_USER_PARAM_token,
  SipParser_TTL_PARAM_URL_token, SipParser_MADDR_PARAM_URL_token,
  SipParser_METHOD_PARAM_URL_token, SipParser_LR_PARAM_URL_token,
  SipParser_COMPRESSION_PARAM_URL_token,
  SipParser_SIGCOMPID_PARAM_URL_token,
  SipParser_TOKENIZED_BY_PARAM_URL_token, SipParser_ORIG_PARAM_URL_token,
  SipParser_USER_PARAM_TYPE_token = 404, SipParser_OTHER_USER_token,
  SipParser_PARAMCHAR_token = 415,
  SipParser_URL_OTHER_PARAM_VAL_token = 421, SipParser_PVALUE_token,
  SipParser_HEADERS_LIST_token, SipParser_HEADER_token,
  SipParser_HNAME_token = 426, SipParser_HVALUE_token,
  SipParser_REQUEST_LINE_VALUE_token = 434, SipParser_REQUEST_METHOD_token,
  SipParser_PARTY_VALUE_token = 449, SipParser_NAME_ADDR_token,
  SipParser_PARTY_PARAMS_LIST_token, SipParser_MORE_THAN_ONE_TOKEN_token,
  SipParser_TAG_PARAM_token = 456, SipParser_PARTY_PARAM_DATA_token = 458,
  SipParser_CONTACT_HEADER_VALUES_token,
  SipParser_CONTACT_HEADER_OTHER_token,
  SipParser_CONTACT_HEADER_VALUE_LIST_token,
  SipParser_CONTACT_HEADER_VALUE_token = 463,
  SipParser_CONTACT_PARAMS_token, SipParser_CONTACT_PARAM_token,
  SipParser_Q_token, SipParser_ACTION_token,
  SipParser_CONTACT_EXPIRES_token, SipParser_Q_VALUE_token = 470,
  SipParser_Q_VALUE_OPTIONAL_token = 472,
  SipParser_ACTION_VALUE_token = 476, SipParser_EXPIRES_VALUE_token = 479,
  SipParser_EXPIRES_VALUE_FOR_CONTACT_token = 481,
  SipParser_DELTA_SECONDS_token, SipParser_SIP_DATE_token,
  SipParser_SIP_DATE_HEADER_VAL_token, SipParser_DATE_token,
  SipParser_TIME_token, SipParser_DAY_token, SipParser_DD_token = 495,
  SipParser_MM_token, SipParser_YY_token, SipParser_HOUR_token = 510,
  SipParser_MINUTE_token, SipParser_SECOND_token, SipParser_TIME_ZONE_token,
  SipParser_ROUTE_LIST_token = 517, SipParser_SINGLE_ROUTE_token,
  SipParser_RETRY_AFTER_VAL_token, SipParser_RETRY_AFTER_COMMENT_token,
  SipParser_RETRY_PARAM_token = 522, SipParser_DURATION_PARAM_token,
  SipParser_RSEQ_VAL_token = 525, SipParser_RESPONSE_NUM_token,
  SipParser_RACK_VAL_token, SipParser_OPTION_TAG_token,
  SipParser_OPTION_TAG_LIST_token, SipParser_SUPPORTED_OPTION_TAG_token,
  SipParser_REFER_TO_HEADER_VALUE_token,
  SipParser_REFERRED_BY_HEADER_VALUE_token = 533,
  SipParser_REFERRER_URI_token, SipParser_REFERRED_BY_PARAMS_token,
  SipParser_REFERRED_BY_PARAM_token = 537,
  SipParser_REFERRED_BY_PARAM_CID_token,
  SipParser_CID_PARAM_STRING_token = 540, SipParser_DOT_ATOM_token,
  SipParser_ATOM_CHAR_token, SipParser_DOT_ATOM_CHAR_token,
  SipParser_EVENT_HEADER_VAL_token = 546, SipParser_EVENT_TYPE_token,
  SipParser_EVENT_PARAMS_token, SipParser_EVENT_PACKAGE_token,
  SipParser_EVENT_TEMPLATE_token, SipParser_EVENT_PARAM_token,
  SipParser_EVENT_ID_token, SipParser_ALLOW_EVENTS_HEADER_VAL_token = 554,
  SipParser_SUBSCRIPTION_STATE_HEADER_VAL_token,
  SipParser_SUBSTATE_VALUE_token, SipParser_SUBSTATE_PARAMS_token,
  SipParser_SUBSTATE_PARAM_token = 561,
  SipParser_SUBSTATE_PARAM_REASON_token,
  SipParser_SUBSTATE_PARAM_RETRY_AFTER_token,
  SipParser_SUBSTATE_PARAM_EXPIRES_token,
  SipParser_SUBSTATE_PARAM_REASON_VAL_token = 567,
  SipParser_DISPOSITION_VAL_token = 574, SipParser_DISPOSITION_TYPE_token,
  SipParser_DISPOSITION_PARAMS_token, SipParser_HANDLING_PARAM_token = 582,
  SipParser_SESSION_EXPIRES_VALUE_token = 586,
  SipParser_REFRESHER_TYPE_token, SipParser_REFRESHER_TYPE_NAME_token,
  SipParser_REFRESHER_TYPE_VALUE_token, SipParser_MIN_SE_VALUE_token = 593,
  SipParser_REPLACES_HEADER_VALUE_token, SipParser_REPLACES_PARAMS_token,
  SipParser_REPLACES_PARAM_token, SipParser_TO_TAG_PARAM_token,
  SipParser_FROM_TAG_PARAM_token, SipParser_EARLY_FLAG_PARAM_token
} SipParser_token_type;

typedef struct SipParser_pcb_struct{
  SipParser_token_type token_number, reduction_token, error_frame_token;
  int input_code;
  int input_value;
  int line, column;
  int ssx, sn, error_frame_ssx;
  int drt, dssx, dsn;
  int ss[29];
  SipParser_vs_type vs[29];
  int ag_ap;
  char *error_message;
  char read_flag;
  char exit_flag;
  unsigned char * pointer;
  unsigned char * la_ptr;
  const unsigned char *key_sp;
  int save_index, key_state;
  const  int *ag_dtl;
  int ag_dsn;
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



        








        ParserContactHeader     contact;        
        ParserExpiresHeader     expires;        
        ParserPartyHeader       party;          

        ParserRequestLine       requestLine;    
        ParserRouteHeader       route;          
        ParserNameAddr          nameAddr;       
        ParserExUri             exUri;          
        ParserSipUrl            sipUrl;         
		ParserSingleVia         singleVia;      
        ParserContentType       contentType;    
        ParserUrlParameter      urlParameter;   

		ParserTelUri            telUri;         
        ParserTelUriParameter   telUriParameter;
		RvBool                  isTopLabel;     





		ParserSentProtocol      sentProtocol;   








        ParserEvent				   event;       

        ParserReferredByHeader     referredBy;  

        ParserReferToHeader        referTo;     
        ParserSubscriptionState    subsState;   

        ParserContentID            contentID;      
        ParserRetryAfter           retryAfter;  
        ParserContentDisposition   contentDisposition; 
        ParserRack                 rack;        
        ParserReplacesParams       replacesParams; 
        ParserReplacesParam        replacesParam;  

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

#elif defined(RV_SIP_TEL_URI_SUPPORT) && !defined(RV_SIP_IMS_DCS_HEADER_SUPPORT) && !defined(RV_SIP_IMS_HEADER_SUPPORT) && !defined(RV_SIP_EXTENDED_HEADER_SUPPORT) && !defined(RV_SIP_JSR32_SUPPORT) && !defined(RV_SIP_LIGHT) && !defined(RV_SIP_PRIMITIVES) && defined(RV_SIP_AUTH_ON) && defined(RV_SIP_SUBS_ON)
#ifndef PARSERENGINETEL_H_1248033528
#define PARSERENGINETEL_H_1248033528

typedef union {
  long alignment;
  char ag_vt_2[sizeof(int)];
  char ag_vt_4[sizeof(ParserBasicToken)];
  char ag_vt_5[sizeof(ParserQuotedString)];
  char ag_vt_6[sizeof(ParserMethod)];
  char ag_vt_7[sizeof(ParserCSeq)];
  char ag_vt_8[sizeof(ParserSequenceNumber)];
  char ag_vt_9[sizeof(ParserContentLength)];
  char ag_vt_10[sizeof(ParserCallId)];
  char ag_vt_11[sizeof(ParserMediaType)];
  char ag_vt_12[sizeof(ParserMediaSubType)];
  char ag_vt_13[sizeof(ParserBoundary)];
  char ag_vt_14[sizeof(ParserVersion)];
  char ag_vt_15[sizeof(ParserBase)];
  char ag_vt_16[sizeof(ParserGenericParamDataVal)];
  char ag_vt_17[sizeof(ParserSipVersion)];
  char ag_vt_18[sizeof(ParserStatusCode)];
  char ag_vt_19[sizeof(ParserReasonPhrase)];
  char ag_vt_20[sizeof(ParserProtocolName)];
  char ag_vt_21[sizeof(ParserProtocolVersion)];
  char ag_vt_22[sizeof(ParserTransport)];
  char ag_vt_23[sizeof(ParserPort)];
  char ag_vt_24[sizeof(ParserViaParam)];
  char ag_vt_25[sizeof(ParserHidden)];
  char ag_vt_26[sizeof(ParserAlias)];
  char ag_vt_27[sizeof(ParserTTLParam)];
  char ag_vt_28[sizeof(ParserTtlParam)];
  char ag_vt_29[sizeof(ParserMaddrParam)];
  char ag_vt_30[sizeof(ParserReceived)];
  char ag_vt_31[sizeof(ParserBranch)];
  char ag_vt_32[sizeof(ParserCompParam)];
  char ag_vt_33[sizeof(ParserSigCompIdParam)];
  char ag_vt_34[sizeof(ParserAbsoluteUri)];
  char ag_vt_35[sizeof(ParserDisplayName)];
  char ag_vt_36[sizeof(ParserEnumdiParamType)];
  char ag_vt_37[sizeof(ParserUserParam)];
  char ag_vt_38[sizeof(RvSipUserParam)];
  char ag_vt_39[sizeof(ParserOtherUser)];
  char ag_vt_40[sizeof(ParserLrParamType)];
  char ag_vt_41[sizeof(ParserTokenizedByParam)];
  char ag_vt_42[sizeof(RvBool)];
  char ag_vt_43[sizeof(ParserOptionalHeaders)];
  char ag_vt_44[sizeof(ParserTagParam)];
  char ag_vt_45[sizeof(ParserPartyParams)];
  char ag_vt_46[sizeof(ParserQVal)];
  char ag_vt_47[sizeof(ParserContactActionType)];
  char ag_vt_48[sizeof(ParserDeltaSeconds)];
  char ag_vt_49[sizeof(ParserSipDate)];
  char ag_vt_50[sizeof(RvSipDateWeekDay)];
  char ag_vt_51[sizeof(ParserDDMMYY)];
  char ag_vt_52[sizeof(ParserInt32)];
  char ag_vt_53[sizeof(RvSipDateMonth)];
  char ag_vt_54[sizeof(ParserTime)];
  char ag_vt_55[sizeof(ParserTimeZone)];
  char ag_vt_56[sizeof(RvSipAuthStale)];
  char ag_vt_57[sizeof(ParserAlgorithm)];
  char ag_vt_58[sizeof(ParserQopValue)];
  char ag_vt_59[sizeof(ParserAuthQopOptions)];
  char ag_vt_60[sizeof(RvSipAuthQopOption)];
  char ag_vt_61[sizeof(RvSipAuthIntegrityProtected)];
  char ag_vt_62[sizeof(ParserRetryParam)];
  char ag_vt_63[sizeof(ParserRSeq)];
  char ag_vt_64[sizeof(ParserResponseNum)];
  char ag_vt_65[sizeof(ParserEventType)];
  char ag_vt_66[sizeof(ParserSubsStateValue)];
  char ag_vt_67[sizeof(ParserSubsStateReason)];
  char ag_vt_68[sizeof(ParserDisposition)];
  char ag_vt_69[sizeof(ParserDispositionParams)];
  char ag_vt_70[sizeof(ParserDispositionParam)];
  char ag_vt_71[sizeof(ParserSessionExpiresHeader)];
  char ag_vt_72[sizeof(RvSipSessionExpiresRefresherType)];
  char ag_vt_73[sizeof(ParserMinSEHeader)];
  char ag_vt_74[sizeof(ParserReplacesEarlyFlagParamType)];
} SipParser_vs_type;

typedef enum {
  SipParser_START_token = 1, SipParser_FIRST_COLON_token = 3,
  SipParser_CSEQ_HEADER_token, SipParser_NULL_token,
  SipParser_PARTY_HEADER_token = 7, SipParser_EXPIRES_HEADER_token = 9,
  SipParser_SIP_DATE_HEADER_token = 11, SipParser_CONTACT_HEADER_token = 13,
  SipParser_CALL_ID_HEADER_token = 15, SipParser_CONTENT_LENGTH_token = 17,
  SipParser_VIA_HEADER_token = 19, SipParser_ROUTE_HOP_HEADER_token = 21,
  SipParser_AUTHENTICATION_HEADER_token = 23,
  SipParser_AUTHORIZATION_HEADER_token = 25,
  SipParser_ALLOW_HEADER_token = 27, SipParser_RACK_HEADER_token = 29,
  SipParser_RSEQ_HEADER_token = 31, SipParser_SUPPORTED_HEADER_token = 33,
  SipParser_OPTION_TAG_BASE_HEADER_token = 35,
  SipParser_CONTENT_DISPOSITION_HEADER_token = 37,
  SipParser_CONTENT_TYPE_HEADER_token = 39,
  SipParser_CONTENT_ID_HEADER_token = 41,
  SipParser_RETRY_AFTER_HEADER_token = 43,
  SipParser_SESSION_EXPIRES_HEADER_token = 45,
  SipParser_MIN_SE_HEADER_token = 47, SipParser_REPLACES_HEADER_token = 49,
  SipParser_EVENT_HEADER_token = 51, SipParser_REFER_TO_HEADER_token = 53,
  SipParser_REFERRED_BY_HEADER_token = 55,
  SipParser_ALLOW_EVENTS_HEADER_token = 57,
  SipParser_SUBSCRIPTION_STATE_HEADER_token = 59,
  SipParser_AUTHENTICATION_INFO_HEADER_token = 61,
  SipParser_STATUS_LINE_token, SipParser_REQUEST_LINE_token,
  SipParser_ABSOLUTE_URI_HEADER_token = 65, SipParser_PRES_URI_HEADER_token,
  SipParser_IM_URI_HEADER_token, SipParser_SIP_URL_HEADER_token,
  SipParser_TEL_URI_HEADER_token, SipParser_OTHER_HEADER_token,
  SipParser_WHITE_token = 72, SipParser_SP_token = 74,
  SipParser_COMMA_token = 76, SipParser_COMMA_BETWEEN_HEADERS_token = 78,
  SipParser_DOT_token, SipParser_EQUAL_token = 83, SipParser_AT_token = 85,
  SipParser_COLON_token = 87, SipParser_SEMI_COLON_token,
  SipParser_MARK_SIGN_token = 90, SipParser_SLASH_token = 92,
  SipParser_LEFT_PARENTHESIS_token = 94,
  SipParser_LEFT_ANGLE_BRACKET_token = 96,
  SipParser_RIGHT_ANGLE_BRACKET_token = 98,
  SipParser_LEFT_QUOTATION_token = 100, SipParser_HYPHEN_token = 102,
  SipParser_TOKEN_token = 104, SipParser_TOKEN_CHAR_token,
  SipParser_CALL_ID_WORD_token = 107, SipParser_CALL_ID_WORD_CHAR_token,
  SipParser_TOKEN_FOR_CONTENT_TYPE_token = 110,
  SipParser_TOKEN_CHAR_FOR_CONTENT_TYPE_token,
  SipParser_TOKEN_NO_DOT_token = 113, SipParser_TOKEN_CHAR_NO_DOT_token,
  SipParser_FIELD_VALUE_token = 116, SipParser_QUOTED_STRING_token = 122,
  SipParser_QUOTED_CHAR_token, SipParser_HEX_VALUE_token = 130,
  SipParser_HEX_token, SipParser_METHOD_token = 133,
  SipParser_CSEQ_VAL_token = 143, SipParser_SEQUNCE_NUMBER_token,
  SipParser_DIGIT_token, SipParser_CONTENT_LENGTH_VAL_token = 147,
  SipParser_CALL_ID_token, SipParser_MEDIA_TYPE_token,
  SipParser_MEDIA_TYPE_TYPE_token, SipParser_MEDIA_TYPE_SUB_TYPE_token,
  SipParser_OPTIONAL_CONTENT_TYPE_PARAM_LIST_token,
  SipParser_CONTENT_TYPE_PARAM_LIST_token, SipParser_BOUNDARY_PARAM_token,
  SipParser_VERSION_PARAM_token, SipParser_BASE_PARAM_token,
  SipParser_START_PARAM_token, SipParser_TYPE_PARAM_token,
  SipParser_CONTENT_TYPE_GENERIC_PARAM_DATA_token,
  SipParser_START_PARAM_NAME_token = 184,
  SipParser_OLD_OR_NEW_ADDR_SPEC_token,
  SipParser_NEW_ADDR_SPEC_PREFIX_token = 189, SipParser_ADDR_SPEC_token,
  SipParser_OLD_ADDR_SPEC_PREFIX_token, SipParser_USER_INFO_TOKEN_token,
  SipParser_HOST_NAME_token, SipParser_TYPE_PARAM_MEDIA_token = 197,
  SipParser_CONTENT_TYPE_GENERIC_PARAM_DATA_VAL_token,
  SipParser_CONTENT_ID_HEADER_VALUE_token,
  SipParser_CONTENT_ID_HEADER_LEFT_BRACKET_token,
  SipParser_CONTENT_ID_OLD_OR_NEW_ADDR_SPEC_token,
  SipParser_GENERIC_PARAM_LIST_token, SipParser_GENERIC_PARAM_DATA_token,
  SipParser_GENERIC_PARAM_DATA_VAL_token, SipParser_SIP_VERSION_token,
  SipParser_STATUS_CODE_token = 207, SipParser_REASON_PHRASE_token,
  SipParser_EXTENSION_CODE_token = 210, SipParser_REASONE_TEXT_UTF8_token,
  SipParser_ALLOW_VAL_token = 215, SipParser_VIA_LIST_token,
  SipParser_SINGLE_VIA_token, SipParser_SENT_PROTOCOL_token,
  SipParser_SENT_BY_token, SipParser_VIA_PARAM_LIST_token,
  SipParser_VIA_PARAM_token, SipParser_PROTOCOL_NAME_token,
  SipParser_PROTOCOL_VERSION_token, SipParser_TRANSPORT_token,
  SipParser_TRANSPORT_TYPE_token = 226, SipParser_PORT_token = 231,
  SipParser_VIA_HIDDEN_token, SipParser_VIA_ALIAS_token,
  SipParser_VIA_TTL_token, SipParser_VIA_MADDR_token,
  SipParser_VIA_RECEIVED_token, SipParser_VIA_BRANCH_token,
  SipParser_RESPONSE_PORT_token, SipParser_VIA_COMPRESSION_token,
  SipParser_VIA_SIP_SIGCOMP_ID_token, SipParser_OTHER_PARAM_token,
  SipParser_OTHER_PARAM_EXPR_token, SipParser_IPV_6_REFERENCE_token,
  SipParser_TTL_PARAM_token = 246, SipParser_TTL_token = 248,
  SipParser_RPORT_NAME_token, SipParser_RPORT_VAL_token,
  SipParser_MADDR_PARAM_token = 252,
  SipParser_HOST_NAME_FOR_RECEIVED_token = 255,
  SipParser_COMPRESSION_TYPE_token = 258,
  SipParser_SIGCOMP_ID_TOKEN_token = 261, SipParser_IPV_6_ADDRESS_token,
  SipParser_TEST_COMMENT_token, SipParser_COMMENT_TEXT_token,
  SipParser_END_COMMENT_token = 266, SipParser_REQUEST_URI_token = 269,
  SipParser_ABSOLUTE_URI_token, SipParser_ABS_URI_SCHEME_token,
  SipParser_URI_IDENTIFIER_token, SipParser_ABS_URI_SCHEME_CHAR_token,
  SipParser_URI_SCHEME_OR_DISPLAY_NAME_token = 275,
  SipParser_DISPLAY_NAME_token, SipParser_URI_IDENTIFIER_CHAR_token,
  SipParser_URI_IDENTIFIER_WITHOUT_SEPARATORS_token = 279,
  SipParser_TEL_URI_token = 282, SipParser_TEL_PREFIX_token,
  SipParser_TEL_token, SipParser_GLOBAL_OR_LOCAL_NUMBER_token = 286,
  SipParser_OPTIONAL_TEL_URI_PARAMETERS_token,
  SipParser_GLOBAL_NUMBER_token, SipParser_LOCAL_NUMBER_token,
  SipParser_GLOBAL_NUMBER_DIGITS_token, SipParser_PHONEDIGIT_token = 292,
  SipParser_VISUAL_SEPERATOR_token = 296,
  SipParser_LOCAL_NUMBER_DIGITS_token = 298,
  SipParser_HEX_OR_PHONECHAR_token, SipParser_PHONEDIGIT_HEX_token,
  SipParser_PHONECHAR_token = 304, SipParser_TEL_URI_PARAMETERS_token,
  SipParser_TEL_URI_PARAMETER_token,
  SipParser_TEL_URI_EXTENSION_PARAM_token,
  SipParser_TEL_URI_POST_DIAL_PARAM_token,
  SipParser_TEL_URI_ISDN_SUB_ADDR_PARAM_URL_token,
  SipParser_TEL_URI_CONTEXT_PARAM_token,
  SipParser_TEL_URI_ENUMDI_PARAM_token,
  SipParser_URL_GENERIC_PARAM_DATA_token,
  SipParser_TEL_URI_EXTENTION_VALUE_token = 314,
  SipParser_TEL_URI_POST_DIAL_VALUE_token = 317,
  SipParser_TEL_URI_POST_DIAL_CHAR_token, SipParser_DTMFDIGIT_token = 320,
  SipParser_PAUSE_CHAR_token,
  SipParser_TEL_URI_ISDN_SUB_ADDR_VALUE_token = 323, SipParser_URIC_token,
  SipParser_RESERVED_FOR_URIC_token = 326, SipParser_UNRESERVED_token,
  SipParser_PCT_ENCODED_token, SipParser_TEL_URI_CONTEXT_VALUE_token = 331,
  SipParser_DESCRIPTOR_token, SipParser_DOMAINNAME_token,
  SipParser_LABEL_token, SipParser_NEXT_LABEL_token,
  SipParser_ALPHA_token = 339, SipParser_ALPHANUM_OR_HYPHEN_token,
  SipParser_ALPHANUM_token = 344, SipParser_SIP_URL_token = 346,
  SipParser_PRES_URI_token, SipParser_IM_URI_token,
  SipParser_SIP_PREFIX_token, SipParser_URL_token,
  SipParser_PRES_PREFIX_token = 353, SipParser_RV_ABS_PREFIX_token,
  SipParser_IM_PREFIX_token = 357, SipParser_URL_USERINFO_token = 359,
  SipParser_OPTIONAL_URL_PARAMETERS_token, SipParser_OPTIONAL_HEADERS_token,
  SipParser_URL_HOST_token, SipParser_GENERAL_URL_WITHOUT_PARAM_token,
  SipParser_SIP_URL_WITHOUT_PARAMS_token,
  SipParser_TEL_URL_WITHOUT_PARAMS_token,
  SipParser_PRES_URL_WITHOUT_PARAMS_token,
  SipParser_IM_URL_WITHOUT_PARAMS_token,
  SipParser_ABS_URI_WITHOUT_PARAMS_token,
  SipParser_URL_USERINFO_PREFIX_token, SipParser_URL_USERINFO_VAL_token,
  SipParser_HOSTPORT_token, SipParser_PASSWORD_token = 373,
  SipParser_TOKEN_USER_INFO_token, SipParser_TOKEN_USER_INFO_CHAR_token,
  SipParser_PASSWORD_STRING_token = 377,
  SipParser_PASSWORD_STRING_CHAR_token,
  SipParser_URL_HOST_PREFIX_token = 382, SipParser_TOKEN_HOST_token = 384,
  SipParser_TOKEN_HOST_CHAR_token, SipParser_IPV_4_ADDRESS_token = 389,
  SipParser_IPV_4_PART_token, SipParser_HEXPART_token,
  SipParser_HEXSEQ_token, SipParser_HEX4_token = 395,
  SipParser_URL_PARAMETERS_token = 399, SipParser_URL_PARAMETER_token,
  SipParser_TRANSPORT_PARAM_token, SipParser_USER_PARAM_token,
  SipParser_TTL_PARAM_URL_token, SipParser_MADDR_PARAM_URL_token,
  SipParser_METHOD_PARAM_URL_token, SipParser_LR_PARAM_URL_token,
  SipParser_COMPRESSION_PARAM_URL_token,
  SipParser_SIGCOMPID_PARAM_URL_token,
  SipParser_TOKENIZED_BY_PARAM_URL_token, SipParser_ORIG_PARAM_URL_token,
  SipParser_USER_PARAM_TYPE_token = 413, SipParser_OTHER_USER_token,
  SipParser_PARAMCHAR_token = 424,
  SipParser_URL_OTHER_PARAM_VAL_token = 430, SipParser_PVALUE_token,
  SipParser_HEADERS_LIST_token, SipParser_HEADER_token,
  SipParser_HNAME_token = 435, SipParser_HVALUE_token,
  SipParser_REQUEST_LINE_VALUE_token = 443, SipParser_REQUEST_METHOD_token,
  SipParser_PARTY_VALUE_token = 458, SipParser_NAME_ADDR_token,
  SipParser_PARTY_PARAMS_LIST_token, SipParser_MORE_THAN_ONE_TOKEN_token,
  SipParser_TAG_PARAM_token = 465, SipParser_PARTY_PARAM_DATA_token = 467,
  SipParser_CONTACT_HEADER_VALUES_token,
  SipParser_CONTACT_HEADER_OTHER_token,
  SipParser_CONTACT_HEADER_VALUE_LIST_token,
  SipParser_CONTACT_HEADER_VALUE_token = 472,
  SipParser_CONTACT_PARAMS_token, SipParser_CONTACT_PARAM_token,
  SipParser_Q_token, SipParser_ACTION_token,
  SipParser_CONTACT_EXPIRES_token, SipParser_Q_VALUE_token = 479,
  SipParser_Q_VALUE_OPTIONAL_token = 481,
  SipParser_ACTION_VALUE_token = 485, SipParser_EXPIRES_VALUE_token = 488,
  SipParser_EXPIRES_VALUE_FOR_CONTACT_token = 490,
  SipParser_DELTA_SECONDS_token, SipParser_SIP_DATE_token,
  SipParser_SIP_DATE_HEADER_VAL_token, SipParser_DATE_token,
  SipParser_TIME_token, SipParser_DAY_token, SipParser_DD_token = 504,
  SipParser_MM_token, SipParser_YY_token, SipParser_HOUR_token = 519,
  SipParser_MINUTE_token, SipParser_SECOND_token, SipParser_TIME_ZONE_token,
  SipParser_AUTHENTICATION_VAL_token = 526, SipParser_CHALLENGE_VALUE_token,
  SipParser_DIGEST_CHALLENGE_LIST_token, SipParser_AUTH_PARAM_LIST_token,
  SipParser_DIGEST_CHALLENGE_token, SipParser_REALM_token,
  SipParser_DOMAIN_token, SipParser_NONCE_token, SipParser_OPAQUE_token,
  SipParser_INTEGRITY_KEY_token, SipParser_CIPHER_KEY_token,
  SipParser_AUTH_PARAM_token, SipParser_STALE_token,
  SipParser_ALGORITHM_token, SipParser_QOP_OPTIONS_token,
  SipParser_PARAM_IN_NEXT_CHALLENGE_token,
  SipParser_REALM_VALUE_token = 543, SipParser_DOMAIN_EQUAL_token = 545,
  SipParser_DOMAIN_WHITE_token, SipParser_DOMAIN_VAL_token = 548,
  SipParser_URI_LIST_token, SipParser_STRING_DOMAIN_PATCH_token,
  SipParser_TOKEN_FOR_DOMAIN_token, SipParser_TOKEN_CHAR_FOR_DOMAIN_token,
  SipParser_DOMAIN_SPACE_token = 554, SipParser_NONCE_VALUE_token = 557,
  SipParser_OPAQUE_VALUE_token = 559,
  SipParser_ALGORITHM_DECLARATION_token = 562,
  SipParser_ALGORITHM_AND_AKAV_token, SipParser_AKAV_PREFIX_token = 565,
  SipParser_AKAV_token, SipParser_AKA_VERSION_VAL_token,
  SipParser_ALGORITHM_VAL_token, SipParser_ALGORITHM_PREFIX_token,
  SipParser_QOP_VALUE_LIST_token = 577, SipParser_QOP_VALUE_token,
  SipParser_QUOTED_HEX_VALUE_token = 582,
  SipParser_AUTH_PARAM_DATA_token = 585, SipParser_AUTHORIZATION_VAL_token,
  SipParser_CREDENTIALS_VALUE_token, SipParser_DIGEST_RESPONSE_LIST_token,
  SipParser_DIGEST_RESPONSE_token, SipParser_USERNAME_token,
  SipParser_RESPONSE_token, SipParser_CNONCE_token,
  SipParser_NONCE_COUNT_token, SipParser_AUTS_PARAM_token,
  SipParser_INTEGRITY_PROTECTED_token, SipParser_MESSAGE_QOP_token,
  SipParser_DIGEST_URI_token, SipParser_USERNAME_VALUE_token = 599,
  SipParser_DIGEST_URI_VALUE_token = 601,
  SipParser_LEFT_URI_QUOTATION_token, SipParser_CNONCE_VALUE_token = 604,
  SipParser_NC_VALUE_token = 606, SipParser_LHEX_token,
  SipParser_REQUEST_DIGEST_token = 609, SipParser_THIRTY_TWO_LHEX_token,
  SipParser_SIXTY_FOUR_BASE_ENCODING_token = 614,
  SipParser_SIXTY_FOUR_BASE_ENCODING_CHAR_token,
  SipParser_ONE_OR_TWO_EQUAL_SIGNS_token = 617,
  SipParser_ROUTE_LIST_token = 624, SipParser_SINGLE_ROUTE_token,
  SipParser_RETRY_AFTER_VAL_token, SipParser_RETRY_AFTER_COMMENT_token,
  SipParser_RETRY_PARAM_token = 629, SipParser_DURATION_PARAM_token,
  SipParser_RSEQ_VAL_token = 632, SipParser_RESPONSE_NUM_token,
  SipParser_RACK_VAL_token, SipParser_OPTION_TAG_token,
  SipParser_OPTION_TAG_LIST_token, SipParser_SUPPORTED_OPTION_TAG_token,
  SipParser_REFER_TO_HEADER_VALUE_token,
  SipParser_REFERRED_BY_HEADER_VALUE_token = 640,
  SipParser_REFERRER_URI_token, SipParser_REFERRED_BY_PARAMS_token,
  SipParser_REFERRED_BY_PARAM_token = 644,
  SipParser_REFERRED_BY_PARAM_CID_token,
  SipParser_CID_PARAM_STRING_token = 647, SipParser_DOT_ATOM_token,
  SipParser_ATOM_CHAR_token, SipParser_DOT_ATOM_CHAR_token,
  SipParser_EVENT_HEADER_VAL_token = 653, SipParser_EVENT_TYPE_token,
  SipParser_EVENT_PARAMS_token, SipParser_EVENT_PACKAGE_token,
  SipParser_EVENT_TEMPLATE_token, SipParser_EVENT_PARAM_token,
  SipParser_EVENT_ID_token, SipParser_ALLOW_EVENTS_HEADER_VAL_token = 661,
  SipParser_SUBSCRIPTION_STATE_HEADER_VAL_token,
  SipParser_SUBSTATE_VALUE_token, SipParser_SUBSTATE_PARAMS_token,
  SipParser_SUBSTATE_PARAM_token = 668,
  SipParser_SUBSTATE_PARAM_REASON_token,
  SipParser_SUBSTATE_PARAM_RETRY_AFTER_token,
  SipParser_SUBSTATE_PARAM_EXPIRES_token,
  SipParser_SUBSTATE_PARAM_REASON_VAL_token = 674,
  SipParser_DISPOSITION_VAL_token = 681, SipParser_DISPOSITION_TYPE_token,
  SipParser_DISPOSITION_PARAMS_token, SipParser_HANDLING_PARAM_token = 689,
  SipParser_SESSION_EXPIRES_VALUE_token = 693,
  SipParser_REFRESHER_TYPE_token, SipParser_REFRESHER_TYPE_NAME_token,
  SipParser_REFRESHER_TYPE_VALUE_token, SipParser_MIN_SE_VALUE_token = 700,
  SipParser_REPLACES_HEADER_VALUE_token, SipParser_REPLACES_PARAMS_token,
  SipParser_REPLACES_PARAM_token, SipParser_TO_TAG_PARAM_token,
  SipParser_FROM_TAG_PARAM_token, SipParser_EARLY_FLAG_PARAM_token,
  SipParser_AUTHENTICATION_INFO_HEADER_LIST_token = 710,
  SipParser_AUTHENTICATION_INFO_HEADER_PARAM_token,
  SipParser_NEXT_NONCE_token, SipParser_RESPONSE_AUTH_token,
  SipParser_MSG_QOP_token, SipParser_RESPONSE_AUTH_VAL_token = 717
} SipParser_token_type;

typedef struct SipParser_pcb_struct{
  SipParser_token_type token_number, reduction_token, error_frame_token;
  int input_code;
  int input_value;
  int line, column;
  int ssx, sn, error_frame_ssx;
  int drt, dssx, dsn;
  int ss[43];
  SipParser_vs_type vs[43];
  int ag_ap;
  char *error_message;
  char read_flag;
  char exit_flag;
  unsigned char * pointer;
  unsigned char * la_ptr;
  const unsigned char *key_sp;
  int save_index, key_state;
  const  int *ag_dtl;
  int ag_dsn;
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



        








        ParserContactHeader     contact;        
        ParserExpiresHeader     expires;        
        ParserPartyHeader       party;          

        ParserRequestLine       requestLine;    
        ParserRouteHeader       route;          
        ParserNameAddr          nameAddr;       
        ParserExUri             exUri;          
        ParserSipUrl            sipUrl;         
		ParserSingleVia         singleVia;      
        ParserContentType       contentType;    
        ParserUrlParameter      urlParameter;   

		ParserTelUri            telUri;         
        ParserTelUriParameter   telUriParameter;
		RvBool                  isTopLabel;     





		ParserSentProtocol      sentProtocol;   

		ParserAuthorizationHeader  authorization; 
        ParserAuthenticationHeader authentication;
        ParserDigestChallenge      digestChallenge;
        ParserAuthenticationInfoHeader auth_info;		



        ParserEvent				   event;       

        ParserReferredByHeader     referredBy;  

        ParserReferToHeader        referTo;     
        ParserSubscriptionState    subsState;   

        ParserContentID            contentID;      
        ParserRetryAfter           retryAfter;  
        ParserContentDisposition   contentDisposition; 
        ParserRack                 rack;        
        ParserReplacesParams       replacesParams; 
        ParserReplacesParam        replacesParam;  

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

#elif defined(RV_SIP_TEL_URI_SUPPORT) && !defined(RV_SIP_IMS_DCS_HEADER_SUPPORT) && !defined(RV_SIP_IMS_HEADER_SUPPORT) && !defined(RV_SIP_EXTENDED_HEADER_SUPPORT) && !defined(RV_SIP_JSR32_SUPPORT) && !defined(RV_SIP_LIGHT) && !defined(RV_SIP_PRIMITIVES) && !defined(RV_SIP_AUTH_ON) && !defined(RV_SIP_SUBS_ON)
#ifndef PARSERENGINETEL_H_1248033534
#define PARSERENGINETEL_H_1248033534

typedef union {
  long alignment;
  char ag_vt_2[sizeof(int)];
  char ag_vt_4[sizeof(ParserBasicToken)];
  char ag_vt_5[sizeof(ParserQuotedString)];
  char ag_vt_6[sizeof(ParserMethod)];
  char ag_vt_7[sizeof(ParserCSeq)];
  char ag_vt_8[sizeof(ParserSequenceNumber)];
  char ag_vt_9[sizeof(ParserContentLength)];
  char ag_vt_10[sizeof(ParserCallId)];
  char ag_vt_11[sizeof(ParserMediaType)];
  char ag_vt_12[sizeof(ParserMediaSubType)];
  char ag_vt_13[sizeof(ParserBoundary)];
  char ag_vt_14[sizeof(ParserVersion)];
  char ag_vt_15[sizeof(ParserBase)];
  char ag_vt_16[sizeof(ParserGenericParamDataVal)];
  char ag_vt_17[sizeof(ParserSipVersion)];
  char ag_vt_18[sizeof(ParserStatusCode)];
  char ag_vt_19[sizeof(ParserReasonPhrase)];
  char ag_vt_20[sizeof(ParserProtocolName)];
  char ag_vt_21[sizeof(ParserProtocolVersion)];
  char ag_vt_22[sizeof(ParserTransport)];
  char ag_vt_23[sizeof(ParserPort)];
  char ag_vt_24[sizeof(ParserViaParam)];
  char ag_vt_25[sizeof(ParserHidden)];
  char ag_vt_26[sizeof(ParserAlias)];
  char ag_vt_27[sizeof(ParserTTLParam)];
  char ag_vt_28[sizeof(ParserTtlParam)];
  char ag_vt_29[sizeof(ParserMaddrParam)];
  char ag_vt_30[sizeof(ParserReceived)];
  char ag_vt_31[sizeof(ParserBranch)];
  char ag_vt_32[sizeof(ParserCompParam)];
  char ag_vt_33[sizeof(ParserSigCompIdParam)];
  char ag_vt_34[sizeof(ParserAbsoluteUri)];
  char ag_vt_35[sizeof(ParserDisplayName)];
  char ag_vt_36[sizeof(ParserEnumdiParamType)];
  char ag_vt_37[sizeof(ParserUserParam)];
  char ag_vt_38[sizeof(RvSipUserParam)];
  char ag_vt_39[sizeof(ParserOtherUser)];
  char ag_vt_40[sizeof(ParserLrParamType)];
  char ag_vt_41[sizeof(ParserTokenizedByParam)];
  char ag_vt_42[sizeof(RvBool)];
  char ag_vt_43[sizeof(ParserOptionalHeaders)];
  char ag_vt_44[sizeof(ParserTagParam)];
  char ag_vt_45[sizeof(ParserPartyParams)];
  char ag_vt_46[sizeof(ParserQVal)];
  char ag_vt_47[sizeof(ParserContactActionType)];
  char ag_vt_48[sizeof(ParserDeltaSeconds)];
  char ag_vt_49[sizeof(ParserSipDate)];
  char ag_vt_50[sizeof(RvSipDateWeekDay)];
  char ag_vt_51[sizeof(ParserDDMMYY)];
  char ag_vt_52[sizeof(ParserInt32)];
  char ag_vt_53[sizeof(RvSipDateMonth)];
  char ag_vt_54[sizeof(ParserTime)];
  char ag_vt_55[sizeof(ParserTimeZone)];
  char ag_vt_56[sizeof(ParserRetryParam)];
  char ag_vt_57[sizeof(ParserRSeq)];
  char ag_vt_58[sizeof(ParserResponseNum)];
  char ag_vt_59[sizeof(ParserEventType)];
  char ag_vt_60[sizeof(ParserDisposition)];
  char ag_vt_61[sizeof(ParserDispositionParams)];
  char ag_vt_62[sizeof(ParserDispositionParam)];
  char ag_vt_63[sizeof(ParserSessionExpiresHeader)];
  char ag_vt_64[sizeof(RvSipSessionExpiresRefresherType)];
  char ag_vt_65[sizeof(ParserMinSEHeader)];
  char ag_vt_66[sizeof(ParserReplacesEarlyFlagParamType)];
} SipParser_vs_type;

typedef enum {
  SipParser_START_token = 1, SipParser_FIRST_COLON_token = 3,
  SipParser_CSEQ_HEADER_token, SipParser_NULL_token,
  SipParser_PARTY_HEADER_token = 7, SipParser_EXPIRES_HEADER_token = 9,
  SipParser_SIP_DATE_HEADER_token = 11, SipParser_CONTACT_HEADER_token = 13,
  SipParser_CALL_ID_HEADER_token = 15, SipParser_CONTENT_LENGTH_token = 17,
  SipParser_VIA_HEADER_token = 19, SipParser_ROUTE_HOP_HEADER_token = 21,
  SipParser_ALLOW_HEADER_token = 23, SipParser_RACK_HEADER_token = 25,
  SipParser_RSEQ_HEADER_token = 27, SipParser_SUPPORTED_HEADER_token = 29,
  SipParser_OPTION_TAG_BASE_HEADER_token = 31,
  SipParser_CONTENT_DISPOSITION_HEADER_token = 33,
  SipParser_CONTENT_TYPE_HEADER_token = 35,
  SipParser_CONTENT_ID_HEADER_token = 37,
  SipParser_RETRY_AFTER_HEADER_token = 39,
  SipParser_SESSION_EXPIRES_HEADER_token = 41,
  SipParser_MIN_SE_HEADER_token = 43, SipParser_REPLACES_HEADER_token = 45,
  SipParser_EVENT_HEADER_token = 47, SipParser_STATUS_LINE_token,
  SipParser_REQUEST_LINE_token, SipParser_ABSOLUTE_URI_HEADER_token = 51,
  SipParser_PRES_URI_HEADER_token, SipParser_IM_URI_HEADER_token,
  SipParser_SIP_URL_HEADER_token, SipParser_TEL_URI_HEADER_token,
  SipParser_OTHER_HEADER_token, SipParser_WHITE_token = 58,
  SipParser_SP_token = 60, SipParser_COMMA_token = 62,
  SipParser_COMMA_BETWEEN_HEADERS_token = 64, SipParser_DOT_token,
  SipParser_EQUAL_token = 69, SipParser_AT_token = 71,
  SipParser_COLON_token = 73, SipParser_SEMI_COLON_token,
  SipParser_MARK_SIGN_token = 76, SipParser_SLASH_token = 78,
  SipParser_LEFT_PARENTHESIS_token = 80,
  SipParser_LEFT_ANGLE_BRACKET_token = 82,
  SipParser_RIGHT_ANGLE_BRACKET_token = 84,
  SipParser_LEFT_QUOTATION_token = 86, SipParser_TOKEN_token = 88,
  SipParser_TOKEN_CHAR_token, SipParser_CALL_ID_WORD_token = 91,
  SipParser_CALL_ID_WORD_CHAR_token,
  SipParser_TOKEN_FOR_CONTENT_TYPE_token = 94,
  SipParser_TOKEN_CHAR_FOR_CONTENT_TYPE_token,
  SipParser_TOKEN_NO_DOT_token = 97, SipParser_TOKEN_CHAR_NO_DOT_token,
  SipParser_FIELD_VALUE_token = 100, SipParser_QUOTED_STRING_token = 106,
  SipParser_QUOTED_CHAR_token, SipParser_METHOD_token = 114,
  SipParser_CSEQ_VAL_token = 121, SipParser_SEQUNCE_NUMBER_token,
  SipParser_DIGIT_token, SipParser_CONTENT_LENGTH_VAL_token = 125,
  SipParser_CALL_ID_token, SipParser_MEDIA_TYPE_token,
  SipParser_MEDIA_TYPE_TYPE_token, SipParser_MEDIA_TYPE_SUB_TYPE_token,
  SipParser_OPTIONAL_CONTENT_TYPE_PARAM_LIST_token,
  SipParser_CONTENT_TYPE_PARAM_LIST_token, SipParser_BOUNDARY_PARAM_token,
  SipParser_VERSION_PARAM_token, SipParser_BASE_PARAM_token,
  SipParser_START_PARAM_token, SipParser_TYPE_PARAM_token,
  SipParser_CONTENT_TYPE_GENERIC_PARAM_DATA_token,
  SipParser_START_PARAM_NAME_token = 162,
  SipParser_OLD_OR_NEW_ADDR_SPEC_token,
  SipParser_NEW_ADDR_SPEC_PREFIX_token = 167, SipParser_ADDR_SPEC_token,
  SipParser_OLD_ADDR_SPEC_PREFIX_token, SipParser_USER_INFO_TOKEN_token,
  SipParser_HOST_NAME_token, SipParser_TYPE_PARAM_MEDIA_token = 175,
  SipParser_CONTENT_TYPE_GENERIC_PARAM_DATA_VAL_token,
  SipParser_CONTENT_ID_HEADER_VALUE_token,
  SipParser_CONTENT_ID_HEADER_LEFT_BRACKET_token,
  SipParser_CONTENT_ID_OLD_OR_NEW_ADDR_SPEC_token,
  SipParser_GENERIC_PARAM_LIST_token, SipParser_GENERIC_PARAM_DATA_token,
  SipParser_GENERIC_PARAM_DATA_VAL_token, SipParser_SIP_VERSION_token,
  SipParser_STATUS_CODE_token = 185, SipParser_REASON_PHRASE_token,
  SipParser_EXTENSION_CODE_token = 188, SipParser_REASONE_TEXT_UTF8_token,
  SipParser_ALLOW_VAL_token = 193, SipParser_VIA_LIST_token,
  SipParser_SINGLE_VIA_token, SipParser_SENT_PROTOCOL_token,
  SipParser_SENT_BY_token, SipParser_VIA_PARAM_LIST_token,
  SipParser_VIA_PARAM_token, SipParser_PROTOCOL_NAME_token,
  SipParser_PROTOCOL_VERSION_token, SipParser_TRANSPORT_token,
  SipParser_TRANSPORT_TYPE_token = 204, SipParser_PORT_token = 209,
  SipParser_VIA_HIDDEN_token, SipParser_VIA_ALIAS_token,
  SipParser_VIA_TTL_token, SipParser_VIA_MADDR_token,
  SipParser_VIA_RECEIVED_token, SipParser_VIA_BRANCH_token,
  SipParser_RESPONSE_PORT_token, SipParser_VIA_COMPRESSION_token,
  SipParser_VIA_SIP_SIGCOMP_ID_token, SipParser_OTHER_PARAM_token,
  SipParser_OTHER_PARAM_EXPR_token, SipParser_IPV_6_REFERENCE_token,
  SipParser_TTL_PARAM_token = 224, SipParser_TTL_token = 226,
  SipParser_RPORT_NAME_token, SipParser_RPORT_VAL_token,
  SipParser_MADDR_PARAM_token = 230,
  SipParser_HOST_NAME_FOR_RECEIVED_token = 233,
  SipParser_COMPRESSION_TYPE_token = 236,
  SipParser_SIGCOMP_ID_TOKEN_token = 239, SipParser_IPV_6_ADDRESS_token,
  SipParser_TEST_COMMENT_token, SipParser_COMMENT_TEXT_token,
  SipParser_END_COMMENT_token = 244, SipParser_REQUEST_URI_token = 247,
  SipParser_ABSOLUTE_URI_token, SipParser_ABS_URI_SCHEME_token,
  SipParser_URI_IDENTIFIER_token, SipParser_ABS_URI_SCHEME_CHAR_token,
  SipParser_URI_SCHEME_OR_DISPLAY_NAME_token = 253,
  SipParser_DISPLAY_NAME_token, SipParser_URI_IDENTIFIER_CHAR_token,
  SipParser_URI_IDENTIFIER_WITHOUT_SEPARATORS_token = 257,
  SipParser_TEL_URI_token = 260, SipParser_TEL_PREFIX_token,
  SipParser_TEL_token, SipParser_GLOBAL_OR_LOCAL_NUMBER_token = 264,
  SipParser_OPTIONAL_TEL_URI_PARAMETERS_token,
  SipParser_GLOBAL_NUMBER_token, SipParser_LOCAL_NUMBER_token,
  SipParser_GLOBAL_NUMBER_DIGITS_token, SipParser_PHONEDIGIT_token = 270,
  SipParser_VISUAL_SEPERATOR_token = 274,
  SipParser_LOCAL_NUMBER_DIGITS_token = 276,
  SipParser_HEX_OR_PHONECHAR_token, SipParser_PHONEDIGIT_HEX_token,
  SipParser_HEX_token = 282, SipParser_PHONECHAR_token,
  SipParser_TEL_URI_PARAMETERS_token, SipParser_TEL_URI_PARAMETER_token,
  SipParser_TEL_URI_EXTENSION_PARAM_token,
  SipParser_TEL_URI_POST_DIAL_PARAM_token,
  SipParser_TEL_URI_ISDN_SUB_ADDR_PARAM_URL_token,
  SipParser_TEL_URI_CONTEXT_PARAM_token,
  SipParser_TEL_URI_ENUMDI_PARAM_token,
  SipParser_URL_GENERIC_PARAM_DATA_token,
  SipParser_TEL_URI_EXTENTION_VALUE_token = 293,
  SipParser_TEL_URI_POST_DIAL_VALUE_token = 296,
  SipParser_TEL_URI_POST_DIAL_CHAR_token, SipParser_DTMFDIGIT_token = 299,
  SipParser_PAUSE_CHAR_token,
  SipParser_TEL_URI_ISDN_SUB_ADDR_VALUE_token = 302, SipParser_URIC_token,
  SipParser_RESERVED_FOR_URIC_token = 305, SipParser_UNRESERVED_token,
  SipParser_PCT_ENCODED_token, SipParser_TEL_URI_CONTEXT_VALUE_token = 310,
  SipParser_DESCRIPTOR_token, SipParser_DOMAINNAME_token,
  SipParser_LABEL_token, SipParser_NEXT_LABEL_token,
  SipParser_ALPHA_token = 318, SipParser_ALPHANUM_OR_HYPHEN_token,
  SipParser_ALPHANUM_token = 324, SipParser_SIP_URL_token = 326,
  SipParser_PRES_URI_token, SipParser_IM_URI_token,
  SipParser_SIP_PREFIX_token, SipParser_URL_token,
  SipParser_PRES_PREFIX_token = 333, SipParser_RV_ABS_PREFIX_token,
  SipParser_IM_PREFIX_token = 337, SipParser_URL_USERINFO_token = 339,
  SipParser_OPTIONAL_URL_PARAMETERS_token, SipParser_OPTIONAL_HEADERS_token,
  SipParser_URL_HOST_token, SipParser_GENERAL_URL_WITHOUT_PARAM_token,
  SipParser_SIP_URL_WITHOUT_PARAMS_token,
  SipParser_TEL_URL_WITHOUT_PARAMS_token,
  SipParser_PRES_URL_WITHOUT_PARAMS_token,
  SipParser_IM_URL_WITHOUT_PARAMS_token,
  SipParser_ABS_URI_WITHOUT_PARAMS_token,
  SipParser_URL_USERINFO_PREFIX_token, SipParser_URL_USERINFO_VAL_token,
  SipParser_HOSTPORT_token, SipParser_PASSWORD_token = 353,
  SipParser_TOKEN_USER_INFO_token, SipParser_TOKEN_USER_INFO_CHAR_token,
  SipParser_PASSWORD_STRING_token = 357,
  SipParser_PASSWORD_STRING_CHAR_token,
  SipParser_URL_HOST_PREFIX_token = 362, SipParser_TOKEN_HOST_token = 364,
  SipParser_TOKEN_HOST_CHAR_token, SipParser_IPV_4_ADDRESS_token = 369,
  SipParser_IPV_4_PART_token, SipParser_HEXPART_token,
  SipParser_HEXSEQ_token, SipParser_HEX4_token = 375,
  SipParser_URL_PARAMETERS_token = 379, SipParser_URL_PARAMETER_token,
  SipParser_TRANSPORT_PARAM_token, SipParser_USER_PARAM_token,
  SipParser_TTL_PARAM_URL_token, SipParser_MADDR_PARAM_URL_token,
  SipParser_METHOD_PARAM_URL_token, SipParser_LR_PARAM_URL_token,
  SipParser_COMPRESSION_PARAM_URL_token,
  SipParser_SIGCOMPID_PARAM_URL_token,
  SipParser_TOKENIZED_BY_PARAM_URL_token, SipParser_ORIG_PARAM_URL_token,
  SipParser_USER_PARAM_TYPE_token = 393, SipParser_OTHER_USER_token,
  SipParser_PARAMCHAR_token = 404,
  SipParser_URL_OTHER_PARAM_VAL_token = 410, SipParser_PVALUE_token,
  SipParser_HEADERS_LIST_token, SipParser_HEADER_token,
  SipParser_HNAME_token = 415, SipParser_HVALUE_token,
  SipParser_REQUEST_LINE_VALUE_token = 423, SipParser_REQUEST_METHOD_token,
  SipParser_PARTY_VALUE_token = 438, SipParser_NAME_ADDR_token,
  SipParser_PARTY_PARAMS_LIST_token, SipParser_MORE_THAN_ONE_TOKEN_token,
  SipParser_TAG_PARAM_token = 445, SipParser_PARTY_PARAM_DATA_token = 447,
  SipParser_CONTACT_HEADER_VALUES_token,
  SipParser_CONTACT_HEADER_OTHER_token,
  SipParser_CONTACT_HEADER_VALUE_LIST_token,
  SipParser_CONTACT_HEADER_VALUE_token = 452,
  SipParser_CONTACT_PARAMS_token, SipParser_CONTACT_PARAM_token,
  SipParser_Q_token, SipParser_ACTION_token,
  SipParser_CONTACT_EXPIRES_token, SipParser_Q_VALUE_token = 459,
  SipParser_Q_VALUE_OPTIONAL_token = 461,
  SipParser_ACTION_VALUE_token = 465, SipParser_EXPIRES_VALUE_token = 468,
  SipParser_EXPIRES_VALUE_FOR_CONTACT_token = 470,
  SipParser_DELTA_SECONDS_token, SipParser_SIP_DATE_token,
  SipParser_SIP_DATE_HEADER_VAL_token, SipParser_DATE_token,
  SipParser_TIME_token, SipParser_DAY_token, SipParser_DD_token = 484,
  SipParser_MM_token, SipParser_YY_token, SipParser_HOUR_token = 499,
  SipParser_MINUTE_token, SipParser_SECOND_token, SipParser_TIME_ZONE_token,
  SipParser_ROUTE_LIST_token = 506, SipParser_SINGLE_ROUTE_token,
  SipParser_RETRY_AFTER_VAL_token, SipParser_RETRY_AFTER_COMMENT_token,
  SipParser_RETRY_PARAM_token = 511, SipParser_DURATION_PARAM_token,
  SipParser_RSEQ_VAL_token = 514, SipParser_RESPONSE_NUM_token,
  SipParser_RACK_VAL_token, SipParser_OPTION_TAG_token,
  SipParser_OPTION_TAG_LIST_token, SipParser_SUPPORTED_OPTION_TAG_token,
  SipParser_EVENT_HEADER_VAL_token, SipParser_EVENT_TYPE_token,
  SipParser_EVENT_PARAMS_token, SipParser_EVENT_PACKAGE_token,
  SipParser_EVENT_TEMPLATE_token, SipParser_EVENT_PARAM_token,
  SipParser_EVENT_ID_token, SipParser_DISPOSITION_VAL_token = 528,
  SipParser_DISPOSITION_TYPE_token, SipParser_DISPOSITION_PARAMS_token,
  SipParser_HANDLING_PARAM_token = 536,
  SipParser_SESSION_EXPIRES_VALUE_token = 540,
  SipParser_REFRESHER_TYPE_token, SipParser_REFRESHER_TYPE_NAME_token = 543,
  SipParser_REFRESHER_TYPE_VALUE_token, SipParser_MIN_SE_VALUE_token = 548,
  SipParser_REPLACES_HEADER_VALUE_token, SipParser_REPLACES_PARAMS_token,
  SipParser_REPLACES_PARAM_token, SipParser_TO_TAG_PARAM_token,
  SipParser_FROM_TAG_PARAM_token, SipParser_EARLY_FLAG_PARAM_token
} SipParser_token_type;

typedef struct SipParser_pcb_struct{
  SipParser_token_type token_number, reduction_token, error_frame_token;
  int input_code;
  int input_value;
  int line, column;
  int ssx, sn, error_frame_ssx;
  int drt, dssx, dsn;
  int ss[29];
  SipParser_vs_type vs[29];
  int ag_ap;
  char *error_message;
  char read_flag;
  char exit_flag;
  unsigned char * pointer;
  unsigned char * la_ptr;
  const unsigned char *key_sp;
  int save_index, key_state;
  const  int *ag_dtl;
  int ag_dsn;
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



        








        ParserContactHeader     contact;        
        ParserExpiresHeader     expires;        
        ParserPartyHeader       party;          

        ParserRequestLine       requestLine;    
        ParserRouteHeader       route;          
        ParserNameAddr          nameAddr;       
        ParserExUri             exUri;          
        ParserSipUrl            sipUrl;         
		ParserSingleVia         singleVia;      
        ParserContentType       contentType;    
        ParserUrlParameter      urlParameter;   

		ParserTelUri            telUri;         
        ParserTelUriParameter   telUriParameter;
		RvBool                  isTopLabel;     





		ParserSentProtocol      sentProtocol;   








        ParserEvent				   event;       






        ParserContentID            contentID;      
        ParserRetryAfter           retryAfter;  
        ParserContentDisposition   contentDisposition; 
        ParserRack                 rack;        
        ParserReplacesParams       replacesParams; 
        ParserReplacesParam        replacesParam;  

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
