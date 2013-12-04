#if defined(RV_SIP_AUTH_ON) && defined(RV_SIP_TEL_URI_SUPPORT) && defined(RV_SIP_IMS_HEADER_SUPPORT) && !defined(RV_SIP_IMS_DCS_HEADER_SUPPORT) && !defined(RV_SIP_EXTENDED_HEADER_SUPPORT) && !defined(RV_SIP_JSR32_SUPPORT) && !defined(RV_SIP_LIGHT) && !defined(RV_SIP_PRIMITIVES) && defined(RV_SIP_SUBS_ON)
#ifndef PARSERENGINEIMS_H_1248033541
#define PARSERENGINEIMS_H_1248033541

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
  char ag_vt_37[sizeof(RvBool)];
  char ag_vt_38[sizeof(RvSipDiameterProtocol)];
  char ag_vt_39[sizeof(ParserUserParam)];
  char ag_vt_40[sizeof(RvSipUserParam)];
  char ag_vt_41[sizeof(ParserOtherUser)];
  char ag_vt_42[sizeof(ParserLrParamType)];
  char ag_vt_43[sizeof(ParserTokenizedByParam)];
  char ag_vt_44[sizeof(ParserCPCParam)];
  char ag_vt_45[sizeof(ParserGrParam)];
  char ag_vt_46[sizeof(ParserOptionalHeaders)];
  char ag_vt_47[sizeof(ParserTagParam)];
  char ag_vt_48[sizeof(ParserPartyParams)];
  char ag_vt_49[sizeof(ParserQVal)];
  char ag_vt_50[sizeof(ParserContactActionType)];
  char ag_vt_51[sizeof(ParserContactFeatureTag)];
  char ag_vt_52[sizeof(ParserDeltaSeconds)];
  char ag_vt_53[sizeof(ParserSipDate)];
  char ag_vt_54[sizeof(RvSipDateWeekDay)];
  char ag_vt_55[sizeof(ParserDDMMYY)];
  char ag_vt_56[sizeof(ParserInt32)];
  char ag_vt_57[sizeof(RvSipDateMonth)];
  char ag_vt_58[sizeof(ParserTime)];
  char ag_vt_59[sizeof(ParserTimeZone)];
  char ag_vt_60[sizeof(RvSipAuthStale)];
  char ag_vt_61[sizeof(ParserAlgorithm)];
  char ag_vt_62[sizeof(ParserQopValue)];
  char ag_vt_63[sizeof(ParserAuthQopOptions)];
  char ag_vt_64[sizeof(RvSipAuthQopOption)];
  char ag_vt_65[sizeof(RvSipAuthIntegrityProtected)];
  char ag_vt_66[sizeof(ParserRetryParam)];
  char ag_vt_67[sizeof(ParserRSeq)];
  char ag_vt_68[sizeof(ParserResponseNum)];
  char ag_vt_69[sizeof(ParserEventType)];
  char ag_vt_70[sizeof(ParserSubsStateValue)];
  char ag_vt_71[sizeof(ParserSubsStateReason)];
  char ag_vt_72[sizeof(ParserDisposition)];
  char ag_vt_73[sizeof(ParserDispositionParams)];
  char ag_vt_74[sizeof(ParserDispositionParam)];
  char ag_vt_75[sizeof(ParserSessionExpiresHeader)];
  char ag_vt_76[sizeof(RvSipSessionExpiresRefresherType)];
  char ag_vt_77[sizeof(ParserMinSEHeader)];
  char ag_vt_78[sizeof(ParserReplacesEarlyFlagParamType)];
  char ag_vt_79[sizeof(ParserAccess)];
  char ag_vt_80[sizeof(ParserMechanism)];
  char ag_vt_81[sizeof(ParserSecurityAlgorithmType)];
  char ag_vt_82[sizeof(ParserSecurityProtocolType)];
  char ag_vt_83[sizeof(ParserSecurityModeType)];
  char ag_vt_84[sizeof(ParserSecurityEncryptAlgorithmType)];
  char ag_vt_85[sizeof(ParserAnswer)];
  char ag_vt_86[sizeof(ParserSessionCaseType)];
  char ag_vt_87[sizeof(ParserRegistrationStateType)];
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
  SipParser_TEL_URI_HEADER_token, SipParser_DIAMETER_URI_HEADER_token,
  SipParser_P_URI_HEADER_token = 72,
  SipParser_P_VISITED_NETWORK_ID_HEADER_token = 74,
  SipParser_P_ACCESS_NETWORK_INFO_HEADER_token = 76,
  SipParser_P_CHARGING_FUNCTION_ADDRESSES_HEADER_token = 78,
  SipParser_P_CHARGING_VECTOR_HEADER_token = 80,
  SipParser_P_MEDIA_AUTHORIZATION_HEADER_token = 82,
  SipParser_SECURITY_HEADER_token = 84,
  SipParser_P_PROFILE_KEY_HEADER_token = 86,
  SipParser_P_USER_DATABASE_HEADER_token = 88,
  SipParser_P_ANSWER_STATE_HEADER_token = 90,
  SipParser_P_SERVED_USER_HEADER_token = 92, SipParser_OTHER_HEADER_token,
  SipParser_WHITE_token = 95, SipParser_SP_token = 97,
  SipParser_COMMA_token = 99, SipParser_COMMA_BETWEEN_HEADERS_token = 101,
  SipParser_DOT_token, SipParser_EQUAL_token = 106,
  SipParser_AT_token = 108, SipParser_COLON_token = 110,
  SipParser_SEMI_COLON_token, SipParser_MARK_SIGN_token = 113,
  SipParser_SLASH_token = 115, SipParser_LEFT_PARENTHESIS_token = 117,
  SipParser_LEFT_ANGLE_BRACKET_token = 119,
  SipParser_RIGHT_ANGLE_BRACKET_token = 121,
  SipParser_LEFT_QUOTATION_token = 123, SipParser_HYPHEN_token = 125,
  SipParser_TOKEN_token = 127, SipParser_TOKEN_CHAR_token,
  SipParser_CALL_ID_WORD_token = 130, SipParser_CALL_ID_WORD_CHAR_token,
  SipParser_TOKEN_FOR_CONTENT_TYPE_token = 133,
  SipParser_TOKEN_CHAR_FOR_CONTENT_TYPE_token,
  SipParser_TOKEN_NO_DOT_token = 136, SipParser_TOKEN_CHAR_NO_DOT_token,
  SipParser_FIELD_VALUE_token = 139, SipParser_QUOTED_STRING_token = 145,
  SipParser_QUOTED_CHAR_token, SipParser_HEX_VALUE_token = 153,
  SipParser_HEX_token, SipParser_INT_NUMBER_token = 156,
  SipParser_DIGIT_token, SipParser_TOKEN_OR_QUOTED_STRING_token = 159,
  SipParser_METHOD_token, SipParser_CSEQ_VAL_token = 170,
  SipParser_SEQUNCE_NUMBER_token, SipParser_CONTENT_LENGTH_VAL_token,
  SipParser_CALL_ID_token, SipParser_MEDIA_TYPE_token,
  SipParser_MEDIA_TYPE_TYPE_token, SipParser_MEDIA_TYPE_SUB_TYPE_token,
  SipParser_OPTIONAL_CONTENT_TYPE_PARAM_LIST_token,
  SipParser_CONTENT_TYPE_PARAM_LIST_token, SipParser_BOUNDARY_PARAM_token,
  SipParser_VERSION_PARAM_token, SipParser_BASE_PARAM_token,
  SipParser_START_PARAM_token, SipParser_TYPE_PARAM_token,
  SipParser_CONTENT_TYPE_GENERIC_PARAM_DATA_token,
  SipParser_START_PARAM_NAME_token = 209,
  SipParser_OLD_OR_NEW_ADDR_SPEC_token,
  SipParser_NEW_ADDR_SPEC_PREFIX_token = 214, SipParser_ADDR_SPEC_token,
  SipParser_OLD_ADDR_SPEC_PREFIX_token, SipParser_USER_INFO_TOKEN_token,
  SipParser_HOST_NAME_token, SipParser_TYPE_PARAM_MEDIA_token = 222,
  SipParser_CONTENT_TYPE_GENERIC_PARAM_DATA_VAL_token,
  SipParser_CONTENT_ID_HEADER_VALUE_token,
  SipParser_CONTENT_ID_HEADER_LEFT_BRACKET_token,
  SipParser_CONTENT_ID_OLD_OR_NEW_ADDR_SPEC_token,
  SipParser_GENERIC_PARAM_LIST_token, SipParser_GENERIC_PARAM_DATA_token,
  SipParser_GENERIC_PARAM_DATA_VAL_token, SipParser_SIP_VERSION_token,
  SipParser_STATUS_CODE_token = 232, SipParser_REASON_PHRASE_token,
  SipParser_EXTENSION_CODE_token = 235, SipParser_REASONE_TEXT_UTF8_token,
  SipParser_ALLOW_VAL_token = 240, SipParser_VIA_LIST_token,
  SipParser_SINGLE_VIA_token, SipParser_SENT_PROTOCOL_token,
  SipParser_SENT_BY_token, SipParser_VIA_PARAM_LIST_token,
  SipParser_VIA_PARAM_token, SipParser_PROTOCOL_NAME_token,
  SipParser_PROTOCOL_VERSION_token, SipParser_TRANSPORT_token,
  SipParser_TRANSPORT_TYPE_token = 251, SipParser_PORT_token = 256,
  SipParser_VIA_HIDDEN_token, SipParser_VIA_ALIAS_token,
  SipParser_VIA_TTL_token, SipParser_VIA_MADDR_token,
  SipParser_VIA_RECEIVED_token, SipParser_VIA_BRANCH_token,
  SipParser_RESPONSE_PORT_token, SipParser_VIA_COMPRESSION_token,
  SipParser_VIA_SIP_SIGCOMP_ID_token, SipParser_OTHER_PARAM_token,
  SipParser_OTHER_PARAM_EXPR_token, SipParser_IPV_6_REFERENCE_token,
  SipParser_TTL_PARAM_token = 271, SipParser_TTL_token = 273,
  SipParser_RPORT_NAME_token, SipParser_RPORT_VAL_token,
  SipParser_MADDR_PARAM_token = 277,
  SipParser_HOST_NAME_FOR_RECEIVED_token = 280,
  SipParser_COMPRESSION_TYPE_token = 283,
  SipParser_SIGCOMP_ID_TOKEN_token = 286, SipParser_IPV_6_ADDRESS_token,
  SipParser_TEST_COMMENT_token, SipParser_COMMENT_TEXT_token,
  SipParser_END_COMMENT_token = 291, SipParser_REQUEST_URI_token = 294,
  SipParser_ABSOLUTE_URI_token, SipParser_ABS_URI_SCHEME_token,
  SipParser_URI_IDENTIFIER_token, SipParser_ABS_URI_SCHEME_CHAR_token,
  SipParser_URI_SCHEME_OR_DISPLAY_NAME_token = 300,
  SipParser_DISPLAY_NAME_token, SipParser_URI_IDENTIFIER_CHAR_token,
  SipParser_URI_IDENTIFIER_WITHOUT_SEPARATORS_token = 304,
  SipParser_TEL_URI_token = 307, SipParser_TEL_PREFIX_token,
  SipParser_TEL_token, SipParser_GLOBAL_OR_LOCAL_NUMBER_token = 311,
  SipParser_OPTIONAL_TEL_URI_PARAMETERS_token,
  SipParser_GLOBAL_NUMBER_token, SipParser_LOCAL_NUMBER_token,
  SipParser_GLOBAL_NUMBER_DIGITS_token, SipParser_PHONEDIGIT_token = 317,
  SipParser_VISUAL_SEPERATOR_token = 321,
  SipParser_LOCAL_NUMBER_DIGITS_token = 323,
  SipParser_HEX_OR_PHONECHAR_token, SipParser_PHONEDIGIT_HEX_token,
  SipParser_PHONECHAR_token = 329, SipParser_TEL_URI_PARAMETERS_token,
  SipParser_TEL_URI_PARAMETER_token,
  SipParser_TEL_URI_EXTENSION_PARAM_token,
  SipParser_TEL_URI_POST_DIAL_PARAM_token,
  SipParser_TEL_URI_ISDN_SUB_ADDR_PARAM_URL_token,
  SipParser_TEL_URI_CONTEXT_PARAM_token,
  SipParser_TEL_URI_ENUMDI_PARAM_token,
  SipParser_URL_GENERIC_PARAM_DATA_token, SipParser_CPC_PARAM_URL_token,
  SipParser_TEL_URI_RN_PARAM_token,
  SipParser_TEL_URI_RN_CONTEXT_PARAM_token,
  SipParser_TEL_URI_CIC_PARAM_token,
  SipParser_TEL_URI_CIC_CONTEXT_PARAM_token,
  SipParser_TEL_URI_NPDI_PARAM_token,
  SipParser_TEL_URI_EXTENTION_VALUE_token = 345,
  SipParser_TEL_URI_POST_DIAL_VALUE_token = 348,
  SipParser_TEL_URI_POST_DIAL_CHAR_token, SipParser_DTMFDIGIT_token = 351,
  SipParser_PAUSE_CHAR_token,
  SipParser_TEL_URI_ISDN_SUB_ADDR_VALUE_token = 354, SipParser_URIC_token,
  SipParser_RESERVED_FOR_URIC_token = 357, SipParser_UNRESERVED_token,
  SipParser_PCT_ENCODED_token, SipParser_TEL_URI_CONTEXT_VALUE_token = 362,
  SipParser_DESCRIPTOR_token, SipParser_DOMAINNAME_token,
  SipParser_LABEL_token, SipParser_NEXT_LABEL_token,
  SipParser_ALPHA_token = 370, SipParser_ALPHANUM_OR_HYPHEN_token,
  SipParser_ALPHANUM_token = 375,
  SipParser_TEL_URI_NP_GLOBAL_OR_LOCAL_NUMBER_token = 378,
  SipParser_DIAMETER_URI_token = 383, SipParser_DIAMETER_PREFIX_token,
  SipParser_DIAMETER_HOSTPORT_token, SipParser_DIAMETER_PARAMS_token,
  SipParser_DIAMETER_PARAM_token = 389, SipParser_TRANSPORT_PARAM_token,
  SipParser_DIAMETER_PROTOCOL_PARAM_token = 392,
  SipParser_SIP_URL_token = 396, SipParser_PRES_URI_token,
  SipParser_IM_URI_token, SipParser_SIP_PREFIX_token, SipParser_URL_token,
  SipParser_PRES_PREFIX_token = 403, SipParser_RV_ABS_PREFIX_token,
  SipParser_IM_PREFIX_token = 407, SipParser_URL_USERINFO_token = 409,
  SipParser_OPTIONAL_URL_PARAMETERS_token, SipParser_OPTIONAL_HEADERS_token,
  SipParser_URL_HOST_token, SipParser_GENERAL_URL_WITHOUT_PARAM_token,
  SipParser_SIP_URL_WITHOUT_PARAMS_token,
  SipParser_TEL_URL_WITHOUT_PARAMS_token,
  SipParser_DIAMETER_URI_WITHOUT_PARAMS_token,
  SipParser_PRES_URL_WITHOUT_PARAMS_token,
  SipParser_IM_URL_WITHOUT_PARAMS_token,
  SipParser_ABS_URI_WITHOUT_PARAMS_token,
  SipParser_URL_USERINFO_PREFIX_token, SipParser_URL_USERINFO_VAL_token,
  SipParser_HOSTPORT_token, SipParser_PASSWORD_token = 424,
  SipParser_TOKEN_USER_INFO_token, SipParser_TOKEN_USER_INFO_CHAR_token,
  SipParser_PASSWORD_STRING_token = 428,
  SipParser_PASSWORD_STRING_CHAR_token,
  SipParser_URL_HOST_PREFIX_token = 433, SipParser_TOKEN_HOST_token = 435,
  SipParser_TOKEN_HOST_CHAR_token, SipParser_IPV_4_ADDRESS_token = 440,
  SipParser_IPV_4_PART_token, SipParser_HEXPART_token,
  SipParser_HEXSEQ_token, SipParser_HEX4_token = 446,
  SipParser_URL_PARAMETERS_token = 450, SipParser_URL_PARAMETER_token,
  SipParser_USER_PARAM_token, SipParser_TTL_PARAM_URL_token,
  SipParser_MADDR_PARAM_URL_token, SipParser_METHOD_PARAM_URL_token,
  SipParser_LR_PARAM_URL_token, SipParser_COMPRESSION_PARAM_URL_token,
  SipParser_SIGCOMPID_PARAM_URL_token,
  SipParser_TOKENIZED_BY_PARAM_URL_token, SipParser_ORIG_PARAM_URL_token,
  SipParser_GR_PARAM_URL_token, SipParser_USER_PARAM_TYPE_token = 464,
  SipParser_OTHER_USER_token, SipParser_PARAMCHAR_token = 475,
  SipParser_CPC_PARAM_URL_VALUE_token = 482,
  SipParser_URL_OTHER_PARAM_VAL_token = 484, SipParser_PVALUE_token = 495,
  SipParser_HEADERS_LIST_token, SipParser_HEADER_token,
  SipParser_HNAME_token = 499, SipParser_HVALUE_token,
  SipParser_REQUEST_LINE_VALUE_token = 507, SipParser_REQUEST_METHOD_token,
  SipParser_PARTY_VALUE_token = 522, SipParser_NAME_ADDR_token,
  SipParser_PARTY_PARAMS_LIST_token, SipParser_MORE_THAN_ONE_TOKEN_token,
  SipParser_TAG_PARAM_token = 529, SipParser_PARTY_PARAM_DATA_token = 531,
  SipParser_CONTACT_HEADER_VALUES_token,
  SipParser_CONTACT_HEADER_OTHER_token,
  SipParser_CONTACT_HEADER_VALUE_LIST_token,
  SipParser_CONTACT_HEADER_VALUE_token = 536,
  SipParser_CONTACT_PARAMS_token, SipParser_CONTACT_PARAM_token,
  SipParser_Q_token, SipParser_ACTION_token,
  SipParser_CONTACT_EXPIRES_token, SipParser_TEMP_GRUU_token,
  SipParser_PUB_GRUU_token, SipParser_REG_ID_token,
  SipParser_CONTACT_FEATURE_TAG_AUDIO_token,
  SipParser_CONTACT_FEATURE_TAG_AUTOMATA_token,
  SipParser_CONTACT_FEATURE_TAG_CLASS_token,
  SipParser_CONTACT_FEATURE_TAG_DUPLEX_token,
  SipParser_CONTACT_FEATURE_TAG_DATA_token,
  SipParser_CONTACT_FEATURE_TAG_CONTROL_token,
  SipParser_CONTACT_FEATURE_TAG_MOBILITY_token,
  SipParser_CONTACT_FEATURE_TAG_DESCRIPTION_token,
  SipParser_CONTACT_FEATURE_TAG_EVENTS_token,
  SipParser_CONTACT_FEATURE_TAG_PRIORITY_token,
  SipParser_CONTACT_FEATURE_TAG_METHODS_token,
  SipParser_CONTACT_FEATURE_TAG_SCHEMES_token,
  SipParser_CONTACT_FEATURE_TAG_APPLICATION_token,
  SipParser_CONTACT_FEATURE_TAG_VIDEO_token,
  SipParser_CONTACT_FEATURE_TAG_LANGUAGE_token,
  SipParser_CONTACT_FEATURE_TAG_TYPE_token,
  SipParser_CONTACT_FEATURE_TAG_ISFOCUS_token,
  SipParser_CONTACT_FEATURE_TAG_ACTOR_token,
  SipParser_CONTACT_FEATURE_TAG_TEXT_token,
  SipParser_CONTACT_FEATURE_TAG_EXTENSIONS_token,
  SipParser_CONTACT_FEATURE_TAG_SIP_INSTANCE_token,
  SipParser_Q_VALUE_token = 567, SipParser_Q_VALUE_OPTIONAL_token = 569,
  SipParser_ACTION_VALUE_token = 573, SipParser_GRUU_PARAM_VAL_token = 577,
  SipParser_FEATURE_TAG_STRING_token = 581,
  SipParser_EXPIRES_VALUE_token = 598,
  SipParser_EXPIRES_VALUE_FOR_CONTACT_token = 600,
  SipParser_DELTA_SECONDS_token, SipParser_SIP_DATE_token,
  SipParser_SIP_DATE_HEADER_VAL_token, SipParser_DATE_token,
  SipParser_TIME_token, SipParser_DAY_token, SipParser_DD_token = 614,
  SipParser_MM_token, SipParser_YY_token, SipParser_HOUR_token = 629,
  SipParser_MINUTE_token, SipParser_SECOND_token, SipParser_TIME_ZONE_token,
  SipParser_AUTHENTICATION_VAL_token = 636, SipParser_CHALLENGE_VALUE_token,
  SipParser_DIGEST_CHALLENGE_LIST_token, SipParser_AUTH_PARAM_LIST_token,
  SipParser_DIGEST_CHALLENGE_token, SipParser_REALM_token,
  SipParser_DOMAIN_token, SipParser_NONCE_token, SipParser_OPAQUE_token,
  SipParser_INTEGRITY_KEY_token, SipParser_CIPHER_KEY_token,
  SipParser_AUTH_PARAM_token, SipParser_STALE_token,
  SipParser_ALGORITHM_token, SipParser_QOP_OPTIONS_token,
  SipParser_PARAM_IN_NEXT_CHALLENGE_token,
  SipParser_REALM_VALUE_token = 653, SipParser_DOMAIN_EQUAL_token = 655,
  SipParser_DOMAIN_WHITE_token, SipParser_DOMAIN_VAL_token = 658,
  SipParser_URI_LIST_token, SipParser_STRING_DOMAIN_PATCH_token,
  SipParser_TOKEN_FOR_DOMAIN_token, SipParser_TOKEN_CHAR_FOR_DOMAIN_token,
  SipParser_DOMAIN_SPACE_token = 664, SipParser_NONCE_VALUE_token = 667,
  SipParser_OPAQUE_VALUE_token = 669,
  SipParser_ALGORITHM_DECLARATION_token = 672,
  SipParser_ALGORITHM_AND_AKAV_token, SipParser_AKAV_PREFIX_token = 675,
  SipParser_AKAV_token, SipParser_AKA_VERSION_VAL_token,
  SipParser_ALGORITHM_VAL_token, SipParser_ALGORITHM_PREFIX_token,
  SipParser_QOP_VALUE_LIST_token = 687, SipParser_QOP_VALUE_token,
  SipParser_QUOTED_HEX_VALUE_token = 692,
  SipParser_AUTH_PARAM_DATA_token = 695, SipParser_AUTHORIZATION_VAL_token,
  SipParser_CREDENTIALS_VALUE_token, SipParser_DIGEST_RESPONSE_LIST_token,
  SipParser_DIGEST_RESPONSE_token, SipParser_USERNAME_token,
  SipParser_RESPONSE_token, SipParser_CNONCE_token,
  SipParser_NONCE_COUNT_token, SipParser_AUTS_PARAM_token,
  SipParser_INTEGRITY_PROTECTED_token, SipParser_MESSAGE_QOP_token,
  SipParser_DIGEST_URI_token, SipParser_USERNAME_VALUE_token = 709,
  SipParser_DIGEST_URI_VALUE_token = 711,
  SipParser_LEFT_URI_QUOTATION_token, SipParser_CNONCE_VALUE_token = 714,
  SipParser_NC_VALUE_token = 716, SipParser_LHEX_token,
  SipParser_REQUEST_DIGEST_token = 719, SipParser_THIRTY_TWO_LHEX_token,
  SipParser_SIXTY_FOUR_BASE_ENCODING_token = 724,
  SipParser_SIXTY_FOUR_BASE_ENCODING_CHAR_token,
  SipParser_ONE_OR_TWO_EQUAL_SIGNS_token = 727,
  SipParser_ROUTE_LIST_token = 734, SipParser_SINGLE_ROUTE_token,
  SipParser_RETRY_AFTER_VAL_token, SipParser_RETRY_AFTER_COMMENT_token,
  SipParser_RETRY_PARAM_token = 739, SipParser_DURATION_PARAM_token,
  SipParser_RSEQ_VAL_token = 742, SipParser_RESPONSE_NUM_token,
  SipParser_RACK_VAL_token, SipParser_OPTION_TAG_token,
  SipParser_OPTION_TAG_LIST_token, SipParser_SUPPORTED_OPTION_TAG_token,
  SipParser_REFER_TO_HEADER_VALUE_token,
  SipParser_REFERRED_BY_HEADER_VALUE_token = 750,
  SipParser_REFERRER_URI_token, SipParser_REFERRED_BY_PARAMS_token,
  SipParser_REFERRED_BY_PARAM_token = 754,
  SipParser_REFERRED_BY_PARAM_CID_token,
  SipParser_CID_PARAM_STRING_token = 757, SipParser_DOT_ATOM_token,
  SipParser_ATOM_CHAR_token, SipParser_DOT_ATOM_CHAR_token,
  SipParser_EVENT_HEADER_VAL_token = 763, SipParser_EVENT_TYPE_token,
  SipParser_EVENT_PARAMS_token, SipParser_EVENT_PACKAGE_token,
  SipParser_EVENT_TEMPLATE_token, SipParser_EVENT_PARAM_token,
  SipParser_EVENT_ID_token, SipParser_ALLOW_EVENTS_HEADER_VAL_token = 771,
  SipParser_SUBSCRIPTION_STATE_HEADER_VAL_token,
  SipParser_SUBSTATE_VALUE_token, SipParser_SUBSTATE_PARAMS_token,
  SipParser_SUBSTATE_PARAM_token = 778,
  SipParser_SUBSTATE_PARAM_REASON_token,
  SipParser_SUBSTATE_PARAM_RETRY_AFTER_token,
  SipParser_SUBSTATE_PARAM_EXPIRES_token,
  SipParser_SUBSTATE_PARAM_REASON_VAL_token = 784,
  SipParser_DISPOSITION_VAL_token = 791, SipParser_DISPOSITION_TYPE_token,
  SipParser_DISPOSITION_PARAMS_token, SipParser_HANDLING_PARAM_token = 799,
  SipParser_SESSION_EXPIRES_VALUE_token = 803,
  SipParser_REFRESHER_TYPE_token, SipParser_REFRESHER_TYPE_NAME_token,
  SipParser_REFRESHER_TYPE_VALUE_token, SipParser_MIN_SE_VALUE_token = 810,
  SipParser_REPLACES_HEADER_VALUE_token, SipParser_REPLACES_PARAMS_token,
  SipParser_REPLACES_PARAM_token, SipParser_TO_TAG_PARAM_token,
  SipParser_FROM_TAG_PARAM_token, SipParser_EARLY_FLAG_PARAM_token,
  SipParser_AUTHENTICATION_INFO_HEADER_LIST_token = 820,
  SipParser_AUTHENTICATION_INFO_HEADER_PARAM_token,
  SipParser_NEXT_NONCE_token, SipParser_RESPONSE_AUTH_token,
  SipParser_MSG_QOP_token, SipParser_RESPONSE_AUTH_VAL_token = 827,
  SipParser_P_URI_HEADERS_LIST_token = 831,
  SipParser_P_URI_HEADER_VALUE_token, SipParser_P_URI_HEADER_PARAMS_token,
  SipParser_P_VISITED_NETWORK_ID_HEADERS_LIST_token,
  SipParser_P_VISITED_NETWORK_ID_HEADER_VALUE_token,
  SipParser_VNETWORK_SPEC_token,
  SipParser_P_VISITED_NETWORK_ID_PARAMS_token,
  SipParser_P_ACCESS_NETWORK_INFO_HEADER_VALUE_token,
  SipParser_ACCESS_TYPE_token, SipParser_P_ACCESS_NETWORK_INFO_PARAMS_token,
  SipParser_ACCESS_token, SipParser_ACCESS_PARAM_token = 863,
  SipParser_NETWORK_PROVIDED_token, SipParser_CGI_3GPP_token,
  SipParser_UTRAN_CELL_ID_3GPP_token, SipParser_I_WLAN_NODE_ID_token,
  SipParser_DSL_LOCATION_token, SipParser_CI_3GPP2_token,
  SipParser_GEN_VALUE_OTHER_PARAM_EXPR_token,
  SipParser_P_CHARGING_FUNCTION_ADDRESSES_HEADER_VALUE_token = 877,
  SipParser_CHARGE_ADDR_PARAM_token, SipParser_CHARGE_ADDR_PARAMS_token,
  SipParser_CHARGE_ADDR_PARAM_CCF_token,
  SipParser_CHARGE_ADDR_PARAM_ECF_token,
  SipParser_GENERAL_VALUE_token = 883,
  SipParser_P_CHARGING_VECTOR_HEADER_VALUE_token = 885,
  SipParser_ICID_VALUE_token, SipParser_CHARGE_PARAMS_token,
  SipParser_CHARGE_PARAM_token = 889,
  SipParser_CHARGE_PARAM_ICID_GEN_ADDR_token,
  SipParser_CHARGE_PARAM_ORIG_IOI_token,
  SipParser_CHARGE_PARAM_TERM_IOI_token,
  SipParser_CHARGE_PARAM_GPRS_CHARGING_INFO_token,
  SipParser_CHARGE_PARAM_XDSL_CHARGING_INFO_token,
  SipParser_PDP_INFO_HIERARCHY_token,
  SipParser_DSL_BEARER_INFO_HIERARCHY_token,
  SipParser_I_WLAN_CHARGING_INFO_token,
  SipParser_PACKETCABLE_CHARGING_INFO_token,
  SipParser_PACKETCABLE_CHARGING_INFO_BCID_token,
  SipParser_GGSN_token = 903, SipParser_AUTH_TOKEN_token,
  SipParser_BRAS_token, SipParser_PDP_INFO_token = 910,
  SipParser_DSL_BEARER_INFO_token = 912, SipParser_SINGLE_PDP_INFO_token,
  SipParser_SINGLE_DSL_BEARER_INFO_token, SipParser_PDP_ITEM_token,
  SipParser_PDP_SIG_token, SipParser_PDP_CID_token, SipParser_FLOW_ID_token,
  SipParser_YES_OR_NO_token = 921, SipParser_DSL_BEARER_ITEM_token = 923,
  SipParser_DSL_BEARER_SIG_token, SipParser_DSL_BEARER_CID_token,
  SipParser_LPAREN_token = 930, SipParser_FLOW_ID_VALUE_token,
  SipParser_FLOW_ID_SYNTAX1_token, SipParser_FLOW_ID_SYNTAX2_token,
  SipParser_DIGITS_token = 937, SipParser_WHITE_IN_WORD_token,
  SipParser_PACKETCABLE_CHARGING_INFO_BCID_TOKEN_token = 944,
  SipParser_P_MEDIA_AUTHORIZATION_VAL_token = 946,
  SipParser_P_MEDIA_AUTHORIZATION_TOKEN_token,
  SipParser_SECURITY_HEADERS_LIST_token,
  SipParser_SECURITY_HEADER_VALUE_token, SipParser_MECHANISM_NAME_token,
  SipParser_MECHANISM_PARAMS_token, SipParser_MECHANISM_token,
  SipParser_MECHANISM_PARAM_token = 956, SipParser_DIGEST_ALGORITHM_token,
  SipParser_DIGEST_QOP_token, SipParser_DIGEST_VERIFY_token,
  SipParser_SECURITY_ALGORITHM_token, SipParser_SECURITY_PROTOCOL_token,
  SipParser_SECURITY_MODE_token, SipParser_SECURITY_ENCRYPT_ALGORITHM_token,
  SipParser_SECURITY_SPIC_token, SipParser_SECURITY_SPIS_token,
  SipParser_SECURITY_PORTC_token, SipParser_SECURITY_PORTS_token,
  SipParser_DIGEST_ALGORITHM_DECLARATION_token,
  SipParser_DIGEST_VERIFY_VALUE_token = 972,
  SipParser_SECURITY_ALGORITHM_VAL_token = 974,
  SipParser_SECURITY_PROTOCOL_VAL_token = 978,
  SipParser_SECURITY_MODE_VAL_token = 982,
  SipParser_SECURITY_ENCRYPT_ALGORITHM_VAL_token = 987,
  SipParser_TEN_DIGIT_token = 992,
  SipParser_P_PROFILE_KEY_HEADERS_LIST_token = 996,
  SipParser_P_PROFILE_KEY_HEADER_VALUE_token,
  SipParser_P_PROFILE_KEY_HEADER_PARAMS_token,
  SipParser_P_USER_DATABASE_HEADERS_LIST_token,
  SipParser_P_USER_DATABASE_HEADER_VALUE_token,
  SipParser_P_ANSWER_STATE_HEADER_VALUE_token, SipParser_ANSWER_TYPE_token,
  SipParser_P_ANSWER_STATE_PARAMS_token, SipParser_ANSWER_token,
  SipParser_ANSWER_PARAM_token = 1007,
  SipParser_P_SERVED_USER_HEADERS_LIST_token,
  SipParser_P_SERVED_USER_HEADER_VALUE_token,
  SipParser_P_SERVED_USER_HEADER_PARAMS_token,
  SipParser_P_SERVED_USER_HEADER_PARAM_token,
  SipParser_P_SERVED_USER_SESSION_CASE_token,
  SipParser_P_SERVED_USER_REGISTRATION_STATE_token,
  SipParser_P_SERVED_USER_SESSION_CASE_VAL_token = 1015,
  SipParser_P_SERVED_USER_REGISTRATION_STATE_VAL_token = 1018
} SipParser_token_type;

