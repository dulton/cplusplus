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
#include "ares.h"
#include "ares_private.h"
#include "ares_dns.h"
#include "rvoscomp.h"
#include "rvstrutils.h"
#include "rvassert.h"

#include <stdlib.h>
#include <string.h>
#if !defined(NEED_MANUAL_WRITEV)
#include <errno.h>
#endif

#if (RV_OS_TYPE == RV_OS_TYPE_UNIXWARE)
/* UnixWare hides strcasecmp() in strings.h */
#include <strings.h>
#endif

#if (RV_OS_TYPE == RV_OS_TYPE_SOLARIS)  || (RV_OS_TYPE == RV_OS_TYPE_LINUX) || \
    (RV_OS_TYPE == RV_OS_TYPE_UNIXWARE) || (RV_OS_TYPE == RV_OS_TYPE_TRU64) || \
    (RV_OS_TYPE == RV_OS_TYPE_HPUX)     || (RV_OS_TYPE == RV_OS_TYPE_FREEBSD) || \
	(RV_OS_TYPE == RV_OS_TYPE_MAC)      || (RV_OS_TYPE == RV_OS_TYPE_NETBSD)
#include <sys/uio.h>
#elif (RV_OS_TYPE == RV_OS_TYPE_VXWORKS)
#include <ioLib.h>
#endif

static void selectCb(RvSelectEngine *selectEngine, RvSelectFd *fd,
                     RvSelectEvents selectEvent, RvBool error);
static RvBool timerCb(select_fd_args *fd_args);
static void write_tcp_data(RvDnsEngine *channel, select_fd_args *fd_args, RvInt64 now);
static void read_tcp_data(RvDnsEngine *channel, select_fd_args *fd_args, RvInt64 now);
static void read_udp_packets(RvDnsEngine *channel, select_fd_args *fd_args, RvInt64 now);
static void process_timeouts(RvDnsEngine *channel, RvInt64 now);
static void process_answer(RvDnsEngine *channel, unsigned char *abuf,
                           int alen, int whichserver, int tcp, RvInt64 now);
static void handle_error(RvDnsEngine *channel, int whichserver, RvInt64 now);
static void next_server(RvDnsEngine *channel, rvQuery *query, RvInt64 now);
static int open_socket(RvDnsEngine *channel, int whichserver, RvSocketProtocol protocol);
static int same_questions(const unsigned char *qbuf, int qlen,
                          const unsigned char *abuf, int alen);
static void end_query(RvDnsEngine *channel, rvQuery *query, int status,
                      unsigned char *abuf, int alen);

#define LOG_SRC (channel->dnsSource)
#define LOG_FUNC LOG_SRC, FUNC ": "

#define DnsLogDebug(p) RvLogDebug(LOG_SRC, p)
#define DnsLogExcep(p) RvLogExcep(LOG_SRC, p)
#define DnsLogError(p) RvLogError(LOG_SRC, p)

#if RV_DNS_SANITY_CHECK

/* Assumes lock hold on 'channel' object */
RvBool rvDnsEngineSanityCheck(RvDnsEngine *channel) {
#undef FUNC
#define FUNC "RvDnsEngineSanityCheck"
    rvQuery *cur;

    for(cur = channel->queries; cur; cur = cur->qnext) {
        if(cur->mark) {
            DnsLogExcep((LOG_FUNC "List of queries includes cycle at query %d", cur->user_qid));
            RvAssert(0);
            /*lint -e{527} */ /*suppressing 'unreachable code' error*/
            return RV_FALSE;
        }

        cur->mark = RV_TRUE;
    }

    for(cur = channel->queries; cur; cur = cur->qnext) {
        cur->mark = RV_FALSE;
    }

    return RV_TRUE;
}

#endif

#ifndef RV_DNS_STATISTICS

#  define RV_DNS_STATISTICS 1

#endif

#if RV_DNS_STATISTICS
#  define STATS RvInt32 _nCompares = 0
#  define INCR_CMP(ch) (_nCompares++)
#  define INCR_SRCH(ch) ((ch)->nSearches++)
#  define CALC_AVG(ch)  \
    if(_nCompares > ch->nMaxCompares) {ch->nMaxCompares = _nCompares;} \
    ch->nCompares += _nCompares; \
    ((ch)->nAvgCompares = (RvInt32)((ch)->nCompares / (ch)->nSearches))
#else
#  define STATS
#  define INCR_CMP(ch) 
#  define INCR_SRCH(ch) 
#  define CALC_AVG(ch)  
#endif

