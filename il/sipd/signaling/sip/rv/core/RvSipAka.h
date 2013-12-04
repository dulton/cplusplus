/// @file
/// @brief SIP core package AKA header.
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef RVSIPAKA_H_
#define RVSIPAKA_H_

#include <string>

#include "RvSipHeaders.h"
#include "RvSipUtils.h"

#define USER_NAME_LEN  20
#define USER_KEY_LEN   16
#define AKA_OP_LEN   16
#define AKA_AUTN_LEN   16
#define AKA_RES_LEN    16
#define AKA_RAND_LEN   16
#define AKA_SQN_LEN     6

#define AMF ("11")

BEGIN_DECL_RV_SIP_ENGINE_NAMESPACE

namespace RvSipUtils { 

  namespace RvSipAka {

    /* this type defines an Authentication Vector. the AKA_AuC functions can fill
       such an AV, by getting the correct key, SQN, and AMF(authentication management field) */
    typedef struct {
      uint8_t IK[16];   /*Integrity Key */
      uint8_t CK[16];   /*Cipher Key */
      uint8_t AUTN[AKA_AUTN_LEN]; /*128 bits - Authentication Token */
      uint8_t RAND[AKA_RAND_LEN]; /*128 bits - Random Challenge */
      uint8_t XRES[AKA_RES_LEN]; /*4-16 octets(=128 bits) */
    } AKA_Av;
    
    bool SIP_isAkaMsg(RvSipMsgHandle hSipMsg);
    
    RvStatus SIP_AkaInsertInitialAuthorizationToMessage(RvSipRegClientHandle hRegClient, const std::string &authname, const std::string &authrealm);
    RvStatus SIP_AkaInsertInitialAuthorizationToMessage(RvSipCallLegHandle hCallLeg, const std::string &authname, const std::string &authrealm);
    RvStatus SIP_AkaInsertInitialAuthorizationToMessage(RvSipPubClientHandle hPubClient, const std::string &authname, const std::string &authrealm);
    
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
					    RvSipAuthObjHandle hAuthObj, AKA_Av* pAuthVector);
    
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
     * input   : rand - random number.
     *           k    - secret key (password).
     *           autn - AUTN value.
     * output  : pAv  - The Authentication vector struct, filled with information.
     *           pbCorrectMac - indication if MAC equals to XMAC.
     * return  : none.
     ********************************************************************************************/
    void AkaCreateClientRES(const uint8_t* pOP,
			    IN uint8_t rand[16], IN uint8_t k[16], IN uint8_t autn[16], 
			    OUT AKA_Av* pAV, OUT RvBool *pbCorrectMac);
    
  };
};

END_DECL_RV_SIP_ENGINE_NAMESPACE;

#endif /*RVSIPAKA_H_*/