typedef struct SipParser_pcb_struct{
  SipParser_token_type token_number, reduction_token, error_frame_token;
  int input_code;
  int input_value;
  int line, column;
  int ssx, sn, error_frame_ssx;
  int drt, dssx, dsn;
  int ss[46];
  SipParser_vs_type vs[46];
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



        ParserDiameterUri       diameterUri;    

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












		ParserPUriHeader				puriHeader; 
		ParserPVisitedNetworkIDHeader	pvisitedNetworkIDHeader; 
		ParserPAccessNetworkInfoHeader	paccessNetworkInfoHeader; 
		ParserPChargingFunctionAddressesHeader	pchargingFunctionAddressesHeader; 
		ParserPChargingVectorHeader		pchargingVectorHeader; 
		ParserPMediaAuthorizationHeader	pmediaAuthorizationHeader; 
		ParserSecurityHeader			securityHeader; 
		ParserPProfileKeyHeader	        pprofileKeyHeader; 
		ParserPUserDatabaseHeader       puserDatabaseHeader; 
		ParserPAnswerStateHeader        panswerStateHeader; 
		ParserPServedUserHeader         pservedUserHeader; 









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

#elif defined(RV_SIP_AUTH_ON) && defined(RV_SIP_TEL_URI_SUPPORT) && defined(RV_SIP_IMS_HEADER_SUPPORT) && defined(RV_SIP_IMS_DCS_HEADER_SUPPORT) && !defined(RV_SIP_EXTENDED_HEADER_SUPPORT) && !defined(RV_SIP_JSR32_SUPPORT) && !defined(RV_SIP_LIGHT) && !defined(RV_SIP_PRIMITIVES) && !defined(RV_SIP_SUBS_ON)
#ifndef PARSERENGINEIMS_H_1248033550
#define PARSERENGINEIMS_H_1248033550

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
  char ag_vt_37[sizeof(RvBool)];
  char ag_vt_38[sizeof(RvSipDiameterProtocol)];
  char ag_vt_39[sizeof(ParserUserParam)];
  char ag_vt_40[sizeof(RvSipUserParam)];
  char ag_vt_41[sizeof(ParserOtherUser)];
  char ag_vt_42[sizeof(ParserLrParamType)];
  char ag_vt_43[sizeof(ParserTokenizedByParam)];
  char ag_vt_44[sizeof(ParserCPCParam)];
  char ag_vt_45[sizeof(ParserGrParam)];
  char ag_vt_46[sizeof(ParserOptionalHeaders)];
  char ag_vt_47[sizeof(ParserTagParam)];
  char ag_vt_48[sizeof(ParserPartyParams)];
  char ag_vt_49[sizeof(ParserQVal)];
  char ag_vt_50[sizeof(ParserContactActionType)];
  char ag_vt_51[sizeof(ParserContactFeatureTag)];
  char ag_vt_52[sizeof(ParserDeltaSeconds)];
  char ag_vt_53[sizeof(ParserSipDate)];
  char ag_vt_54[sizeof(RvSipDateWeekDay)];
  char ag_vt_55[sizeof(ParserDDMMYY)];
  char ag_vt_56[sizeof(ParserInt32)];
  char ag_vt_57[sizeof(RvSipDateMonth)];
  char ag_vt_58[sizeof(ParserTime)];
  char ag_vt_59[sizeof(ParserTimeZone)];
  char ag_vt_60[sizeof(RvSipAuthStale)];
  char ag_vt_61[sizeof(ParserAlgorithm)];
  char ag_vt_62[sizeof(ParserQopValue)];
  char ag_vt_63[sizeof(ParserAuthQopOptions)];
  char ag_vt_64[sizeof(RvSipAuthQopOption)];
  char ag_vt_65[sizeof(RvSipAuthIntegrityProtected)];
  char ag_vt_66[sizeof(ParserRetryParam)];
  char ag_vt_67[sizeof(ParserRSeq)];
  char ag_vt_68[sizeof(ParserResponseNum)];
  char ag_vt_69[sizeof(ParserEventType)];
  char ag_vt_70[sizeof(ParserDisposition)];
  char ag_vt_71[sizeof(ParserDispositionParams)];
  char ag_vt_72[sizeof(ParserDispositionParam)];
  char ag_vt_73[sizeof(ParserSessionExpiresHeader)];
  char ag_vt_74[sizeof(RvSipSessionExpiresRefresherType)];
  char ag_vt_75[sizeof(ParserMinSEHeader)];
  char ag_vt_76[sizeof(ParserReplacesEarlyFlagParamType)];
  char ag_vt_77[sizeof(ParserAccess)];
  char ag_vt_78[sizeof(ParserMechanism)];
  char ag_vt_79[sizeof(ParserSecurityAlgorithmType)];
  char ag_vt_80[sizeof(ParserSecurityProtocolType)];
  char ag_vt_81[sizeof(ParserSecurityModeType)];
  char ag_vt_82[sizeof(ParserSecurityEncryptAlgorithmType)];
  char ag_vt_83[sizeof(ParserAnswer)];
  char ag_vt_84[sizeof(ParserSessionCaseType)];
  char ag_vt_85[sizeof(ParserRegistrationStateType)];
  char ag_vt_86[sizeof(ParserOSPSTag)];
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
  SipParser_TEL_URI_HEADER_token, SipParser_DIAMETER_URI_HEADER_token,
  SipParser_P_URI_HEADER_token = 64,
  SipParser_P_VISITED_NETWORK_ID_HEADER_token = 66,
  SipParser_P_ACCESS_NETWORK_INFO_HEADER_token = 68,
  SipParser_P_CHARGING_FUNCTION_ADDRESSES_HEADER_token = 70,
  SipParser_P_CHARGING_VECTOR_HEADER_token = 72,
  SipParser_P_MEDIA_AUTHORIZATION_HEADER_token = 74,
  SipParser_SECURITY_HEADER_token = 76,
  SipParser_P_PROFILE_KEY_HEADER_token = 78,
  SipParser_P_USER_DATABASE_HEADER_token = 80,
  SipParser_P_ANSWER_STATE_HEADER_token = 82,
  SipParser_P_SERVED_USER_HEADER_token = 84,
  SipParser_P_DCS_TRACE_PARTY_ID_HEADER_token = 86,
  SipParser_P_DCS_OSPS_HEADER_token = 88,
  SipParser_P_DCS_BILLING_INFO_HEADER_token = 90,
  SipParser_P_DCS_LAES_HEADER_token = 92,
  SipParser_P_DCS_REDIRECT_HEADER_token = 94, SipParser_OTHER_HEADER_token,
  SipParser_WHITE_token = 97, SipParser_SP_token = 99,
  SipParser_COMMA_token = 101, SipParser_COMMA_BETWEEN_HEADERS_token = 103,
  SipParser_DOT_token, SipParser_EQUAL_token = 108,
  SipParser_AT_token = 110, SipParser_COLON_token = 112,
  SipParser_SEMI_COLON_token, SipParser_MARK_SIGN_token = 115,
  SipParser_SLASH_token = 117, SipParser_LEFT_PARENTHESIS_token = 119,
  SipParser_LEFT_ANGLE_BRACKET_token = 121,
  SipParser_RIGHT_ANGLE_BRACKET_token = 123,
  SipParser_LEFT_QUOTATION_token = 125, SipParser_HYPHEN_token = 127,
  SipParser_TOKEN_token = 129, SipParser_TOKEN_CHAR_token,
  SipParser_CALL_ID_WORD_token = 132, SipParser_CALL_ID_WORD_CHAR_token,
  SipParser_TOKEN_FOR_CONTENT_TYPE_token = 135,
  SipParser_TOKEN_CHAR_FOR_CONTENT_TYPE_token,
  SipParser_TOKEN_NO_DOT_token = 138, SipParser_TOKEN_CHAR_NO_DOT_token,
  SipParser_FIELD_VALUE_token = 141, SipParser_QUOTED_STRING_token = 147,
  SipParser_QUOTED_CHAR_token, SipParser_HEX_VALUE_token = 155,
  SipParser_HEX_token, SipParser_INT_NUMBER_token = 158,
  SipParser_DIGIT_token, SipParser_TOKEN_OR_QUOTED_STRING_token = 161,
  SipParser_METHOD_token, SipParser_CSEQ_VAL_token = 169,
  SipParser_SEQUNCE_NUMBER_token, SipParser_CONTENT_LENGTH_VAL_token,
  SipParser_CALL_ID_token, SipParser_MEDIA_TYPE_token,
  SipParser_MEDIA_TYPE_TYPE_token, SipParser_MEDIA_TYPE_SUB_TYPE_token,
  SipParser_OPTIONAL_CONTENT_TYPE_PARAM_LIST_token,
  SipParser_CONTENT_TYPE_PARAM_LIST_token, SipParser_BOUNDARY_PARAM_token,
  SipParser_VERSION_PARAM_token, SipParser_BASE_PARAM_token,
  SipParser_START_PARAM_token, SipParser_TYPE_PARAM_token,
  SipParser_CONTENT_TYPE_GENERIC_PARAM_DATA_token,
  SipParser_START_PARAM_NAME_token = 208,
  SipParser_OLD_OR_NEW_ADDR_SPEC_token,
  SipParser_NEW_ADDR_SPEC_PREFIX_token = 213, SipParser_ADDR_SPEC_token,
  SipParser_OLD_ADDR_SPEC_PREFIX_token, SipParser_USER_INFO_TOKEN_token,
  SipParser_HOST_NAME_token, SipParser_TYPE_PARAM_MEDIA_token = 221,
  SipParser_CONTENT_TYPE_GENERIC_PARAM_DATA_VAL_token,
  SipParser_CONTENT_ID_HEADER_VALUE_token,
  SipParser_CONTENT_ID_HEADER_LEFT_BRACKET_token,
  SipParser_CONTENT_ID_OLD_OR_NEW_ADDR_SPEC_token,
  SipParser_GENERIC_PARAM_LIST_token, SipParser_GENERIC_PARAM_DATA_token,
  SipParser_GENERIC_PARAM_DATA_VAL_token, SipParser_SIP_VERSION_token,
  SipParser_STATUS_CODE_token = 231, SipParser_REASON_PHRASE_token,
  SipParser_EXTENSION_CODE_token = 234, SipParser_REASONE_TEXT_UTF8_token,
  SipParser_ALLOW_VAL_token = 239, SipParser_VIA_LIST_token,
  SipParser_SINGLE_VIA_token, SipParser_SENT_PROTOCOL_token,
  SipParser_SENT_BY_token, SipParser_VIA_PARAM_LIST_token,
  SipParser_VIA_PARAM_token, SipParser_PROTOCOL_NAME_token,
  SipParser_PROTOCOL_VERSION_token, SipParser_TRANSPORT_token,
  SipParser_TRANSPORT_TYPE_token = 250, SipParser_PORT_token = 255,
  SipParser_VIA_HIDDEN_token, SipParser_VIA_ALIAS_token,
  SipParser_VIA_TTL_token, SipParser_VIA_MADDR_token,
  SipParser_VIA_RECEIVED_token, SipParser_VIA_BRANCH_token,
  SipParser_RESPONSE_PORT_token, SipParser_VIA_COMPRESSION_token,
  SipParser_VIA_SIP_SIGCOMP_ID_token, SipParser_OTHER_PARAM_token,
  SipParser_OTHER_PARAM_EXPR_token, SipParser_IPV_6_REFERENCE_token,
  SipParser_TTL_PARAM_token = 270, SipParser_TTL_token = 272,
  SipParser_RPORT_NAME_token, SipParser_RPORT_VAL_token,
  SipParser_MADDR_PARAM_token = 276,
  SipParser_HOST_NAME_FOR_RECEIVED_token = 279,
  SipParser_COMPRESSION_TYPE_token = 282,
  SipParser_SIGCOMP_ID_TOKEN_token = 285, SipParser_IPV_6_ADDRESS_token,
  SipParser_TEST_COMMENT_token, SipParser_COMMENT_TEXT_token,
  SipParser_END_COMMENT_token = 290, SipParser_REQUEST_URI_token = 293,
  SipParser_ABSOLUTE_URI_token, SipParser_ABS_URI_SCHEME_token,
  SipParser_URI_IDENTIFIER_token, SipParser_ABS_URI_SCHEME_CHAR_token,
  SipParser_URI_SCHEME_OR_DISPLAY_NAME_token = 299,
  SipParser_DISPLAY_NAME_token, SipParser_URI_IDENTIFIER_CHAR_token,
  SipParser_URI_IDENTIFIER_WITHOUT_SEPARATORS_token = 303,
  SipParser_TEL_URI_token = 306, SipParser_TEL_PREFIX_token,
  SipParser_TEL_token, SipParser_GLOBAL_OR_LOCAL_NUMBER_token = 310,
  SipParser_OPTIONAL_TEL_URI_PARAMETERS_token,
  SipParser_GLOBAL_NUMBER_token, SipParser_LOCAL_NUMBER_token,
  SipParser_GLOBAL_NUMBER_DIGITS_token, SipParser_PHONEDIGIT_token = 316,
  SipParser_VISUAL_SEPERATOR_token = 320,
  SipParser_LOCAL_NUMBER_DIGITS_token = 322,
  SipParser_HEX_OR_PHONECHAR_token, SipParser_PHONEDIGIT_HEX_token,
  SipParser_PHONECHAR_token = 328, SipParser_TEL_URI_PARAMETERS_token,
  SipParser_TEL_URI_PARAMETER_token,
  SipParser_TEL_URI_EXTENSION_PARAM_token,
  SipParser_TEL_URI_POST_DIAL_PARAM_token,
  SipParser_TEL_URI_ISDN_SUB_ADDR_PARAM_URL_token,
  SipParser_TEL_URI_CONTEXT_PARAM_token,
  SipParser_TEL_URI_ENUMDI_PARAM_token,
  SipParser_URL_GENERIC_PARAM_DATA_token, SipParser_CPC_PARAM_URL_token,
  SipParser_TEL_URI_RN_PARAM_token,
  SipParser_TEL_URI_RN_CONTEXT_PARAM_token,
  SipParser_TEL_URI_CIC_PARAM_token,
  SipParser_TEL_URI_CIC_CONTEXT_PARAM_token,
  SipParser_TEL_URI_NPDI_PARAM_token,
  SipParser_TEL_URI_EXTENTION_VALUE_token = 344,
  SipParser_TEL_URI_POST_DIAL_VALUE_token = 347,
  SipParser_TEL_URI_POST_DIAL_CHAR_token, SipParser_DTMFDIGIT_token = 350,
  SipParser_PAUSE_CHAR_token,
  SipParser_TEL_URI_ISDN_SUB_ADDR_VALUE_token = 353, SipParser_URIC_token,
  SipParser_RESERVED_FOR_URIC_token = 356, SipParser_UNRESERVED_token,
  SipParser_PCT_ENCODED_token, SipParser_TEL_URI_CONTEXT_VALUE_token = 361,
  SipParser_DESCRIPTOR_token, SipParser_DOMAINNAME_token,
  SipParser_LABEL_token, SipParser_NEXT_LABEL_token,
  SipParser_ALPHA_token = 369, SipParser_ALPHANUM_OR_HYPHEN_token,
  SipParser_ALPHANUM_token = 374,
  SipParser_TEL_URI_NP_GLOBAL_OR_LOCAL_NUMBER_token = 377,
  SipParser_DIAMETER_URI_token = 382, SipParser_DIAMETER_PREFIX_token,
  SipParser_DIAMETER_HOSTPORT_token, SipParser_DIAMETER_PARAMS_token,
  SipParser_DIAMETER_PARAM_token = 388, SipParser_TRANSPORT_PARAM_token,
  SipParser_DIAMETER_PROTOCOL_PARAM_token = 391,
  SipParser_SIP_URL_token = 395, SipParser_PRES_URI_token,
  SipParser_IM_URI_token, SipParser_SIP_PREFIX_token, SipParser_URL_token,
  SipParser_PRES_PREFIX_token = 402, SipParser_RV_ABS_PREFIX_token,
  SipParser_IM_PREFIX_token = 406, SipParser_URL_USERINFO_token = 408,
  SipParser_OPTIONAL_URL_PARAMETERS_token, SipParser_OPTIONAL_HEADERS_token,
  SipParser_URL_HOST_token, SipParser_GENERAL_URL_WITHOUT_PARAM_token,
  SipParser_SIP_URL_WITHOUT_PARAMS_token,
  SipParser_TEL_URL_WITHOUT_PARAMS_token,
  SipParser_DIAMETER_URI_WITHOUT_PARAMS_token,
  SipParser_PRES_URL_WITHOUT_PARAMS_token,
  SipParser_IM_URL_WITHOUT_PARAMS_token,
  SipParser_ABS_URI_WITHOUT_PARAMS_token,
  SipParser_URL_USERINFO_PREFIX_token, SipParser_URL_USERINFO_VAL_token,
  SipParser_HOSTPORT_token, SipParser_PASSWORD_token = 423,
  SipParser_TOKEN_USER_INFO_token, SipParser_TOKEN_USER_INFO_CHAR_token,
  SipParser_PASSWORD_STRING_token = 427,
  SipParser_PASSWORD_STRING_CHAR_token,
  SipParser_URL_HOST_PREFIX_token = 432, SipParser_TOKEN_HOST_token = 434,
  SipParser_TOKEN_HOST_CHAR_token, SipParser_IPV_4_ADDRESS_token = 439,
  SipParser_IPV_4_PART_token, SipParser_HEXPART_token,
  SipParser_HEXSEQ_token, SipParser_HEX4_token = 445,
  SipParser_URL_PARAMETERS_token = 449, SipParser_URL_PARAMETER_token,
  SipParser_USER_PARAM_token, SipParser_TTL_PARAM_URL_token,
  SipParser_MADDR_PARAM_URL_token, SipParser_METHOD_PARAM_URL_token,
  SipParser_LR_PARAM_URL_token, SipParser_COMPRESSION_PARAM_URL_token,
  SipParser_SIGCOMPID_PARAM_URL_token,
  SipParser_TOKENIZED_BY_PARAM_URL_token, SipParser_ORIG_PARAM_URL_token,
  SipParser_GR_PARAM_URL_token, SipParser_USER_PARAM_TYPE_token = 463,
  SipParser_OTHER_USER_token, SipParser_PARAMCHAR_token = 474,
  SipParser_CPC_PARAM_URL_VALUE_token = 481,
  SipParser_URL_OTHER_PARAM_VAL_token = 483, SipParser_PVALUE_token = 494,
  SipParser_HEADERS_LIST_token, SipParser_HEADER_token,
  SipParser_HNAME_token = 498, SipParser_HVALUE_token,
  SipParser_REQUEST_LINE_VALUE_token = 506, SipParser_REQUEST_METHOD_token,
  SipParser_PARTY_VALUE_token = 521, SipParser_NAME_ADDR_token,
  SipParser_PARTY_PARAMS_LIST_token, SipParser_MORE_THAN_ONE_TOKEN_token,
  SipParser_TAG_PARAM_token = 528, SipParser_PARTY_PARAM_DATA_token = 530,
  SipParser_CONTACT_HEADER_VALUES_token,
  SipParser_CONTACT_HEADER_OTHER_token,
  SipParser_CONTACT_HEADER_VALUE_LIST_token,
  SipParser_CONTACT_HEADER_VALUE_token = 535,
  SipParser_CONTACT_PARAMS_token, SipParser_CONTACT_PARAM_token,
  SipParser_Q_token, SipParser_ACTION_token,
  SipParser_CONTACT_EXPIRES_token, SipParser_TEMP_GRUU_token,
  SipParser_PUB_GRUU_token, SipParser_REG_ID_token,
  SipParser_CONTACT_FEATURE_TAG_AUDIO_token,
  SipParser_CONTACT_FEATURE_TAG_AUTOMATA_token,
  SipParser_CONTACT_FEATURE_TAG_CLASS_token,
  SipParser_CONTACT_FEATURE_TAG_DUPLEX_token,
  SipParser_CONTACT_FEATURE_TAG_DATA_token,
  SipParser_CONTACT_FEATURE_TAG_CONTROL_token,
  SipParser_CONTACT_FEATURE_TAG_MOBILITY_token,
  SipParser_CONTACT_FEATURE_TAG_DESCRIPTION_token,
  SipParser_CONTACT_FEATURE_TAG_EVENTS_token,
  SipParser_CONTACT_FEATURE_TAG_PRIORITY_token,
  SipParser_CONTACT_FEATURE_TAG_METHODS_token,
  SipParser_CONTACT_FEATURE_TAG_SCHEMES_token,
  SipParser_CONTACT_FEATURE_TAG_APPLICATION_token,
  SipParser_CONTACT_FEATURE_TAG_VIDEO_token,
  SipParser_CONTACT_FEATURE_TAG_LANGUAGE_token,
  SipParser_CONTACT_FEATURE_TAG_TYPE_token,
  SipParser_CONTACT_FEATURE_TAG_ISFOCUS_token,
  SipParser_CONTACT_FEATURE_TAG_ACTOR_token,
  SipParser_CONTACT_FEATURE_TAG_TEXT_token,
  SipParser_CONTACT_FEATURE_TAG_EXTENSIONS_token,
  SipParser_CONTACT_FEATURE_TAG_SIP_INSTANCE_token,
  SipParser_Q_VALUE_token = 566, SipParser_Q_VALUE_OPTIONAL_token = 568,
  SipParser_ACTION_VALUE_token = 572, SipParser_GRUU_PARAM_VAL_token = 576,
  SipParser_FEATURE_TAG_STRING_token = 580,
  SipParser_EXPIRES_VALUE_token = 597,
  SipParser_EXPIRES_VALUE_FOR_CONTACT_token = 599,
  SipParser_DELTA_SECONDS_token, SipParser_SIP_DATE_token,
  SipParser_SIP_DATE_HEADER_VAL_token, SipParser_DATE_token,
  SipParser_TIME_token, SipParser_DAY_token, SipParser_DD_token = 613,
  SipParser_MM_token, SipParser_YY_token, SipParser_HOUR_token = 628,
  SipParser_MINUTE_token, SipParser_SECOND_token, SipParser_TIME_ZONE_token,
  SipParser_AUTHENTICATION_VAL_token = 635, SipParser_CHALLENGE_VALUE_token,
  SipParser_DIGEST_CHALLENGE_LIST_token, SipParser_AUTH_PARAM_LIST_token,
  SipParser_DIGEST_CHALLENGE_token, SipParser_REALM_token,
  SipParser_DOMAIN_token, SipParser_NONCE_token, SipParser_OPAQUE_token,
  SipParser_INTEGRITY_KEY_token, SipParser_CIPHER_KEY_token,
  SipParser_AUTH_PARAM_token, SipParser_STALE_token,
  SipParser_ALGORITHM_token, SipParser_QOP_OPTIONS_token,
  SipParser_PARAM_IN_NEXT_CHALLENGE_token,
  SipParser_REALM_VALUE_token = 652, SipParser_DOMAIN_EQUAL_token = 654,
  SipParser_DOMAIN_WHITE_token, SipParser_DOMAIN_VAL_token = 657,
  SipParser_URI_LIST_token, SipParser_STRING_DOMAIN_PATCH_token,
  SipParser_TOKEN_FOR_DOMAIN_token, SipParser_TOKEN_CHAR_FOR_DOMAIN_token,
  SipParser_DOMAIN_SPACE_token = 663, SipParser_NONCE_VALUE_token = 666,
  SipParser_OPAQUE_VALUE_token = 668,
  SipParser_ALGORITHM_DECLARATION_token = 671,
  SipParser_ALGORITHM_AND_AKAV_token, SipParser_AKAV_PREFIX_token = 674,
  SipParser_AKAV_token, SipParser_AKA_VERSION_VAL_token,
  SipParser_ALGORITHM_VAL_token, SipParser_ALGORITHM_PREFIX_token,
  SipParser_QOP_VALUE_LIST_token = 686, SipParser_QOP_VALUE_token,
  SipParser_QUOTED_HEX_VALUE_token = 691,
  SipParser_AUTH_PARAM_DATA_token = 694, SipParser_AUTHORIZATION_VAL_token,
  SipParser_CREDENTIALS_VALUE_token, SipParser_DIGEST_RESPONSE_LIST_token,
  SipParser_DIGEST_RESPONSE_token, SipParser_USERNAME_token,
  SipParser_RESPONSE_token, SipParser_CNONCE_token,
  SipParser_NONCE_COUNT_token, SipParser_AUTS_PARAM_token,
  SipParser_INTEGRITY_PROTECTED_token, SipParser_MESSAGE_QOP_token,
  SipParser_DIGEST_URI_token, SipParser_USERNAME_VALUE_token = 708,
  SipParser_DIGEST_URI_VALUE_token = 710,
  SipParser_LEFT_URI_QUOTATION_token, SipParser_CNONCE_VALUE_token = 713,
  SipParser_NC_VALUE_token = 715, SipParser_LHEX_token,
  SipParser_REQUEST_DIGEST_token = 718, SipParser_THIRTY_TWO_LHEX_token,
  SipParser_SIXTY_FOUR_BASE_ENCODING_token = 723,
  SipParser_SIXTY_FOUR_BASE_ENCODING_CHAR_token,
  SipParser_ONE_OR_TWO_EQUAL_SIGNS_token = 726,
  SipParser_ROUTE_LIST_token = 733, SipParser_SINGLE_ROUTE_token,
  SipParser_RETRY_AFTER_VAL_token, SipParser_RETRY_AFTER_COMMENT_token,
  SipParser_RETRY_PARAM_token = 738, SipParser_DURATION_PARAM_token,
  SipParser_RSEQ_VAL_token = 741, SipParser_RESPONSE_NUM_token,
  SipParser_RACK_VAL_token, SipParser_OPTION_TAG_token,
  SipParser_OPTION_TAG_LIST_token, SipParser_SUPPORTED_OPTION_TAG_token,
  SipParser_EVENT_HEADER_VAL_token, SipParser_EVENT_TYPE_token,
  SipParser_EVENT_PARAMS_token, SipParser_EVENT_PACKAGE_token,
  SipParser_EVENT_TEMPLATE_token, SipParser_EVENT_PARAM_token,
  SipParser_EVENT_ID_token, SipParser_DISPOSITION_VAL_token = 755,
  SipParser_DISPOSITION_TYPE_token, SipParser_DISPOSITION_PARAMS_token,
  SipParser_HANDLING_PARAM_token = 763,
  SipParser_SESSION_EXPIRES_VALUE_token = 767,
  SipParser_REFRESHER_TYPE_token, SipParser_REFRESHER_TYPE_NAME_token = 770,
  SipParser_REFRESHER_TYPE_VALUE_token, SipParser_MIN_SE_VALUE_token = 775,
  SipParser_REPLACES_HEADER_VALUE_token, SipParser_REPLACES_PARAMS_token,
  SipParser_REPLACES_PARAM_token, SipParser_TO_TAG_PARAM_token,
  SipParser_FROM_TAG_PARAM_token, SipParser_EARLY_FLAG_PARAM_token,
  SipParser_AUTHENTICATION_INFO_HEADER_LIST_token = 785,
  SipParser_AUTHENTICATION_INFO_HEADER_PARAM_token,
  SipParser_NEXT_NONCE_token, SipParser_RESPONSE_AUTH_token,
  SipParser_MSG_QOP_token, SipParser_RESPONSE_AUTH_VAL_token = 792,
  SipParser_P_URI_HEADERS_LIST_token = 796,
  SipParser_P_URI_HEADER_VALUE_token, SipParser_P_URI_HEADER_PARAMS_token,
  SipParser_P_VISITED_NETWORK_ID_HEADERS_LIST_token,
  SipParser_P_VISITED_NETWORK_ID_HEADER_VALUE_token,
  SipParser_VNETWORK_SPEC_token,
  SipParser_P_VISITED_NETWORK_ID_PARAMS_token,
  SipParser_P_ACCESS_NETWORK_INFO_HEADER_VALUE_token,
  SipParser_ACCESS_TYPE_token, SipParser_P_ACCESS_NETWORK_INFO_PARAMS_token,
  SipParser_ACCESS_token, SipParser_ACCESS_PARAM_token = 828,
  SipParser_NETWORK_PROVIDED_token, SipParser_CGI_3GPP_token,
  SipParser_UTRAN_CELL_ID_3GPP_token, SipParser_I_WLAN_NODE_ID_token,
  SipParser_DSL_LOCATION_token, SipParser_CI_3GPP2_token,
  SipParser_GEN_VALUE_OTHER_PARAM_EXPR_token,
  SipParser_P_CHARGING_FUNCTION_ADDRESSES_HEADER_VALUE_token = 842,
  SipParser_CHARGE_ADDR_PARAM_token, SipParser_CHARGE_ADDR_PARAMS_token,
  SipParser_CHARGE_ADDR_PARAM_CCF_token,
  SipParser_CHARGE_ADDR_PARAM_ECF_token,
  SipParser_GENERAL_VALUE_token = 848,
  SipParser_P_CHARGING_VECTOR_HEADER_VALUE_token = 850,
  SipParser_ICID_VALUE_token, SipParser_CHARGE_PARAMS_token,
  SipParser_CHARGE_PARAM_token = 854,
  SipParser_CHARGE_PARAM_ICID_GEN_ADDR_token,
  SipParser_CHARGE_PARAM_ORIG_IOI_token,
  SipParser_CHARGE_PARAM_TERM_IOI_token,
  SipParser_CHARGE_PARAM_GPRS_CHARGING_INFO_token,
  SipParser_CHARGE_PARAM_XDSL_CHARGING_INFO_token,
  SipParser_PDP_INFO_HIERARCHY_token,
  SipParser_DSL_BEARER_INFO_HIERARCHY_token,
  SipParser_I_WLAN_CHARGING_INFO_token,
  SipParser_PACKETCABLE_CHARGING_INFO_token,
  SipParser_PACKETCABLE_CHARGING_INFO_BCID_token,
  SipParser_GGSN_token = 868, SipParser_AUTH_TOKEN_token,
  SipParser_BRAS_token, SipParser_PDP_INFO_token = 875,
  SipParser_DSL_BEARER_INFO_token = 877, SipParser_SINGLE_PDP_INFO_token,
  SipParser_SINGLE_DSL_BEARER_INFO_token, SipParser_PDP_ITEM_token,
  SipParser_PDP_SIG_token, SipParser_PDP_CID_token, SipParser_FLOW_ID_token,
  SipParser_YES_OR_NO_token = 886, SipParser_DSL_BEARER_ITEM_token = 888,
  SipParser_DSL_BEARER_SIG_token, SipParser_DSL_BEARER_CID_token,
  SipParser_LPAREN_token = 895, SipParser_FLOW_ID_VALUE_token,
  SipParser_FLOW_ID_SYNTAX1_token, SipParser_FLOW_ID_SYNTAX2_token,
  SipParser_DIGITS_token = 902, SipParser_WHITE_IN_WORD_token,
  SipParser_PACKETCABLE_CHARGING_INFO_BCID_TOKEN_token = 909,
  SipParser_P_MEDIA_AUTHORIZATION_VAL_token = 911,
  SipParser_P_MEDIA_AUTHORIZATION_TOKEN_token,
  SipParser_SECURITY_HEADERS_LIST_token,
  SipParser_SECURITY_HEADER_VALUE_token, SipParser_MECHANISM_NAME_token,
  SipParser_MECHANISM_PARAMS_token, SipParser_MECHANISM_token,
  SipParser_MECHANISM_PARAM_token = 921, SipParser_DIGEST_ALGORITHM_token,
  SipParser_DIGEST_QOP_token, SipParser_DIGEST_VERIFY_token,
  SipParser_SECURITY_ALGORITHM_token, SipParser_SECURITY_PROTOCOL_token,
  SipParser_SECURITY_MODE_token, SipParser_SECURITY_ENCRYPT_ALGORITHM_token,
  SipParser_SECURITY_SPIC_token, SipParser_SECURITY_SPIS_token,
  SipParser_SECURITY_PORTC_token, SipParser_SECURITY_PORTS_token,
  SipParser_DIGEST_ALGORITHM_DECLARATION_token,
  SipParser_DIGEST_VERIFY_VALUE_token = 937,
  SipParser_SECURITY_ALGORITHM_VAL_token = 939,
  SipParser_SECURITY_PROTOCOL_VAL_token = 943,
  SipParser_SECURITY_MODE_VAL_token = 947,
  SipParser_SECURITY_ENCRYPT_ALGORITHM_VAL_token = 952,
  SipParser_TEN_DIGIT_token = 957,
  SipParser_P_PROFILE_KEY_HEADERS_LIST_token = 961,
  SipParser_P_PROFILE_KEY_HEADER_VALUE_token,
  SipParser_P_PROFILE_KEY_HEADER_PARAMS_token,
  SipParser_P_USER_DATABASE_HEADERS_LIST_token,
  SipParser_P_USER_DATABASE_HEADER_VALUE_token,
  SipParser_P_ANSWER_STATE_HEADER_VALUE_token, SipParser_ANSWER_TYPE_token,
  SipParser_P_ANSWER_STATE_PARAMS_token, SipParser_ANSWER_token,
  SipParser_ANSWER_PARAM_token = 972,
  SipParser_P_SERVED_USER_HEADERS_LIST_token,
  SipParser_P_SERVED_USER_HEADER_VALUE_token,
  SipParser_P_SERVED_USER_HEADER_PARAMS_token,
  SipParser_P_SERVED_USER_HEADER_PARAM_token,
  SipParser_P_SERVED_USER_SESSION_CASE_token,
  SipParser_P_SERVED_USER_REGISTRATION_STATE_token,
  SipParser_P_SERVED_USER_SESSION_CASE_VAL_token = 980,
  SipParser_P_SERVED_USER_REGISTRATION_STATE_VAL_token = 983,
  SipParser_P_DCS_TRACE_PARTY_ID_HEADER_VALUE_token = 986,
  SipParser_P_DCS_OSPS_HEADER_VALUE_token, SipParser_OSPS_TAG_token,
  SipParser_P_DCS_BILLING_INFO_HEADER_VALUE_token = 992,
  SipParser_BILLING_CORRELATION_ID_token, SipParser_FEID_token,
  SipParser_FEID_HOST_token, SipParser_BILLING_INFO_PARAMS_token,
  SipParser_BILLING_CORRELATION_ID_VALUE_token,
  SipParser_FORTY_EIGHT_HEXDIG_token, SipParser_FEID_VALUE_token,
  SipParser_SIXTEEN_HEXDIG_token, SipParser_BILLING_INFO_PARAM_token,
  SipParser_P_DCS_LAES_HEADER_VALUE_token = 1008, SipParser_LAES_SIG_token,
  SipParser_LAES_PARAMS_token, SipParser_LAES_PARAM_token,
  SipParser_LAES_CONTENT_token = 1013,
  SipParser_P_DCS_REDIRECT_HEADER_VALUE_token = 1015,
  SipParser_CALLED_ID_URI_token, SipParser_REDIRECT_PARAMS_token,
  SipParser_REDIRECT_PARAM_token
} SipParser_token_type;

