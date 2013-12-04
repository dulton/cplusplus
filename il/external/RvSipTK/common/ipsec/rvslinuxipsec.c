#include "rvtypes.h"
#include "rvaddress.h"
#include "rvalloc.h"
#include "rvlog.h"
#include "rvlock.h"

#if (RV_OS_TYPE == RV_OS_TYPE_NETBSD || \
     RV_OS_TYPE == RV_OS_TYPE_LINUX && (RV_OS_VERSION&RV_OS_LINUX_REDHAT || \
                                        RV_OS_VERSION == RV_OS_LINUX_MVISTA_4_0 || RV_OS_VERSION == RV_OS_LINUX_MVISTA_5_0 || \
                                        RV_OS_VERSION&RV_OS_LINUX_SUSE))

#if (RV_IMS_IPSEC_ENABLED == RV_YES)

#include "rvimsipsecprivate.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <strings.h>
#include <string.h>
#include <stdio.h>

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
#include <sys/ioctl.h>
#endif 
/* SPIRENT_END */


#define POLICY_PRIORITY_SUPPORTED 0

#if (RV_OS_TYPE == RV_OS_TYPE_NETBSD)
#include <netinet6/ipsec.h> 
#include <net/pfkeyv2.h> 
#else
#include <linux/pfkeyv2.h> 
#include <linux/ipsec.h> 
#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,6)
#undef POLICY_PRIORITY_SUPPORTED
#define POLICY_PRIORITY_SUPPORTED 1
#define DEFAULT_PRIORITY 0x80000000
#endif

#endif

static RvLock gsLinuxSPI_ID_Lock;
#if defined(UPDATED_BY_SPIRENT)
#define RV_LINUX_IPSEC_MAX_SA_ID 0x3FFF
#else
#define RV_LINUX_IPSEC_MAX_SA_ID 0x7FFF
#endif
static RvChar  gsLinuxSPI_ID_Arr[(RV_LINUX_IPSEC_MAX_SA_ID+7)/8];

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT_ABACUS)

#include "rvexternal.h"

#if defined(__ABACUS_DEBUG__)

#undef IMSIPSEC_LOG_ENTER_0
#undef IMSIPSEC_LOG_ENTER_1
#undef IMSIPSEC_LOG_ENTER_2
#undef IMSIPSEC_LOG_ENTER_3
#undef IMSIPSEC_LOG_ENTER

#undef IMSIPSEC_LOG_LEAVE_0
#undef IMSIPSEC_LOG_LEAVE_1
#undef IMSIPSEC_LOG_LEAVE_2
#undef IMSIPSEC_LOG_LEAVE_3
#undef IMSIPSEC_LOG_LEAVE_4
#undef IMSIPSEC_LOG_LEAVE

#undef IMSIPSEC_LOG_ERROR_0
#undef IMSIPSEC_LOG_ERROR_1
#undef IMSIPSEC_LOG_ERROR_2
#undef IMSIPSEC_LOG_ERROR_3
#undef IMSIPSEC_LOG_ERROR_4
#undef IMSIPSEC_LOG_ERROR

#undef IMSIPSEC_LOG_INFO_0
#undef IMSIPSEC_LOG_INFO_1
#undef IMSIPSEC_LOG_INFO_2
#undef IMSIPSEC_LOG_INFO_3
#undef IMSIPSEC_LOG_INFO_4
#undef IMSIPSEC_LOG_INFO_5
#undef IMSIPSEC_LOG_INFO_6
#undef IMSIPSEC_LOG_INFO_7
#undef IMSIPSEC_LOG_INFO

#undef IMSIPSEC_LOG_DEBUG_0
#undef IMSIPSEC_LOG_DEBUG_1
#undef IMSIPSEC_LOG_DEBUG_2
#undef IMSIPSEC_LOG_DEBUG_3
#undef IMSIPSEC_LOG_DEBUG

#undef IMSIPSEC_LOG_WARN_0
#undef IMSIPSEC_LOG_WARN_1
#undef IMSIPSEC_LOG_WARN_2
#undef IMSIPSEC_LOG_WARN_3
#undef IMSIPSEC_LOG_WARN

#define IMSIPSEC_LOG_ENTER_0(f) {dprintf(FUNCTION_NAME ":Enter: " f);dprintf("\n");}
#define IMSIPSEC_LOG_ENTER_1(f, p1) {dprintf(FUNCTION_NAME ":Enter: " f, p1);dprintf("\n");}
#define IMSIPSEC_LOG_ENTER_2(f, p1, p2) {dprintf(FUNCTION_NAME ":Enter: " f, p1, p2);dprintf("\n");}
#define IMSIPSEC_LOG_ENTER_3(f, p1, p2, p3) {dprintf(FUNCTION_NAME ":Enter: " f, p1, p2, p3);dprintf("\n");}
#define IMSIPSEC_LOG_ENTER      IMSIPSEC_LOG_ENTER_0("")

#define IMSIPSEC_LOG_LEAVE_0(f) {dprintf(FUNCTION_NAME ":Leave: " f);dprintf("\n");}
#define IMSIPSEC_LOG_LEAVE_1(f, p1) {dprintf(FUNCTION_NAME ":Leave: " f, p1);dprintf("\n");}
#define IMSIPSEC_LOG_LEAVE_2(f, p1, p2) {dprintf(FUNCTION_NAME ":Leave: " f, p1, p2);dprintf("\n");}
#define IMSIPSEC_LOG_LEAVE_3(f, p1, p2, p3) {dprintf(FUNCTION_NAME ":Leave: " f, p1, p2, p3);dprintf("\n");}
#define IMSIPSEC_LOG_LEAVE_4(f, p1, p2, p3, p4) {dprintf(FUNCTION_NAME ":Leave: " f, p1, p2, p3, p4);dprintf("\n");}
#define IMSIPSEC_LOG_LEAVE      IMSIPSEC_LOG_LEAVE_0("")

#define IMSIPSEC_LOG_ERROR_0(f) {dprintf(FUNCTION_NAME ":Error: " f);dprintf("\n");}
#define IMSIPSEC_LOG_ERROR_1(f, p1) {dprintf(FUNCTION_NAME ":Error: " f, p1);dprintf("\n");}
#define IMSIPSEC_LOG_ERROR_2(f, p1, p2) {dprintf(FUNCTION_NAME ":Error: " f, p1, p2);dprintf("\n");}
#define IMSIPSEC_LOG_ERROR_3(f, p1, p2, p3) {dprintf(FUNCTION_NAME ":Error: " f, p1, p2, p3);dprintf("\n");}
#define IMSIPSEC_LOG_ERROR_4(f, p1, p2, p3, p4) {dprintf(FUNCTION_NAME ":Error: " f, p1, p2, p3, p4);dprintf("\n");}
#define IMSIPSEC_LOG_ERROR      IMSIPSEC_LOG_ERROR_0("")