static 
rvQuery *findQueryByQid(RvDnsEngine *channel, int id) {
    rvQuery *query;
    STATS;

    INCR_SRCH(channel);

    for (query = channel->queries; query && query->qid != id; query = query->qnext)
    {
        INCR_CMP(channel);
    }

    CALC_AVG(channel);
    return query;
}

rvQuery* findQueryByUqid(RvDnsEngine *channel, unsigned int qid) {
    rvQuery *cur;
    STATS;

    INCR_SRCH(channel);
    for(cur = channel->queries; cur && cur->user_qid != qid; cur = cur->qnext) {
        INCR_CMP(channel);
    }

    CALC_AVG(channel);
    return cur;
}

rvQuery* removeQueryByUqid(RvDnsEngine *channel, unsigned int qid) {
    rvQuery *cur;
    rvQuery *prev;
    STATS;

    INCR_SRCH(channel);
    for(prev = 0, cur = channel->queries; cur && cur->user_qid != qid; prev = cur, cur = cur->qnext) {
        INCR_CMP(channel);
    }

    CALC_AVG(channel);

    if(cur == 0) {
        return cur;
    }

    if(prev == 0) {
        channel->queries = cur->qnext;
    } else {
        prev->qnext = cur->qnext;
    }

    if(cur == channel->lastQuery) {
        channel->lastQuery = prev;
    }

    if(!cur->using_tcp) {
        RvTimerCancel(&cur->timer, RV_TIMER_CANCEL_DONT_WAIT_FOR_CB);
    }

    return cur;
}

static
rvQuery* findExpiredQuery(RvDnsEngine *channel, RvInt64 now) {
    rvQuery *cur;
    STATS;

    INCR_SRCH(channel);

    for(cur = channel->queries; cur != 0; cur = cur->qnext) {
        INCR_CMP(channel);
        if ((RvInt64IsNotEqual(cur->timeout, RvInt64Const(0,0,0)) &&
            RvInt64IsGreaterThanOrEqual(now, cur->timeout))) {
                break;
        }
    }

    CALC_AVG(channel);

    return cur;
    
}

static
rvQuery* findQueryByServer(RvDnsEngine *channel, RvInt server) {
    rvQuery *cur;
    STATS;

    INCR_SRCH(channel);

    for(cur = channel->queries; cur != 0 && cur->server != server; cur = cur->qnext) {
        INCR_CMP(channel);
    }

    CALC_AVG(channel);
    return cur;
}

void rvAresAddQuery(RvDnsEngine *channel, rvQuery *query) {
    query->qnext = 0;
    if(channel->lastQuery != 0) {
        channel->lastQuery->qnext = query;
    } else {
        channel->queries = query;
    }

    channel->lastQuery = query;
}


