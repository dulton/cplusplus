/* Copyright 1998 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

#include "rvccore.h"

#if (RV_DNS_TYPE == RV_DNS_ARES)
#include "rvares.h"
#include "ares_private.h"
#include "ares_dns.h"
#include "rvarescache.h"
#include <stdlib.h>
#include <string.h>
#include "rvehd.h"

static int encoded_name_length(const char *name);
static int ares_mkquery(const char *name, int dnsclass, int type,
                        unsigned short id, int rd, char *buf);

#undef LOG_SRC 
#define LOG_SRC dnsEngine->dnsSource

#undef LOG_FUNC
#define LOG_FUNC LOG_SRC, FUNC ": "

#define DnsLogDebug(p) RvLogDebug(LOG_SRC, p)
#define DnsLogExcep(p) RvLogExcep(LOG_SRC, p)
#define DnsLogError(p) RvLogError(LOG_SRC, p)
#define DnsLogWarn(p)  RvLogWarning(LOG_SRC, p)



RvSize_t RvAresComputeQuerySize(RvDnsEngine *dnsEngine, const char *name, const char *suffix) {
    RvSize_t qlen;
    RvSize_t nameLen = 0;
    RvSize_t suffixLen = 0;
    RvSize_t queryWords;

    nameLen = strlen(name);
    if(suffix) {
        suffixLen = strlen(suffix);
    }

    nameLen += suffixLen + 3;

    qlen = HFIXEDSZ + nameLen + QFIXEDSZ;

    /* Allocate space for query and allocated fields (in one large block). */
    queryWords = sizeof(rvQuery) / sizeof(int) + 1;
    /* the 2 extra bytes are dedicated for storing the qlen value */
    qlen += (queryWords + dnsEngine->skipServerWords) * sizeof(int) + 2;
    return qlen;
}


int ares_query(RvDnsEngine *dnsEngine, const char *name, int dnsclass, int type, void *query_buf,
               RvSize_t *qbuf_len, RvDnsNewRecordCB newRecordCB, rvAresCallback callback, void *usr_arg, unsigned int qid)
{
    rvQuery *query = (rvQuery *)query_buf;
    int status, qlen, queryWords,  rd, i;
    RvInt64 now;
    RvSize_t len;


    /* Compose the query. */
    qlen = HFIXEDSZ + encoded_name_length(name) + QFIXEDSZ;

    /* Allocate space for query and allocated fields (in one large block). */
    queryWords = sizeof(rvQuery) / sizeof(int) + 1;
    /* the 2 extra bytes are dedicated for storing the qlen value */
    len = (queryWords + dnsEngine->nservers) * sizeof(int) + 2 + qlen;
    if (len > *qbuf_len)
    {
        *qbuf_len = len;
        return ARES_ENOMEM;
    }

    /* Fill in the query structure. */
    query->qid = dnsEngine->next_id++;
    query->user_qid = qid;
#if RV_DNS_SANITY_CHECK
    query->mark = RV_FALSE;
#endif    
    query->timeout = RvInt64Const(0,0,0);  /* Start with no timeout. */
    query->newRecordCB = newRecordCB;
    query->user_callback = callback;
    query->user_arg = usr_arg;
    RvTimerConstruct(&query->timer);

    /* Initialize query status. */
    query->skip_server = (int*)((char*)query + queryWords * sizeof(int));
    query->try_index = 0;

    query->server = 0;
    query->serversGeneration = dnsEngine->serversGeneration;

    for (i = 0; i < dnsEngine->skipServerWords; i++) {
        query->skip_server[i] = 0;
    }
    
    query->using_tcp = (dnsEngine->flags & ARES_FLAG_USEVC) || qlen > PACKETSZ;
    query->error_status = ARES_ECONNREFUSED;

    /* Set the qbuf pointer */
    query->qbuf = (char*)query->skip_server + (dnsEngine->skipServerWords * sizeof(int));
    query->qlen = qlen;

    /* Form the TCP query header by prepending qlen (as two
     * network-order bytes) to qbuf.
     */
    query->qbuf[0] = (char)((qlen >> 8) & 0xff);
    query->qbuf[1] = (char)(qlen & 0xff);

    /* Skip the TCP header */
    query->qbuf += 2;

    rd = !(dnsEngine->flags & ARES_FLAG_NORECURSE);
    status = ares_mkquery(name, dnsclass, type, query->qid, rd, query->qbuf);
    if (status != ARES_SUCCESS)
        return status;

    /* Chain the query into this dnsEngine's query list. */
    rvAresAddQuery(dnsEngine, query);

    /* Perform the first query action. */
    now = RvTimestampGet(dnsEngine->logMgr);
    status = ares__send_query(dnsEngine, query, now);

    return status;
}