typedef struct SipParser_pcb_struct{
  SipParser_token_type token_number, reduction_token, error_frame_token;
  int input_code;
  int input_value;
  int line, column;
  int ssx, sn, error_frame_ssx;
  int drt, dssx, dsn;
  int ss[49];
  SipParser_vs_type vs[49];
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



        ParserDiameterUri       diameterUri;    

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












		ParserPUriHeader				puriHeader; 
		ParserPVisitedNetworkIDHeader	pvisitedNetworkIDHeader; 
		ParserPAccessNetworkInfoHeader	paccessNetworkInfoHeader; 
		ParserPChargingFunctionAddressesHeader	pchargingFunctionAddressesHeader; 
		ParserPChargingVectorHeader		pchargingVectorHeader; 
		ParserPMediaAuthorizationHeader	pmediaAuthorizationHeader; 
		ParserSecurityHeader			securityHeader; 
		ParserPProfileKeyHeader	        pprofileKeyHeader; 
		ParserPUserDatabaseHeader       puserDatabaseHeader; 
		ParserPAnswerStateHeader        panswerStateHeader; 
		ParserPServedUserHeader         pservedUserHeader; 


		ParserPDCSTracePartyIDHeader	pdcsTracePartyIDHeader; 
		ParserPDCSOSPSHeader			pdcsOSPSHeader; 
		ParserPDCSBillingInfoHeader		pdcsBillingInfoHeader; 
		ParserPDCSLAESHeader			pdcsLAESHeader; 
		ParserPDCSRedirectHeader		pdcsRedirectHeader; 


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

#elif defined(RV_SIP_AUTH_ON) && defined(RV_SIP_TEL_URI_SUPPORT) && defined(RV_SIP_IMS_HEADER_SUPPORT) && defined(RV_SIP_IMS_DCS_HEADER_SUPPORT) && !defined(RV_SIP_EXTENDED_HEADER_SUPPORT) && !defined(RV_SIP_JSR32_SUPPORT) && !defined(RV_SIP_LIGHT) && !defined(RV_SIP_PRIMITIVES) && defined(RV_SIP_SUBS_ON)
#ifndef PARSERENGINEIMS_H_1248033582
#define PARSERENGINEIMS_H_1248033582

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
  char ag_vt_37[sizeof(RvBool)];
  char ag_vt_38[sizeof(RvSipDiameterProtocol)];
  char ag_vt_39[sizeof(ParserUserParam)];
  char ag_vt_40[sizeof(RvSipUserParam)];
  char ag_vt_41[sizeof(ParserOtherUser)];
  char ag_vt_42[sizeof(ParserLrParamType)];
  char ag_vt_43[sizeof(ParserTokenizedByParam)];
  char ag_vt_44[sizeof(ParserCPCParam)];
  char ag_vt_45[sizeof(ParserGrParam)];
  char ag_vt_46[sizeof(ParserOptionalHeaders)];
  char ag_vt_47[sizeof(ParserTagParam)];
  char ag_vt_48[sizeof(ParserPartyParams)];
  char ag_vt_49[sizeof(ParserQVal)];
  char ag_vt_50[sizeof(ParserContactActionType)];
  char ag_vt_51[sizeof(ParserContactFeatureTag)];
  char ag_vt_52[sizeof(ParserDeltaSeconds)];
  char ag_vt_53[sizeof(ParserSipDate)];
  char ag_vt_54[sizeof(RvSipDateWeekDay)];
  char ag_vt_55[sizeof(ParserDDMMYY)];
  char ag_vt_56[sizeof(ParserInt32)];
  char ag_vt_57[sizeof(RvSipDateMonth)];
  char ag_vt_58[sizeof(ParserTime)];
  char ag_vt_59[sizeof(ParserTimeZone)];
  char ag_vt_60[sizeof(RvSipAuthStale)];
  char ag_vt_61[sizeof(ParserAlgorithm)];
  char ag_vt_62[sizeof(ParserQopValue)];
  char ag_vt_63[sizeof(ParserAuthQopOptions)];
  char ag_vt_64[sizeof(RvSipAuthQopOption)];
  char ag_vt_65[sizeof(RvSipAuthIntegrityProtected)];
  char ag_vt_66[sizeof(ParserRetryParam)];
  char ag_vt_67[sizeof(ParserRSeq)];
  char ag_vt_68[sizeof(ParserResponseNum)];
  char ag_vt_69[sizeof(ParserEventType)];
  char ag_vt_70[sizeof(ParserSubsStateValue)];
  char ag_vt_71[sizeof(ParserSubsStateReason)];
  char ag_vt_72[sizeof(ParserDisposition)];
  char ag_vt_73[sizeof(ParserDispositionParams)];
  char ag_vt_74[sizeof(ParserDispositionParam)];
  char ag_vt_75[sizeof(ParserSessionExpiresHeader)];
  char ag_vt_76[sizeof(RvSipSessionExpiresRefresherType)];
  char ag_vt_77[sizeof(ParserMinSEHeader)];
  char ag_vt_78[sizeof(ParserReplacesEarlyFlagParamType)];
  char ag_vt_79[sizeof(ParserAccess)];
  char ag_vt_80[sizeof(ParserMechanism)];
  char ag_vt_81[sizeof(ParserSecurityAlgorithmType)];
  char ag_vt_82[sizeof(ParserSecurityProtocolType)];
  char ag_vt_83[sizeof(ParserSecurityModeType)];
  char ag_vt_84[sizeof(ParserSecurityEncryptAlgorithmType)];
  char ag_vt_85[sizeof(ParserAnswer)];
  char ag_vt_86[sizeof(ParserSessionCaseType)];
  char ag_vt_87[sizeof(ParserRegistrationStateType)];
  char ag_vt_88[sizeof(ParserOSPSTag)];
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
  SipParser_TEL_URI_HEADER_token, SipParser_DIAMETER_URI_HEADER_token,
  SipParser_P_URI_HEADER_token = 72,
  SipParser_P_VISITED_NETWORK_ID_HEADER_token = 74,
  SipParser_P_ACCESS_NETWORK_INFO_HEADER_token = 76,
  SipParser_P_CHARGING_FUNCTION_ADDRESSES_HEADER_token = 78,
  SipParser_P_CHARGING_VECTOR_HEADER_token = 80,
  SipParser_P_MEDIA_AUTHORIZATION_HEADER_token = 82,
  SipParser_SECURITY_HEADER_token = 84,
  SipParser_P_PROFILE_KEY_HEADER_token = 86,
  SipParser_P_USER_DATABASE_HEADER_token = 88,
  SipParser_P_ANSWER_STATE_HEADER_token = 90,
  SipParser_P_SERVED_USER_HEADER_token = 92,
  SipParser_P_DCS_TRACE_PARTY_ID_HEADER_token = 94,
  SipParser_P_DCS_OSPS_HEADER_token = 96,
  SipParser_P_DCS_BILLING_INFO_HEADER_token = 98,
  SipParser_P_DCS_LAES_HEADER_token = 100,
  SipParser_P_DCS_REDIRECT_HEADER_token = 102, SipParser_OTHER_HEADER_token,
  SipParser_WHITE_token = 105, SipParser_SP_token = 107,
  SipParser_COMMA_token = 109, SipParser_COMMA_BETWEEN_HEADERS_token = 111,
  SipParser_DOT_token, SipParser_EQUAL_token = 116,
  SipParser_AT_token = 118, SipParser_COLON_token = 120,
  SipParser_SEMI_COLON_token, SipParser_MARK_SIGN_token = 123,
  SipParser_SLASH_token = 125, SipParser_LEFT_PARENTHESIS_token = 127,
  SipParser_LEFT_ANGLE_BRACKET_token = 129,
  SipParser_RIGHT_ANGLE_BRACKET_token = 131,
  SipParser_LEFT_QUOTATION_token = 133, SipParser_HYPHEN_token = 135,
  SipParser_TOKEN_token = 137, SipParser_TOKEN_CHAR_token,
  SipParser_CALL_ID_WORD_token = 140, SipParser_CALL_ID_WORD_CHAR_token,
  SipParser_TOKEN_FOR_CONTENT_TYPE_token = 143,
  SipParser_TOKEN_CHAR_FOR_CONTENT_TYPE_token,
  SipParser_TOKEN_NO_DOT_token = 146, SipParser_TOKEN_CHAR_NO_DOT_token,
  SipParser_FIELD_VALUE_token = 149, SipParser_QUOTED_STRING_token = 155,
  SipParser_QUOTED_CHAR_token, SipParser_HEX_VALUE_token = 163,
  SipParser_HEX_token, SipParser_INT_NUMBER_token = 166,
  SipParser_DIGIT_token, SipParser_TOKEN_OR_QUOTED_STRING_token = 169,
  SipParser_METHOD_token, SipParser_CSEQ_VAL_token = 180,
  SipParser_SEQUNCE_NUMBER_token, SipParser_CONTENT_LENGTH_VAL_token,
  SipParser_CALL_ID_token, SipParser_MEDIA_TYPE_token,
  SipParser_MEDIA_TYPE_TYPE_token, SipParser_MEDIA_TYPE_SUB_TYPE_token,
  SipParser_OPTIONAL_CONTENT_TYPE_PARAM_LIST_token,
  SipParser_CONTENT_TYPE_PARAM_LIST_token, SipParser_BOUNDARY_PARAM_token,
  SipParser_VERSION_PARAM_token, SipParser_BASE_PARAM_token,
  SipParser_START_PARAM_token, SipParser_TYPE_PARAM_token,
  SipParser_CONTENT_TYPE_GENERIC_PARAM_DATA_token,
  SipParser_START_PARAM_NAME_token = 219,
  SipParser_OLD_OR_NEW_ADDR_SPEC_token,
  SipParser_NEW_ADDR_SPEC_PREFIX_token = 224, SipParser_ADDR_SPEC_token,
  SipParser_OLD_ADDR_SPEC_PREFIX_token, SipParser_USER_INFO_TOKEN_token,
  SipParser_HOST_NAME_token, SipParser_TYPE_PARAM_MEDIA_token = 232,
  SipParser_CONTENT_TYPE_GENERIC_PARAM_DATA_VAL_token,
  SipParser_CONTENT_ID_HEADER_VALUE_token,
  SipParser_CONTENT_ID_HEADER_LEFT_BRACKET_token,
  SipParser_CONTENT_ID_OLD_OR_NEW_ADDR_SPEC_token,
  SipParser_GENERIC_PARAM_LIST_token, SipParser_GENERIC_PARAM_DATA_token,
  SipParser_GENERIC_PARAM_DATA_VAL_token, SipParser_SIP_VERSION_token,
  SipParser_STATUS_CODE_token = 242, SipParser_REASON_PHRASE_token,
  SipParser_EXTENSION_CODE_token = 245, SipParser_REASONE_TEXT_UTF8_token,
  SipParser_ALLOW_VAL_token = 250, SipParser_VIA_LIST_token,
  SipParser_SINGLE_VIA_token, SipParser_SENT_PROTOCOL_token,
  SipParser_SENT_BY_token, SipParser_VIA_PARAM_LIST_token,
  SipParser_VIA_PARAM_token, SipParser_PROTOCOL_NAME_token,
  SipParser_PROTOCOL_VERSION_token, SipParser_TRANSPORT_token,
  SipParser_TRANSPORT_TYPE_token = 261, SipParser_PORT_token = 266,
  SipParser_VIA_HIDDEN_token, SipParser_VIA_ALIAS_token,
  SipParser_VIA_TTL_token, SipParser_VIA_MADDR_token,
  SipParser_VIA_RECEIVED_token, SipParser_VIA_BRANCH_token,
  SipParser_RESPONSE_PORT_token, SipParser_VIA_COMPRESSION_token,
  SipParser_VIA_SIP_SIGCOMP_ID_token, SipParser_OTHER_PARAM_token,
  SipParser_OTHER_PARAM_EXPR_token, SipParser_IPV_6_REFERENCE_token,
  SipParser_TTL_PARAM_token = 281, SipParser_TTL_token = 283,
  SipParser_RPORT_NAME_token, SipParser_RPORT_VAL_token,
  SipParser_MADDR_PARAM_token = 287,
  SipParser_HOST_NAME_FOR_RECEIVED_token = 290,
  SipParser_COMPRESSION_TYPE_token = 293,
  SipParser_SIGCOMP_ID_TOKEN_token = 296, SipParser_IPV_6_ADDRESS_token,
  SipParser_TEST_COMMENT_token, SipParser_COMMENT_TEXT_token,
  SipParser_END_COMMENT_token = 301, SipParser_REQUEST_URI_token = 304,
  SipParser_ABSOLUTE_URI_token, SipParser_ABS_URI_SCHEME_token,
  SipParser_URI_IDENTIFIER_token, SipParser_ABS_URI_SCHEME_CHAR_token,
  SipParser_URI_SCHEME_OR_DISPLAY_NAME_token = 310,
  SipParser_DISPLAY_NAME_token, SipParser_URI_IDENTIFIER_CHAR_token,
  SipParser_URI_IDENTIFIER_WITHOUT_SEPARATORS_token = 314,
  SipParser_TEL_URI_token = 317, SipParser_TEL_PREFIX_token,
  SipParser_TEL_token, SipParser_GLOBAL_OR_LOCAL_NUMBER_token = 321,
  SipParser_OPTIONAL_TEL_URI_PARAMETERS_token,
  SipParser_GLOBAL_NUMBER_token, SipParser_LOCAL_NUMBER_token,
  SipParser_GLOBAL_NUMBER_DIGITS_token, SipParser_PHONEDIGIT_token = 327,
  SipParser_VISUAL_SEPERATOR_token = 331,
  SipParser_LOCAL_NUMBER_DIGITS_token = 333,
  SipParser_HEX_OR_PHONECHAR_token, SipParser_PHONEDIGIT_HEX_token,
  SipParser_PHONECHAR_token = 339, SipParser_TEL_URI_PARAMETERS_token,
  SipParser_TEL_URI_PARAMETER_token,
  SipParser_TEL_URI_EXTENSION_PARAM_token,
  SipParser_TEL_URI_POST_DIAL_PARAM_token,
  SipParser_TEL_URI_ISDN_SUB_ADDR_PARAM_URL_token,
  SipParser_TEL_URI_CONTEXT_PARAM_token,
  SipParser_TEL_URI_ENUMDI_PARAM_token,
  SipParser_URL_GENERIC_PARAM_DATA_token, SipParser_CPC_PARAM_URL_token,
  SipParser_TEL_URI_RN_PARAM_token,
  SipParser_TEL_URI_RN_CONTEXT_PARAM_token,
  SipParser_TEL_URI_CIC_PARAM_token,
  SipParser_TEL_URI_CIC_CONTEXT_PARAM_token,
  SipParser_TEL_URI_NPDI_PARAM_token,
  SipParser_TEL_URI_EXTENTION_VALUE_token = 355,
  SipParser_TEL_URI_POST_DIAL_VALUE_token = 358,
  SipParser_TEL_URI_POST_DIAL_CHAR_token, SipParser_DTMFDIGIT_token = 361,
  SipParser_PAUSE_CHAR_token,
  SipParser_TEL_URI_ISDN_SUB_ADDR_VALUE_token = 364, SipParser_URIC_token,
  SipParser_RESERVED_FOR_URIC_token = 367, SipParser_UNRESERVED_token,
  SipParser_PCT_ENCODED_token, SipParser_TEL_URI_CONTEXT_VALUE_token = 372,
  SipParser_DESCRIPTOR_token, SipParser_DOMAINNAME_token,
  SipParser_LABEL_token, SipParser_NEXT_LABEL_token,
  SipParser_ALPHA_token = 380, SipParser_ALPHANUM_OR_HYPHEN_token,
  SipParser_ALPHANUM_token = 385,
  SipParser_TEL_URI_NP_GLOBAL_OR_LOCAL_NUMBER_token = 388,
  SipParser_DIAMETER_URI_token = 393, SipParser_DIAMETER_PREFIX_token,
  SipParser_DIAMETER_HOSTPORT_token, SipParser_DIAMETER_PARAMS_token,
  SipParser_DIAMETER_PARAM_token = 399, SipParser_TRANSPORT_PARAM_token,
  SipParser_DIAMETER_PROTOCOL_PARAM_token = 402,
  SipParser_SIP_URL_token = 406, SipParser_PRES_URI_token,
  SipParser_IM_URI_token, SipParser_SIP_PREFIX_token, SipParser_URL_token,
  SipParser_PRES_PREFIX_token = 413, SipParser_RV_ABS_PREFIX_token,
  SipParser_IM_PREFIX_token = 417, SipParser_URL_USERINFO_token = 419,
  SipParser_OPTIONAL_URL_PARAMETERS_token, SipParser_OPTIONAL_HEADERS_token,
  SipParser_URL_HOST_token, SipParser_GENERAL_URL_WITHOUT_PARAM_token,
  SipParser_SIP_URL_WITHOUT_PARAMS_token,
  SipParser_TEL_URL_WITHOUT_PARAMS_token,
  SipParser_DIAMETER_URI_WITHOUT_PARAMS_token,
  SipParser_PRES_URL_WITHOUT_PARAMS_token,
  SipParser_IM_URL_WITHOUT_PARAMS_token,
  SipParser_ABS_URI_WITHOUT_PARAMS_token,
  SipParser_URL_USERINFO_PREFIX_token, SipParser_URL_USERINFO_VAL_token,
  SipParser_HOSTPORT_token, SipParser_PASSWORD_token = 434,
  SipParser_TOKEN_USER_INFO_token, SipParser_TOKEN_USER_INFO_CHAR_token,
  SipParser_PASSWORD_STRING_token = 438,
  SipParser_PASSWORD_STRING_CHAR_token,
  SipParser_URL_HOST_PREFIX_token = 443, SipParser_TOKEN_HOST_token = 445,
  SipParser_TOKEN_HOST_CHAR_token, SipParser_IPV_4_ADDRESS_token = 450,
  SipParser_IPV_4_PART_token, SipParser_HEXPART_token,
  SipParser_HEXSEQ_token, SipParser_HEX4_token = 456,
  SipParser_URL_PARAMETERS_token = 460, SipParser_URL_PARAMETER_token,
  SipParser_USER_PARAM_token, SipParser_TTL_PARAM_URL_token,
  SipParser_MADDR_PARAM_URL_token, SipParser_METHOD_PARAM_URL_token,
  SipParser_LR_PARAM_URL_token, SipParser_COMPRESSION_PARAM_URL_token,
  SipParser_SIGCOMPID_PARAM_URL_token,
  SipParser_TOKENIZED_BY_PARAM_URL_token, SipParser_ORIG_PARAM_URL_token,
  SipParser_GR_PARAM_URL_token, SipParser_USER_PARAM_TYPE_token = 474,
  SipParser_OTHER_USER_token, SipParser_PARAMCHAR_token = 485,
  SipParser_CPC_PARAM_URL_VALUE_token = 492,
  SipParser_URL_OTHER_PARAM_VAL_token = 494, SipParser_PVALUE_token = 505,
  SipParser_HEADERS_LIST_token, SipParser_HEADER_token,
  SipParser_HNAME_token = 509, SipParser_HVALUE_token,
  SipParser_REQUEST_LINE_VALUE_token = 517, SipParser_REQUEST_METHOD_token,
  SipParser_PARTY_VALUE_token = 532, SipParser_NAME_ADDR_token,
  SipParser_PARTY_PARAMS_LIST_token, SipParser_MORE_THAN_ONE_TOKEN_token,
  SipParser_TAG_PARAM_token = 539, SipParser_PARTY_PARAM_DATA_token = 541,
  SipParser_CONTACT_HEADER_VALUES_token,
  SipParser_CONTACT_HEADER_OTHER_token,
  SipParser_CONTACT_HEADER_VALUE_LIST_token,
  SipParser_CONTACT_HEADER_VALUE_token = 546,
  SipParser_CONTACT_PARAMS_token, SipParser_CONTACT_PARAM_token,
  SipParser_Q_token, SipParser_ACTION_token,
  SipParser_CONTACT_EXPIRES_token, SipParser_TEMP_GRUU_token,
  SipParser_PUB_GRUU_token, SipParser_REG_ID_token,
  SipParser_CONTACT_FEATURE_TAG_AUDIO_token,
  SipParser_CONTACT_FEATURE_TAG_AUTOMATA_token,
  SipParser_CONTACT_FEATURE_TAG_CLASS_token,
  SipParser_CONTACT_FEATURE_TAG_DUPLEX_token,
  SipParser_CONTACT_FEATURE_TAG_DATA_token,
  SipParser_CONTACT_FEATURE_TAG_CONTROL_token,
  SipParser_CONTACT_FEATURE_TAG_MOBILITY_token,
  SipParser_CONTACT_FEATURE_TAG_DESCRIPTION_token,
  SipParser_CONTACT_FEATURE_TAG_EVENTS_token,
  SipParser_CONTACT_FEATURE_TAG_PRIORITY_token,
  SipParser_CONTACT_FEATURE_TAG_METHODS_token,
  SipParser_CONTACT_FEATURE_TAG_SCHEMES_token,
  SipParser_CONTACT_FEATURE_TAG_APPLICATION_token,
  SipParser_CONTACT_FEATURE_TAG_VIDEO_token,
  SipParser_CONTACT_FEATURE_TAG_LANGUAGE_token,
  SipParser_CONTACT_FEATURE_TAG_TYPE_token,
  SipParser_CONTACT_FEATURE_TAG_ISFOCUS_token,
  SipParser_CONTACT_FEATURE_TAG_ACTOR_token,
  SipParser_CONTACT_FEATURE_TAG_TEXT_token,
  SipParser_CONTACT_FEATURE_TAG_EXTENSIONS_token,
  SipParser_CONTACT_FEATURE_TAG_SIP_INSTANCE_token,
  SipParser_Q_VALUE_token = 577, SipParser_Q_VALUE_OPTIONAL_token = 579,
  SipParser_ACTION_VALUE_token = 583, SipParser_GRUU_PARAM_VAL_token = 587,
  SipParser_FEATURE_TAG_STRING_token = 591,
  SipParser_EXPIRES_VALUE_token = 608,
  SipParser_EXPIRES_VALUE_FOR_CONTACT_token = 610,
  SipParser_DELTA_SECONDS_token, SipParser_SIP_DATE_token,
  SipParser_SIP_DATE_HEADER_VAL_token, SipParser_DATE_token,
  SipParser_TIME_token, SipParser_DAY_token, SipParser_DD_token = 624,
  SipParser_MM_token, SipParser_YY_token, SipParser_HOUR_token = 639,
  SipParser_MINUTE_token, SipParser_SECOND_token, SipParser_TIME_ZONE_token,
  SipParser_AUTHENTICATION_VAL_token = 646, SipParser_CHALLENGE_VALUE_token,
  SipParser_DIGEST_CHALLENGE_LIST_token, SipParser_AUTH_PARAM_LIST_token,
  SipParser_DIGEST_CHALLENGE_token, SipParser_REALM_token,
  SipParser_DOMAIN_token, SipParser_NONCE_token, SipParser_OPAQUE_token,
  SipParser_INTEGRITY_KEY_token, SipParser_CIPHER_KEY_token,
  SipParser_AUTH_PARAM_token, SipParser_STALE_token,
  SipParser_ALGORITHM_token, SipParser_QOP_OPTIONS_token,
  SipParser_PARAM_IN_NEXT_CHALLENGE_token,
  SipParser_REALM_VALUE_token = 663, SipParser_DOMAIN_EQUAL_token = 665,
  SipParser_DOMAIN_WHITE_token, SipParser_DOMAIN_VAL_token = 668,
  SipParser_URI_LIST_token, SipParser_STRING_DOMAIN_PATCH_token,
  SipParser_TOKEN_FOR_DOMAIN_token, SipParser_TOKEN_CHAR_FOR_DOMAIN_token,
  SipParser_DOMAIN_SPACE_token = 674, SipParser_NONCE_VALUE_token = 677,
  SipParser_OPAQUE_VALUE_token = 679,
  SipParser_ALGORITHM_DECLARATION_token = 682,
  SipParser_ALGORITHM_AND_AKAV_token, SipParser_AKAV_PREFIX_token = 685,
  SipParser_AKAV_token, SipParser_AKA_VERSION_VAL_token,
  SipParser_ALGORITHM_VAL_token, SipParser_ALGORITHM_PREFIX_token,
  SipParser_QOP_VALUE_LIST_token = 697, SipParser_QOP_VALUE_token,
  SipParser_QUOTED_HEX_VALUE_token = 702,
  SipParser_AUTH_PARAM_DATA_token = 705, SipParser_AUTHORIZATION_VAL_token,
  SipParser_CREDENTIALS_VALUE_token, SipParser_DIGEST_RESPONSE_LIST_token,
  SipParser_DIGEST_RESPONSE_token, SipParser_USERNAME_token,
  SipParser_RESPONSE_token, SipParser_CNONCE_token,
  SipParser_NONCE_COUNT_token, SipParser_AUTS_PARAM_token,
  SipParser_INTEGRITY_PROTECTED_token, SipParser_MESSAGE_QOP_token,
  SipParser_DIGEST_URI_token, SipParser_USERNAME_VALUE_token = 719,
  SipParser_DIGEST_URI_VALUE_token = 721,
  SipParser_LEFT_URI_QUOTATION_token, SipParser_CNONCE_VALUE_token = 724,
  SipParser_NC_VALUE_token = 726, SipParser_LHEX_token,
  SipParser_REQUEST_DIGEST_token = 729, SipParser_THIRTY_TWO_LHEX_token,
  SipParser_SIXTY_FOUR_BASE_ENCODING_token = 734,
  SipParser_SIXTY_FOUR_BASE_ENCODING_CHAR_token,
  SipParser_ONE_OR_TWO_EQUAL_SIGNS_token = 737,
  SipParser_ROUTE_LIST_token = 744, SipParser_SINGLE_ROUTE_token,
  SipParser_RETRY_AFTER_VAL_token, SipParser_RETRY_AFTER_COMMENT_token,
  SipParser_RETRY_PARAM_token = 749, SipParser_DURATION_PARAM_token,
  SipParser_RSEQ_VAL_token = 752, SipParser_RESPONSE_NUM_token,
  SipParser_RACK_VAL_token, SipParser_OPTION_TAG_token,
  SipParser_OPTION_TAG_LIST_token, SipParser_SUPPORTED_OPTION_TAG_token,
  SipParser_REFER_TO_HEADER_VALUE_token,
  SipParser_REFERRED_BY_HEADER_VALUE_token = 760,
  SipParser_REFERRER_URI_token, SipParser_REFERRED_BY_PARAMS_token,
  SipParser_REFERRED_BY_PARAM_token = 764,
  SipParser_REFERRED_BY_PARAM_CID_token,
  SipParser_CID_PARAM_STRING_token = 767, SipParser_DOT_ATOM_token,
  SipParser_ATOM_CHAR_token, SipParser_DOT_ATOM_CHAR_token,
  SipParser_EVENT_HEADER_VAL_token = 773, SipParser_EVENT_TYPE_token,
  SipParser_EVENT_PARAMS_token, SipParser_EVENT_PACKAGE_token,
  SipParser_EVENT_TEMPLATE_token, SipParser_EVENT_PARAM_token,
  SipParser_EVENT_ID_token, SipParser_ALLOW_EVENTS_HEADER_VAL_token = 781,
  SipParser_SUBSCRIPTION_STATE_HEADER_VAL_token,
  SipParser_SUBSTATE_VALUE_token, SipParser_SUBSTATE_PARAMS_token,
  SipParser_SUBSTATE_PARAM_token = 788,
  SipParser_SUBSTATE_PARAM_REASON_token,
  SipParser_SUBSTATE_PARAM_RETRY_AFTER_token,
  SipParser_SUBSTATE_PARAM_EXPIRES_token,
  SipParser_SUBSTATE_PARAM_REASON_VAL_token = 794,
  SipParser_DISPOSITION_VAL_token = 801, SipParser_DISPOSITION_TYPE_token,
  SipParser_DISPOSITION_PARAMS_token, SipParser_HANDLING_PARAM_token = 809,
  SipParser_SESSION_EXPIRES_VALUE_token = 813,
  SipParser_REFRESHER_TYPE_token, SipParser_REFRESHER_TYPE_NAME_token,
  SipParser_REFRESHER_TYPE_VALUE_token, SipParser_MIN_SE_VALUE_token = 820,
  SipParser_REPLACES_HEADER_VALUE_token, SipParser_REPLACES_PARAMS_token,
  SipParser_REPLACES_PARAM_token, SipParser_TO_TAG_PARAM_token,
  SipParser_FROM_TAG_PARAM_token, SipParser_EARLY_FLAG_PARAM_token,
  SipParser_AUTHENTICATION_INFO_HEADER_LIST_token = 830,
  SipParser_AUTHENTICATION_INFO_HEADER_PARAM_token,
  SipParser_NEXT_NONCE_token, SipParser_RESPONSE_AUTH_token,
  SipParser_MSG_QOP_token, SipParser_RESPONSE_AUTH_VAL_token = 837,
  SipParser_P_URI_HEADERS_LIST_token = 841,
  SipParser_P_URI_HEADER_VALUE_token, SipParser_P_URI_HEADER_PARAMS_token,
  SipParser_P_VISITED_NETWORK_ID_HEADERS_LIST_token,
  SipParser_P_VISITED_NETWORK_ID_HEADER_VALUE_token,
  SipParser_VNETWORK_SPEC_token,
  SipParser_P_VISITED_NETWORK_ID_PARAMS_token,
  SipParser_P_ACCESS_NETWORK_INFO_HEADER_VALUE_token,
  SipParser_ACCESS_TYPE_token, SipParser_P_ACCESS_NETWORK_INFO_PARAMS_token,
  SipParser_ACCESS_token, SipParser_ACCESS_PARAM_token = 873,
  SipParser_NETWORK_PROVIDED_token, SipParser_CGI_3GPP_token,
  SipParser_UTRAN_CELL_ID_3GPP_token, SipParser_I_WLAN_NODE_ID_token,
  SipParser_DSL_LOCATION_token, SipParser_CI_3GPP2_token,
  SipParser_GEN_VALUE_OTHER_PARAM_EXPR_token,
  SipParser_P_CHARGING_FUNCTION_ADDRESSES_HEADER_VALUE_token = 887,
  SipParser_CHARGE_ADDR_PARAM_token, SipParser_CHARGE_ADDR_PARAMS_token,
  SipParser_CHARGE_ADDR_PARAM_CCF_token,
  SipParser_CHARGE_ADDR_PARAM_ECF_token,
  SipParser_GENERAL_VALUE_token = 893,
  SipParser_P_CHARGING_VECTOR_HEADER_VALUE_token = 895,
  SipParser_ICID_VALUE_token, SipParser_CHARGE_PARAMS_token,
  SipParser_CHARGE_PARAM_token = 899,
  SipParser_CHARGE_PARAM_ICID_GEN_ADDR_token,
  SipParser_CHARGE_PARAM_ORIG_IOI_token,
  SipParser_CHARGE_PARAM_TERM_IOI_token,
  SipParser_CHARGE_PARAM_GPRS_CHARGING_INFO_token,
  SipParser_CHARGE_PARAM_XDSL_CHARGING_INFO_token,
  SipParser_PDP_INFO_HIERARCHY_token,
  SipParser_DSL_BEARER_INFO_HIERARCHY_token,
  SipParser_I_WLAN_CHARGING_INFO_token,
  SipParser_PACKETCABLE_CHARGING_INFO_token,
  SipParser_PACKETCABLE_CHARGING_INFO_BCID_token,
  SipParser_GGSN_token = 913, SipParser_AUTH_TOKEN_token,
  SipParser_BRAS_token, SipParser_PDP_INFO_token = 920,
  SipParser_DSL_BEARER_INFO_token = 922, SipParser_SINGLE_PDP_INFO_token,
  SipParser_SINGLE_DSL_BEARER_INFO_token, SipParser_PDP_ITEM_token,
  SipParser_PDP_SIG_token, SipParser_PDP_CID_token, SipParser_FLOW_ID_token,
  SipParser_YES_OR_NO_token = 931, SipParser_DSL_BEARER_ITEM_token = 933,
  SipParser_DSL_BEARER_SIG_token, SipParser_DSL_BEARER_CID_token,
  SipParser_LPAREN_token = 940, SipParser_FLOW_ID_VALUE_token,
  SipParser_FLOW_ID_SYNTAX1_token, SipParser_FLOW_ID_SYNTAX2_token,
  SipParser_DIGITS_token = 947, SipParser_WHITE_IN_WORD_token,
  SipParser_PACKETCABLE_CHARGING_INFO_BCID_TOKEN_token = 954,
  SipParser_P_MEDIA_AUTHORIZATION_VAL_token = 956,
  SipParser_P_MEDIA_AUTHORIZATION_TOKEN_token,
  SipParser_SECURITY_HEADERS_LIST_token,
  SipParser_SECURITY_HEADER_VALUE_token, SipParser_MECHANISM_NAME_token,
  SipParser_MECHANISM_PARAMS_token, SipParser_MECHANISM_token,
  SipParser_MECHANISM_PARAM_token = 966, SipParser_DIGEST_ALGORITHM_token,
  SipParser_DIGEST_QOP_token, SipParser_DIGEST_VERIFY_token,
  SipParser_SECURITY_ALGORITHM_token, SipParser_SECURITY_PROTOCOL_token,
  SipParser_SECURITY_MODE_token, SipParser_SECURITY_ENCRYPT_ALGORITHM_token,
  SipParser_SECURITY_SPIC_token, SipParser_SECURITY_SPIS_token,
  SipParser_SECURITY_PORTC_token, SipParser_SECURITY_PORTS_token,
  SipParser_DIGEST_ALGORITHM_DECLARATION_token,
  SipParser_DIGEST_VERIFY_VALUE_token = 982,
  SipParser_SECURITY_ALGORITHM_VAL_token = 984,
  SipParser_SECURITY_PROTOCOL_VAL_token = 988,
  SipParser_SECURITY_MODE_VAL_token = 992,
  SipParser_SECURITY_ENCRYPT_ALGORITHM_VAL_token = 997,
  SipParser_TEN_DIGIT_token = 1002,
  SipParser_P_PROFILE_KEY_HEADERS_LIST_token = 1006,
  SipParser_P_PROFILE_KEY_HEADER_VALUE_token,
  SipParser_P_PROFILE_KEY_HEADER_PARAMS_token,
  SipParser_P_USER_DATABASE_HEADERS_LIST_token,
  SipParser_P_USER_DATABASE_HEADER_VALUE_token,
  SipParser_P_ANSWER_STATE_HEADER_VALUE_token, SipParser_ANSWER_TYPE_token,
  SipParser_P_ANSWER_STATE_PARAMS_token, SipParser_ANSWER_token,
  SipParser_ANSWER_PARAM_token = 1017,
  SipParser_P_SERVED_USER_HEADERS_LIST_token,
  SipParser_P_SERVED_USER_HEADER_VALUE_token,
  SipParser_P_SERVED_USER_HEADER_PARAMS_token,
  SipParser_P_SERVED_USER_HEADER_PARAM_token,
  SipParser_P_SERVED_USER_SESSION_CASE_token,
  SipParser_P_SERVED_USER_REGISTRATION_STATE_token,
  SipParser_P_SERVED_USER_SESSION_CASE_VAL_token = 1025,
  SipParser_P_SERVED_USER_REGISTRATION_STATE_VAL_token = 1028,
  SipParser_P_DCS_TRACE_PARTY_ID_HEADER_VALUE_token = 1031,
  SipParser_P_DCS_OSPS_HEADER_VALUE_token, SipParser_OSPS_TAG_token,
  SipParser_P_DCS_BILLING_INFO_HEADER_VALUE_token = 1037,
  SipParser_BILLING_CORRELATION_ID_token, SipParser_FEID_token,
  SipParser_FEID_HOST_token, SipParser_BILLING_INFO_PARAMS_token,
  SipParser_BILLING_CORRELATION_ID_VALUE_token,
  SipParser_FORTY_EIGHT_HEXDIG_token, SipParser_FEID_VALUE_token,
  SipParser_SIXTEEN_HEXDIG_token, SipParser_BILLING_INFO_PARAM_token,
  SipParser_P_DCS_LAES_HEADER_VALUE_token = 1053, SipParser_LAES_SIG_token,
  SipParser_LAES_PARAMS_token, SipParser_LAES_PARAM_token,
  SipParser_LAES_CONTENT_token = 1058,
  SipParser_P_DCS_REDIRECT_HEADER_VALUE_token = 1060,
  SipParser_CALLED_ID_URI_token, SipParser_REDIRECT_PARAMS_token,
  SipParser_REDIRECT_PARAM_token
} SipParser_token_type;