int ares__send_query(RvDnsEngine *channel, rvQuery *query, RvInt64 now)
{
#undef FUNC
#define FUNC "ares__send_query"

    RvStatus status;
    rvServerState *server;
    RvSize_t count = 0;
    RvInt64 timeout;

    if (channel->nservers <= 0) {
        end_query(channel, query, ARES_ENOSERVERS, 0, 0);
        return ARES_ENOSERVERS;
    }

    DnsLogDebug((LOG_FUNC "Sending query qid=%d to net", query->user_qid));

    server = &channel->servers[query->server];
    if (query->using_tcp)
    {
        /* Make sure the TCP socket for this server is set up and queue
         * a send request.
         */
        if (server->tcp_socket.fd.fd == RV_INVALID_SOCKET)
        {
            if (open_socket(channel, query->server, RvSocketProtocolTcp) != ARES_SUCCESS)
            {
                /*query->skip_server[query->server] = 1;*/
                SKIP_SERVER_SET(query, query->server);
                next_server(channel, query, now);
                return ARES_SUCCESS;
            }
        }

        /* qbuf points to the UDP-style query
         * TCP-style queries are prepended by 2-bytes query length
         */
        query->tcp_data  = query->qbuf - 2;
        query->tcp_len   = query->qlen + 2;
        query->tcp_next  = NULL;
        if (server->qtail)
            server->qtail->tcp_next = query;
        else
        {
            server->qhead = query;
            status = RvSelectUpdate(channel->selectEngine, &server->tcp_socket.fd,
                                    RV_SELECT_READ | RV_SELECT_WRITE | RV_SELECT_CONNECT, selectCb);
            if (status != RV_OK)
            {
                end_query(channel, query, ARES_ESERVICE, NULL, 0);
                return ARES_ESERVICE;
            }
        }
        server->qtail = query;
        query->timeout = RvInt64Const(0,0,0);
    }
    else  /* Send using UDP */
    {
        if (server->udp_socket.fd.fd == RV_INVALID_SOCKET)
        {
            if (open_socket(channel, query->server, RvSocketProtocolUdp) != ARES_SUCCESS)
            {
                DnsLogError((LOG_FUNC "Opening UDP socket for server %d failed (qid=%d)", query->server, query->user_qid));
                SKIP_SERVER_SET(query, query->server);
                /*query->skip_server[query->server] = 1;*/
                next_server(channel, query, now);
                return ARES_SUCCESS;
            }
        }

        /* calculate timeout and start a timer BEFORE sending the query
         * only to make sure that the we have available timer
         */
        {

            RvRandom rf;
            RvInt64  rfs;

            RvRandomGeneratorGetInRange(&channel->rnd, 500, &rf);
            rfs = rf * 1000000;
            timeout = channel->timeout + rfs;
        }

        status = RvTimerStart(&query->timer, channel->timerQueue, RV_TIMER_TYPE_ONESHOT,
                              timeout, (RvTimerFunc)timerCb, &server->udp_socket);
        if (status != RV_OK)
        {
            DnsLogError((LOG_FUNC "Starting timer failed for qid=%d", query->user_qid));
            end_query(channel, query, ARES_ESERVICE, NULL, 0);
            return ARES_ESERVICE;
        }

        query->timeout = RvInt64Add(now, timeout);
        DnsLogDebug((LOG_FUNC "Timer for %d secs was started for qid=%d", (RvInt)(timeout /  RV_TIME64_NSECPERSEC), query->user_qid));

        status = RvSocketSendBuffer(&server->udp_socket.fd.fd, (RvUint8*)query->qbuf,
                                    query->qlen, &server->addr, channel->logMgr, &count);
        if (status != RV_OK)
        {
            DnsLogError((LOG_FUNC "Sending query for qid=%d failed", query->user_qid));
            RvTimerCancel(&query->timer, RV_TIMER_CANCEL_DONT_WAIT_FOR_CB);
            query->timeout = RvInt64Const(0,0,0);
            SKIP_SERVER_SET(query, query->server);
            /*query->skip_server[query->server] = 1;*/
            next_server(channel, query, now);
            return ARES_SUCCESS;
        }
        DnsLogDebug((LOG_FUNC "Query with qid=%d sent using UDP", query->user_qid));
    }

    return ARES_SUCCESS;
}

void ares__close_sockets(RvDnsEngine *channel, int i)
{
    rvServerState *server = &channel->servers[i];

    /* Close the TCP and UDP sockets. */
    if (server->tcp_socket.fd.fd != RV_INVALID_SOCKET)
    {
        RvSelectRemove(channel->selectEngine, &server->tcp_socket.fd);
        RvSocketDestruct(&server->tcp_socket.fd.fd, RV_FALSE, NULL, NULL);
        server->tcp_socket.fd.fd = RV_INVALID_SOCKET;
    }

    if (server->udp_socket.fd.fd != RV_INVALID_SOCKET)
    {
        RvSelectRemove(channel->selectEngine, &server->udp_socket.fd);
        RvSocketDestruct(&server->udp_socket.fd.fd, RV_FALSE, NULL, NULL);
        server->udp_socket.fd.fd = RV_INVALID_SOCKET;
    }
}

/* Something interesting happened on the wire, or there was a timeout.
 * See what's up and respond accordingly.
 */
static void selectCb(RvSelectEngine *selectEngine, RvSelectFd *fd,
                     RvSelectEvents selectEvent, RvBool error)
{
    RvStatus status;
    select_fd_args *fd_args = (select_fd_args*)fd;
    RvDnsEngine *channel = fd_args->channel;
    RvInt64 now = RvTimestampGet(channel->logMgr);

    RV_UNUSED_ARG(selectEngine);
    RV_UNUSED_ARG(selectEvent);

    RvLogEnter(channel->dnsSource,
        (channel->dnsSource, "selectCb(engine=%p)", channel));

    status = RvLockGet(&channel->lock, channel->logMgr);
    if (status == RV_OK)
    {
        if (!error)  /* Note: error is not really boolean but integer.
                        error != 0 is normally TCP connection problem */
        {
            if (fd_args->protocol == RvSocketProtocolTcp)
            {
                write_tcp_data(channel, fd_args, now);
                read_tcp_data(channel, fd_args, now);
            }
            if (fd_args->protocol == RvSocketProtocolUdp)
            {
                read_udp_packets(channel, fd_args, now);
            }
        } else {
            handle_error(channel, fd_args->server, now);
        }

        /* process_timeouts(channel, now); */

        RvLockRelease(&channel->lock, channel->logMgr);
    }

    RvLogLeave(channel->dnsSource,
        (channel->dnsSource, "selectCb(engine=%p)", channel));
}