int ares_cancel_query(RvDnsEngine *dnsEngine, unsigned int qid, RvBool waitForCallbacks)
{
#undef FUNC
#ifdef __FUNCTION__
#  define FUNC __FUNCTION__
#else
#  define FUNC "ares_cancel_query"
#endif

    rvQuery *q;

    if(dnsEngine->inCallbackQid == qid) {
        /* Callback for this query is in progress */
        RvThreadId curThread;

        if(!waitForCallbacks) {
            DnsLogWarn((LOG_FUNC "Cancelling incallback query (qid=%d) without waiting for callback completion", qid));
            return RV_DNS_ERROR_INCALLBACK;
        }

        curThread = RvThreadCurrentId();
        if(curThread == dnsEngine->inCallbackTid) {
            DnsLogDebug((LOG_FUNC "Cancelling query (qid=%d) from it's callback is unnecessary", qid));
            return RV_OK;
        }

        RV_COND_WAITL(dnsEngine->inCallbackQid != qid, 
            &dnsEngine->inCallbackCond, 
            &dnsEngine->lock,
            dnsEngine->logMgr);
        return RV_OK;
    }

    q = removeQueryByUqid(dnsEngine, qid);

    if(q == 0) {
        DnsLogDebug((LOG_FUNC "query qid=%d not found", qid));
        return RV_DNS_ERROR_NOTFOUND;
    }

    DnsLogDebug((LOG_FUNC "cancelling query qid=%d (%d attempts tried, %d left)", 
        qid, q->try_index, dnsEngine->tries));


    return RV_OK;
}



static int encoded_name_length(const char *name)
{
    int len;
    const char *p;

    /* Compute the length of the encoded name so we can check buflen.
     * Start counting at 1 for the zero-length label at the end.
     */
    len = 1;
    for (p = name; *p; p++)
    {
        if (*p == '\\' && *(p + 1) != 0)
            p++;
        len++;
    }
    /* If there are n periods in the name, there are n + 1 labels, and
     * thus n + 1 length fields, unless the name is empty or ends with a
     * period.  So add 1 unless name is empty or ends with a period.
     */

    if (*name && *(p - 1) != '.')
        len++;

    return len;
}

/* Header format, from RFC 1035:
 *                                  1  1  1  1  1  1
 *    0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |                      ID                       |
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |QR|   Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |                    QDCOUNT                    |
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |                    ANCOUNT                    |
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |                    NSCOUNT                    |
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |                    ARCOUNT                    |
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *
 * AA, TC, RA, and RCODE are only set in responses.  Brief description
 * of the remaining fields:
 *  ID  Identifier to match responses with queries
 *  QR  Query (0) or response (1)
 *  Opcode  For our purposes, always QUERY
 *  RD  Recursion desired
 *  Z   Reserved (zero)
 *  QDCOUNT Number of queries
 *  ANCOUNT Number of answers
 *  NSCOUNT Number of name server records
 *  ARCOUNT Number of additional records
 *
 * Question format, from RFC 1035:
 *                                  1  1  1  1  1  1
 *    0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |                                               |
 *  /                     QNAME                     /
 *  /                                               /
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |                     QTYPE                     |
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |                     QCLASS                    |
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *
 * The query name is encoded as a series of labels, each represented
 * as a one-byte length (maximum 63) followed by the text of the
 * label.  The list is terminated by a label of length zero (which can
 * be thought of as the root domain).
 */

static int ares_mkquery(const char *name, int dnsclass, int type,
                        unsigned short id, int rd, char *buf)
{
    int len;
    const char *p;

    /* Set up the header. */
    memset(buf, 0, HFIXEDSZ);
    DNS_HEADER_SET_QID(buf, id);
    DNS_HEADER_SET_OPCODE(buf, QUERY);
    DNS_HEADER_SET_RD(buf, (rd) ? 1 : 0);
    DNS_HEADER_SET_QDCOUNT(buf, 1);

    /* A name of "." is a screw case for the loop below, so adjust it. */
    if (strcmp(name, ".") == 0)
        name++;

    /* Start writing out the name after the header. */
    buf += HFIXEDSZ;
    while (*name)
    {
        if (*name == '.')
            return ARES_EBADNAME;

        /* Count the number of bytes in this label. */
        len = 0;
        for (p = name; *p && *p != '.'; p++)
        {
            if (*p == '\\' && *(p + 1) != 0)
                p++;
            len++;
        }
        if (len > MAXLABEL)
            return ARES_EBADNAME;

        /* Encode the length and copy the data. */
        *buf++ = (unsigned char)len;
        for (p = name; *p && *p != '.'; p++)
        {
            if (*p == '\\' && *(p + 1) != 0)
                p++;
            *buf++ = *p;
        }

        /* Go to the next label and repeat, unless we hit the end. */
        if (!*p)
            break;
        name = p + 1;
    }

    /* Add the zero-length label at the end. */
    *buf++ = 0;

    /* Finish off the question with the type and class. */
    DNS_QUESTION_SET_TYPE(buf, type);
    DNS_QUESTION_SET_CLASS(buf, dnsclass);

    return ARES_SUCCESS;
}
#endif /* RV_DNS_ARES */
