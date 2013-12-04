
#include "rvtypes.h"
#include "rvaddress.h"
#include "rvalloc.h"
#include "rvlog.h"

#if (RV_OS_TYPE == RV_OS_TYPE_SOLARIS && RV_OS_VERSION >= RV_OS_SOLARIS_10)
#if (RV_IMS_IPSEC_ENABLED == RV_YES)

#include "rvimsipsecprivate.h"

#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <sys/stat.h>
#include <fcntl.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <net/pfkeyv2.h>
#include <net/pfpolicy.h>

RvStatus rvAdIpsecInit()
{
    return RV_OK;
}

RvStatus rvAdIpsecEnd()
{
    return RV_OK;
}


/********************************************************************************************
 * rvAdSetOSSpecific
 *
 * Set OS specific data for SA/SP manipulations
 *
 * INPUT:
 *    sad       - contains data needed for SA/SP creation;
 *  logMgr      - handle of the log manager for this instance 
 *
 * RETURNS:
 *  RV_OK on success, other values on failure
********************************************************************************************/
RvStatus rvAdSetOSSpecific(
    IN RvImsSAData *sad, 
    IN RvLogMgr *logMgr)
{
#define FUNCTION_NAME "rvAdSetOSSpecific"
    RvStatus retV = RV_OK;
    IMSIPSEC_LOG_ENTER;

    switch (sad->iEncrAlg)
    {
    case RvImsEncrNull:
        sad->iIpSecSpecific.iOSEncrAlg = SADB_EALG_NULL;
        break;
    case RvImsEncr3Des:
        sad->iIpSecSpecific.iOSEncrAlg = SADB_EALG_3DESCBC;
        break;
    case RvImsEncrAes:
        sad->iIpSecSpecific.iOSEncrAlg = SADB_EALG_AES;
        break;
    default:
        IMSIPSEC_LOG_ERROR_1("Unknown encryption algorithm %d",sad->iEncrAlg);
        retV = RV_ERROR_UNKNOWN;
        goto end;
    }
    switch (sad->iAuthAlg)
    {
    case RvImsAuthMd5:
        sad->iIpSecSpecific.iOSAuthAlg = SADB_AALG_MD5HMAC;
        break;
    case RvImsAuthSha1:
        sad->iIpSecSpecific.iOSAuthAlg = SADB_AALG_SHA1HMAC;
        break;        
    default:
        IMSIPSEC_LOG_ERROR_1("Unknown authentication algorithm %d",sad->iAuthAlg);
        retV = RV_ERROR_UNKNOWN;
        goto end;
    }
    
end:
    IMSIPSEC_LOG_LEAVE_0("");
    return retV;
#undef FUNCTION_NAME
}


/********************************************************************************************
 * rvSolIPSecAddDeleteKeys
 *
 * OS specific function to add/delete of security associations
 *
 * INPUT:
 *     add       - if RV_TRUE the SAs will be added, otherwise deleted
 *    sad        - array of SA data
 * startIndex       - the first SA to create in sad->iIpSecSpecific.iSAP
 * stopIndex       - the last SA to create in sad->iIpSecSpecific.iSAP
 *  logMgr      - handle of the log manager for this instance 
 * OUTPUT:
 *  saCnt       - on return will contain the number actually created SAs
 *
 * RETURNS:
 *  RV_OK on success, other values on failure
 ********************************************************************************************/