static RvBool timerCb(select_fd_args *fd_args)
{
    RvStatus status;
    RvDnsEngine *channel = fd_args->channel;
    RvInt64 now = RvTimestampGet(channel->logMgr);

    RvLogEnter(channel->dnsSource,
        (channel->dnsSource, "timerCb(engine=%p)", channel));

    status = RvLockGet(&channel->lock, channel->logMgr);
    if (status == RV_OK)
    {
        process_timeouts(fd_args->channel, now);

        RvLockRelease(&channel->lock, channel->logMgr);
    }

    RvLogLeave(channel->dnsSource,
        (channel->dnsSource, "timerCb(engine=%p)", channel));

    return RV_FALSE;  /* don't reschedule */
}

/* If any TCP sockets select true for writing, write out queued data
 * we have for them.
 */
static void write_tcp_data(RvDnsEngine *channel, select_fd_args *fd_args, RvInt64 now)
{
    RvStatus status;
    rvServerState *server;
    rvQuery *query;
    RvInt bytesSent;
#if !defined(NEED_MANUAL_WRITEV)
    IOVEC vec[DEFAULT_IOVEC_LEN];
    int n;
#endif

    /* Make sure server has data to send and is selected in write_fds. */
    server = &channel->servers[fd_args->server];
    if (!server->qhead || &server->tcp_socket != fd_args)
        return;

#if !defined(NEED_MANUAL_WRITEV)
    /* Count the number of send queue items. */
    n = 0;
    for (query = server->qhead; query; query = query->tcp_next)
        n++;

    if (n <= DEFAULT_IOVEC_LEN)
    {
        /* Fill in the iovecs and send. */
        n = 0;
        for (query = server->qhead; query; query = query->tcp_next)
        {
            vec[n].iov_base = query->tcp_data;
            vec[n].iov_len  = query->tcp_len;
            n++;
        }
        bytesSent = (int)writev((int)(server->tcp_socket.fd.fd), vec, n);
        if (bytesSent < 0)
        {
            RvLogError(channel->dnsSource, (channel->dnsSource,
                "write_tcp_data: Error in writev(errno=%d)", errno));
            handle_error(channel, fd_args->server, now);
            return;
        }

        /* Advance the send queue by as many bytes as we sent. */
        while (bytesSent)
        {
            query = server->qhead;
            if (bytesSent >= query->tcp_len)
            {
                bytesSent -= query->tcp_len;
                server->qhead = query->tcp_next;
                if (server->qhead == NULL)
                    server->qtail = NULL;
            }
            else
            {
                query->tcp_data += bytesSent;
                query->tcp_len  -= bytesSent;
                break;
            }
        }
    }
    else
#endif
    {
        RvSize_t count = 0;

        /* Can't use writev; just send the first request. */
        query = server->qhead;
        status = RvSocketSendBuffer(&server->tcp_socket.fd.fd, (RvUint8*)query->tcp_data,
                                    query->tcp_len, NULL, channel->logMgr, &count);
        if (status != RV_OK)
        {
            RvLogError(channel->dnsSource, (channel->dnsSource,
                "write_tcp_data: Error in RvSocketSendBuffer(status=%d)", status));
            handle_error(channel, fd_args->server, now);
            return;
        }

        bytesSent = (RvInt)count;
        /* Advance the send queue by as many bytes as we sent. */
        if (bytesSent == query->tcp_len)
        {
            server->qhead = query->tcp_next;
            if (server->qhead == NULL)
                server->qtail = NULL;
        }
        else
        {
            query->tcp_data += bytesSent;
            query->tcp_len  -= bytesSent;
        }
    }
    
    if (server->qhead == NULL)
    {
        status = RvSelectUpdate(channel->selectEngine, &server->tcp_socket.fd,
                                RV_SELECT_READ, selectCb);
        if (status != RV_OK)
        {
            RvLogError(channel->dnsSource, (channel->dnsSource,
                "write_tcp_data: Error in RvSelectUpdate(status=%d)", status));
            handle_error(channel, fd_args->server, now);
            return;
        }
    }
}

/* If any TCP socket selects true for reading, read some data,
 * allocate a buffer if we finish reading the length word, and process
 * a packet if we finish reading one.
 */