#define IMSIPSEC_LOG_INFO_0(f) {dprintf(FUNCTION_NAME ":Info: " f);dprintf("\n");}
#define IMSIPSEC_LOG_INFO_1(f, p1) {dprintf(FUNCTION_NAME ":Info: " f, p1);dprintf("\n");}
#define IMSIPSEC_LOG_INFO_2(f, p1, p2) {dprintf(FUNCTION_NAME ":Info: " f, p1, p2);dprintf("\n");}
#define IMSIPSEC_LOG_INFO_3(f, p1, p2, p3) {dprintf(FUNCTION_NAME ":Info: " f, p1, p2, p3);dprintf("\n");}
#define IMSIPSEC_LOG_INFO_4(f, p1, p2, p3, p4) {dprintf(FUNCTION_NAME ":Info: " f, p1, p2, p3, p4);dprintf("\n");}
#define IMSIPSEC_LOG_INFO_5(f, p1, p2, p3, p4, p5) {dprintf(FUNCTION_NAME ":Info: " f, p1, p2, p3, p4, p5);dprintf("\n");}
#define IMSIPSEC_LOG_INFO_6(f, p1, p2, p3, p4, p5, p6) {dprintf(FUNCTION_NAME ":Info: " f, p1, p2, p3, p4, p5, p6);dprintf("\n");}
#define IMSIPSEC_LOG_INFO_7(f, p1, p2, p3, p4, p5, p6, p7) {dprintf(FUNCTION_NAME ":Info: " f, p1, p2, p3, p4, p5, p6, p7);dprintf("\n");}
#define IMSIPSEC_LOG_INFO      IMSIPSEC_LOG_INFO_0("")

#define IMSIPSEC_LOG_DEBUG_0(f) {dprintf(FUNCTION_NAME ":Debug: " f);dprintf("\n");}
#define IMSIPSEC_LOG_DEBUG_1(f, p1) {dprintf(FUNCTION_NAME ":Debug: " f, p1);dprintf("\n");}
#define IMSIPSEC_LOG_DEBUG_2(f, p1, p2) {dprintf(FUNCTION_NAME ":Debug: " f, p1, p2);dprintf("\n");}
#define IMSIPSEC_LOG_DEBUG_3(f, p1, p2, p3) {dprintf(FUNCTION_NAME ":Debug: " f, p1, p2, p3);dprintf("\n");}
#define IMSIPSEC_LOG_DEBUG      IMSIPSEC_LOG_DEBUG_0("")

#define IMSIPSEC_LOG_WARN_0(f) {dprintf(FUNCTION_NAME ":Warn: " f);dprintf("\n");}
#define IMSIPSEC_LOG_WARN_1(f, p1) {dprintf(FUNCTION_NAME ":Warn: " f, p1);dprintf("\n");}
#define IMSIPSEC_LOG_WARN_2(f, p1, p2) {dprintf(FUNCTION_NAME ":Warn: " f, p1, p2);dprintf("\n");}
#define IMSIPSEC_LOG_WARN_3(f, p1, p2, p3) {dprintf(FUNCTION_NAME ":Warn: " f, p1, p2, p3);dprintf("\n");}
#define IMSIPSEC_LOG_WARN      IMSIPSEC_LOG_WARN_0("")

#endif //__ABACUS_DEBUG__

#endif //SPIRENT

RvInt32 pfkey_open()
{
    RvInt32 s;
    s = socket(PF_KEY, SOCK_RAW, PF_KEY_V2);

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
#if defined(__ABACUS_DEBUG__)
#define FUNCTION_NAME "pfkey_open"
    IMSIPSEC_LOG_ENTER;
#endif
#endif
/* SPIRENT_END */

#if (RV_OS_TYPE == RV_OS_TYPE_NETBSD)
    if (s >= 0)
    {
        const int bufsiz = 128 * 1024;
	    (void)setsockopt(s, SOL_SOCKET, SO_SNDBUF, &bufsiz, sizeof(bufsiz));
	    (void)setsockopt(s, SOL_SOCKET, SO_RCVBUF, &bufsiz, sizeof(bufsiz));
    }
#endif

    /* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    if (s >= 0)
    {
      RvUint32 enableNonBlockingIO = RV_TRUE;
      const int bufsiz = 256 * 1024;
      setsockopt(s, SOL_SOCKET, SO_SNDBUF, &bufsiz, sizeof(bufsiz));
      setsockopt(s, SOL_SOCKET, SO_RCVBUF, &bufsiz, sizeof(bufsiz));

      // set this socket to use NON-BLOCKING IO
      ioctl(s, FIONBIO, &enableNonBlockingIO);
    }
#if defined(__ABACUS_DEBUG__)
    IMSIPSEC_LOG_LEAVE;

#undef FUNCTION_NAME
#endif
#endif
    /* SPIRENT_END */
        
    return s;
}

#if RV_IMS_IPSEC_DEBUG_MESSAGES

void verbose(struct sadb_msg *msg, int len, RvBool msgOnly)
{
    int mlen;
    struct sadb_ext* saext;
    struct sadb_key* keyext;
    struct sadb_sa* sa;
    struct sadb_x_sa2 *sa2;
    struct sadb_address *addr;
    unsigned char *p;
    char *b = (char*) msg;
    struct sadb_x_policy    *policy;
    /*struct sadb_x_ipsecrequest *polreq;*/
    

    printf("will be sending %d bytes\n",len);

    printf("       vers %d\n",msg->sadb_msg_version);
    printf("       type %d (ADD 3, DELETE 4, FLUSH 9, XADD 14, XDELETE2 22, XFLUSH 19\n",msg->sadb_msg_type);
    printf("      errno %d\n",msg->sadb_msg_errno);
    printf("     satype %d\n",msg->sadb_msg_satype);
    printf("        len %d\n",msg->sadb_msg_len);
    printf("        res %d\n",msg->sadb_msg_reserved);
    printf("        seq %d\n",msg->sadb_msg_seq);
    printf("        pid %d\n",msg->sadb_msg_pid);

    if (msgOnly)
        return;
    
    mlen = sizeof(*msg);
    
    while (mlen < len)
    {
        saext = (struct sadb_ext*)(b+mlen);
        printf("\ntype %d len %d\n",saext->sadb_ext_type,saext->sadb_ext_len);
        
        switch (saext->sadb_ext_type)
        {
        case SADB_EXT_KEY_ENCRYPT:
        case SADB_EXT_KEY_AUTH:
            keyext = (struct sadb_key*) saext;
            printf("       ENCRYPT/AUTH keys bits %d res %d key %s\n",
                keyext->sadb_key_bits,keyext->sadb_key_reserved,(b+mlen+sizeof(*keyext)));
            break;
            
        case SADB_EXT_SA:
            sa = (struct sadb_sa*) saext;
            printf("      EXT_SA  spi %u replay %d state %d auth %d encrypt %d flags %d\n",
                (unsigned int)ntohl(sa->sadb_sa_spi),sa->sadb_sa_replay,sa->sadb_sa_state,sa->sadb_sa_auth,
                sa->sadb_sa_encrypt,sa->sadb_sa_flags);
            break;
            
        case SADB_X_EXT_SA2:
            sa2 = (struct sadb_x_sa2*) saext;
            printf("     EXT_SA2   mode %d res1 %d res2 %d seq %u reqid %d\n",
                sa2->sadb_x_sa2_mode,sa2->sadb_x_sa2_reserved1,sa2->sadb_x_sa2_reserved2,
                ntohl(sa2->sadb_x_sa2_sequence),sa2->sadb_x_sa2_reqid);
            break;
            
        case SADB_EXT_ADDRESS_SRC:
        case SADB_EXT_ADDRESS_DST:
            addr = (struct sadb_address*) saext;
            printf("     SRC/DST   proto %d pref %d res %d \n",
                addr->sadb_address_proto,addr->sadb_address_prefixlen,addr->sadb_address_reserved);
            p = b+mlen+sizeof(*addr);
            printf(" %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d\n",
                p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7],p[8],p[9],p[10],p[11],p[12],p[13],p[14],p[15]);
            break;

        case SADB_X_EXT_POLICY:

            policy = (struct sadb_x_policy*) saext;
            printf("     POLICY   exttype %d type %d dir %d id %d priority %d\n",
                policy->sadb_x_policy_exttype,policy->sadb_x_policy_type,policy->sadb_x_policy_dir,
                policy->sadb_x_policy_id,policy->sadb_x_policy_priority);
            break;
            
        }
        
        mlen += saext->sadb_ext_len*8;
    }    
}
#endif