RvStatus rvSolIPSecAddDeleteKeys(
    IN RvBool add,
    IN RvImsSAData *sad, 
    IN int startIndex,
    IN int stopIndex,
    OUT int* saCnt,
    IN RvLogMgr *logMgr)
{
#define FUNCTION_NAME "rvSolIPSecAddDeleteKeys"
    RvUint16                cnt;
    RvInt32                 s,len,ret,offs;
    RvUint32                mypid;
    RvUint8                 buf[2048], *p;  /* 2048 bytes should be enough */  
    RvUint8                 bufOut[2048];
    struct sadb_msg         *msg;
    struct sadb_sa          *saext;
    struct sadb_address     *daddr, *saddr=NULL;
    struct sockaddr_in6     *pDst, *pSrc=NULL;    
    struct in6_addr         lcl,peer;
    struct sadb_key         *keyext;
    static int              saSeqNum = 1;

    IMSIPSEC_LOG_ENTER;

/*
 *	the common idea:
 *  we have to create/delete some SAs here. All of them are almost the same.
 *  Flow:
 *      1. Prepare the message with data common to all SAs.
           Keep the pointers in the message to the data items that have 
           specific values for each SA.
 *      2. Create the PFKEYV2 socket.
 *      3. For each SA: set the differing values in the message and write the
 *         message into socket.
 *      4. Test the result.
 *      5. Close the socket.  
 */

    s = -1;
    if (saCnt)
        *saCnt = 0;

    mypid = getpid();
    bzero(&buf, sizeof(buf));
    p = buf;
    msg = (struct sadb_msg *) p;
    msg->sadb_msg_version = PF_KEY_V2;
    msg->sadb_msg_type = (add) ? SADB_ADD : SADB_DELETE;
    msg->sadb_msg_satype = SADB_SATYPE_ESP; /* we know it should be ESP in transport mode */
    msg->sadb_msg_pid = mypid;    
    len = sizeof(*msg);
    p += len;

    saext = (struct sadb_sa *) p;
    saext->sadb_sa_len = sizeof(*saext) >> 3;  /* divide by 8 */
    saext->sadb_sa_exttype = SADB_EXT_SA;
    if (add)
    {
        saext->sadb_sa_state = SADB_SASTATE_MATURE;
        saext->sadb_sa_encrypt = sad->iIpSecSpecific.iOSEncrAlg;
        saext->sadb_sa_auth = sad->iIpSecSpecific.iOSAuthAlg;
    }
    len += sizeof(struct sadb_sa);
    p += sizeof(struct sadb_sa);

    if (add)
    {
        /* the keys are the same for all SAs */
        /* first set the auth key, it must be present */
        keyext = (struct sadb_key *) p;
        keyext->sadb_key_len = (sizeof(struct sadb_key) + (sad->iAuthKey.iKeyBits >> 3) + 7) >> 3;
        keyext->sadb_key_exttype = SADB_EXT_KEY_AUTH;
        keyext->sadb_key_bits = sad->iAuthKey.iKeyBits;
        memcpy(keyext + 1, sad->iAuthKey.iKeyValue, sad->iAuthKey.iKeyBits >> 3);
        offs = keyext->sadb_key_len << 3;
        len += offs;
        p += offs;

        if (sad->iEncrAlg != RvImsEncrNull)
        /* encryption key is set only if that is not null encryption */
        {
            keyext = (struct sadb_key *) p;
            keyext->sadb_key_len = (sizeof(struct sadb_key) + (sad->iEncrKey.iKeyBits >> 3) + 7) >> 3;
            keyext->sadb_key_exttype = SADB_EXT_KEY_ENCRYPT;
            keyext->sadb_key_bits = sad->iEncrKey.iKeyBits;
            memcpy(keyext + 1, sad->iEncrKey.iKeyValue, sad->iEncrKey.iKeyBits >> 3);
            offs = keyext->sadb_key_len << 3;
            len += offs;
            p += offs;
        }
    }
    
    /* the destination address set when adding AND deleting 
       the source address is needed only when adding */
    daddr = (struct sadb_address *) p;
    daddr->sadb_address_len = (sizeof(struct sadb_address) + sizeof(struct sockaddr_storage) + 7) >> 3;
    daddr->sadb_address_exttype = SADB_EXT_ADDRESS_DST;
    pDst = (struct sockaddr_in6*) (p + sizeof(struct sadb_address));    
    pDst->sin6_family = AF_INET6;
    offs = daddr->sadb_address_len << 3;
    p += offs;
    len += offs;    

    if (add)
    {
        saddr = (struct sadb_address *) p;
        saddr->sadb_address_len = (sizeof(struct sadb_address) + sizeof(struct sockaddr_storage) + 7) >> 3;
        saddr->sadb_address_exttype = SADB_EXT_ADDRESS_SRC;
        pSrc = (struct sockaddr_in6*)(p + sizeof(struct sadb_address));
        pSrc->sin6_family = AF_INET6;
        offs = saddr->sadb_address_len << 3;
        p += offs;
        len += offs;        
    }
    
    msg->sadb_msg_len = len >> 3;


#if (RV_NET_TYPE & RV_NET_IPV6)    
    if (sad->iLclAddress.addrtype == RV_ADDRESS_TYPE_IPV6)
    {
        memcpy(&lcl,sad->iLclAddress.data.ipv6.ip,sizeof(lcl));
        memcpy(&peer,sad->iPeerAddress.data.ipv6.ip,sizeof(peer));
    }
    else
#endif
    {
        struct in_addr in;
        memcpy(&in,&sad->iLclAddress.data.ipv4.ip,sizeof(in));
        IN6_INADDR_TO_V4MAPPED(&in,&lcl);
        memcpy(&in,&sad->iPeerAddress.data.ipv4.ip,sizeof(in));
        IN6_INADDR_TO_V4MAPPED(&in,&peer);
    }    

    s = socket(PF_KEY, SOCK_RAW, PF_KEY_V2);
    if (s == -1)
    {
        IMSIPSEC_LOG_ERROR_1("Failed to create PF_KEY socket, error %d",errno);
        goto failure;
    }    
    
    for (cnt = startIndex; cnt <= stopIndex; cnt++)
    {

        msg->sadb_msg_seq = saSeqNum++;
        saext->sadb_sa_spi = htonl(sad->iIpSecSpecific.iSAP[cnt].iSPI);


        if (add)
        {
            pDst->sin6_port = 0;
            pSrc->sin6_port = 0;
        }
        if (sad->iIpSecSpecific.iSAP[cnt].iDirection == RV_IPSECMODE_DIR_IN) 
        {
            memcpy(pDst->sin6_addr.s6_addr,&lcl,sizeof(lcl));
            if (add)
                memcpy(pSrc->sin6_addr.s6_addr,&peer,sizeof(peer));
        }
        else
        {
            memcpy(pDst->sin6_addr.s6_addr,&peer,sizeof(peer));
            if (add)
            {
                memcpy(pSrc->sin6_addr.s6_addr,&lcl,sizeof(lcl));
                pDst->sin6_port = htons(sad->iIpSecSpecific.iSAP[cnt].iRemPort);
                pSrc->sin6_port = htons(sad->iIpSecSpecific.iSAP[cnt].iLclPort);

                if (sad->iProto == RvImsProtoTcp)
                    saddr->sadb_address_proto = daddr->sadb_address_proto = IPPROTO_TCP;
                else if (sad->iProto == RvImsProtoUdp)
                    saddr->sadb_address_proto = daddr->sadb_address_proto = IPPROTO_UDP;
                
            }
        }

#if RV_IMS_IPSEC_DEBUG_MESSAGES
        {
            int i1,i2,i3 = 0;
            u_char* p;
            p = buf;
            for (i1 = 0; i1 < 100; i1++)
            {
                printf("%4d:",i1);
                for (i2 = 0; i2 < 16; i2++)
                {
                    printf(" %3d",*p);
                    p++;
                    i3++;
                    if (i3 == len)
                        break;
                }
                printf("\n");
                if (i3 == len)
                    break;
            }
            
        }
#endif
        ret = write(s, buf, len);   

        if (ret != len)
        {
            IMSIPSEC_LOG_ERROR_4("Failed on writing %d bytes to PF_KEY socket, ret %d, error %d, cnt %d",
                len,ret,errno,cnt);
            goto failure;
        }
        
        /* Read and print SADB_ADD reply, discarding any others */
        for ( ; ; ) {
            int     msglen;
            struct sadb_msg *msgp;
        
            msglen = read(s, bufOut, sizeof(bufOut));
            if (msglen <= 0)
            {
                IMSIPSEC_LOG_ERROR_3("Failed on reading from PF_KEY socket, ret %d, error %d, cnt %d",
                    ret,errno,cnt);
                goto failure;
            }
            
            msgp = (struct sadb_msg *) bufOut;
            if (msgp->sadb_msg_pid == mypid && msg->sadb_msg_type == msgp->sadb_msg_type) {

                if (msgp->sadb_msg_errno)
                /* if errno is set the operation failed */
                {
                    IMSIPSEC_LOG_ERROR_3("Failed to set/remove SA, error %d, diag %d, cnt %d",
                        msgp->sadb_msg_errno,msgp->sadb_x_msg_diagnostic,cnt);
                    goto failure;
                }
                if (saCnt)
                    (*saCnt) ++;
                break;
            }
        }
    }
    
    close(s);
    
    IMSIPSEC_LOG_LEAVE_0("Success");
    return RV_OK;

failure:
    if (s != -1)
        close(s);

    IMSIPSEC_LOG_LEAVE_0("Failure");
    return RV_ERROR_UNKNOWN;

#undef FUNCTION_NAME
}