static void read_tcp_data(RvDnsEngine *channel, select_fd_args *fd_args, RvInt64 now)
{
    RvStatus status;
    rvServerState *server;
    RvSize_t count;

    /* Make sure the server has a socket and is selected in read_fds. */
    server = &channel->servers[fd_args->server];
    if (&server->tcp_socket != fd_args)
        return;

    if (server->tcp_lenbuf_pos < TCP_LENWORD_SIZE)
    {
        /* We haven't yet read a length word, so read that (or what's left to read of it). */
        status = RvSocketReceiveBuffer(&server->tcp_socket.fd.fd,
                                       server->tcp_lenbuf + server->tcp_lenbuf_pos,
                                       TCP_LENWORD_SIZE - server->tcp_lenbuf_pos,
                                       channel->logMgr, &count, NULL);
        if (status != RV_OK)
        {
            RvLogError(channel->dnsSource, (channel->dnsSource,
                "read_tcp_data: Error in RvSocketReceiveBuffer(status=%d)", status));
            handle_error(channel, fd_args->server, now);
            return;
        }

        server->tcp_lenbuf_pos += (RvInt)count;
        if (server->tcp_lenbuf_pos < TCP_LENWORD_SIZE)
            return;

        /* We finished reading the length word so decode the length. */
        server->tcp_length = (server->tcp_lenbuf[0] << 8) | server->tcp_lenbuf[1];
        if (server->tcp_length > channel->tcp_bufflen)
        {
            RvLogError(channel->dnsSource, (channel->dnsSource,
                "read_tcp_data: Invalid length word(%d)", server->tcp_length));
            handle_error(channel, fd_args->server, now);
            return;
        }
        server->tcp_buffer_pos = 0;
    }

    /* Read data into the allocated buffer. */
    status = RvSocketReceiveBuffer(&server->tcp_socket.fd.fd,
                                   server->tcp_buffer + server->tcp_buffer_pos,
                                   server->tcp_length - server->tcp_buffer_pos,
                                   channel->logMgr, &count, NULL);
    if (status != RV_OK)
    {
        RvLogError(channel->dnsSource, (channel->dnsSource,
            "read_tcp_data: Error in RvSocketReceiveBuffer(status=%d)", status));
        handle_error(channel, fd_args->server, now);
        return;
    }

    server->tcp_buffer_pos += (RvInt)count;
    if (server->tcp_buffer_pos < server->tcp_length)
        return;

    /* We finished reading this answer; process it and
     * prepare to read another length word.
     */
    process_answer(channel, server->tcp_buffer, server->tcp_length,
                   fd_args->server, 1, now);

    server->tcp_lenbuf_pos = 0;
}

/* If any UDP sockets select true for reading, process them. */
static void read_udp_packets(RvDnsEngine *channel, select_fd_args *fd_args, RvInt64 now)
{
    RvStatus status;
    rvServerState *server;
    RvSize_t count;
    unsigned char buf[PACKETSZ + 1];
    RvAddress remoteAddress; /* just to make sure that RvSocketReceiveBuffer will use UDP (on Nucleus) */
    RvSocket sock;

    /* Make sure the server has a socket and is selected in read_fds. */
    server = &channel->servers[fd_args->server];
    if (&server->udp_socket != fd_args)
            return;
    sock = server->udp_socket.fd.fd;

    for(;;) {
        count = 0;
        status = RvSocketReceiveBuffer(&sock, buf, sizeof(buf),
            channel->logMgr, &count, &remoteAddress);

        if(count == 0) {
            break;
        }

        if (status != RV_OK)
        {
            /* IP address exist but doesn't have a DNS server */
            RvLogError(channel->dnsSource, (channel->dnsSource,
                "read_udp_packets: Error in RvSocketReceiveBuffer(status=%d)", status));
            handle_error(channel, fd_args->server, now);
            break;   
        }

        process_answer(channel, buf, (int)count, fd_args->server, 0, now);
        if(sock != server->udp_socket.fd.fd) {
            /* Sanity check: as the result of process_answer original socket may be destroyed */
            break;
        }
    }
}

/* If any queries have timed out, note the timeout and move them on. */
static void process_timeouts(RvDnsEngine *channel, RvInt64 now)
{
#undef FUNC
#define FUNC "process_timeouts"

    rvQuery *query;

    (void)rvDnsEngineSanityCheck(channel);

    query = findExpiredQuery(channel, now);

    if(query == 0) {
        return;
    }

#ifndef RV_TIMER_CLEAR
    query->timer.event = 0;
#else
    RV_TIMER_CLEAR(&query->timer);
#endif
    query->error_status = ARES_ETIMEOUT;

    DnsLogDebug((LOG_FUNC "Timeout on query qid=%d", query->user_qid));

    /* next_server function eventually may unlock 'channel', so after this function
    *  we may not assume nothing about query queue, so we start from the beginning 
    */
    next_server(channel, query, now);
}

