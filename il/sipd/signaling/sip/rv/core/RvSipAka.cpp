/// @file
/// @brief SIP core package AKA utils.
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <string>

#include "RvSipAka.h"

//============================>>>>>>>>>> static <<<<<<<<<<<<==============================

/***************************************************************************
 * SIP_AkaClientGetRandAndAutnFromHeader
 * ------------------------------------------------------------------------
 * General:  Gets the nonce value from authentication header.
 *           Decode it base 64, and break it into AUTN and RAND strings.
 * Return Value: RV_OK - if successful. error code o/w
 * ------------------------------------------------------------------------
 * Arguments:
 * input   : hAuthHeader - The Authentication header.
 * output  : pRAND - pointer to the RAND buffer.
 *           pAUTN - pointer to the AUTN buffer.
 ***************************************************************************/
static RvStatus SIP_AkaClientGetRandAndAutnFromHeader (RvSipAuthenticationHeaderHandle hAuthHeader,
                                                       uint8_t *pRAND,
                                                       uint8_t *pAUTN)
{
  uint8_t  nonce[150];
  RvInt    len;
  RvStatus rv;
  uint8_t  nonceDecodeB64[AKA_RAND_LEN+AKA_AUTN_LEN];
  
  /* Get the nonce value from authentication header. */
  rv = RvSipAuthenticationHeaderGetNonce (hAuthHeader, (RvChar*)nonce, 150, (RvUint*)&len);
  if(rv != RV_OK)
    {
      return RV_ERROR_NOT_FOUND;
    }
  
  /* Give the decode function, the nonce string without the quotation marks:
     giving nonce+1 -> skip the first mark.
     giving len-2   -> don't reach the second mark. */
  len = RvSipMidDecodeB64(nonce+1, len-2, nonceDecodeB64, AKA_RAND_LEN+AKA_AUTN_LEN);
  
  /* Break the nonce string to RAND and AUTN */
  memcpy(pRAND, nonceDecodeB64, AKA_RAND_LEN);
  memcpy(pAUTN, nonceDecodeB64 + AKA_RAND_LEN, AKA_AUTN_LEN);
  return RV_OK;
}

//==============================================================================================================//