RvInt pfkey_send(RvInt32 s, RvChar* b, RvUint32 len)
{
    RvInt ret;        

    /* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
#if defined(__ABACUS_DEBUG__)
#define FUNCTION_NAME "pfkey_send"
    IMSIPSEC_LOG_ENTER;
#endif
#endif
/* SPIRENT_END */

    #if (RV_OS_TYPE == RV_OS_TYPE_NETBSD)        
    ret = send(s, (void*) b, len, 0);
    #else
    /* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    {
      RvUint32 written=0;
      ret=0;
      while(written<len) {
        do {
          ret = write(s, (void*)(b + written), (len-written));
        } while(ret==-1 && errno==EINTR);
        if(ret<0) break;
        written+=ret;
      }
      if(ret>=0) ret=written;
    }
#else
    ret = write(s, (void*) b, len);
#endif
    /* SPIRENT_END */
    #endif

    /* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
#if defined(__ABACUS_DEBUG__)
    IMSIPSEC_LOG_LEAVE_1("ret=%d",ret);

#undef FUNCTION_NAME
#endif
#endif
    /* SPIRENT_END */

    return ret;
}


#if (RV_OS_TYPE == RV_OS_TYPE_NETBSD)
RvStatus rvSendSadbMsg(
    RvUint32 s,
    RvUint8 msgType,
    IN RvLogMgr *logMgr)
{
#define FUNCTION_NAME "rvSendSadbMsg"
    RvStatus                sts;
    RvInt32                 len,ret;
    struct sadb_msg         msg;
    
    IMSIPSEC_LOG_ENTER;

    bzero(&msg, sizeof(msg));
    msg.sadb_msg_version = PF_KEY_V2;
    msg.sadb_msg_satype = 0;
    msg.sadb_msg_pid = getpid();    
    len = sizeof(msg);
    msg.sadb_msg_len = len >> 3;
    sts = RV_OK;
        
    msg.sadb_msg_seq = 0;
    msg.sadb_msg_type = msgType;

    ret = pfkey_send(s, (RvChar*)&msg, len);

    if (ret != len)
    {
        IMSIPSEC_LOG_ERROR_3("Failed on writing %d bytes to PF_KEY socket, ret %d, error %d",
            len,ret,errno);
        sts = RV_ERROR_UNKNOWN;        
        goto end;
    }
        
    /* Read and print SADB_ADD reply, discarding any others */
    for ( ; ; ) 
    {
        int     msglen;
        struct sadb_msg msgR;

        /* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
            do {
              msglen = read(s, (void*)&msgR, sizeof(msgR));
            } while(msglen==-1 && errno==EINTR);
#else
            msglen = read(s, (void*)&msgR, sizeof(msgR));
#endif
/* SPIRENT_END */

        if (msglen <= 0)
        {
            IMSIPSEC_LOG_ERROR_2("Failed on reading from PF_KEY socket, ret %d, error %d",
                ret,errno);
            sts = RV_ERROR_UNKNOWN;        
            goto end;
        }
    
        if (msgR.sadb_msg_pid == msg.sadb_msg_pid && msg.sadb_msg_type == msgR.sadb_msg_type) {

            if (msgR.sadb_msg_errno && msgR.sadb_msg_errno != ENOENT)
            {
                IMSIPSEC_LOG_ERROR_2("Failed to dump SA/SP, error %d (%s)",
                    msgR.sadb_msg_errno,strerror(msgR.sadb_msg_errno));
                sts = RV_ERROR_UNKNOWN;        
            }
            break;
        }
    }
    
end:
    IMSIPSEC_LOG_LEAVE_1("Completed with %s",(sts == RV_OK) ? "success" : "failure");
    return sts;
#undef FUNCTION_NAME
}
#endif


RvStatus rvAdIpsecInit()
{
    RvLockConstruct(NULL,&gsLinuxSPI_ID_Lock);

    bzero(gsLinuxSPI_ID_Arr,sizeof(gsLinuxSPI_ID_Arr));

    return RV_OK;
}