/* Handle an answer from a server. */
static void process_answer(RvDnsEngine *channel, unsigned char *abuf,
                           int alen, int whichserver, int tcp, RvInt64 now)
{
    int tc, rcode;
    rvQuery *query;
    int id;

    /* If there's no room in the answer for a header, we can't do much with it. */
    if (alen < HFIXEDSZ)
        return;

    /* Find the query corresponding to this packet. */
    id = DNS_HEADER_QID(abuf);
    query = findQueryByQid(channel, id);


    if (!query)
        return;

    /* Grab the truncate bit and response code from the packet. */
    tc = DNS_HEADER_TC(abuf);
    rcode = DNS_HEADER_RCODE(abuf);

    /* If we got a truncated UDP packet and are not ignoring truncation,
     * don't accept the packet, and switch the query to TCP if we hadn't
     * done so already.
     */
    if ((tc || alen > PACKETSZ) && !tcp && !(channel->flags & ARES_FLAG_IGNTC))
    {
        if (!query->using_tcp)
        {
            query->using_tcp = 1;
            ares__send_query(channel, query, now);
        }
        return;
    }

    /* Limit alen to PACKETSZ if we aren't using TCP (only relevant if we
     * are ignoring truncation.
     */
    if (alen > PACKETSZ && !tcp)
        alen = PACKETSZ;

    /* If we aren't passing through all error packets, discard packets
     * with SERVFAIL, NOTIMP, or REFUSED response codes.
     */
    if(rcode == SERVFAIL || rcode == NOTIMP || rcode == REFUSED)
    {
        SKIP_SERVER_SET(query, whichserver);
        /*query->skip_server[whichserver] = 1;*/
        if (query->server == whichserver)
            next_server(channel, query, now);
        return;
    }
    
    if (!same_questions((const unsigned char*)query->qbuf, query->qlen, abuf, alen))
    {
        if (query->server == whichserver)
            next_server(channel, query, now);
        return;
    }

    end_query(channel, query, ARES_SUCCESS, abuf, alen);
}


static int same_questions(const unsigned char *qbuf, int qlen,
                          const unsigned char *abuf, int alen)
{
    struct {
        const unsigned char *p;
        int qdcount;
        char name[RV_DNS_MAX_NAME_LEN+1];
        int namelen;
        int type;
        int dnsclass;
    } q, a;
    int i, j;

    if (qlen < HFIXEDSZ || alen < HFIXEDSZ)
        return 0;

    /* Extract qdcount from the request and reply buffers and compare them. */
    q.qdcount = DNS_HEADER_QDCOUNT(qbuf);
    a.qdcount = DNS_HEADER_QDCOUNT(abuf);
    if (q.qdcount != a.qdcount)
        return 0;

    /* For each question in qbuf, find it in abuf. */
    q.p = qbuf + HFIXEDSZ;
    for (i = 0; i < q.qdcount; i++)
    {
        /* Decode the question in the query. */
        q.namelen = ares_expand_name(q.p, qbuf, qlen, q.name, sizeof(q.name));
        if (q.namelen < 0)
            return 0;
        q.p += q.namelen;
        if (q.p + QFIXEDSZ > qbuf + qlen)
            return 0;
        q.type = DNS_QUESTION_TYPE(q.p);
        q.dnsclass = DNS_QUESTION_CLASS(q.p);
        q.p += QFIXEDSZ;

        /* Search for this question in the answer. */
        a.p = abuf + HFIXEDSZ;
        for (j = 0; j < a.qdcount; j++)
        {
            /* Decode the question in the answer. */
            a.namelen = ares_expand_name(a.p, abuf, alen, a.name, sizeof(a.name));
            if (a.namelen < 0)
                return 0;
            a.p += a.namelen;
            if (a.p + QFIXEDSZ > abuf + alen)
                return 0;
            a.type = DNS_QUESTION_TYPE(a.p);
            a.dnsclass = DNS_QUESTION_CLASS(a.p);
            a.p += QFIXEDSZ;

            /* Compare the decoded questions. */
            if (strcasecmp(q.name, a.name) == 0 &&
                q.type == a.type && q.dnsclass == a.dnsclass)
                break;
        }

        if (j == a.qdcount)
            return 0;
    }
    return 1;
}

static void handle_error(RvDnsEngine *channel, int whichserver, RvInt64 now)
{
    rvQuery *query;

    RvLogDebug(channel->dnsSource, (channel->dnsSource,
        "handle_error: Closing socket for server no. %d", whichserver));

    /* Reset communications with this server. */
    ares__close_sockets(channel, whichserver);

    /* Tell all queries talking to this server to move on and not try
     * this server again.
     */
    for(;;) {

        query = findQueryByServer(channel, whichserver);
        if(query == 0) {
            break;
        }

        SKIP_SERVER_SET(query, whichserver);
        /*query->skip_server[whichserver] = 1;*/
        next_server(channel, query, now);
    }
}