/********************************************************************************************
 * rvSolIPSecAddDeleteConf
 *
 * OS specific function to add/delete 'sapdNum' of security policies.
 *
 * INPUT:
 *     add       - if RV_TRUE the SPs will be added, otherwise deleted
 *    sapd       - array of SA data
 * startIndex       - the first SP to create in sad->iIpSecSpecific.iSAP
 * stopIndex       - the last SP to create in sad->iIpSecSpecific.iSAP
 *  logMgr      - handle of the log manager for this instance 
 *
 * OUTPUT:
 *  spCnt       - on return will contain the number actually created SAs
 *
 * RETURNS:
 *  RV_OK on success, other values on failure
 ********************************************************************************************/
RvStatus rvSolIPSecAddDeleteConf(
    IN RvBool add,
    IN RvImsSAData *sad, 
    IN int startIndex,
    IN int stopIndex,
    OUT int* spCnt,
    IN RvLogMgr *logMgr)
{
#define FUNCTION_NAME "rvSolIPSecAddDeleteConf"
    RvUint16                cnt;
    RvInt32                 s,len,actLen,ret,addrSz;
    RvUint32                mypid;
    RvUint8                 buf[2048], *p;  /* 2048 bytes should be enough */  
    RvUint8                 bufOut[2048];
    struct spd_msg          *msg;
    struct spd_rule         *rule;
    struct spd_portrange    *lcl_ports=NULL, *rem_ports=NULL;
    struct spd_address      *address, *prv_address;
    struct spd_ext_actions  *actions;
    struct spd_attribute    *attr, *sa_attr=NULL;
    struct spd_proto        *spd_proto;
    static int              spSeqNum = 1;
    RvIPSecSAData           *sapd;
#define	ESP_POLICY_BOOST        0x00400000
#define	INITIAL_BASE_PRIORITY   0x000fffff
    static unsigned int rule_priority = ESP_POLICY_BOOST | INITIAL_BASE_PRIORITY;
    
    IMSIPSEC_LOG_ENTER;


   /* 
        The general flow is just as in rvSolIPSecAddDeleteKeys.
        Create the message containg data common to all SPs
        Create the socket;
        Modify the message for specific SP and write the message.
    */

    s = -1;
    if (spCnt)
        *spCnt = 0;

    mypid = getpid();
    bzero(&buf, sizeof(buf));
    p = buf;
    msg = (struct spd_msg *) p;
    msg->spd_msg_version = PF_POLICY_V1;
    msg->spd_msg_type = (add) ?  SPD_ADDRULE : SPD_DELETERULE;
    msg->spd_msg_spdid = SPD_ACTIVE; /*SPD_STANDBY;*/
    msg->spd_msg_pid = mypid;
    len = sizeof(*msg);
    p += len;

    rule = (struct spd_rule *) p;
    rule->spd_rule_len = sizeof(struct spd_rule) >> 3;
    rule->spd_rule_type = SPD_EXT_RULE; 
    if (add)
        rule->spd_rule_priority = rule_priority --;
    len += sizeof(struct spd_rule);
    p += sizeof(struct spd_rule);

    if (add)
    {

        if (sad->iProto == RvImsProtoTcp || sad->iProto == RvImsProtoUdp)
        {
            spd_proto = (struct spd_proto *) p;
            spd_proto->spd_proto_len = sizeof(struct spd_proto) >> 3;
            spd_proto->spd_proto_exttype = SPD_EXT_PROTO;
            spd_proto->spd_proto_number = (sad->iProto == RvImsProtoTcp) ? IPPROTO_TCP : IPPROTO_UDP;
            len += sizeof(struct spd_proto);
            p += sizeof(struct spd_proto);        
        }

        lcl_ports = (struct spd_portrange *) p;
        lcl_ports->spd_ports_len = sizeof(struct spd_portrange) >> 3;
        lcl_ports->spd_ports_exttype = SPD_EXT_LCLPORT; 
        len += sizeof(struct spd_portrange);
        p += sizeof(struct spd_portrange);
    
        rem_ports = (struct spd_portrange *) p;
        rem_ports->spd_ports_len = sizeof(struct spd_portrange) >> 3;
        rem_ports->spd_ports_exttype = SPD_EXT_REMPORT; 
        len += sizeof(struct spd_portrange);
        p += sizeof(struct spd_portrange);
    
        address = (struct spd_address *) p;
#if (RV_NET_TYPE & RV_NET_IPV6)    
        if (sad->iPeerAddress.addrtype == RV_ADDRESS_TYPE_IPV6)
        {
            addrSz = 16;
            address->spd_address_af = AF_INET6; 
        }
        else
#endif
        {
            addrSz = 4;
            address->spd_address_af = AF_INET; 
        }    
    
        address->spd_address_len = (sizeof(struct spd_address) + addrSz + 7) >> 3 ;
        address->spd_address_exttype = SPD_EXT_LCLADDR; 
        address->spd_address_prefixlen = addrSz << 3; 
        memcpy(address + 1,&sad->iLclAddress.data,addrSz);    
        len += address->spd_address_len << 3;
        p += address->spd_address_len << 3;
    
        prv_address = address;
        address = (struct spd_address *) p;
        memcpy(address,prv_address,sizeof(struct spd_address));
        address->spd_address_exttype = SPD_EXT_REMADDR; 
        memcpy(address + 1,&sad->iPeerAddress.data,addrSz);
        len += address->spd_address_len << 3;
        p += address->spd_address_len << 3;


        actions = (struct spd_ext_actions *) p;
        actions->spd_actions_exttype = SPD_EXT_ACTION; 
        actions->spd_actions_count  = 1;
        actLen = len;
        len += sizeof(*actions);
        p += sizeof(*actions);
    
    
        attr = (struct spd_attribute*) p;
        attr->spd_attr_tag = SPD_ATTR_EMPTY;
        len += sizeof(*attr);
        p += sizeof(*attr);
    
        attr = (struct spd_attribute*) p;
        attr->spd_attr_tag = SPD_ATTR_TYPE;
        attr->spd_attr_value = SPD_ACTTYPE_IPSEC;
        len += sizeof(*attr);
        p += sizeof(*attr);
    
    
        attr = (struct spd_attribute*) p;
        sa_attr = attr;
        attr->spd_attr_tag = SPD_ATTR_FLAGS;
        attr->spd_attr_value = SPD_APPLY_ESPA | SPD_APPLY_ESP;
        len += sizeof(*attr);
        p += sizeof(*attr);

        attr = (struct spd_attribute*) p;
        attr->spd_attr_tag = SPD_ATTR_ESP_ENCR;
        attr->spd_attr_value = sad->iIpSecSpecific.iOSEncrAlg;
        len += sizeof(*attr);
        p += sizeof(*attr);

/*
        attr = (struct spd_attribute*) p;
        attr->spd_attr_tag = SPD_ATTR_ENCR_MINBITS;
        attr->spd_attr_value = minbits;
        len += sizeof(*attr);
        p += sizeof(*attr);
*/

        attr = (struct spd_attribute*) p;
        attr->spd_attr_tag = SPD_ATTR_ESP_AUTH;
        attr->spd_attr_value = sad->iIpSecSpecific.iOSAuthAlg;
        len += sizeof(*attr);
        p += sizeof(*attr);    
        
/*
        attr = (struct spd_attribute*) p;
        attr->spd_attr_tag = SPD_ATTR_AUTH_MINBITS;
        attr->spd_attr_value = minbits;
        len += sizeof(*attr);
        p += sizeof(*attr);
*/

    
        attr = (struct spd_attribute*) p;
        attr->spd_attr_tag = SPD_ATTR_END;
        len += sizeof(*attr);
        p += sizeof(*attr);
        actLen = len - actLen;
        actions->spd_actions_len = actLen/8 ;
    }
    
    msg->spd_msg_len = len >> 3;


    s = socket(PF_POLICY, SOCK_RAW, PF_POLICY_V1);
    if (s == -1)
    {
        IMSIPSEC_LOG_ERROR_1("Failed to create PF_KEY socket, error %d",errno);
        goto failure;
    }    
    
    sapd = sad->iIpSecSpecific.iSAP+startIndex;
    for (cnt = startIndex; cnt <= stopIndex; cnt++, sapd++)
    {

        if (add)
        {
            rule->spd_rule_flags = (sapd->iDirection == RV_IPSECMODE_DIR_IN) 
                        ? SPD_RULE_FLAG_INBOUND : SPD_RULE_FLAG_OUTBOUND;
            lcl_ports->spd_ports_minport = lcl_ports->spd_ports_maxport = htons(sapd->iLclPort);
            rem_ports->spd_ports_minport = rem_ports->spd_ports_maxport = htons(sapd->iRemPort);
            sa_attr->spd_attr_value = SPD_APPLY_ESPA | SPD_APPLY_ESP;

            /* we set the UNIQUE flag only if the IP PROTO is one of TCP or UDP
               and is not ANY */
            if (sapd->iDirection == RV_IPSECMODE_DIR_OUT &&
                (sad->iProto == RvImsProtoTcp || sad->iProto == RvImsProtoUdp))
                sa_attr->spd_attr_value |= SPD_APPLY_UNIQUE;
        }
        else
            /* when deleting SP set the index that was saved when the SA was created */
            rule->spd_rule_index = sapd->iSPDIndex;
                
        msg->spd_msg_seq = spSeqNum++;

#if RV_IMS_IPSEC_DEBUG_MESSAGES
        {
            int i1,i2,i3 = 0;
            u_char* p;
            p = buf;
            for (i1 = 0; i1 < 100; i1++)
            {
                printf("%4d:",i1);
                for (i2 = 0; i2 < 16; i2++)
                {
                    printf(" %3d",*p);
                    p++;
                    i3++;
                    if (i3 == len)
                        break;
                }
                printf("\n");
                if (i3 == len)
                    break;
            }
            
        }
#endif
        ret = write(s, buf, len);   

        if (ret != len)
        {
            IMSIPSEC_LOG_ERROR_4("Failed on writing %d bytes to PF_KEY socket, ret %d, error %d, cnt %d",
                len,ret,errno,cnt);
            goto failure;
        }
        
        /* Read and print SADB_ADD reply, discarding any others */
        for ( ; ; ) {
            u_int     msglen;
            struct spd_msg *msgp;
            struct spd_rule *rulep;
        
            msglen = read(s, bufOut, sizeof(bufOut));
            if (msglen <= 0)
            {
                IMSIPSEC_LOG_ERROR_3("Failed on reading from PF_KEY socket, ret %d, error %d, cnt %d",
                    ret,errno,cnt);
                goto failure;
            }
            
            msgp = (struct spd_msg *) bufOut;
            if (msgp->spd_msg_pid == mypid && msg->spd_msg_type == msgp->spd_msg_type) {

                if (msgp->spd_msg_errno) 
                /* if errno is not zero the operation failed */
                {
                    IMSIPSEC_LOG_ERROR_3("Failed to set/remove SP, error %d, daignostics %d, cnt %d",
                        msgp->spd_msg_errno,msgp->spd_msg_diagnostic,cnt);
                    goto failure;
                }

                if (add)
                {
                    /* fetch the rule index */
                    if (msglen < sizeof(struct spd_msg) + sizeof(struct spd_rule))
                    {
                        IMSIPSEC_LOG_ERROR_2("Improper size of received message %d, cnt %d",
                            msglen,cnt);
                        goto failure;
                    }
                    rulep = (struct spd_rule*) (bufOut + sizeof(struct spd_msg));
                    if (rulep->spd_rule_type != SPD_EXT_RULE)
                    {
                        IMSIPSEC_LOG_ERROR_1("Can't find the rule, cnt %d",cnt);
                        goto failure;
                    }
                    /* save the rule index we'll need to delete the rule */
                    sapd->iSPDIndex = rulep->spd_rule_index;
                }

                if (spCnt)
                    (*spCnt) ++;
                break;
            }
        }
    }
    
    close(s);
    
    IMSIPSEC_LOG_LEAVE_0("Success");
    return RV_OK;

failure:
    if (s != -1)
        close(s);

    IMSIPSEC_LOG_LEAVE_0("Failure");
    return RV_ERROR_UNKNOWN;

#undef FUNCTION_NAME
}