RvStatus rvAdIpsecEnd()
{
    RvLockDestruct(&gsLinuxSPI_ID_Lock,NULL);
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
        sad->iIpSecSpecific.iOSEncrAlg = SADB_EALG_3DESCBC;   /* DES-EDE3-CBC, RFC2451, key 192 bits */
        break;
    case RvImsEncrAes:
        sad->iIpSecSpecific.iOSEncrAlg = 12;/*SADB_X_EALG_AESCBC;*/  /* AES-CBC, RFC3602, key 128 bits*/
        break;
    default:
        IMSIPSEC_LOG_ERROR_1("Unknown encryption algorithm %d",sad->iEncrAlg);
        retV = RV_ERROR_UNKNOWN;
        goto end;
    }
    switch (sad->iAuthAlg)
    {
    case RvImsAuthMd5:
    sad->iIpSecSpecific.iOSAuthAlg = SADB_AALG_MD5HMAC; /* RFC 2403 (1998) "The Use of HMAC-MD5-96 within ESP and AH".*/                                                        
        break;
    case RvImsAuthSha1:
        sad->iIpSecSpecific.iOSAuthAlg = SADB_AALG_SHA1HMAC; /* RFC 2404 (1998) "The Use of HMAC-SHA-1-96 within ESP and AH" */
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

static int              gsSaSeqNum = 1;

/* 
 * this function maintains the bitstream of 2^16 bits length. the function is capable to 
 * mark 4 positions in the bitstream as busy or to free them (depending of 'add' argument value).
 * If the 'add' is RV_TRUE 4 bits will be set a busy and their indexes will be used as IDs of 
 * created SAs.
 */
static void getSAIDs(
    IN RvImsSAData *sad,
    IN RvBool add
)
{
    RvInt j,i,cnt;
    RvChar *p;
    RvBool stopHere = RV_TRUE;
    /* the msks  is built in such a way that in the first element of msks the rightmost bit
       is set to 0 and others are 1. The second element has second (from right) bit 0 and so on.
       The last (number eight) element has the leftmost bit set to 0.
    */
    static RvUint8 msks[] = {0xff-1,0xff-2,0xff-4,0xff-8,0xff-16,0xff-32,0xff-64,0xff-128};
    RvIPSecSAData           *sapd;
    static RvUint32 gsLinuxSPI_ID_Cnt = 12345;   /* starting value of SA id; will be incremented
                                                    by one for each id allocated */
    /* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
#if defined(__ABACUS_DEBUG__)
#define FUNCTION_NAME "getSAIDs"
    IMSIPSEC_LOG_ENTER;
#endif
#endif
    /* SPIRENT_END */
    sapd = sad->iIpSecSpecific.iSAP;

    RvLockGet(&gsLinuxSPI_ID_Lock,NULL);
    for (cnt = 0; cnt < 4; cnt ++, sapd ++)
    {
        if (sapd->iDirection != RV_IPSECMODE_DIR_OUT)
            /* we do need the ID for outbound SAs */
            continue;
        if (add)
        {
            for (;;)
            {
                /* 
                 * define the 'i' and 'j' 
                 * 'j' is the index of byte within the bitstream
                 * 'i' is the index of bit within the byte 
                 */
                j = gsLinuxSPI_ID_Cnt>>3; /* j is gsLinuxSPI_ID_Cnt / 8 */
                i = gsLinuxSPI_ID_Cnt & 0x7; /* i is gsLinuxSPI_ID_Cnt mod 8 */
                p = gsLinuxSPI_ID_Arr + j; /* p is correct byte within gsLinuxSPI_ID_Arr bitmask */
                if ((*p>>i)&0x1) 
                    stopHere = RV_FALSE; /* this bit is used already */
                else
                {
                    stopHere = RV_TRUE; /* this bit is not used */
                    sapd->iSADIndex = gsLinuxSPI_ID_Cnt;
                }
                gsLinuxSPI_ID_Cnt++; /* increment the gsLinuxSPI_ID_Cnt and test it for overflow */
                if (gsLinuxSPI_ID_Cnt > RV_LINUX_IPSEC_MAX_SA_ID)
                    gsLinuxSPI_ID_Cnt = 1;
                if (stopHere)
                {
                    *p |= (0x1<<i); /* mark the bit as used */
                    break;
                }
            }            
        }
        else
        {
            j = sapd->iSADIndex>>3;
            i = sapd->iSADIndex & 0xf;
            gsLinuxSPI_ID_Arr[j] &= msks[i]; /* mark the bit as unused */
        }
    }
    RvLockRelease(&gsLinuxSPI_ID_Lock,NULL);

    /* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
#if defined(__ABACUS_DEBUG__)
    IMSIPSEC_LOG_LEAVE;
#undef FUNCTION_NAME
#endif
#endif
    /* SPIRENT_END */

}

/********************************************************************************************
 * rvLinuxIPSecAddDeleteKeys
 *
 * OS specific function to add/delete 'sapdNum' of security associations.
 *
 * INPUT:
 *     add       - if RV_TRUE the SAs will be added, otherwise deleted
 *    sad        - array of SA data
 * sapdNum       - number of elements in 'sad'
 *  logMgr      - handle of the log manager for this instance 
 *
 * RETURNS:
 *  RV_OK on success, other values on failure
 ********************************************************************************************/
RvStatus rvLinuxIPSecAddDeleteKeys(
    IN RvBool add,
    IN RvImsSAData *sad, 
    IN int sapdNum, 
    IN RvLogMgr *logMgr)
{
#define FUNCTION_NAME "rvLinuxIPSecAddDeleteKeys"
    RvUint16                cnt;
    RvInt32                 s,len,ret,addrOffs,offs,saCnt,salen,plen,addrSz;
    RvUint32                mypid;
    RvUint8                 buf[2048], *p;  /* 2048 bytes should be enough */  
    RvUint8                 bufOut[2048];
    struct sadb_msg         *msg;
    struct sadb_sa          *saext;
    struct sadb_x_sa2       *saext2;
    struct sadb_address     *addrext;
    u_char                  *pDst, *pSrc;
    struct sadb_key         *keyext;
    short                   sockFamily;
    RvIPSecSAData           *sapd;


/*
 *	the common idea:
 *  we have to create/delete 4 SAs here. All of them are almost the same.
 *  Flow:
 *      1. Prepare the message with data common to all 4 SAs.
           Keep the pointers in the message to the data items that have 
           specific values for each SA.
 *      2. Create the PFKEYV2 socket.
 *      3. For each 4 SAs: set the differing values in the message and write the
 *         message into socket.
 *      4. Test the result.
 *      5. Close the socket.  
 */
    
    IMSIPSEC_LOG_ENTER;

    s = -1;
    saCnt = 0;

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

        /* now set the encryption key */
        keyext = (struct sadb_key *) p;
        if (sad->iEncrAlg == RvImsEncrNull)
            sad->iEncrKey.iKeyBits = 0;
        keyext->sadb_key_len = (sizeof(struct sadb_key) + (sad->iEncrKey.iKeyBits >> 3) + 7) >> 3;
        keyext->sadb_key_exttype = SADB_EXT_KEY_ENCRYPT;
        keyext->sadb_key_bits = sad->iEncrKey.iKeyBits;
        if (sad->iEncrKey.iKeyBits)
            memcpy(keyext + 1, sad->iEncrKey.iKeyValue, sad->iEncrKey.iKeyBits >> 3);
        offs = keyext->sadb_key_len << 3;
        len += offs;
        p += offs;
    }
    
    /* now set the SA extension */
    /* now we can set the encryption and authentication algorithms 
       (is the same for all 4 SAs).
    */
    saext = (struct sadb_sa *) p;
    saext->sadb_sa_len = sizeof(*saext) >> 3;  /* divide by 8 */
    saext->sadb_sa_exttype = SADB_EXT_SA;
    if (add)
    {
#if (RV_OS_TYPE == RV_OS_TYPE_NETBSD)        
        saext->sadb_sa_state = SADB_SASTATE_LARVAL;
#else
        saext->sadb_sa_state = SADB_SASTATE_MATURE;
#endif
        saext->sadb_sa_encrypt = sad->iIpSecSpecific.iOSEncrAlg;
        saext->sadb_sa_auth = sad->iIpSecSpecific.iOSAuthAlg;
    }
    len += sizeof(struct sadb_sa);
    p += sizeof(struct sadb_sa);


    /* set the SA extension 2 for mode (transport/tunnel) */
    saext2 = (struct sadb_x_sa2*) p;
    saext2->sadb_x_sa2_len = sizeof(struct sadb_x_sa2) >> 3;
    saext2->sadb_x_sa2_exttype = SADB_X_EXT_SA2;
    saext2->sadb_x_sa2_mode = IPSEC_MODE_TRANSPORT;    
    len += sizeof(struct sadb_x_sa2);
    p += sizeof(struct sadb_x_sa2);      


    /* set the addresses: local and remote 
     * now we set only IPv4/IPv6 stuff and 
     * keep the pointers to address itself
     */
#if (RV_NET_TYPE & RV_NET_IPV6)    
    if (sad->iLclAddress.addrtype == RV_ADDRESS_TYPE_IPV6)
    {
        salen = sizeof(struct sockaddr_in6);
        addrSz = sizeof(struct in6_addr);
        plen = sizeof(struct in6_addr) << 3;
        sockFamily = AF_INET6;
        addrOffs = RV_OFFSETOF(struct sockaddr_in6,sin6_addr);
    }
    else
#endif
    {
        salen = sizeof(struct sockaddr_in);
        addrSz = sizeof(struct in_addr);
        plen = sizeof(struct in_addr) << 3;
        sockFamily = AF_INET;
        addrOffs = RV_OFFSETOF(struct sockaddr_in,sin_addr);
    }

    addrext = (struct sadb_address *) p;
    addrext->sadb_address_len = (sizeof(struct sadb_address) + salen + 7) >> 3;
    addrext->sadb_address_exttype = SADB_EXT_ADDRESS_SRC;
    addrext->sadb_address_proto = IPSEC_ULPROTO_ANY;
    addrext->sadb_address_prefixlen = plen;
    pSrc = (u_char*) (addrext+1); /* keep the pointer */
    ((struct sockaddr*)pSrc)->sa_family = sockFamily;
    /* on NetBSD the sockaddr structure starts with its length */
#if (RV_OS_TYPE == RV_OS_TYPE_NETBSD)    
    ((struct sockaddr*)pSrc)->sa_len = salen;
#endif    
    pSrc += addrOffs;
    offs = addrext->sadb_address_len << 3;
    p += offs;
    len += offs;        
    
    addrext = (struct sadb_address *) p;
    addrext->sadb_address_len = (sizeof(struct sadb_address) + salen + 7) >> 3;
    addrext->sadb_address_exttype = SADB_EXT_ADDRESS_DST;
    addrext->sadb_address_proto = IPSEC_ULPROTO_ANY;
    addrext->sadb_address_prefixlen = plen;
    pDst = (u_char*) (addrext+1); /* keep the pointer */
    ((struct sockaddr*)pDst)->sa_family = sockFamily;
#if (RV_OS_TYPE == RV_OS_TYPE_NETBSD)    
    ((struct sockaddr*)pDst)->sa_len = salen;
#endif    
    pDst += addrOffs;
    offs = addrext->sadb_address_len << 3;
    p += offs;
    len += offs;    
    
    msg->sadb_msg_len = len >> 3;

    s = pfkey_open();
    if (s == -1)
    {
        IMSIPSEC_LOG_ERROR_1("Failed to create PF_KEY socket, error %d",errno);
        goto failure;
    }    

#if (RV_OS_TYPE == RV_OS_TYPE_NETBSD)    
    if (rvSendSadbMsg(s,SADB_REGISTER,logMgr) != RV_OK)
    {
        IMSIPSEC_LOG_ERROR_0("Failed in rvSendSadbMsg");
        goto failure;
    }    
#endif
    
    /* allocate/deallocate 4 unique IDs */
    getSAIDs(sad,add);
    
    sapd = sad->iIpSecSpecific.iSAP;
    for (cnt = 0; cnt < sapdNum; cnt++, sapd++)
    {

        msg->sadb_msg_seq = gsSaSeqNum++;
        /* set the ID of SA */
        saext2->sadb_x_sa2_reqid = sad->iIpSecSpecific.iSAP[cnt].iSADIndex;
        /* set the SPI */
        saext->sadb_sa_spi = htonl(sad->iIpSecSpecific.iSAP[cnt].iSPI);

        /* copy the addresses */
        if (sapd->iDirection == RV_IPSECMODE_DIR_IN) 
        {
            memcpy(pDst,&sad->iLclAddress.data,addrSz);
            memcpy(pSrc,&sad->iPeerAddress.data,addrSz);            
        }
        else
        {
            memcpy(pDst,&sad->iPeerAddress.data,addrSz);
            memcpy(pSrc,&sad->iLclAddress.data,addrSz);            
        }

#if RV_IMS_IPSEC_DEBUG_MESSAGES
        {
            int i1,i2,i3 = 0;
            u_char* p;
            p = buf;
            printf("SAs message\n");
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

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)            
        {
                RvChar addr1[256],addr2[256];
                RvAddress *dst, *src;
                RvChar *direction;

                if (sapd->iDirection == RV_IPSECMODE_DIR_IN) 
                {
                    direction = "IN";
                    dst = &sad->iLclAddress;
                    src = &sad->iPeerAddress;
                }
                else
                {
                    direction = "OUT";
                    src = &sad->iLclAddress;
                    dst = &sad->iPeerAddress;
                }

                IMSIPSEC_LOG_INFO_7("%d: %s SA: dir %s, src %s, dst %s, spi %d, id %d",
                    cnt,
                    (add) ? "Create":"Delete",direction,
                    RvAddressGetString(src,sizeof(addr1),addr1),
                    RvAddressGetString(dst,sizeof(addr2),addr2),
                    sad->iIpSecSpecific.iSAP[cnt].iSPI,
                    sad->iIpSecSpecific.iSAP[cnt].iSADIndex);                                   
        }        
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)            */
        

        ret = pfkey_send(s, (RvChar*)msg, len);

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
            
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
            do {
              msglen = read(s, bufOut, sizeof(bufOut));
            } while(msglen==-1 && errno==EINTR);
#else
            msglen = read(s, bufOut, sizeof(bufOut));
#endif
/* SPIRENT_END */

            if (msglen <= 0)
            {
                IMSIPSEC_LOG_ERROR_3("Failed on reading from PF_KEY socket, ret %d, error %d, cnt %d",
                    ret,errno,cnt);
                goto failure;
            }
            
            msgp = (struct sadb_msg *) bufOut;
            if (msgp->sadb_msg_pid == mypid && msg->sadb_msg_type == msgp->sadb_msg_type) {

                if (msgp->sadb_msg_errno) 
                /* if errno is not zero the operation failed */
                {
                    IMSIPSEC_LOG_ERROR_3("Failed to set/remove SA, error %d (%s), cnt %d",
                        msgp->sadb_msg_errno,strerror(msgp->sadb_msg_errno),cnt);
                    goto failure;
                }
                saCnt ++;
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

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    /* clear unused SAIDs from the gsLinuxSPI_ID_Arr array if we don't plan to use them */
    if (s != -1 && add)
#else
    if (saCnt && add)
#endif
/* SPIRENT_END */

        /* if failed on adding and saCnt is not zero that means that some of 
           SAs were created successfully. Thus we have to destroy them */
        rvLinuxIPSecAddDeleteKeys(RV_FALSE,sad,saCnt,logMgr);

    IMSIPSEC_LOG_LEAVE_0("Failure");
    return RV_ERROR_UNKNOWN;

#undef FUNCTION_NAME
}


/********************************************************************************************
 * rvLinuxIPSecAddDeleteConf
 *
 * OS specific function to add/delete 'sapdNum' of security policies.
 *
 * INPUT:
 *     add       - if RV_TRUE the SPs will be added, otherwise deleted
 *    sapd       - array of SA data
 * sapdNum       - number of elements in 'sapd'
 *  logMgr      - handle of the log manager for this instance 
 *
 * RETURNS:
 *  RV_OK on success, other values on failure
 ********************************************************************************************/
RvStatus rvLinuxIPSecAddDeleteConf(
    IN RvBool add,
    IN RvImsSAData *sad, 
    IN int sapdNum, 
    IN RvLogMgr *logMgr)
{
#define FUNCTION_NAME "rvLinuxIPSecAddDeleteConf"
    RvUint16                cnt, *pLclPort=NULL, *pPeerPort=NULL;
    RvInt32                 s,len,ret,spCnt,addrSz=0,plen,offs,ip_proto;
    RvUint32                mypid;
    RvUint8                 buf[2048], *p;  /* 2048 bytes should be enough */  
    RvUint8                 bufOut[2048];
    struct sadb_msg         *msg;
    struct sadb_address     *addr;
    struct sadb_x_policy    *policy;
    struct sadb_x_ipsecrequest *polreq=NULL;
    static int              spSeqNum = 1;
    RvIPSecSAData           *sapd;
    struct sockaddr_in      lcl,peer;
    u_char                  *lclP=NULL,*peerP=NULL, *pSrc=NULL, *pDst=NULL;

    RvUint8                 *tunnel_polreq_holder = NULL;
    RvInt32                 currDirection = IPSEC_DIR_OUTBOUND;
    RvImsAppExtensionData   *extData, *outTunnelData=NULL, *inTunnelData=NULL, *currTunnelData = NULL;
    uint32_t                *outPolPriority = NULL, *inPolPriority = NULL;
    RvBool                  bothIterationsDone = RV_FALSE;
    
    IMSIPSEC_LOG_ENTER;

    for (cnt = 0, extData = sad->iExtData; cnt < sad->iExtDataLen; cnt ++, extData++)
    {
        switch(extData->iExtType) 
        { 
        case RvImsAppExtLinuxOutboundTunnelData:
            outTunnelData = extData;
        	break;
        case RvImsAppExtLinuxInboundTunnelData:
            inTunnelData = extData;
            break;            
        case RvImsAppExtLinuxOutboundPolicyPriority:
            outPolPriority = (uint32_t*) extData->iExtData;            
        	break;
        case RvImsAppExtLinuxInboundPolicyPriority:
            inPolPriority = (uint32_t*) extData->iExtData;            
            break;
        default:
            IMSIPSEC_LOG_ERROR_2("Unsupported extension data type %d (value length is %d)",extData->iExtType,extData->iExtLen);
        }
    }

    s = -1;
    spCnt = 0;

    /* 
        The general flow is just as in rvLinuxIPSecAddDeleteKeys.
        Create the message containg data common to all SPs
        Create the socket;
        Modify the message for specific SP and write the message.
    */

    mypid = getpid();
    bzero(&buf, sizeof(buf));
    p = buf;
    msg = (struct sadb_msg *) p;
    msg->sadb_msg_version = PF_KEY_V2;
    msg->sadb_msg_type = (add) ? SADB_X_SPDADD : SADB_X_SPDDELETE2;
    msg->sadb_msg_satype = SADB_SATYPE_UNSPEC;
    msg->sadb_msg_pid = mypid;
    len = sizeof(*msg);
    p += len;

    policy = (struct sadb_x_policy *)p;
    if (add)
        policy->sadb_x_policy_len = (sizeof(struct sadb_x_policy) + sizeof(struct sadb_x_ipsecrequest)) >> 3;
    else
        policy->sadb_x_policy_len = sizeof(struct sadb_x_policy) >> 3;
    policy->sadb_x_policy_exttype = SADB_X_EXT_POLICY;
    policy->sadb_x_policy_type = IPSEC_POLICY_IPSEC;


    len += sizeof(struct sadb_x_policy);
    p += sizeof(struct sadb_x_policy);

    if (add)
    {        
        polreq = (struct sadb_x_ipsecrequest *)p;
        polreq->sadb_x_ipsecrequest_len = sizeof(struct sadb_x_ipsecrequest);
        polreq->sadb_x_ipsecrequest_proto = IPPROTO_ESP;
        polreq->sadb_x_ipsecrequest_mode = IPSEC_MODE_TRANSPORT;
        polreq->sadb_x_ipsecrequest_reqid = 0;
        len += sizeof(struct sadb_x_ipsecrequest);
        p += sizeof(struct sadb_x_ipsecrequest);        

        if (outTunnelData || inTunnelData)
        {
            /* if there is out and/or in tunnel policy data do the following:
                -save the location where this data will be inserted;
                -leave the room to fit the biggest portion of data (in or out)
            */
            tunnel_polreq_holder = p;
            if (!outTunnelData)
                currTunnelData = inTunnelData;
            else if (!inTunnelData)
                currTunnelData = outTunnelData;
            else if (inTunnelData->iExtLen > outTunnelData->iExtLen)
                currTunnelData = inTunnelData;
            else
                currTunnelData = outTunnelData;

            len += currTunnelData->iExtLen;
            p += currTunnelData->iExtLen;
            policy->sadb_x_policy_len <<= 3;
            policy->sadb_x_policy_len += currTunnelData->iExtLen;
            policy->sadb_x_policy_len >>= 3;
        }

#if (RV_NET_TYPE & RV_NET_IPV6)    
        if (sad->iPeerAddress.addrtype == RV_ADDRESS_TYPE_IPV6)
        {
            struct sockaddr_in6     lcl6,peer6;            
            addrSz = sizeof(struct sockaddr_in6);
            plen = 128;
            bzero(&lcl6,sizeof(lcl6));
            bzero(&peer6,sizeof(peer6));
            lcl6.sin6_family = peer6.sin6_family = AF_INET6;
            pLclPort = &lcl6.sin6_port;
            memcpy(&lcl6.sin6_addr,&sad->iLclAddress.data.ipv6,16);
            pPeerPort = &peer6.sin6_port;
            memcpy(&peer6.sin6_addr,&sad->iPeerAddress.data.ipv6,16);
            lclP = (u_char*)&lcl6;      /* keep the address pointers*/
            peerP = (u_char*)&peer6;
        }
        else
#endif
        {
            addrSz = sizeof(struct sockaddr_in);
            plen = 32;
            bzero(&lcl,sizeof(lcl));
            bzero(&peer,sizeof(peer));

#if (RV_OS_TYPE == RV_OS_TYPE_NETBSD)                
            lcl.sin_len = peer.sin_len = sizeof(lcl);
#endif
            lcl.sin_family = peer.sin_family = AF_INET;
            pLclPort = &lcl.sin_port;
            memcpy(&lcl.sin_addr,&sad->iLclAddress.data.ipv4,4);
            pPeerPort = &peer.sin_port;
            memcpy(&peer.sin_addr,&sad->iPeerAddress.data.ipv4,4);
            lclP = (u_char*)&lcl;   /* keep the address pointers*/
            peerP = (u_char*)&peer;
        }    

        /* set src */
        addr = (struct sadb_address*) p;
        addr->sadb_address_len = (sizeof(struct sadb_address) + addrSz + 7) >> 3;
        addr->sadb_address_exttype = SADB_EXT_ADDRESS_SRC;
        if (sad->iProto == RvImsProtoTcp)       
            ip_proto = IPPROTO_TCP;
        else if (sad->iProto == RvImsProtoUdp)
            ip_proto = IPPROTO_UDP;
        else
            ip_proto = IPSEC_ULPROTO_ANY;
        addr->sadb_address_proto = ip_proto;
        addr->sadb_address_prefixlen = plen;
        pSrc = (u_char*) (addr+1);
        offs = addr->sadb_address_len << 3;
        len += offs;
        p += offs;
    
        /* set dst */
        addr = (struct sadb_address*) p;
        addr->sadb_address_len = (sizeof(struct sadb_address) + addrSz + 7) >> 3;
        addr->sadb_address_exttype = SADB_EXT_ADDRESS_DST;
        addr->sadb_address_proto = ip_proto;
        addr->sadb_address_prefixlen = plen;
        pDst = (u_char*) (addr+1);
        offs = addr->sadb_address_len << 3;
        len += offs;
        p += offs;
    }

    s = pfkey_open();
    if (s == -1)
    {
        IMSIPSEC_LOG_ERROR_1("Failed to create PF_KEY socket, error %d",errno);
        goto failure;
    }    

#if (RV_OS_TYPE == RV_OS_TYPE_NETBSD)    
    if (rvSendSadbMsg(s,SADB_REGISTER,logMgr) != RV_OK)
    {
        IMSIPSEC_LOG_ERROR_0("Failed in rvSendSadbMsg");
        goto failure;
    }    
#endif
        

    if (currTunnelData)
    {
        /* if there is OUT and/or IN tunnel policy we will start with the 
           the direction where the policy data contian more bytes (is longer) */
        if (outTunnelData && outTunnelData == currTunnelData)
            currDirection = IPSEC_DIR_OUTBOUND;
        else
            currDirection = IPSEC_DIR_INBOUND;
        /* copy the tunnel policy data to the location set by place holder */
        memcpy(tunnel_polreq_holder,currTunnelData->iExtData,currTunnelData->iExtLen);
    }
    
again:
    /* set the length of the message */
    msg->sadb_msg_len = len >> 3;
    
    sapd = sad->iIpSecSpecific.iSAP;
    for (cnt = 0; cnt < sapdNum; cnt++, sapd++)
    {
        
        if (add)
        {
            /* set the direction */
            policy->sadb_x_policy_dir = (sapd->iDirection == RV_IPSECMODE_DIR_IN) 
                ? IPSEC_DIR_INBOUND : IPSEC_DIR_OUTBOUND;

            if (tunnel_polreq_holder && policy->sadb_x_policy_dir != currDirection)
                continue;


#if (POLICY_PRIORITY_SUPPORTED == 1)
            if (policy->sadb_x_policy_dir == IPSEC_DIR_OUTBOUND && outPolPriority)
                policy->sadb_x_policy_priority = *outPolPriority;
            else if (policy->sadb_x_policy_dir == IPSEC_DIR_INBOUND && inPolPriority)
                policy->sadb_x_policy_priority = *inPolPriority;
            else
                policy->sadb_x_policy_priority = DEFAULT_PRIORITY;
#endif /*(POLICY_PRIORITY_SUPPORTED == 1)*/
            

            /* and ports */
            *pLclPort = htons(sapd->iLclPort);
            *pPeerPort = htons(sapd->iRemPort);

            /* now copy the addresses */
            if (sapd->iDirection == RV_IPSECMODE_DIR_OUT)
            {
                memcpy(pSrc,lclP,addrSz);
                memcpy(pDst,peerP,addrSz);
            }
            else
            {
                memcpy(pSrc,peerP,addrSz);
                memcpy(pDst,lclP,addrSz);
            }

            /* we set the UNIQUE SA for outbound SPs and 
               REQUIRE level for inbound 
            */            
            if (sapd->iDirection == RV_IPSECMODE_DIR_OUT)
            {
                polreq->sadb_x_ipsecrequest_reqid = sapd->iSADIndex;                        
                polreq->sadb_x_ipsecrequest_level = IPSEC_LEVEL_UNIQUE;
            }
            else
            {
                polreq->sadb_x_ipsecrequest_reqid = 0;
                polreq->sadb_x_ipsecrequest_level = IPSEC_LEVEL_REQUIRE;                
            }

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)            
            {
                RvChar *direction;                
                if (sapd->iDirection == RV_IPSECMODE_DIR_IN) 
                {
                    direction = "IN";
                }
                else
                {
                    direction = "OUT";
                }
                
                IMSIPSEC_LOG_INFO_5("%d: SP: %s, lcl %d, dst %d, saInd %d",
                    cnt,direction,sapd->iLclPort,sapd->iRemPort,sapd->iSADIndex);
            }        
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)            */
            
        }
        else
            /* if we delete SPs use the index saved when the SP was created */
            policy->sadb_x_policy_id = sapd->iSPDIndex;


        msg->sadb_msg_seq = spSeqNum++;

#if RV_IMS_IPSEC_DEBUG_MESSAGES
        {
            int i1,i2,i3 = 0;
            u_char* p;
            p = buf;
            printf("Policies message\n");
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
        verbose(msg,len,RV_FALSE);
#endif

        ret = pfkey_send(s, (RvChar*)msg, len);
        
        if (ret != len)
        {
            IMSIPSEC_LOG_ERROR_4("Failed on writing %d bytes to PF_KEY socket, ret %d, error %d, cnt %d",
                len,ret,errno,cnt);
            goto failure;
        }
        
        /* Read and print SADB_ADD reply, discarding any others */
        for ( ; ; ) {

            /* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
        int     msglen;
#else
        u_int     msglen;
#endif
/* SPIRENT_END */

            struct sadb_msg *msgp;
        
            /* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
            do {
              msglen = read(s, bufOut, sizeof(bufOut));
            } while(msglen==-1 && errno==EINTR);
#else
            msglen = read(s, bufOut, sizeof(bufOut));
#endif
/* SPIRENT_END */

            if (msglen <= 0)
            {
                IMSIPSEC_LOG_ERROR_3("Failed on reading from PF_KEY socket, ret %d, error %d, cnt %d",
                    ret,errno,cnt);
                goto failure;
            }
            
            msgp = (struct sadb_msg *) bufOut;
            if (msgp->sadb_msg_pid == mypid && msg->sadb_msg_type == msgp->sadb_msg_type) 
            {
              /* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
              unsigned char *p=NULL, *pE=NULL;
#else
              char *p, *pE;
#endif
              /* SPIRENT_END */
                
                struct sadb_ext *sae;

                if (msgp->sadb_msg_errno)
                /* if errno is not zero the operation failed */
                {
                    IMSIPSEC_LOG_ERROR_3("Failed to set/remove SP, error %d (%s), cnt %d",
                        msgp->sadb_msg_errno,strerror(msgp->sadb_msg_errno),cnt);
                    goto failure;
                }

                if (add)
                {
                    /* find the sadb_x_sa2_sequence */
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
                    p = (unsigned char*)(bufOut + sizeof(*msgp));
#else
                    p = (char*)(bufOut + sizeof(*msgp));
#endif
/* SPIRENT_END */
                    pE = p + msglen;
                    while (p < pE)
                    {
                        sae = (struct sadb_ext*)p;
                        if (sae->sadb_ext_type == SADB_X_EXT_POLICY)
                        /* found; save the policy ID; we'll need it to delete the SP */
                        {
                            sapd->iSPDIndex = ((struct sadb_x_policy*)p)->sadb_x_policy_id;
                            break;
                        }
                        p += sae->sadb_ext_len<<3;
                    }
                }


                spCnt ++;
                break;
            }
        }
    }
    
    if (add && currTunnelData && !bothIterationsDone)
    {
        int lenDiff;
        if (currTunnelData == outTunnelData && inTunnelData)
        {
            lenDiff = outTunnelData->iExtLen - inTunnelData->iExtLen;
            currTunnelData = inTunnelData;
        }
        else if (currTunnelData == inTunnelData && outTunnelData)
        {
            lenDiff = inTunnelData->iExtLen - outTunnelData->iExtLen;
            currTunnelData = outTunnelData;
        }
        else
        {
            lenDiff = currTunnelData->iExtLen;
            currTunnelData = NULL;
        }
        currDirection = (currDirection == IPSEC_DIR_OUTBOUND) ? IPSEC_DIR_INBOUND:IPSEC_DIR_OUTBOUND;
        
        if (currTunnelData)
        {
            /* if there ar both IN and out tunnel polci defined we have to copy
               the other policy data */
            memcpy(tunnel_polreq_holder,currTunnelData->iExtData,currTunnelData->iExtLen);
            
            if (lenDiff)
                /* and may be re-fit the other part of the message if the IN and OUT policy
                   data differ in size */
                memcpy(tunnel_polreq_holder+currTunnelData->iExtLen,    /*destination: right after policy data */
                       tunnel_polreq_holder+currTunnelData->iExtLen+lenDiff,    /* source: right after previous policy data */
                       buf+len-(tunnel_polreq_holder+currTunnelData->iExtLen+lenDiff)); /* size: offset from the last byte of the message and the source of memcpy */
        }
        else
        {
            /* there is only one IN or OUT tunnel policy data given, thus we have remove it from the message */
            memcpy(tunnel_polreq_holder,tunnel_polreq_holder+lenDiff,buf+len-(tunnel_polreq_holder+lenDiff));
        }


        /* update the whole message lenght */
        len -= lenDiff;

        /* update the locations of pSrc and pDst (source and destination addresses) */
        pSrc -= lenDiff;
        pDst -= lenDiff;

        /* update the total policy length */
        policy->sadb_x_policy_len <<= 3;
        policy->sadb_x_policy_len -= lenDiff;
        policy->sadb_x_policy_len >>= 3;

        bothIterationsDone = RV_TRUE;
        goto again;            
    }

    close(s);
    
    IMSIPSEC_LOG_LEAVE_0("Success");
    return RV_OK;

failure:
    if (s != -1)
        close(s);

    if (spCnt && add)
        rvLinuxIPSecAddDeleteConf(RV_FALSE,sad,spCnt,logMgr);    

    IMSIPSEC_LOG_LEAVE_0("Failure");
    return RV_ERROR_UNKNOWN;

#undef FUNCTION_NAME
    return RV_OK;
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
    IMSIPSEC_LOG_ENTER;
    ret = rvLinuxIPSecAddDeleteKeys(RV_TRUE,sad,sapdNum,logMgr);   
    if (ret == RV_OK)
    {
        ret = rvLinuxIPSecAddDeleteConf(RV_TRUE,sad,sapdNum,logMgr);
        if (ret != RV_OK)
            rvLinuxIPSecAddDeleteKeys(RV_FALSE,sad,sapdNum,logMgr);   

    }
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
    IMSIPSEC_LOG_ENTER;
    ret = rvLinuxIPSecAddDeleteKeys(RV_FALSE,sad,sapdNum,logMgr);    
    ret = rvLinuxIPSecAddDeleteConf(RV_FALSE,sad,sapdNum,logMgr);    
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
    
    IMSIPSEC_LOG_ENTER;

    bzero(&msg, sizeof(msg));
    msg.sadb_msg_version = PF_KEY_V2;
    msg.sadb_msg_satype = SADB_SATYPE_ESP;
    msg.sadb_msg_pid = getpid();    
    len = sizeof(msg);
    msg.sadb_msg_len = len >> 3;
    sts = RV_OK;
    
    s = pfkey_open();
    if (s == -1)
    {
        IMSIPSEC_LOG_ERROR_1("Failed to create PF_KEY socket, error %d",errno);
        sts = RV_ERROR_UNKNOWN;        
        goto end;
    }    

#if (RV_OS_TYPE == RV_OS_TYPE_NETBSD)    
    if (rvSendSadbMsg(s,SADB_REGISTER,logMgr) != RV_OK)
    {
        IMSIPSEC_LOG_ERROR_0("Failed in rvSendSadbMsg");
        sts = RV_ERROR_UNKNOWN;        
        goto end;
    }    
#endif
    
    
    for (cnt = 0; cnt < 2; cnt ++)
    {

        msg.sadb_msg_seq = gsSaSeqNum++;
        msg.sadb_msg_type = (cnt == 0) ? SADB_FLUSH : SADB_X_SPDFLUSH;

        ret = pfkey_send(s, (RvChar*)&msg, len);

        if (ret != len)
        {
            IMSIPSEC_LOG_ERROR_3("Failed on writing %d bytes to PF_KEY socket, ret %d, error %d",
                len,ret,errno);
            sts = RV_ERROR_UNKNOWN;        
            goto end;
        }
        
        /* Read and print SADB_ADD reply, discarding any others */
        for ( ; ; ) 
        {
            int     msglen;
            struct sadb_msg msgR;
    
            /* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
            do {
              msglen = read(s, (void*)&msgR, sizeof(msgR));
            } while(msglen==-1 && errno==EINTR);
#else
            msglen = read(s, (void*)&msgR, sizeof(msgR));
#endif
/* SPIRENT_END */

            if (msglen <= 0)
            {
                IMSIPSEC_LOG_ERROR_3("Failed on reading from PF_KEY socket, ret %d, error %d, cnt %d",
                    ret,errno,cnt);
                sts = RV_ERROR_UNKNOWN;        
                goto end;
            }
        
            if (msgR.sadb_msg_pid == msg.sadb_msg_pid && msg.sadb_msg_type == msgR.sadb_msg_type) {

                if (msgR.sadb_msg_errno && msgR.sadb_msg_errno != ENOENT)
                {
                    IMSIPSEC_LOG_ERROR_2("Failed to dump SA/SP, error %d (%s)",
                        msgR.sadb_msg_errno,strerror(msgR.sadb_msg_errno));
                    sts = RV_ERROR_UNKNOWN;        
                }
                break;
            }
        }
    }

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
end:
#endif
/* SPIRENT_END */

    if (s >= 0)
        close(s);

/* SPIRENT_BEGIN */
#if !defined(UPDATED_BY_SPIRENT)
end:
#endif
/* SPIRENT_END */

    IMSIPSEC_LOG_LEAVE_1("Completed with %s",(sts == RV_OK) ? "success" : "failure");
    return sts;
#undef FUNCTION_NAME
}


#endif /*(RV_IMS_IPSEC_ENABLED == RV_YES) */

#else
int prevent_warning_of_ranlib_has_no_symbols_rvslinuxipsec=0;
#endif /*(RV_OS_TYPE == RV_OS_TYPE_LINUX && RV_OS_VERSION & RV_OS_LINUX_REDHAT)*/