static int open_socket(RvDnsEngine *channel, int whichserver, RvSocketProtocol protocol)
{
    RvStatus status;
    rvServerState *server;
    RvUint16 port;
    RvUint16 origPort;
    select_fd_args *fd_args;
    RvSocket s;

    server = &channel->servers[whichserver];

    if (protocol == RvSocketProtocolTcp)
    {
        port = channel->tcp_port;
        fd_args = &server->tcp_socket;
    }
    else
    {
        port = channel->udp_port;
        fd_args = &server->udp_socket;
    }

    /* Acquire a socket. */
#if (RV_NET_TYPE & RV_NET_IPV6)
    if (server->addr.addrtype == RV_ADDRESS_TYPE_IPV6)
        status = RvSocketConstruct(RV_ADDRESS_TYPE_IPV6, protocol, channel->logMgr, &s);
    else
#endif
        status = RvSocketConstruct(RV_ADDRESS_TYPE_IPV4, protocol, channel->logMgr, &s);
    if (status != RV_OK)
        return ARES_ESERVICE;

#if defined(UPDATED_BY_SPIRENT)
    if(channel->if_name[0]){ 
        Spirent_RvSocketBindDevice(&s,channel->if_name,channel->logMgr);
        if(RvAddressSetIpPort(&channel->localAddr,0)){
            if(RvSocketBind(&s,&channel->localAddr,NULL,channel->logMgr)){
                    RvLogError(channel->dnsSource, (channel->dnsSource, "open_socket: bind socket failed." ));
            }
        }

    }

#endif

    /* Set the server port number. */
    origPort = RvAddressGetIpPort(&server->addr);
    if(origPort == 0) {
        RvAddressSetIpPort(&server->addr, port);
    }

    /* Set the socket non-blocking. */
    status = RvSocketSetBlocking(&s, RV_FALSE, channel->logMgr);
    if (status != RV_OK)
    {
        RvLogError(channel->dnsSource, (channel->dnsSource,
            "open_socket: Error in RvSocketSetBlocking(%d)", status));
        RvSocketDestruct(&s, RV_FALSE, NULL, channel->logMgr);
        return ARES_ESERVICE;
    }

    /* If using TCP connect to the server. */
    if (protocol == RvSocketProtocolTcp)
    {
        status = RvSocketConnect(&s, &server->addr, channel->logMgr);
        if (status != RV_OK)
        {
            RvLogError(channel->dnsSource, (channel->dnsSource,
                "open_socket: Error in RvSocketConnect(%d)", status));
            RvSocketDestruct(&s, RV_FALSE, NULL, channel->logMgr);
            return ARES_ESERVICE;
        }

        server->tcp_lenbuf_pos = 0;
        server->qhead          = NULL;
        server->qtail          = NULL;
    }

    RvFdConstruct(&fd_args->fd, &s, channel->logMgr);
    /* save fd_args data */
    fd_args->protocol = protocol;
    fd_args->server   = whichserver;
    fd_args->channel  = channel;

    /* Register the selectCB function for this socket in the select engine */
    status = RvSelectAdd(channel->selectEngine, &fd_args->fd, RV_SELECT_READ, selectCb);
    if (status != RV_OK)
    {
        RvLogError(channel->dnsSource, (channel->dnsSource,
            "open_socket: Error in RvSelectAdd(%d)", status));
        RvSocketDestruct(&s, RV_FALSE, NULL, channel->logMgr);
        fd_args->fd.fd = RV_INVALID_SOCKET;
        return ARES_ESERVICE;
    }


    return ARES_SUCCESS;
}