typedef struct SipParser_pcb_struct{
  SipParser_token_type token_number, reduction_token, error_frame_token;
  int input_code;
  int input_value;
  int line, column;
  int ssx, sn, error_frame_ssx;
  int drt, dssx, dsn;
  int ss[49];
  SipParser_vs_type vs[49];
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



        ParserDiameterUri       diameterUri;    

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












		ParserPUriHeader				puriHeader; 
		ParserPVisitedNetworkIDHeader	pvisitedNetworkIDHeader; 
		ParserPAccessNetworkInfoHeader	paccessNetworkInfoHeader; 
		ParserPChargingFunctionAddressesHeader	pchargingFunctionAddressesHeader; 
		ParserPChargingVectorHeader		pchargingVectorHeader; 
		ParserPMediaAuthorizationHeader	pmediaAuthorizationHeader; 
		ParserSecurityHeader			securityHeader; 
		ParserPProfileKeyHeader	        pprofileKeyHeader; 
		ParserPUserDatabaseHeader       puserDatabaseHeader; 
		ParserPAnswerStateHeader        panswerStateHeader; 
		ParserPServedUserHeader         pservedUserHeader; 


		ParserPDCSTracePartyIDHeader	pdcsTracePartyIDHeader; 
		ParserPDCSOSPSHeader			pdcsOSPSHeader; 
		ParserPDCSBillingInfoHeader		pdcsBillingInfoHeader; 
		ParserPDCSLAESHeader			pdcsLAESHeader; 
		ParserPDCSRedirectHeader		pdcsRedirectHeader; 


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

#elif defined(RV_SIP_AUTH_ON) && defined(RV_SIP_TEL_URI_SUPPORT) && defined(RV_SIP_IMS_HEADER_SUPPORT) && !defined(RV_SIP_IMS_DCS_HEADER_SUPPORT) && !defined(RV_SIP_EXTENDED_HEADER_SUPPORT) && !defined(RV_SIP_JSR32_SUPPORT) && !defined(RV_SIP_LIGHT) && !defined(RV_SIP_PRIMITIVES) && !defined(RV_SIP_SUBS_ON)
#ifndef PARSERENGINEIMS_H_1248033633
#define PARSERENGINEIMS_H_1248033633

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
  char ag_vt_37[sizeof(RvBool)];
  char ag_vt_38[sizeof(RvSipDiameterProtocol)];
  char ag_vt_39[sizeof(ParserUserParam)];
  char ag_vt_40[sizeof(RvSipUserParam)];
  char ag_vt_41[sizeof(ParserOtherUser)];
  char ag_vt_42[sizeof(ParserLrParamType)];
  char ag_vt_43[sizeof(ParserTokenizedByParam)];
  char ag_vt_44[sizeof(ParserCPCParam)];
  char ag_vt_45[sizeof(ParserGrParam)];
  char ag_vt_46[sizeof(ParserOptionalHeaders)];
  char ag_vt_47[sizeof(ParserTagParam)];
  char ag_vt_48[sizeof(ParserPartyParams)];
  char ag_vt_49[sizeof(ParserQVal)];
  char ag_vt_50[sizeof(ParserContactActionType)];
  char ag_vt_51[sizeof(ParserContactFeatureTag)];
  char ag_vt_52[sizeof(ParserDeltaSeconds)];
  char ag_vt_53[sizeof(ParserSipDate)];
  char ag_vt_54[sizeof(RvSipDateWeekDay)];
  char ag_vt_55[sizeof(ParserDDMMYY)];
  char ag_vt_56[sizeof(ParserInt32)];
  char ag_vt_57[sizeof(RvSipDateMonth)];
  char ag_vt_58[sizeof(ParserTime)];
  char ag_vt_59[sizeof(ParserTimeZone)];
  char ag_vt_60[sizeof(RvSipAuthStale)];
  char ag_vt_61[sizeof(ParserAlgorithm)];
  char ag_vt_62[sizeof(ParserQopValue)];
  char ag_vt_63[sizeof(ParserAuthQopOptions)];
  char ag_vt_64[sizeof(RvSipAuthQopOption)];
  char ag_vt_65[sizeof(RvSipAuthIntegrityProtected)];
  char ag_vt_66[sizeof(ParserRetryParam)];
  char ag_vt_67[sizeof(ParserRSeq)];
  char ag_vt_68[sizeof(ParserResponseNum)];
  char ag_vt_69[sizeof(ParserEventType)];
  char ag_vt_70[sizeof(ParserDisposition)];
  char ag_vt_71[sizeof(ParserDispositionParams)];
  char ag_vt_72[sizeof(ParserDispositionParam)];
  char ag_vt_73[sizeof(ParserSessionExpiresHeader)];
  char ag_vt_74[sizeof(RvSipSessionExpiresRefresherType)];
  char ag_vt_75[sizeof(ParserMinSEHeader)];
  char ag_vt_76[sizeof(ParserReplacesEarlyFlagParamType)];
  char ag_vt_77[sizeof(ParserAccess)];
  char ag_vt_78[sizeof(ParserMechanism)];
  char ag_vt_79[sizeof(ParserSecurityAlgorithmType)];
  char ag_vt_80[sizeof(ParserSecurityProtocolType)];
  char ag_vt_81[sizeof(ParserSecurityModeType)];
  char ag_vt_82[sizeof(ParserSecurityEncryptAlgorithmType)];
  char ag_vt_83[sizeof(ParserAnswer)];
  char ag_vt_84[sizeof(ParserSessionCaseType)];
  char ag_vt_85[sizeof(ParserRegistrationStateType)];
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
  SipParser_TEL_URI_HEADER_token, SipParser_DIAMETER_URI_HEADER_token,
  SipParser_P_URI_HEADER_token = 64,
  SipParser_P_VISITED_NETWORK_ID_HEADER_token = 66,
  SipParser_P_ACCESS_NETWORK_INFO_HEADER_token = 68,
  SipParser_P_CHARGING_FUNCTION_ADDRESSES_HEADER_token = 70,
  SipParser_P_CHARGING_VECTOR_HEADER_token = 72,
  SipParser_P_MEDIA_AUTHORIZATION_HEADER_token = 74,
  SipParser_SECURITY_HEADER_token = 76,
  SipParser_P_PROFILE_KEY_HEADER_token = 78,
  SipParser_P_USER_DATABASE_HEADER_token = 80,
  SipParser_P_ANSWER_STATE_HEADER_token = 82,
  SipParser_P_SERVED_USER_HEADER_token = 84, SipParser_OTHER_HEADER_token,
  SipParser_WHITE_token = 87, SipParser_SP_token = 89,
  SipParser_COMMA_token = 91, SipParser_COMMA_BETWEEN_HEADERS_token = 93,
  SipParser_DOT_token, SipParser_EQUAL_token = 98, SipParser_AT_token = 100,
  SipParser_COLON_token = 102, SipParser_SEMI_COLON_token,
  SipParser_MARK_SIGN_token = 105, SipParser_SLASH_token = 107,
  SipParser_LEFT_PARENTHESIS_token = 109,
  SipParser_LEFT_ANGLE_BRACKET_token = 111,
  SipParser_RIGHT_ANGLE_BRACKET_token = 113,
  SipParser_LEFT_QUOTATION_token = 115, SipParser_HYPHEN_token = 117,
  SipParser_TOKEN_token = 119, SipParser_TOKEN_CHAR_token,
  SipParser_CALL_ID_WORD_token = 122, SipParser_CALL_ID_WORD_CHAR_token,
  SipParser_TOKEN_FOR_CONTENT_TYPE_token = 125,
  SipParser_TOKEN_CHAR_FOR_CONTENT_TYPE_token,
  SipParser_TOKEN_NO_DOT_token = 128, SipParser_TOKEN_CHAR_NO_DOT_token,
  SipParser_FIELD_VALUE_token = 131, SipParser_QUOTED_STRING_token = 137,
  SipParser_QUOTED_CHAR_token, SipParser_HEX_VALUE_token = 145,
  SipParser_HEX_token, SipParser_INT_NUMBER_token = 148,
  SipParser_DIGIT_token, SipParser_TOKEN_OR_QUOTED_STRING_token = 151,
  SipParser_METHOD_token, SipParser_CSEQ_VAL_token = 159,
  SipParser_SEQUNCE_NUMBER_token, SipParser_CONTENT_LENGTH_VAL_token,
  SipParser_CALL_ID_token, SipParser_MEDIA_TYPE_token,
  SipParser_MEDIA_TYPE_TYPE_token, SipParser_MEDIA_TYPE_SUB_TYPE_token,
  SipParser_OPTIONAL_CONTENT_TYPE_PARAM_LIST_token,
  SipParser_CONTENT_TYPE_PARAM_LIST_token, SipParser_BOUNDARY_PARAM_token,
  SipParser_VERSION_PARAM_token, SipParser_BASE_PARAM_token,
  SipParser_START_PARAM_token, SipParser_TYPE_PARAM_token,
  SipParser_CONTENT_TYPE_GENERIC_PARAM_DATA_token,
  SipParser_START_PARAM_NAME_token = 198,
  SipParser_OLD_OR_NEW_ADDR_SPEC_token,
  SipParser_NEW_ADDR_SPEC_PREFIX_token = 203, SipParser_ADDR_SPEC_token,
  SipParser_OLD_ADDR_SPEC_PREFIX_token, SipParser_USER_INFO_TOKEN_token,
  SipParser_HOST_NAME_token, SipParser_TYPE_PARAM_MEDIA_token = 211,
  SipParser_CONTENT_TYPE_GENERIC_PARAM_DATA_VAL_token,
  SipParser_CONTENT_ID_HEADER_VALUE_token,
  SipParser_CONTENT_ID_HEADER_LEFT_BRACKET_token,
  SipParser_CONTENT_ID_OLD_OR_NEW_ADDR_SPEC_token,
  SipParser_GENERIC_PARAM_LIST_token, SipParser_GENERIC_PARAM_DATA_token,
  SipParser_GENERIC_PARAM_DATA_VAL_token, SipParser_SIP_VERSION_token,
  SipParser_STATUS_CODE_token = 221, SipParser_REASON_PHRASE_token,
  SipParser_EXTENSION_CODE_token = 224, SipParser_REASONE_TEXT_UTF8_token,
  SipParser_ALLOW_VAL_token = 229, SipParser_VIA_LIST_token,
  SipParser_SINGLE_VIA_token, SipParser_SENT_PROTOCOL_token,
  SipParser_SENT_BY_token, SipParser_VIA_PARAM_LIST_token,
  SipParser_VIA_PARAM_token, SipParser_PROTOCOL_NAME_token,
  SipParser_PROTOCOL_VERSION_token, SipParser_TRANSPORT_token,
  SipParser_TRANSPORT_TYPE_token = 240, SipParser_PORT_token = 245,
  SipParser_VIA_HIDDEN_token, SipParser_VIA_ALIAS_token,
  SipParser_VIA_TTL_token, SipParser_VIA_MADDR_token,
  SipParser_VIA_RECEIVED_token, SipParser_VIA_BRANCH_token,
  SipParser_RESPONSE_PORT_token, SipParser_VIA_COMPRESSION_token,
  SipParser_VIA_SIP_SIGCOMP_ID_token, SipParser_OTHER_PARAM_token,
  SipParser_OTHER_PARAM_EXPR_token, SipParser_IPV_6_REFERENCE_token,
  SipParser_TTL_PARAM_token = 260, SipParser_TTL_token = 262,
  SipParser_RPORT_NAME_token, SipParser_RPORT_VAL_token,
  SipParser_MADDR_PARAM_token = 266,
  SipParser_HOST_NAME_FOR_RECEIVED_token = 269,
  SipParser_COMPRESSION_TYPE_token = 272,
  SipParser_SIGCOMP_ID_TOKEN_token = 275, SipParser_IPV_6_ADDRESS_token,
  SipParser_TEST_COMMENT_token, SipParser_COMMENT_TEXT_token,
  SipParser_END_COMMENT_token = 280, SipParser_REQUEST_URI_token = 283,
  SipParser_ABSOLUTE_URI_token, SipParser_ABS_URI_SCHEME_token,
  SipParser_URI_IDENTIFIER_token, SipParser_ABS_URI_SCHEME_CHAR_token,
  SipParser_URI_SCHEME_OR_DISPLAY_NAME_token = 289,
  SipParser_DISPLAY_NAME_token, SipParser_URI_IDENTIFIER_CHAR_token,
  SipParser_URI_IDENTIFIER_WITHOUT_SEPARATORS_token = 293,
  SipParser_TEL_URI_token = 296, SipParser_TEL_PREFIX_token,
  SipParser_TEL_token, SipParser_GLOBAL_OR_LOCAL_NUMBER_token = 300,
  SipParser_OPTIONAL_TEL_URI_PARAMETERS_token,
  SipParser_GLOBAL_NUMBER_token, SipParser_LOCAL_NUMBER_token,
  SipParser_GLOBAL_NUMBER_DIGITS_token, SipParser_PHONEDIGIT_token = 306,
  SipParser_VISUAL_SEPERATOR_token = 310,
  SipParser_LOCAL_NUMBER_DIGITS_token = 312,
  SipParser_HEX_OR_PHONECHAR_token, SipParser_PHONEDIGIT_HEX_token,
  SipParser_PHONECHAR_token = 318, SipParser_TEL_URI_PARAMETERS_token,
  SipParser_TEL_URI_PARAMETER_token,
  SipParser_TEL_URI_EXTENSION_PARAM_token,
  SipParser_TEL_URI_POST_DIAL_PARAM_token,
  SipParser_TEL_URI_ISDN_SUB_ADDR_PARAM_URL_token,
  SipParser_TEL_URI_CONTEXT_PARAM_token,
  SipParser_TEL_URI_ENUMDI_PARAM_token,
  SipParser_URL_GENERIC_PARAM_DATA_token, SipParser_CPC_PARAM_URL_token,
  SipParser_TEL_URI_RN_PARAM_token,
  SipParser_TEL_URI_RN_CONTEXT_PARAM_token,
  SipParser_TEL_URI_CIC_PARAM_token,
  SipParser_TEL_URI_CIC_CONTEXT_PARAM_token,
  SipParser_TEL_URI_NPDI_PARAM_token,
  SipParser_TEL_URI_EXTENTION_VALUE_token = 334,
  SipParser_TEL_URI_POST_DIAL_VALUE_token = 337,
  SipParser_TEL_URI_POST_DIAL_CHAR_token, SipParser_DTMFDIGIT_token = 340,
  SipParser_PAUSE_CHAR_token,
  SipParser_TEL_URI_ISDN_SUB_ADDR_VALUE_token = 343, SipParser_URIC_token,
  SipParser_RESERVED_FOR_URIC_token = 346, SipParser_UNRESERVED_token,
  SipParser_PCT_ENCODED_token, SipParser_TEL_URI_CONTEXT_VALUE_token = 351,
  SipParser_DESCRIPTOR_token, SipParser_DOMAINNAME_token,
  SipParser_LABEL_token, SipParser_NEXT_LABEL_token,
  SipParser_ALPHA_token = 359, SipParser_ALPHANUM_OR_HYPHEN_token,
  SipParser_ALPHANUM_token = 364,
  SipParser_TEL_URI_NP_GLOBAL_OR_LOCAL_NUMBER_token = 367,
  SipParser_DIAMETER_URI_token = 372, SipParser_DIAMETER_PREFIX_token,
  SipParser_DIAMETER_HOSTPORT_token, SipParser_DIAMETER_PARAMS_token,
  SipParser_DIAMETER_PARAM_token = 378, SipParser_TRANSPORT_PARAM_token,
  SipParser_DIAMETER_PROTOCOL_PARAM_token = 381,
  SipParser_SIP_URL_token = 385, SipParser_PRES_URI_token,
  SipParser_IM_URI_token, SipParser_SIP_PREFIX_token, SipParser_URL_token,
  SipParser_PRES_PREFIX_token = 392, SipParser_RV_ABS_PREFIX_token,
  SipParser_IM_PREFIX_token = 396, SipParser_URL_USERINFO_token = 398,
  SipParser_OPTIONAL_URL_PARAMETERS_token, SipParser_OPTIONAL_HEADERS_token,
  SipParser_URL_HOST_token, SipParser_GENERAL_URL_WITHOUT_PARAM_token,
  SipParser_SIP_URL_WITHOUT_PARAMS_token,
  SipParser_TEL_URL_WITHOUT_PARAMS_token,
  SipParser_DIAMETER_URI_WITHOUT_PARAMS_token,
  SipParser_PRES_URL_WITHOUT_PARAMS_token,
  SipParser_IM_URL_WITHOUT_PARAMS_token,
  SipParser_ABS_URI_WITHOUT_PARAMS_token,
  SipParser_URL_USERINFO_PREFIX_token, SipParser_URL_USERINFO_VAL_token,
  SipParser_HOSTPORT_token, SipParser_PASSWORD_token = 413,
  SipParser_TOKEN_USER_INFO_token, SipParser_TOKEN_USER_INFO_CHAR_token,
  SipParser_PASSWORD_STRING_token = 417,
  SipParser_PASSWORD_STRING_CHAR_token,
  SipParser_URL_HOST_PREFIX_token = 422, SipParser_TOKEN_HOST_token = 424,
  SipParser_TOKEN_HOST_CHAR_token, SipParser_IPV_4_ADDRESS_token = 429,
  SipParser_IPV_4_PART_token, SipParser_HEXPART_token,
  SipParser_HEXSEQ_token, SipParser_HEX4_token = 435,
  SipParser_URL_PARAMETERS_token = 439, SipParser_URL_PARAMETER_token,
  SipParser_USER_PARAM_token, SipParser_TTL_PARAM_URL_token,
  SipParser_MADDR_PARAM_URL_token, SipParser_METHOD_PARAM_URL_token,
  SipParser_LR_PARAM_URL_token, SipParser_COMPRESSION_PARAM_URL_token,
  SipParser_SIGCOMPID_PARAM_URL_token,
  SipParser_TOKENIZED_BY_PARAM_URL_token, SipParser_ORIG_PARAM_URL_token,
  SipParser_GR_PARAM_URL_token, SipParser_USER_PARAM_TYPE_token = 453,
  SipParser_OTHER_USER_token, SipParser_PARAMCHAR_token = 464,
  SipParser_CPC_PARAM_URL_VALUE_token = 471,
  SipParser_URL_OTHER_PARAM_VAL_token = 473, SipParser_PVALUE_token = 484,
  SipParser_HEADERS_LIST_token, SipParser_HEADER_token,
  SipParser_HNAME_token = 488, SipParser_HVALUE_token,
  SipParser_REQUEST_LINE_VALUE_token = 496, SipParser_REQUEST_METHOD_token,
  SipParser_PARTY_VALUE_token = 511, SipParser_NAME_ADDR_token,
  SipParser_PARTY_PARAMS_LIST_token, SipParser_MORE_THAN_ONE_TOKEN_token,
  SipParser_TAG_PARAM_token = 518, SipParser_PARTY_PARAM_DATA_token = 520,
  SipParser_CONTACT_HEADER_VALUES_token,
  SipParser_CONTACT_HEADER_OTHER_token,
  SipParser_CONTACT_HEADER_VALUE_LIST_token,
  SipParser_CONTACT_HEADER_VALUE_token = 525,
  SipParser_CONTACT_PARAMS_token, SipParser_CONTACT_PARAM_token,
  SipParser_Q_token, SipParser_ACTION_token,
  SipParser_CONTACT_EXPIRES_token, SipParser_TEMP_GRUU_token,
  SipParser_PUB_GRUU_token, SipParser_REG_ID_token,
  SipParser_CONTACT_FEATURE_TAG_AUDIO_token,
  SipParser_CONTACT_FEATURE_TAG_AUTOMATA_token,
  SipParser_CONTACT_FEATURE_TAG_CLASS_token,
  SipParser_CONTACT_FEATURE_TAG_DUPLEX_token,
  SipParser_CONTACT_FEATURE_TAG_DATA_token,
  SipParser_CONTACT_FEATURE_TAG_CONTROL_token,
  SipParser_CONTACT_FEATURE_TAG_MOBILITY_token,
  SipParser_CONTACT_FEATURE_TAG_DESCRIPTION_token,
  SipParser_CONTACT_FEATURE_TAG_EVENTS_token,
  SipParser_CONTACT_FEATURE_TAG_PRIORITY_token,
  SipParser_CONTACT_FEATURE_TAG_METHODS_token,
  SipParser_CONTACT_FEATURE_TAG_SCHEMES_token,
  SipParser_CONTACT_FEATURE_TAG_APPLICATION_token,
  SipParser_CONTACT_FEATURE_TAG_VIDEO_token,
  SipParser_CONTACT_FEATURE_TAG_LANGUAGE_token,
  SipParser_CONTACT_FEATURE_TAG_TYPE_token,
  SipParser_CONTACT_FEATURE_TAG_ISFOCUS_token,
  SipParser_CONTACT_FEATURE_TAG_ACTOR_token,
  SipParser_CONTACT_FEATURE_TAG_TEXT_token,
  SipParser_CONTACT_FEATURE_TAG_EXTENSIONS_token,
  SipParser_CONTACT_FEATURE_TAG_SIP_INSTANCE_token,
  SipParser_Q_VALUE_token = 556, SipParser_Q_VALUE_OPTIONAL_token = 558,
  SipParser_ACTION_VALUE_token = 562, SipParser_GRUU_PARAM_VAL_token = 566,
  SipParser_FEATURE_TAG_STRING_token = 570,
  SipParser_EXPIRES_VALUE_token = 587,
  SipParser_EXPIRES_VALUE_FOR_CONTACT_token = 589,
  SipParser_DELTA_SECONDS_token, SipParser_SIP_DATE_token,
  SipParser_SIP_DATE_HEADER_VAL_token, SipParser_DATE_token,
  SipParser_TIME_token, SipParser_DAY_token, SipParser_DD_token = 603,
  SipParser_MM_token, SipParser_YY_token, SipParser_HOUR_token = 618,
  SipParser_MINUTE_token, SipParser_SECOND_token, SipParser_TIME_ZONE_token,
  SipParser_AUTHENTICATION_VAL_token = 625, SipParser_CHALLENGE_VALUE_token,
  SipParser_DIGEST_CHALLENGE_LIST_token, SipParser_AUTH_PARAM_LIST_token,
  SipParser_DIGEST_CHALLENGE_token, SipParser_REALM_token,
  SipParser_DOMAIN_token, SipParser_NONCE_token, SipParser_OPAQUE_token,
  SipParser_INTEGRITY_KEY_token, SipParser_CIPHER_KEY_token,
  SipParser_AUTH_PARAM_token, SipParser_STALE_token,
  SipParser_ALGORITHM_token, SipParser_QOP_OPTIONS_token,
  SipParser_PARAM_IN_NEXT_CHALLENGE_token,
  SipParser_REALM_VALUE_token = 642, SipParser_DOMAIN_EQUAL_token = 644,
  SipParser_DOMAIN_WHITE_token, SipParser_DOMAIN_VAL_token = 647,
  SipParser_URI_LIST_token, SipParser_STRING_DOMAIN_PATCH_token,
  SipParser_TOKEN_FOR_DOMAIN_token, SipParser_TOKEN_CHAR_FOR_DOMAIN_token,
  SipParser_DOMAIN_SPACE_token = 653, SipParser_NONCE_VALUE_token = 656,
  SipParser_OPAQUE_VALUE_token = 658,
  SipParser_ALGORITHM_DECLARATION_token = 661,
  SipParser_ALGORITHM_AND_AKAV_token, SipParser_AKAV_PREFIX_token = 664,
  SipParser_AKAV_token, SipParser_AKA_VERSION_VAL_token,
  SipParser_ALGORITHM_VAL_token, SipParser_ALGORITHM_PREFIX_token,
  SipParser_QOP_VALUE_LIST_token = 676, SipParser_QOP_VALUE_token,
  SipParser_QUOTED_HEX_VALUE_token = 681,
  SipParser_AUTH_PARAM_DATA_token = 684, SipParser_AUTHORIZATION_VAL_token,
  SipParser_CREDENTIALS_VALUE_token, SipParser_DIGEST_RESPONSE_LIST_token,
  SipParser_DIGEST_RESPONSE_token, SipParser_USERNAME_token,
  SipParser_RESPONSE_token, SipParser_CNONCE_token,
  SipParser_NONCE_COUNT_token, SipParser_AUTS_PARAM_token,
  SipParser_INTEGRITY_PROTECTED_token, SipParser_MESSAGE_QOP_token,
  SipParser_DIGEST_URI_token, SipParser_USERNAME_VALUE_token = 698,
  SipParser_DIGEST_URI_VALUE_token = 700,
  SipParser_LEFT_URI_QUOTATION_token, SipParser_CNONCE_VALUE_token = 703,
  SipParser_NC_VALUE_token = 705, SipParser_LHEX_token,
  SipParser_REQUEST_DIGEST_token = 708, SipParser_THIRTY_TWO_LHEX_token,
  SipParser_SIXTY_FOUR_BASE_ENCODING_token = 713,
  SipParser_SIXTY_FOUR_BASE_ENCODING_CHAR_token,
  SipParser_ONE_OR_TWO_EQUAL_SIGNS_token = 716,
  SipParser_ROUTE_LIST_token = 723, SipParser_SINGLE_ROUTE_token,
  SipParser_RETRY_AFTER_VAL_token, SipParser_RETRY_AFTER_COMMENT_token,
  SipParser_RETRY_PARAM_token = 728, SipParser_DURATION_PARAM_token,
  SipParser_RSEQ_VAL_token = 731, SipParser_RESPONSE_NUM_token,
  SipParser_RACK_VAL_token, SipParser_OPTION_TAG_token,
  SipParser_OPTION_TAG_LIST_token, SipParser_SUPPORTED_OPTION_TAG_token,
  SipParser_EVENT_HEADER_VAL_token, SipParser_EVENT_TYPE_token,
  SipParser_EVENT_PARAMS_token, SipParser_EVENT_PACKAGE_token,
  SipParser_EVENT_TEMPLATE_token, SipParser_EVENT_PARAM_token,
  SipParser_EVENT_ID_token, SipParser_DISPOSITION_VAL_token = 745,
  SipParser_DISPOSITION_TYPE_token, SipParser_DISPOSITION_PARAMS_token,
  SipParser_HANDLING_PARAM_token = 753,
  SipParser_SESSION_EXPIRES_VALUE_token = 757,
  SipParser_REFRESHER_TYPE_token, SipParser_REFRESHER_TYPE_NAME_token = 760,
  SipParser_REFRESHER_TYPE_VALUE_token, SipParser_MIN_SE_VALUE_token = 765,
  SipParser_REPLACES_HEADER_VALUE_token, SipParser_REPLACES_PARAMS_token,
  SipParser_REPLACES_PARAM_token, SipParser_TO_TAG_PARAM_token,
  SipParser_FROM_TAG_PARAM_token, SipParser_EARLY_FLAG_PARAM_token,
  SipParser_AUTHENTICATION_INFO_HEADER_LIST_token = 775,
  SipParser_AUTHENTICATION_INFO_HEADER_PARAM_token,
  SipParser_NEXT_NONCE_token, SipParser_RESPONSE_AUTH_token,
  SipParser_MSG_QOP_token, SipParser_RESPONSE_AUTH_VAL_token = 782,
  SipParser_P_URI_HEADERS_LIST_token = 786,
  SipParser_P_URI_HEADER_VALUE_token, SipParser_P_URI_HEADER_PARAMS_token,
  SipParser_P_VISITED_NETWORK_ID_HEADERS_LIST_token,
  SipParser_P_VISITED_NETWORK_ID_HEADER_VALUE_token,
  SipParser_VNETWORK_SPEC_token,
  SipParser_P_VISITED_NETWORK_ID_PARAMS_token,
  SipParser_P_ACCESS_NETWORK_INFO_HEADER_VALUE_token,
  SipParser_ACCESS_TYPE_token, SipParser_P_ACCESS_NETWORK_INFO_PARAMS_token,
  SipParser_ACCESS_token, SipParser_ACCESS_PARAM_token = 818,
  SipParser_NETWORK_PROVIDED_token, SipParser_CGI_3GPP_token,
  SipParser_UTRAN_CELL_ID_3GPP_token, SipParser_I_WLAN_NODE_ID_token,
  SipParser_DSL_LOCATION_token, SipParser_CI_3GPP2_token,
  SipParser_GEN_VALUE_OTHER_PARAM_EXPR_token,
  SipParser_P_CHARGING_FUNCTION_ADDRESSES_HEADER_VALUE_token = 832,
  SipParser_CHARGE_ADDR_PARAM_token, SipParser_CHARGE_ADDR_PARAMS_token,
  SipParser_CHARGE_ADDR_PARAM_CCF_token,
  SipParser_CHARGE_ADDR_PARAM_ECF_token,
  SipParser_GENERAL_VALUE_token = 838,
  SipParser_P_CHARGING_VECTOR_HEADER_VALUE_token = 840,
  SipParser_ICID_VALUE_token, SipParser_CHARGE_PARAMS_token,
  SipParser_CHARGE_PARAM_token = 844,
  SipParser_CHARGE_PARAM_ICID_GEN_ADDR_token,
  SipParser_CHARGE_PARAM_ORIG_IOI_token,
  SipParser_CHARGE_PARAM_TERM_IOI_token,
  SipParser_CHARGE_PARAM_GPRS_CHARGING_INFO_token,
  SipParser_CHARGE_PARAM_XDSL_CHARGING_INFO_token,
  SipParser_PDP_INFO_HIERARCHY_token,
  SipParser_DSL_BEARER_INFO_HIERARCHY_token,
  SipParser_I_WLAN_CHARGING_INFO_token,
  SipParser_PACKETCABLE_CHARGING_INFO_token,
  SipParser_PACKETCABLE_CHARGING_INFO_BCID_token,
  SipParser_GGSN_token = 858, SipParser_AUTH_TOKEN_token,
  SipParser_BRAS_token, SipParser_PDP_INFO_token = 865,
  SipParser_DSL_BEARER_INFO_token = 867, SipParser_SINGLE_PDP_INFO_token,
  SipParser_SINGLE_DSL_BEARER_INFO_token, SipParser_PDP_ITEM_token,
  SipParser_PDP_SIG_token, SipParser_PDP_CID_token, SipParser_FLOW_ID_token,
  SipParser_YES_OR_NO_token = 876, SipParser_DSL_BEARER_ITEM_token = 878,
  SipParser_DSL_BEARER_SIG_token, SipParser_DSL_BEARER_CID_token,
  SipParser_LPAREN_token = 885, SipParser_FLOW_ID_VALUE_token,
  SipParser_FLOW_ID_SYNTAX1_token, SipParser_FLOW_ID_SYNTAX2_token,
  SipParser_DIGITS_token = 892, SipParser_WHITE_IN_WORD_token,
  SipParser_PACKETCABLE_CHARGING_INFO_BCID_TOKEN_token = 899,
  SipParser_P_MEDIA_AUTHORIZATION_VAL_token = 901,
  SipParser_P_MEDIA_AUTHORIZATION_TOKEN_token,
  SipParser_SECURITY_HEADERS_LIST_token,
  SipParser_SECURITY_HEADER_VALUE_token, SipParser_MECHANISM_NAME_token,
  SipParser_MECHANISM_PARAMS_token, SipParser_MECHANISM_token,
  SipParser_MECHANISM_PARAM_token = 911, SipParser_DIGEST_ALGORITHM_token,
  SipParser_DIGEST_QOP_token, SipParser_DIGEST_VERIFY_token,
  SipParser_SECURITY_ALGORITHM_token, SipParser_SECURITY_PROTOCOL_token,
  SipParser_SECURITY_MODE_token, SipParser_SECURITY_ENCRYPT_ALGORITHM_token,
  SipParser_SECURITY_SPIC_token, SipParser_SECURITY_SPIS_token,
  SipParser_SECURITY_PORTC_token, SipParser_SECURITY_PORTS_token,
  SipParser_DIGEST_ALGORITHM_DECLARATION_token,
  SipParser_DIGEST_VERIFY_VALUE_token = 927,
  SipParser_SECURITY_ALGORITHM_VAL_token = 929,
  SipParser_SECURITY_PROTOCOL_VAL_token = 933,
  SipParser_SECURITY_MODE_VAL_token = 937,
  SipParser_SECURITY_ENCRYPT_ALGORITHM_VAL_token = 942,
  SipParser_TEN_DIGIT_token = 947,
  SipParser_P_PROFILE_KEY_HEADERS_LIST_token = 951,
  SipParser_P_PROFILE_KEY_HEADER_VALUE_token,
  SipParser_P_PROFILE_KEY_HEADER_PARAMS_token,
  SipParser_P_USER_DATABASE_HEADERS_LIST_token,
  SipParser_P_USER_DATABASE_HEADER_VALUE_token,
  SipParser_P_ANSWER_STATE_HEADER_VALUE_token, SipParser_ANSWER_TYPE_token,
  SipParser_P_ANSWER_STATE_PARAMS_token, SipParser_ANSWER_token,
  SipParser_ANSWER_PARAM_token = 962,
  SipParser_P_SERVED_USER_HEADERS_LIST_token,
  SipParser_P_SERVED_USER_HEADER_VALUE_token,
  SipParser_P_SERVED_USER_HEADER_PARAMS_token,
  SipParser_P_SERVED_USER_HEADER_PARAM_token,
  SipParser_P_SERVED_USER_SESSION_CASE_token,
  SipParser_P_SERVED_USER_REGISTRATION_STATE_token,
  SipParser_P_SERVED_USER_SESSION_CASE_VAL_token = 970,
  SipParser_P_SERVED_USER_REGISTRATION_STATE_VAL_token = 973
} SipParser_token_type;

typedef struct SipParser_pcb_struct{
  SipParser_token_type token_number, reduction_token, error_frame_token;
  int input_code;
  int input_value;
  int line, column;
  int ssx, sn, error_frame_ssx;
  int drt, dssx, dsn;
  int ss[46];
  SipParser_vs_type vs[46];
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



        ParserDiameterUri       diameterUri;    

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












		ParserPUriHeader				puriHeader; 
		ParserPVisitedNetworkIDHeader	pvisitedNetworkIDHeader; 
		ParserPAccessNetworkInfoHeader	paccessNetworkInfoHeader; 
		ParserPChargingFunctionAddressesHeader	pchargingFunctionAddressesHeader; 
		ParserPChargingVectorHeader		pchargingVectorHeader; 
		ParserPMediaAuthorizationHeader	pmediaAuthorizationHeader; 
		ParserSecurityHeader			securityHeader; 
		ParserPProfileKeyHeader	        pprofileKeyHeader; 
		ParserPUserDatabaseHeader       puserDatabaseHeader; 
		ParserPAnswerStateHeader        panswerStateHeader; 
		ParserPServedUserHeader         pservedUserHeader; 









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