/********************************************************************************************
 * rvAdIPSecAddSASP
 *
 * OS specific function to add 'sapdNum' of security associations.
 *
 * INPUT:
 *    sad       - array of SA data
 * sapdNum       - number of elements in 'sapd'
 *  logMgr      - handle of the log manager for this instance 
 *
 * RETURNS:
 *  RV_OK on success, other values on failure
 ********************************************************************************************/
RvStatus rvAdIPSecAddSASP(
    IN RvImsSAData *sad, 
    IN int sapdNum, 
    IN RvLogMgr *logMgr)
{
#define FUNCTION_NAME "rvAdIPSecAddSASP"
    RvStatus ret;    
    int cnt;
    RV_UNUSED_ARG(sapdNum);

    IMSIPSEC_LOG_ENTER;

    ret = rvSolIPSecAddDeleteKeys(RV_TRUE,sad,0,3,&cnt,logMgr);
    if (ret != RV_OK)
    {
        if (cnt)
            rvSolIPSecAddDeleteKeys(RV_FALSE,sad,0,cnt-1,NULL,logMgr);
        goto end;
    }

    ret = rvSolIPSecAddDeleteConf(RV_TRUE,sad,0,3,&cnt,logMgr);
    if (ret != RV_OK)
    {
        rvSolIPSecAddDeleteKeys(RV_FALSE,sad,0,3,NULL,logMgr);
        if (cnt)
            rvSolIPSecAddDeleteConf(RV_FALSE,sad,0,cnt-1,NULL,logMgr);
    }

end:
    IMSIPSEC_LOG_LEAVE_1("Completed with %s",(ret == RV_OK) ? "success" : "failure");
    return ret;
#undef FUNCTION_NAME
}