static void next_server(RvDnsEngine *channel, rvQuery *query, RvInt64 now)
{
	RvInt foundServer = -1;

    if(query->serversGeneration != channel->serversGeneration) {
#if defined(RV_DNS_RESTART_QUERIES_ON_SERVERCHANGE)
        RvInt i;

        /* Probably, new servers where set on dnsEngine, restart this query */
        query->serversGeneration = channel->serversGeneration;
        query->server = 0;
        query->try_index = 0;
        for(i = 0; i < channel->skipServerWords; i++) {
            query->skip_server[i] = 0;
        }

        foundServer = 0;
        RvLogDebug(channel->dnsSource, (channel->dnsSource,
            "New servers installed for dnsEngine, restarting query qid=%d", query->user_qid));
#else
        RvLogDebug(channel->dnsSource, (channel->dnsSource,
            "New servers installed for dnsEngine, ending query qid=%d", query->user_qid));
        foundServer = -1;
#endif /*#if defined(RV_DNS_RESTART_QUERIES_ON_SERVERCHANGE)*/
       
    } else {
        query->try_index++;
        if(query->try_index < channel->tries) {
            /* Another attempts left */

            RvInt nServers;
            RvInt lastServer;
            RvInt isTcpQuery = query->using_tcp;
            RvInt curServer;

            nServers = channel->nservers;
            curServer = lastServer = query->server;

            do {
                /* Advance to the next server and wrap around if last server reached */
                curServer++;
                if(curServer == nServers) {
                    curServer = 0;
                    if(isTcpQuery) {
                        /* No more than 1 attempt per server for TCP queries */
                        break;
                    }
                }
                if(!SKIP_SERVER(query, curServer)) {
                    /* If server wasn't marked as 'skip' - choose it */
                    foundServer = curServer;
                    break;
                }
            } while(curServer != lastServer);
        }
    }

	if(foundServer >= 0) {
		query->server = foundServer;
		RvLogDebug(channel->dnsSource, (channel->dnsSource,
			"Attempt #%d for qid=%d server %d, %d attempts left", query->try_index, query->user_qid, query->server, channel->tries - query->try_index - 1));
		ares__send_query(channel, query, now);
	} else {
		end_query(channel, query, ARES_ENDOFSERVERS, NULL, 0);
	}
}

static void end_query(RvDnsEngine *channel, rvQuery *query, int queryStatus,
                      unsigned char *abuf, int alen)
{
#undef FUNC
#define FUNC "end_query"

    rvQuery *q;
    int rcode, ancount, i;
    rvAresCallback user_callback;
    void *user_arg;
    unsigned int user_qid;
    RvDnsNewRecordCB newRecordCB;

    if (RvInt64IsNotEqual(query->timeout, RvInt64Const(0,0,0)))
        RvTimerCancel(&query->timer, RV_TIMER_CANCEL_DONT_WAIT_FOR_CB);

    if (queryStatus == ARES_SUCCESS && abuf != NULL)
    {
        /* Pull the response code and answer count from the packet. */
        rcode = DNS_HEADER_RCODE(abuf);
        ancount = DNS_HEADER_ANCOUNT(abuf);

        /* Convert errors. */
        switch (rcode)
        {
        case NOERROR:
            queryStatus = (ancount > 0) ? ARES_SUCCESS : ARES_ENODATA;
            break;
        case FORMERR:
            queryStatus = ARES_EFORMERR;
            break;
        case SERVFAIL:
            queryStatus = ARES_ESERVFAIL;
            break;
        case NXDOMAIN:
            queryStatus = ARES_ENOTFOUND;
            break;
        case NOTIMP:
            queryStatus = ARES_ENOTIMP;
            break;
        case REFUSED:
            queryStatus = ARES_EREFUSED;
            break;
        }

        if (queryStatus != ARES_SUCCESS)
        {
            RvLogDebug(channel->dnsSource, (channel->dnsSource,
                       "end_query(qid=%d): Erroneous response code (%d)",
                       query->user_qid, queryStatus));
        }

        if(queryStatus == ARES_ENOTFOUND || queryStatus == ARES_ENODATA) {
            rvDnsTreatNxdomain(channel, abuf, alen);
        }

    }


    /* First, remove the query from the chain in the channel */
    q = removeQueryByUqid(channel, query->user_qid);

    if(q != query) {
        DnsLogExcep((LOG_FUNC "Unexpected: wrong query found for qid=%d (expected %p, got %p)", query, q));
    }

    /* Simple cleanup policy: if no queries are remaining, close all
     * network sockets unless STAYOPEN is set.
     */
    if (!channel->queries && !(channel->flags & ARES_FLAG_STAYOPEN))
    {
        for (i = 0; i < channel->nservers; i++)
            ares__close_sockets(channel, i);
    }

    /* Next, call the user callback,
     * but only after releasing the channel lock to prevent deadlock
     */

    user_callback = query->user_callback;
    user_arg = query->user_arg;
    user_qid = query->user_qid;
    newRecordCB = query->newRecordCB;
   
    RvLogDebug(channel->dnsSource, (channel->dnsSource,
               "ARES: calling internal CB (engine=%p,context=%p,qid=%d,status=%d)",
               channel, user_arg, user_qid, queryStatus));

    /* Only internal callbacks are called here - no unlock on DNS engine: DNS will be unlocked
     * inside this callback before calling "real" user callback
     */
    user_callback(channel, newRecordCB, user_arg, queryStatus, user_qid, abuf, alen);
    
    RvLogDebug(channel->dnsSource, (channel->dnsSource,
               "ARES: internal CB returned (engine=%p,context=%p,qid=%d)",
               channel, user_arg, user_qid));

}
#endif /* RV_DNS_ARES */