BEGIN_DECL_RV_SIP_ENGINE_NAMESPACE namespace RvSipUtils { namespace RvSipAka {

  bool SIP_isAkaMsg(RvSipMsgHandle hSipMsg) {

    bool ret=false;
    if(hSipMsg) {
      RvSipAuthenticationHeaderHandle  hSipAuthHeader  = (RvSipAuthenticationHeaderHandle)0;
      RvSipHeaderListElemHandle        hHeaderListElem = NULL;
      hSipAuthHeader = (RvSipAuthenticationHeaderHandle)RvSipMsgGetHeaderByType(hSipMsg,
										RVSIP_HEADERTYPE_AUTHENTICATION,RVSIP_FIRST_HEADER, 
										&hHeaderListElem);
      if(hSipAuthHeader) {
	if (RvSipAuthenticationHeaderGetAKAVersion (hSipAuthHeader) >= 0) {
	  ret=true;
	}
      }
    }
    return ret;
  }

  RvStatus SIP_AkaInsertInitialAuthorizationToMessage(RvSipRegClientHandle hRegClient, 
						      const std::string &authname, const std::string &authrealm) {
    
    RvSipAuthorizationHeaderHandle   hAuthorization;
    RvStatus                         rv = RV_OK;
    RvChar userName[MAX_LENGTH_OF_USER_NAME];
    RvChar realm[MAX_LENGTH_OF_REALM_NAME];

    if ( !hRegClient || authname.empty()) {
      return RV_ERROR_BADPARAM;
    }

    /* Construct Authorization header in the reg-client object */
    rv = RvSipRegClientGetNewMsgElementHandle(hRegClient,
					      RVSIP_HEADERTYPE_AUTHORIZATION,
					      RVSIP_ADDRTYPE_UNDEFINED,
					      (void**)&hAuthorization);
    if(rv != RV_OK) {
      return rv;
    }

    /* the user-name and realm should be set as quoted strings */
    snprintf(userName,sizeof(userName)-1,"\"%s\"",authname.c_str());
    snprintf(realm,sizeof(realm)-1,"\"%s\"",authrealm.c_str());

    /* set the user name, realm, and other basic parameters to the header */
    rv = RvSipAuthorizationHeaderSetUserName(hAuthorization, userName);
    if(rv == RV_OK)
      rv = RvSipAuthorizationHeaderSetRealm(hAuthorization, realm);
    if(rv == RV_OK)
      rv = RvSipAuthorizationHeaderSetAuthScheme(hAuthorization, RVSIP_AUTH_SCHEME_DIGEST, NULL);
    if(rv == RV_OK)
      rv = RvSipAuthorizationHeaderSetHeaderType(hAuthorization, RVSIP_AUTH_AUTHORIZATION_HEADER);

    /* reset the algorithm parameter to the header */
    if(rv == RV_OK)
      rv = RvSipAuthorizationHeaderSetAuthAlgorithm(hAuthorization, RVSIP_AUTH_ALGORITHM_UNDEFINED, NULL);

    if(rv != RV_OK) {
      return rv;
    }

    /* Set the new header to the reg-client object */
    rv = RvSipRegClientSetInitialAuthorization(hRegClient, hAuthorization);

    return rv;
  }

  RvStatus SIP_AkaInsertInitialAuthorizationToMessage(RvSipCallLegHandle hCallLeg, 
						      const std::string &authname, const std::string &authrealm) {

    RvSipAuthorizationHeaderHandle   hAuthorization;
    RvStatus                         rv = RV_OK;
    RvChar userName[MAX_LENGTH_OF_USER_NAME];
    RvChar realm[MAX_LENGTH_OF_REALM_NAME];

    if ( !hCallLeg || authname.empty()) {
      return RV_ERROR_BADPARAM;
    }

    /* Construct Authorization header in the reg-client object */
    rv = RvSipCallLegGetNewMsgElementHandle(hCallLeg,
					    RVSIP_HEADERTYPE_AUTHORIZATION,
					    RVSIP_ADDRTYPE_UNDEFINED,
					    (void**)&hAuthorization);
    if(rv != RV_OK) {
      return rv;
    }
    
    /* the user-name and realm should be set as quoted strings */
    snprintf(userName,sizeof(userName)-1,"\"%s\"",authname.c_str());
    snprintf(realm,sizeof(realm)-1,"\"%s\"",authrealm.c_str());
    
    /* set the user name, realm, and other basic parameters to the header */
    rv = RvSipAuthorizationHeaderSetUserName(hAuthorization, userName);
    if(rv == RV_OK)
      rv = RvSipAuthorizationHeaderSetRealm(hAuthorization, realm);
    if(rv == RV_OK)
      rv = RvSipAuthorizationHeaderSetAuthScheme(hAuthorization, RVSIP_AUTH_SCHEME_DIGEST, NULL);
    if(rv == RV_OK)
      rv = RvSipAuthorizationHeaderSetHeaderType(hAuthorization, RVSIP_AUTH_AUTHORIZATION_HEADER);
    
    /* reset the algorithm parameter to the header */
    if(rv == RV_OK)
      rv = RvSipAuthorizationHeaderSetAuthAlgorithm(hAuthorization, RVSIP_AUTH_ALGORITHM_UNDEFINED, NULL);
    
    if(rv != RV_OK) {
      return rv;
    }
    
    /* Set the new header to the reg-client object */
    rv = RvSipCallLegSetInitialAuthorization(hCallLeg, hAuthorization);

    return rv;
  }

  RvStatus SIP_AkaInsertInitialAuthorizationToMessage(RvSipPubClientHandle hPubClient, 
						      const std::string &authname, const std::string &authrealm) {
    
    RvSipAuthorizationHeaderHandle   hAuthorization;
    RvStatus                         rv = RV_OK;
    RvChar userName[MAX_LENGTH_OF_USER_NAME];
    RvChar realm[MAX_LENGTH_OF_REALM_NAME];

    if ( !hPubClient || authname.empty()) {
      return RV_ERROR_BADPARAM;
    }

    /* Construct Authorization header in the reg-client object */
    rv = RvSipPubClientGetNewMsgElementHandle(hPubClient,
					      RVSIP_HEADERTYPE_AUTHORIZATION,
					      RVSIP_ADDRTYPE_UNDEFINED,
					      (void**)&hAuthorization);
    if(rv != RV_OK) {
      return rv;
    }

    /* the user-name and realm should be set as quoted strings */
    snprintf(userName,sizeof(userName)-1,"\"%s\"",authname.c_str());
    snprintf(realm,sizeof(realm)-1,"\"%s\"",authrealm.c_str());

    /* set the user name, realm, and other basic parameters to the header */
    rv = RvSipAuthorizationHeaderSetUserName(hAuthorization, userName);
    if(rv == RV_OK)
      rv = RvSipAuthorizationHeaderSetRealm(hAuthorization, realm);
    if(rv == RV_OK)
      rv = RvSipAuthorizationHeaderSetAuthScheme(hAuthorization, RVSIP_AUTH_SCHEME_DIGEST, NULL);
    if(rv == RV_OK)
      rv = RvSipAuthorizationHeaderSetHeaderType(hAuthorization, RVSIP_AUTH_AUTHORIZATION_HEADER);

    /* reset the algorithm parameter to the header */
    if(rv == RV_OK)
      rv = RvSipAuthorizationHeaderSetAuthAlgorithm(hAuthorization, RVSIP_AUTH_ALGORITHM_UNDEFINED, NULL);

    if(rv != RV_OK) {
      return rv;
    }

    /* Set the new header to the reg-client object */
    rv = RvSipPubClientSetInitialAuthorization(hPubClient, hAuthorization);

    return rv;
  }
  
  /***************************************************************************************
   * SIP_AkaClientGenerateClientAV
   * -------------------------------------------------------------------------------------
   * General:  This function generates the Authentication Vector for the client side.
   *           1. Get the Authentication header from the given authentication object.
   *           2. Extract the RAND and AUTN values from the nonce value in the Authentication
   *              header.
   *           3. Generate the Authentication Vector, using RAND and the shared secret USER_PW.
   * Return Value: RvStatus.
   * -------------------------------------------------------------------------------------
   * Arguments:
   * Input:  pwd - password,
   *         pwd_length - length of pwd field,
   *         hAuthObj - Handle to the authentication object.
   *         pAuthVector - pointer to an authentication vector
   **************************************************************************************/
  RvStatus SIP_AkaClientGenerateClientAV (const uint8_t * pOP, void* pwd,int pwd_length,
					  RvSipAuthObjHandle hAuthObj, AKA_Av* pAuthVector)
  {
    RvStatus rv;
    RvSipAuthenticationHeaderHandle hAuthHeader;
    RvBool  bValid;
    uint8_t givenRAND[AKA_RAND_LEN+1]; /* +1 for null termination */
    uint8_t givenAUTN[AKA_AUTN_LEN+1]; /* +1 for null termination */
    RvBool  bCorrectMacInAutn;
    uint8_t Key[USER_KEY_LEN];

    if ( !pAuthVector)
      return RV_ERROR_BADPARAM;
    
    /* Get the authentication header from the auth-object
       -------------------------------------------------- */
    rv = RvSipAuthObjGetAuthenticationHeader(hAuthObj, &hAuthHeader, &bValid);
    if(rv != RV_OK || RV_FALSE == bValid) {
      return RV_ERROR_NOT_FOUND;
    }

    /* Extract the RAND and AUTN values from the header nonce value.
       ------------------------------------------------------------- */
    memset(givenAUTN, 0, sizeof(givenAUTN));
    memset(givenRAND, 0, sizeof(givenRAND));
    
    rv = SIP_AkaClientGetRandAndAutnFromHeader (hAuthHeader, givenRAND, givenAUTN);
    if(rv != RV_OK) {
      return RV_ERROR_NOT_FOUND;
    }

    /* Convert the saved password to Key (16 bytes buffer)
       --------------------------------------------------- */
    memset (Key, 0, USER_KEY_LEN);
    
    memcpy((void*)Key, pwd, RvMin(pwd_length, USER_KEY_LEN));

    /* Generate the AV 
       ------------------------------- */
    AkaCreateClientRES (pOP, givenRAND, Key, givenAUTN, pAuthVector, &bCorrectMacInAutn);

    return RV_OK;
  }

  /*-----------------------------------------------------------------------*/
  /*                   Definitions & Constants                             */
  /*-----------------------------------------------------------------------*/


  /*--------------------------- prototypes --------------------------*/
  static void generateRand(INOUT AKA_Av* pAV);

  static void f1    ( const uint8_t* pOP,
		      uint8_t k[16], uint8_t randpar[16], uint8_t sqn[6], uint8_t amf[2],
		      uint8_t mac_a[8] );
  static void f2345 ( const uint8_t* pOP,
		      uint8_t k[16], uint8_t randpar[16],
		      uint8_t res[8], uint8_t ck[16], uint8_t ik[16], uint8_t ak[6] );
  static void f1star( const uint8_t* pOP,
		      uint8_t k[16], uint8_t randpar[16], uint8_t sqn[6], uint8_t amf[2], 
		      uint8_t mac_s[8] );
  static void f5star( const uint8_t* pOP,
		      uint8_t k[16], uint8_t randpar[16],
		      uint8_t ak[6] );
  static void ComputeOPc( const uint8_t* pOP, uint8_t op_c[16] );
  static void RijndaelKeySchedule( uint8_t key[16] );
  static void RijndaelEncrypt( uint8_t input[16], uint8_t output[16] );

  /*-----------------------------------------------------------------------*/
  /*                   Module Functions                                    */
  /*-----------------------------------------------------------------------*/

  /********************************************************************************************
   * AkaCreateClientRES
   * purpose : Creates the RES, IK and CK values, using the given RAND and AUTN values.
   *           Upon receipt of RAND and AUTN:
   *           1. first compute the anonymity key AK = f5K (RAND) and retrieve 
   *              the sequence number SQN = (SQN * AK) * AK.
   *           2. Next computes XMAC = f1(SQN || RAND || AMF) and compares this with MAC which is 
   *              included in AUTN. 
   *              If they are different, set the pbCorrectMac parameter to FALSE.
   *           3. Next the USIM verifies that the received sequence number SQN is in the correct range.
   *              (in our implementation - this is user decision).
   *           4. If the sequence number is considered to be in the correct range however, 
   *              compute RES = f2K (RAND).
   *           5. Finally the USIM computes CK=f3(RAND) and IK=f4(RAND). 
   * input   : randpar - random number.
   *           k    - secret key (password).
   *           autn - AUTN value.
   * output  : pAv  - The Authentication vector struct, filled with information.
   *           pbCorrectMac - indication if MAC equals to XMAC.
   * return  : none.
   ********************************************************************************************/
  void AkaCreateClientRES(const uint8_t* pOP,
			  IN uint8_t randpar[16], IN uint8_t k[16], IN uint8_t autn[16], 
			  OUT AKA_Av* pAV, OUT RvBool *pbCorrectMac)
  {
    uint8_t ak[6], akXorSqn[6], sqn[6]; 
    uint8_t amf[2];
    uint8_t mac[8], xmac[8];
    int i;

    memset(pAV, 0, sizeof(AKA_Av));
    memcpy(pAV->RAND, randpar, 16); /* -->set the given RAND in AV */
    memcpy(pAV->AUTN, autn, 16); /* -->set the given AUTN in AV */

    /* 1. generate RES=f2(RAND) CK=f3(RAND) IK=f4(RAND) and calculate AK */
    /* ------------------------------------------------------------------ */
    f2345(pOP, k, randpar, pAV->XRES, pAV->CK, pAV->IK, ak);

    /* 2. break the AUTN: AUTN := SQN^AK || AMF || MAC
       -----------------------------------------------------------*/
    memcpy(akXorSqn, autn, 6); /* first 6 bytes are akXorSqn */
    memcpy(amf, autn+6, 2); /* next 2 bytes are amf */
    memcpy(mac, autn+8, 8); /* next 8 bytes are mac */

    /* 3. extract SQN -- (SQN^AK) ^ AK 
       ------------------------------------- */
    for (i=0; i<6; i++) { 
      sqn[i] = (uint8_t)(akXorSqn[i] ^ ak[i]);
    }

    /* calculate XMAC */
    f1(pOP, k, pAV->RAND, sqn, amf, xmac); 
    
    /* 4. compare MAC and XMAC */
    if(memcmp(mac, xmac, 8)==0)
      {
        *pbCorrectMac = RV_TRUE;
      }
    else
      {
        *pbCorrectMac = RV_FALSE;
      }
  }

  /********************************************************************************************
   * generateRand
   * purpose : Generate a random number (128 bits).
   *           Create 4 rands and concatenate it together to be 128 bits
   * input   : k - secret key.
   *           sqn - SQN value.
   *           amf - AMF value.
   * output  : pAv - The Authentication vector struct, filled with information.
   * return  : RV_OK - if successful. error code o/w
   ********************************************************************************************/
  static void generateRand(INOUT AKA_Av* pAV)
  {
    uint32_t timeVal;
    uint32_t randNum;
    uint8_t* pRandBlock;
    int i;
    
    timeVal = RvSipMidTimeInMilliGet();
    srand(timeVal);
#define RV_RAND_MAX ((uint32_t)~0)
    
    for(i=0; i<4; i++)
      {
        randNum = (timeVal * 1103515245 + 12345 + rand());
        pRandBlock = &(pAV->RAND[i*4]);
        memcpy(pRandBlock,  (const void *)&randNum, sizeof(randNum));
      }
  }

  /*==================================================================
    IMPLEMENTATION OF F FUNCTIONS. COPYIED FROM 3GPP TS 35.206
    ==================================================================*/
  /*-------------------------------------------------------------------
   *          Example algorithms f1, f1*, f2, f3, f4, f5, f5*
   *-------------------------------------------------------------------
   *
   *  A sample implementation of the example 3GPP authentication and
   *  key agreement functions f1, f1*, f2, f3, f4, f5 and f5*.  This is
   *  a byte-oriented implementation of the functions, and of the block
   *  cipher kernel function Rijndael.
   *
   *  This has been coded for clarity, not necessarily for efficiency.
   *
   *  The functions f2, f3, f4 and f5 share the same inputs and have 
   *  been coded together as a single function.  f1, f1* and f5* are
   *  all coded separately.
   *
   *-----------------------------------------------------------------*/

  /*-------------------------------------------------------------------
   *                            Algorithm f1
   *-------------------------------------------------------------------
   *
   *  Computes network authentication code MAC-A from key K, random
   *  challenge RAND, sequence number SQN and authentication management
   *  field AMF.
   *
   *-----------------------------------------------------------------*/

  static void f1    ( const uint8_t* pOP,
		      uint8_t k[16], uint8_t randpar[16], uint8_t sqn[6], uint8_t amf[2], 
		      uint8_t mac_a[8] )
  {
    uint8_t op_c[16];
    uint8_t temp[16];
    uint8_t in1[16];
    uint8_t out1[16];
    uint8_t rijndaelInput[16];
    uint8_t i;

    RijndaelKeySchedule( k );

    ComputeOPc( pOP, op_c );

    for (i=0; i<16; i++)
      rijndaelInput[i] = (uint8_t)(randpar[i] ^ op_c[i]);
    RijndaelEncrypt( rijndaelInput, temp );

    for (i=0; i<6; i++)
      {
	in1[i]    = sqn[i];
	in1[i+8]  = sqn[i];
      }
    for (i=0; i<2; i++)
      {
	in1[i+6]  = amf[i];
	in1[i+14] = amf[i];
      }

    /* XOR op_c and in1, rotate by r1=64, and XOR *
     * on the constant c1 (which is all zeroes)   */

    for (i=0; i<16; i++)
      rijndaelInput[(i+8) % 16] = (uint8_t)(in1[i] ^ op_c[i]);

    /* XOR on the value temp computed before */

    for (i=0; i<16; i++)
      rijndaelInput[i] ^= temp[i];
  
    RijndaelEncrypt( rijndaelInput, out1 );
    for (i=0; i<16; i++)
      out1[i] ^= op_c[i];

    for (i=0; i<8; i++)
      mac_a[i] = out1[i];

    return;
  } /* end of function f1 */


  
  /*-------------------------------------------------------------------
   *                            Algorithms f2-f5
   *-------------------------------------------------------------------
   *
   *  Takes key K and random challenge RAND, and returns response RES,
   *  confidentiality key CK, integrity key IK and anonymity key AK.
   *
   *-----------------------------------------------------------------*/

  static void f2345 ( const uint8_t* pOP,
		      uint8_t k[16], uint8_t randpar[16],
		      uint8_t res[8], uint8_t ck[16], uint8_t ik[16], uint8_t ak[6] )
  {
    uint8_t op_c[16];
    uint8_t temp[16];
    uint8_t out[16];
    uint8_t rijndaelInput[16];
    uint8_t i;

    RijndaelKeySchedule( k );

    ComputeOPc( pOP, op_c );

    for (i=0; i<16; i++)
      rijndaelInput[i] = (uint8_t)(randpar[i] ^ op_c[i]);
    RijndaelEncrypt( rijndaelInput, temp );

    /* To obtain output block OUT2: XOR OPc and TEMP,    *
     * rotate by r2=0, and XOR on the constant c2 (which *
     * is all zeroes except that the last bit is 1).     */

    for (i=0; i<16; i++)
      rijndaelInput[i] = (uint8_t)(temp[i] ^ op_c[i]);
    rijndaelInput[15] ^= 1;

    RijndaelEncrypt( rijndaelInput, out );
    for (i=0; i<16; i++)
      out[i] ^= op_c[i];

    for (i=0; i<8; i++)
      res[i] = out[i+8];
    for (i=0; i<6; i++)
      ak[i]  = out[i];

    /* To obtain output block OUT3: XOR OPc and TEMP,        *
     * rotate by r3=32, and XOR on the constant c3 (which    *
     * is all zeroes except that the next to last bit is 1). */

    for (i=0; i<16; i++)
      rijndaelInput[(i+12) % 16] = (uint8_t)(temp[i] ^ op_c[i]);
    rijndaelInput[15] ^= 2;

    RijndaelEncrypt( rijndaelInput, out );
    for (i=0; i<16; i++)
      out[i] ^= op_c[i];

    for (i=0; i<16; i++)
      ck[i] = out[i];

    /* To obtain output block OUT4: XOR OPc and TEMP,         *
     * rotate by r4=64, and XOR on the constant c4 (which     *
     * is all zeroes except that the 2nd from last bit is 1). */

    for (i=0; i<16; i++)
      rijndaelInput[(i+8) % 16] = (uint8_t)(temp[i] ^ op_c[i]);
    rijndaelInput[15] ^= 4;

    RijndaelEncrypt( rijndaelInput, out );
    for (i=0; i<16; i++)
      out[i] ^= op_c[i];

    for (i=0; i<16; i++)
      ik[i] = out[i];

    return;
  } /* end of function f2345 */

  
  /*-------------------------------------------------------------------
   *                            Algorithm f1*
   *-------------------------------------------------------------------
   *
   *  Computes resynch authentication code MAC-S from key K, random
   *  challenge RAND, sequence number SQN and authentication management
   *  field AMF.
   *
   *-----------------------------------------------------------------*/

  static void f1star( const uint8_t* pOP,
		      uint8_t k[16], uint8_t randpar[16], uint8_t sqn[6], uint8_t amf[2], 
		      uint8_t mac_s[8] )
  {
    uint8_t op_c[16];
    uint8_t temp[16];
    uint8_t in1[16];
    uint8_t out1[16];
    uint8_t rijndaelInput[16];
    uint8_t i;

    RijndaelKeySchedule( k );

    ComputeOPc( pOP, op_c );

    for (i=0; i<16; i++)
      rijndaelInput[i] = (uint8_t)(randpar[i] ^ op_c[i]);
    RijndaelEncrypt( rijndaelInput, temp );

    for (i=0; i<6; i++)
      {
	in1[i]    = sqn[i];
	in1[i+8]  = sqn[i];
      }
    for (i=0; i<2; i++)
      {
	in1[i+6]  = amf[i];
	in1[i+14] = amf[i];
      }

    /* XOR op_c and in1, rotate by r1=64, and XOR *
     * on the constant c1 (which is all zeroes)   */

    for (i=0; i<16; i++)
      rijndaelInput[(i+8) % 16] = (uint8_t)(in1[i] ^ op_c[i]);

    /* XOR on the value temp computed before */

    for (i=0; i<16; i++)
      rijndaelInput[i] ^= temp[i];
  
    RijndaelEncrypt( rijndaelInput, out1 );
    for (i=0; i<16; i++)
      out1[i] ^= op_c[i];

    for (i=0; i<8; i++)
      mac_s[i] = out1[i+8];

    return;
  } /* end of function f1star */

  
  /*-------------------------------------------------------------------
   *                            Algorithm f5*
   *-------------------------------------------------------------------
   *
   *  Takes key K and random challenge RAND, and returns resynch
   *  anonymity key AK.
   *
   *-----------------------------------------------------------------*/

  static void f5star( const uint8_t* pOP,
		      uint8_t k[16], uint8_t randpar[16],
		      uint8_t ak[6] )
  {
    uint8_t op_c[16];
    uint8_t temp[16];
    uint8_t out[16];
    uint8_t rijndaelInput[16];
    uint8_t i;

    RijndaelKeySchedule( k );

    ComputeOPc( pOP, op_c );

    for (i=0; i<16; i++)
      rijndaelInput[i] = (uint8_t)(randpar[i] ^ op_c[i]);
    RijndaelEncrypt( rijndaelInput, temp );

    /* To obtain output block OUT5: XOR OPc and TEMP,         *
     * rotate by r5=96, and XOR on the constant c5 (which     *
     * is all zeroes except that the 3rd from last bit is 1). */

    for (i=0; i<16; i++)
      rijndaelInput[(i+4) % 16] = (uint8_t)(temp[i] ^ op_c[i]);
    rijndaelInput[15] ^= 8;

    RijndaelEncrypt( rijndaelInput, out );
    for (i=0; i<16; i++)
      out[i] ^= op_c[i];

    for (i=0; i<6; i++)
      ak[i] = out[i];

    return;
  } /* end of function f5star */

  
  /*-------------------------------------------------------------------
   *  Function to compute OPc from OP and K.  Assumes key schedule has
    already been performed.
    *-----------------------------------------------------------------*/

  static void ComputeOPc( const uint8_t* pOP, uint8_t op_c[16] )
  {
    uint8_t i;
  
    RijndaelEncrypt( (uint8_t *)pOP, op_c );
    for (i=0; i<16; i++)
      op_c[i] ^= pOP[i];

    return;
  } /* end of function ComputeOPc */



  /*-------------------- Rijndael round subkeys ---------------------*/
  uint8_t roundKeys[11][4][4];

  /*--------------------- Rijndael S box table ----------------------*/
  uint8_t S[256] = {
    99,124,119,123,242,107,111,197, 48,  1,103, 43,254,215,171,118,
    202,130,201,125,250, 89, 71,240,173,212,162,175,156,164,114,192,
    183,253,147, 38, 54, 63,247,204, 52,165,229,241,113,216, 49, 21,
    4,199, 35,195, 24,150,  5,154,  7, 18,128,226,235, 39,178,117,
    9,131, 44, 26, 27,110, 90,160, 82, 59,214,179, 41,227, 47,132,
    83,209,  0,237, 32,252,177, 91,106,203,190, 57, 74, 76, 88,207,
    208,239,170,251, 67, 77, 51,133, 69,249,  2,127, 80, 60,159,168,
    81,163, 64,143,146,157, 56,245,188,182,218, 33, 16,255,243,210,
    205, 12, 19,236, 95,151, 68, 23,196,167,126, 61,100, 93, 25,115,
    96,129, 79,220, 34, 42,144,136, 70,238,184, 20,222, 94, 11,219,
    224, 50, 58, 10, 73,  6, 36, 92,194,211,172, 98,145,149,228,121,
    231,200, 55,109,141,213, 78,169,108, 86,244,234,101,122,174,  8,
    186,120, 37, 46, 28,166,180,198,232,221,116, 31, 75,189,139,138,
    112, 62,181,102, 72,  3,246, 14, 97, 53, 87,185,134,193, 29,158,
    225,248,152, 17,105,217,142,148,155, 30,135,233,206, 85, 40,223,
    140,161,137, 13,191,230, 66,104, 65,153, 45, 15,176, 84,187, 22,
  };

  /*------- This array does the multiplication by x in GF(2^8) ------*/
  uint8_t Xtime[256] = {
    0,  2,  4,  6,  8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30,
    32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62,
    64, 66, 68, 70, 72, 74, 76, 78, 80, 82, 84, 86, 88, 90, 92, 94,
    96, 98,100,102,104,106,108,110,112,114,116,118,120,122,124,126,
    128,130,132,134,136,138,140,142,144,146,148,150,152,154,156,158,
    160,162,164,166,168,170,172,174,176,178,180,182,184,186,188,190,
    192,194,196,198,200,202,204,206,208,210,212,214,216,218,220,222,
    224,226,228,230,232,234,236,238,240,242,244,246,248,250,252,254,
    27, 25, 31, 29, 19, 17, 23, 21, 11,  9, 15, 13,  3,  1,  7,  5,
    59, 57, 63, 61, 51, 49, 55, 53, 43, 41, 47, 45, 35, 33, 39, 37,
    91, 89, 95, 93, 83, 81, 87, 85, 75, 73, 79, 77, 67, 65, 71, 69,
    123,121,127,125,115,113,119,117,107,105,111,109, 99, 97,103,101,
    155,153,159,157,147,145,151,149,139,137,143,141,131,129,135,133,
    187,185,191,189,179,177,183,181,171,169,175,173,163,161,167,165,
    219,217,223,221,211,209,215,213,203,201,207,205,195,193,199,197,
    251,249,255,253,243,241,247,245,235,233,239,237,227,225,231,229
  };


  /*-------------------------------------------------------------------
   *  Rijndael key schedule function.  Takes 16-byte key and creates 
   *  all Rijndael's internal subkeys ready for encryption.
   *-----------------------------------------------------------------*/

  static void RijndaelKeySchedule( uint8_t key[16] )
  {
    uint8_t roundConst;
    int i, j;

    /* first round key equals key */
    for (i=0; i<16; i++)
      roundKeys[0][i & 0x03][i>>2] = key[i];

    roundConst = 1;

    /* now calculate round keys */
    for (i=1; i<11; i++)
      {
	roundKeys[i][0][0] = (uint8_t)(S[roundKeys[i-1][1][3]] ^ roundKeys[i-1][0][0] ^ roundConst);
	roundKeys[i][1][0] = (uint8_t)(S[roundKeys[i-1][2][3]] ^ roundKeys[i-1][1][0]);
	roundKeys[i][2][0] = (uint8_t)(S[roundKeys[i-1][3][3]] ^ roundKeys[i-1][2][0]);
	roundKeys[i][3][0] = (uint8_t)(S[roundKeys[i-1][0][3]] ^ roundKeys[i-1][3][0]);

	for (j=0; j<4; j++)
	  {
	    roundKeys[i][j][1] = (uint8_t)(roundKeys[i-1][j][1] ^ roundKeys[i][j][0]);
	    roundKeys[i][j][2] = (uint8_t)(roundKeys[i-1][j][2] ^ roundKeys[i][j][1]);
	    roundKeys[i][j][3] = (uint8_t)(roundKeys[i-1][j][3] ^ roundKeys[i][j][2]);
	  }

	/* update round constant */
	roundConst = Xtime[roundConst];
      }

    return;
  } /* end of function RijndaelKeySchedule */


  /* Round key addition function */
  static void KeyAdd(uint8_t state[4][4], uint8_t roundKeys[11][4][4], int round)
  {
    int i, j;

    for (i=0; i<4; i++)
      for (j=0; j<4; j++)
	state[i][j] ^= roundKeys[round][i][j];

    return;
  }


  /* Byte substitution transformation */
  static int ByteSub(uint8_t state[4][4])
  {
    int i, j;

    for (i=0; i<4; i++)
      for (j=0; j<4; j++)
	state[i][j] = S[state[i][j]];
  
    return 0;
  }


  /* Row shift transformation */
  static void ShiftRow(uint8_t state[4][4])
  {
    uint8_t temp;

    /* left rotate row 1 by 1 */
    temp = state[1][0];
    state[1][0] = state[1][1];
    state[1][1] = state[1][2];
    state[1][2] = state[1][3];
    state[1][3] = temp;

    /* left rotate row 2 by 2 */
    temp = state[2][0];
    state[2][0] = state[2][2];
    state[2][2] = temp;
    temp = state[2][1];
    state[2][1] = state[2][3];
    state[2][3] = temp;

    /* left rotate row 3 by 3 */
    temp = state[3][0];
    state[3][0] = state[3][3];
    state[3][3] = state[3][2];
    state[3][2] = state[3][1];
    state[3][1] = temp;

    return;
  }


  /* MixColumn transformation*/
  static void MixColumn(uint8_t state[4][4])
  {
    uint8_t temp, tmp, tmp0;
    int i;

    /* do one column at a time */
    for (i=0; i<4;i++)
      {
	temp = (uint8_t)(state[0][i] ^ state[1][i] ^ state[2][i] ^ state[3][i]);
	tmp0 = state[0][i];

	/* Xtime array does multiply by x in GF2^8 */
	tmp = Xtime[state[0][i] ^ state[1][i]];
	state[0][i] ^= temp ^ tmp;

	tmp = Xtime[state[1][i] ^ state[2][i]];
	state[1][i] ^= temp ^ tmp;

	tmp = Xtime[state[2][i] ^ state[3][i]];
	state[2][i] ^= temp ^ tmp;

	tmp = Xtime[state[3][i] ^ tmp0];
	state[3][i] ^= temp ^ tmp;
      }

    return;
  }


  /*-------------------------------------------------------------------
   *  Rijndael encryption function.  Takes 16-byte input and creates 
   *  16-byte output (using round keys already derived from 16-byte 
   *  key).
   *-----------------------------------------------------------------*/

  static void RijndaelEncrypt( uint8_t input[16], uint8_t output[16] )
  {
    uint8_t state[4][4];
    int i, r;

    /* initialise state array from input byte string */
    for (i=0; i<16; i++)
      state[i & 0x3][i>>2] = input[i];

    /* add first round_key */
    KeyAdd(state, roundKeys, 0);
  
    /* do lots of full rounds */
    for (r=1; r<=9; r++)
      {
	ByteSub(state);
	ShiftRow(state);
	MixColumn(state);
	KeyAdd(state, roundKeys, r);
      }

    /* final round */
    ByteSub(state);
    ShiftRow(state);
    KeyAdd(state, roundKeys, r);

    /* produce output byte string from state array */
    for (i=0; i<16; i++)
      {
	output[i] = state[i & 0x3][i>>2];
      }

    return;

  } /* end of function RijndaelEncrypt */
  
}} END_DECL_RV_SIP_ENGINE_NAMESPACE;