/********************************************************************************************
 * rvAdIPSecDeleteSASP
 *
 * OS specific function to delete 'sapdNum' of security associations.
 *
 * INPUT:
 *    sad       - array of SA data
 * sapdNum       - number of elements in 'sapd'
 *  logMgr      - handle of the log manager for this instance 
 *
 * RETURNS:
 *  RV_OK on success, other values on failure
 ********************************************************************************************/
RvStatus rvAdIPSecDeleteSASP(
    IN RvImsSAData *sad, 
    IN int sapdNum, 
    IN RvLogMgr *logMgr)
{
#define FUNCTION_NAME "rvAdIPSecDeleteSASP"
    RvStatus ret;    
    int cnt;
    RV_UNUSED_ARG(sapdNum);

    IMSIPSEC_LOG_ENTER;
    ret = rvSolIPSecAddDeleteKeys(RV_FALSE,sad,0,3,&cnt,logMgr);    
    ret = rvSolIPSecAddDeleteConf(RV_FALSE,sad,0,3,&cnt,logMgr);
    IMSIPSEC_LOG_LEAVE_1("Completed with %s",(ret == RV_OK) ? "success" : "failure");
    return ret;
#undef FUNCTION_NAME
}


/********************************************************************************************
 * rvAdIPSecCleanAll
 *
 * Removes all SA/SP set on this host.
 *
 * INPUT:
 *  logMgr      - handle of the log manager for this instance 
 *
 * RETURNS:
 *  RV_OK on success, other values on failure
********************************************************************************************/
RvStatus rvAdIPSecCleanAll(
    IN RvLogMgr *logMgr)
{
#define FUNCTION_NAME "rvAdIPSecCleanAll"
    RvStatus                sts;
    RvInt32                 s,len,ret,cnt;
    struct sadb_msg         msg;
    struct spd_msg          spdMsg;
    void                    *p;
    RvUint8                 buff[2048];
    
    IMSIPSEC_LOG_ENTER;

    sts = RV_OK;

    /* have to send  two messages
       one to flush all SAs the second to flush SPs
     */

    for (cnt = 0; cnt < 2; cnt ++)
    {

        if (cnt == 0)
        {
            bzero(&msg, sizeof(msg));
            msg.sadb_msg_version = PF_KEY_V2;
            msg.sadb_msg_satype = SADB_SATYPE_ESP;
            msg.sadb_msg_pid = getpid();    
            len = sizeof(msg);    
            msg.sadb_msg_type = SADB_FLUSH;
            msg.sadb_msg_len = len >> 3;
            s = socket(PF_KEY, SOCK_RAW, PF_KEY_V2);
            p = &msg;
        }
        else
        {
            bzero(&spdMsg, sizeof(spdMsg));
            spdMsg.spd_msg_version = PF_POLICY_V1;
            spdMsg.spd_msg_type = SPD_FLUSH;
            spdMsg.spd_msg_spdid = SPD_ACTIVE;
            spdMsg.spd_msg_pid = getpid();
            len = sizeof(spdMsg);
            spdMsg.spd_msg_len = len >> 3;            
            s = socket(PF_POLICY, SOCK_RAW, PF_POLICY_V1);
            p = &spdMsg;
        }
    
        if (s == -1)
        {
            IMSIPSEC_LOG_ERROR_1("Failed to create PF_KEY socket, error %d",errno);
            sts = RV_ERROR_UNKNOWN;        
            continue;
        }    
            
        ret = write(s, p, len);

        if (ret != len)
        {
            IMSIPSEC_LOG_ERROR_3("Failed on writing %d bytes to PF_KEY socket, ret %d, error %d",
                len,ret,errno);
            sts = RV_ERROR_UNKNOWN;        
            goto next;
        }
    
        /* Read and print SADB_ADD reply, discarding any others */
        for ( ; ; ) 
        {
            int     msglen;
            struct sadb_msg *sa_msg;
            struct spd_msg *sp_msg;

        
            msglen = read(s, buff, sizeof(buff));
            if (msglen <= 0)
            {
                IMSIPSEC_LOG_ERROR_2("Failed on reading from PF_KEY socket, ret %d, error %d",
                    ret,errno);
                sts = RV_ERROR_UNKNOWN;        
                goto next;
            }

            if (cnt == 0)
            {
                sa_msg = (struct sadb_msg*)buff;
                if (sa_msg->sadb_msg_pid == msg.sadb_msg_pid && msg.sadb_msg_type == sa_msg->sadb_msg_type) {
            
                    if (sa_msg->sadb_msg_errno && sa_msg->sadb_msg_errno != ENOENT)
                    {
                        IMSIPSEC_LOG_ERROR_2("Failed to dump SA/SP, error %d, diagnostics %d",
                            sa_msg->sadb_msg_errno,sa_msg->sadb_x_msg_diagnostic);
                        sts = RV_ERROR_UNKNOWN;        
                    }
                    break;
                }
            }
            else
            {
                sp_msg = (struct spd_msg*)buff;
                if (sp_msg->spd_msg_pid == spdMsg.spd_msg_pid) {
                    
                    if (sp_msg->spd_msg_errno && sp_msg->spd_msg_errno != ENOENT)
                    {
                        IMSIPSEC_LOG_ERROR_1("Failed to dump SA/SP, error %d",
                            sp_msg->spd_msg_errno);
                        sts = RV_ERROR_UNKNOWN;        
                    }
                    break;
                }
            }
        }
next:
        close(s);
        
    }
    
    IMSIPSEC_LOG_LEAVE_1("Completed with %s",(sts == RV_OK) ? "success" : "failure");
    return sts;
#undef FUNCTION_NAME
}


#endif /*(RV_IMS_IPSEC_ENABLED == RV_YES) */

#else
int prevent_warning_of_ranlib_has_no_symbols_rvsolarisipsec=0;
#endif /*(RV_OS_TYPE == RV_OS_TYPE_SOLARIS && RV_OS_VERSION >= RV_OS_SOLARIS_10)*/
