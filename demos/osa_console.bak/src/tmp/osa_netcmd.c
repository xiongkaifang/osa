/** ============================================================================
 *
 *  codecs_audo_discover.c
 *
 *  Author     : xkf
 *
 *  Date       : Oct 28, 2013
 *
 *  Description: 
 *  ============================================================================
 */

/*  --------------------- Include system headers ---------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/*  --------------------- Include user headers   ---------------------------- */
#include "codecs_common.h"
#include "codecs_auto_discover.h"

#include "osa.h"
#include "osa_task.h"
#include "osa_timer.h"
#include "../include/osa_mutex.h"
#include "debug.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 *  --------------------- Macro definition -------------------------------------
 */

/** ============================================================================
 *  @Macro:         Macro name
 *
 *  @Description:   Description of this macro.
 *  ============================================================================
 */
#define USE_MCAST

#ifdef	USE_MCAST
#define MCAST_GROUP			(0x250000EF)	/* 239.0.0.37 */
#endif

#define CODECS_AUTO_DISCOVER_PORT           (24602)

#define CODECS_AUTO_DISCOVER_CMD_PROC       (0x6000)

#define CODECS_AUTO_DISCOVER_CMD_START      (0x6001)

#define CODECS_AUTO_DISCOVER_CMD_STOP       (0x6002)

#define CODECS_AUTO_DISCOVER_ENTER()        \
    do {                                    \
        mutex_lock(&glb_codecs_auto_mutex); \
    } while (0)

#define CODECS_AUTO_DISCOVER_LEAVE()        \
    do {                                    \
        mutex_unlock(&glb_codecs_auto_mutex);\
    } while (0)

/*
 *  --------------------- Structure definition ---------------------------------
 */

/** ----------------------------------------------------------------------------
 *  @Name:          Structure name
 *
 *  @Description:   Description of the structure.
 *
 *  @Field:         Field1 member
 *
 *  @Field          Field2 member
 *  ----------------------------------------------------------------------------
 */
struct __auto_discover_payload_t
{
	char            m_flag[32];
	char            m_serial_number[64];
	char            m_mac_addr[20];
	char            m_version[24];
	char            m_hostname[64];
	unsigned int    m_hw_rev;
	unsigned int    m_ip_be;
};

struct __codecs_auto_discover_object_t;
typedef struct __codecs_auto_discover_object_t codecs_auto_discover_object_t;
struct __codecs_auto_discover_object_t
{
    codecs_auto_discover_prm_t
                    m_prm;
    int             m_sockfd;

    int             m_port;

    unsigned char   m_buffer[64];

    int             m_auto_id;
    osa_event_t     m_auto_event;

    unsigned int    m_started;

    struct __auto_discover_payload_t
                    m_payload;
};

/*
 *  --------------------- Global variable definition ---------------------------
 */

/** ----------------------------------------------------------------------------
 *  @Name:          Variable name
 *
 *  @Description:   Description of the variable.
 * -----------------------------------------------------------------------------
 */
static status_t 
__codecs_auto_discover_external_main(void *ud, task_t tsk, msg_t **msg);

static status_t
__codecs_auto_discover_init(codecs_auto_discover_object_t *pobj, const codecs_auto_discover_prm_t *prm);

static status_t
__codecs_auto_discover_do_initialize(codecs_auto_discover_object_t *pobj);

static status_t
__codecs_auto_discover_do_deinitialize(codecs_auto_discover_object_t *pobj);

static status_t
__codecs_auto_discover_do_process(codecs_auto_discover_object_t *pobj);

static status_t
__codecs_auto_discover_start(codecs_auto_discover_object_t *pobj);

static status_t
__codecs_auto_discover_stop(codecs_auto_discover_object_t *pobj);

static status_t
__codecs_auto_discover_event_handler(void *ud);

static status_t
__codecs_auto_discover_deinit(codecs_auto_discover_object_t *pobj);

static codecs_auto_discover_object_t glb_auto_discover_obj;

static task_object_t glb_auto_discover_tsk_obj = 
{
    .m_name       = "AUTO_DISCOVER_TSK",
    .m_main       = __codecs_auto_discover_external_main,
    .m_pri        = 0,
    .m_stack_size = 0,
    .m_init_state = 0,
    .m_userdata   = &glb_auto_discover_obj,
    .m_task       = TASK_INVALID_TSK
};

OSA_DECLARE_AND_INIT_MUTEX(glb_codecs_auto_mutex);

static unsigned int glb_cur_init = 0;

static char query_msg[32] = "Who is VED/encoder?";

static struct timespec glb_timespec =
{
	.tv_sec  = 0,
	.tv_nsec = 100000000 // 100ms
};

/*
 *  --------------------- Local function forward declaration -------------------
 */

/*
 *  --------------------- Public function definition ---------------------------
 */
status_t codecs_auto_discover_init(const codecs_auto_discover_prm_t *prm)
{
    status_t status = OSA_SOK;
    codecs_auto_discover_object_t *pobj = &glb_auto_discover_obj;

    CODECS_AUTO_DISCOVER_ENTER();

    if (glb_cur_init++ == 0) {
        status = __codecs_auto_discover_init(pobj, prm);
    }

    CODECS_AUTO_DISCOVER_LEAVE();

    return status;
}

status_t codecs_auto_discover_start(void)
{
    task_object_t *ptsk = &glb_auto_discover_tsk_obj;

    return task_mgr_synchronize(ptsk, CODECS_AUTO_DISCOVER_CMD_START, NULL, 0, 0);
}


status_t codecs_auto_discover_stop(void)
{
    task_object_t *ptsk = &glb_auto_discover_tsk_obj;

    return task_mgr_synchronize(ptsk, CODECS_AUTO_DISCOVER_CMD_STOP, NULL, 0, 0);
}

status_t codecs_auto_discover_deinit(void)
{
    status_t status = OSA_SOK;
    codecs_auto_discover_object_t *pobj = &glb_auto_discover_obj;
    
    CODECS_AUTO_DISCOVER_ENTER();

    if (--glb_cur_init == 0) {
        status = __codecs_auto_discover_deinit(pobj);
    }

    CODECS_AUTO_DISCOVER_LEAVE();

    return status;
}

status_t codecs_auto_discover(unsigned int bcast_ip_be, unsigned int bind_ip_be, CODECS_AUTO_DISCOVER_CALLBACK fxn, void *ud)
{
	int fd;
	struct sockaddr_in bcast_addr;
	struct sockaddr_in bind_addr;
	struct sockaddr_in peer_addr;
	socklen_t peer_addr_len;
	int i, j;
	int r;
	struct __auto_discover_payload_t payload;

	if(bcast_ip_be == 0) {
		bcast_ip_be = MCAST_GROUP;
	}

	memset(&bcast_addr, 0, sizeof(bcast_addr));
	bcast_addr.sin_family = AF_INET;
	bcast_addr.sin_addr.s_addr = bcast_ip_be;
	bcast_addr.sin_port = htons(CODECS_AUTO_DISCOVER_PORT);

	memset(&peer_addr, 0, sizeof(peer_addr));
	peer_addr.sin_family = AF_INET;

	memset(&bind_addr, 0, sizeof(bind_addr));
	bind_addr.sin_family = AF_INET;

	bind_addr.sin_addr.s_addr = bind_ip_be;
	bind_addr.sin_port = htons(0);

	fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (fd < 0) {
		DBG(DBG_ERROR, "auto_discover: Failed to create socket.\n");
		return OSA_EFAIL;
	}

	if (bind_ip_be) {
		if (bind(fd, (struct sockaddr *)&bind_addr, sizeof(bind_addr))) {
			DBG(DBG_ERROR, "auto_discover: Failed to bind source ip address.\n");
			close(fd);
			return OSA_EFAIL;
		}
	}

	i = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &i, sizeof(i))) {
		DBG(DBG_ERROR, "auto_discover: Failed to set socket into broadcast mode.\n");
		close(fd);
		return OSA_EFAIL;
	}

	r = sendto(fd, &query_msg, 32, 0, (struct sockaddr *)&bcast_addr, sizeof(bcast_addr));

	if (r < 0) {
		DBG(DBG_ERROR, "auto_discover: Failed to broadcast auto-discover packet.\n");
		close(fd);
		return OSA_EFAIL;
	}

	usleep(100000);

    peer_addr_len = sizeof(peer_addr);

    status_t status = OSA_EFAIL;

    for (j = 0; j < 5; j ++) {
        r = recvfrom(fd, &payload, sizeof(payload), MSG_DONTWAIT,
                (struct sockaddr *)&peer_addr, &peer_addr_len);

        if (r > 0) {
            
            status = (*fxn)(payload.m_serial_number, payload.m_mac_addr,
                            payload.m_version, payload.m_hostname, 
                            ntohl(payload.m_hw_rev), peer_addr.sin_addr.s_addr, ud);

            if (!OSA_ISERROR(status)) {
                break;
            }
        }

        nanosleep(&glb_timespec, NULL);
    }

    close(fd);

	return status;
}

/*
 *  --------------------- Local function definition ----------------------------
 */
static status_t
__codecs_auto_discover_init(codecs_auto_discover_object_t *pobj, const codecs_auto_discover_prm_t *prm)
{
    status_t status = OSA_SOK;
    task_object_t *ptsk = &glb_auto_discover_tsk_obj;

    memset(pobj, 0, sizeof(*pobj));
    memcpy(&pobj->m_prm, prm, sizeof(*prm));

    /* Register codecs auto discover task */
    status = task_mgr_register(ptsk);
    if (OSA_ISERROR(status)) {
        return status;
    }

    /* Start codecs auto discover task */
    status = task_mgr_start(ptsk);
    if (OSA_ISERROR(status)) {
        status |= task_mgr_unregister(ptsk);
        return status;
    }

#if 0
    /* Register auto discover event */
    pobj->m_auto_event.m_fxn    = __codecs_auto_discover_event_handler;
    pobj->m_auto_event.m_ud     = (void *)ptsk;
    pobj->m_auto_event.m_delete = FALSE;

    status = osa_timer_register(&pobj->m_auto_id, 1000/* ms */, &pobj->m_auto_event);
#endif

    OSA_assert(OSA_SOK == status);

    return status;
}

static status_t
__codecs_auto_discover_deinit(codecs_auto_discover_object_t *pobj)
{
    status_t status = OSA_SOK;
    task_object_t *ptsk = &glb_auto_discover_tsk_obj;

#if 0
    /* Unregister codecs auto discover event */
    status |= osa_timer_unregister(pobj->m_auto_id);
#endif

    /* Stop codecs auto discover task */
    status |= task_mgr_stop(ptsk);

    /* Unregister codecs auto discover task */
    status |= task_mgr_unregister(ptsk);

    return status;
}

static status_t 
__codecs_auto_discover_external_main(void *ud, task_t tsk, msg_t **msg)
{
    status_t status = OSA_SOK;
    
    codecs_auto_discover_object_t * pobj = NULL;

    pobj = (codecs_auto_discover_object_t *)ud;

    switch (msg_get_cmd((*msg)))
    {
        case TASK_CMD_INIT:
            status |= task_set_state(tsk, TASK_STATE_INIT);
            status |= __codecs_auto_discover_do_initialize(pobj);

            break;

        case TASK_CMD_EXIT:
            status |= task_set_state(tsk, TASK_STATE_EXIT);
            status |= __codecs_auto_discover_stop(pobj);
            status |= __codecs_auto_discover_do_deinitialize(pobj);

            break;

        case TASK_CMD_PROC:
            status |= task_set_state(tsk, TASK_STATE_PROC);

            break;

        case CODECS_AUTO_DISCOVER_CMD_PROC:
            status |= __codecs_auto_discover_do_process(pobj);
            break;

        case CODECS_AUTO_DISCOVER_CMD_START:
            status |= __codecs_auto_discover_start(pobj);
            break;

        case CODECS_AUTO_DISCOVER_CMD_STOP:
            status |= __codecs_auto_discover_stop(pobj);
            break;

        default:
            break;
    }

    return status;
}

static status_t __codecs_auto_discover_do_initialize(codecs_auto_discover_object_t *pobj)
{
    int i;
    int sockopt = 1;
    char ip_addr[32];
    struct sockaddr_in svr_addr;
    struct ip_mreq mreq;
    status_t status = OSA_SOK;

    pobj->m_port = CODECS_AUTO_DISCOVER_PORT;

    /* Fill the payload */
    /* Get local ip address */
    status = socket_get_inet_address(pobj->m_prm.m_interface, ip_addr);

    OSA_assert(OSA_SOK == status);

    pobj->m_payload.m_ip_be = inet_addr(ip_addr);

    /* Initialize socket */
    pobj->m_sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (pobj->m_sockfd < 0) {
        DBG(DBG_ERROR, "auto_discover: Failed to open listening socket.\n");
        return OSA_EFAIL;
    }

    if (setsockopt(pobj->m_sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(sockopt))) {
        close(pobj->m_sockfd);
        pobj->m_sockfd = -1;
        DBG(DBG_ERROR, "auto_discover: Failed to set socket option to enable port number reuse.\n");
        return OSA_EFAIL;
    }

	/* Bind the socket */
    memset(&svr_addr, 0, sizeof(svr_addr));

	svr_addr.sin_family = AF_INET;
	svr_addr.sin_addr.s_addr = INADDR_ANY;
	svr_addr.sin_port = htons(pobj->m_port);

	if (bind(pobj->m_sockfd, (struct sockaddr *)&svr_addr, sizeof(svr_addr)) < 0) {
        close(pobj->m_sockfd);
        pobj->m_sockfd = -1;
        DBG(DBG_ERROR, "audo_discover: Failed to bind on UDP port %d.\n", pobj->m_port);
        status = OSA_EFAIL;
	}

    /* Add membership */
    mreq.imr_multiaddr.s_addr = MCAST_GROUP;
    mreq.imr_interface.s_addr = pobj->m_payload.m_ip_be;

    if (setsockopt(pobj->m_sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        close(pobj->m_sockfd);
        pobj->m_sockfd = -1;
        DBG(DBG_ERROR, "auto_discover: Failed to join a multicast group:%s.\n", strerror(errno));
        return OSA_EINVAL;
    }

    DBG(DBG_INFO, "auto_discover: auto discover initialized.\n");

    return status;
}

static status_t __codecs_auto_discover_do_deinitialize(codecs_auto_discover_object_t *pobj)
{
    status_t status = OSA_SOK;
	struct ip_mreq mreq;

	mreq.imr_multiaddr.s_addr = MCAST_GROUP;
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);

	setsockopt(pobj->m_sockfd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq));

    close(pobj->m_sockfd);
    pobj->m_sockfd = -1;

    return status;
}

static status_t __codecs_auto_discover_do_process(codecs_auto_discover_object_t *pobj)
{
    int i;
    int retval;
    struct sockaddr_in peer_addr;
    socklen_t addr_len;
    status_t status = OSA_SOK;

    DBG(DBG_INFO, "auto_discover: Enter.\n");

    memset(&peer_addr, 0, sizeof(peer_addr));
    peer_addr.sin_family = AF_INET;

    addr_len = sizeof(peer_addr);

    retval = recvfrom(pobj->m_sockfd, pobj->m_buffer,
            sizeof(pobj->m_buffer), MSG_DONTWAIT, (struct sockaddr *)&peer_addr, &addr_len);

    if (retval > 0) {

        DBG(DBG_INFO, "auto_discover: request received.\n");

        retval = sendto(pobj->m_sockfd, &pobj->m_payload, sizeof(struct __auto_discover_payload_t), 0,
                (struct sockaddr *)&peer_addr, addr_len);

        DBG(DBG_INFO, "auto_discover: message sent to %d.%d.%d.%d.\n",
                peer_addr.sin_addr.s_addr & 0xff,
                (peer_addr.sin_addr.s_addr >> 8) & 0xff,
                (peer_addr.sin_addr.s_addr >> 16) & 0xff,
                (peer_addr.sin_addr.s_addr >> 24) & 0xff);
    }

    DBG(DBG_INFO, "auto_discover: Leave(status=%d).\n", status);

    return status;
}

static status_t
__codecs_auto_discover_start(codecs_auto_discover_object_t *pobj)
{
    status_t status = OSA_SOK;

    if (!pobj->m_started) {
        /* Register auto discover event */
        pobj->m_auto_event.m_fxn    = __codecs_auto_discover_event_handler;
        pobj->m_auto_event.m_ud     = (void *)&glb_auto_discover_tsk_obj;
        pobj->m_auto_event.m_delete = FALSE;

        status = osa_timer_register(&pobj->m_auto_id, 500/* ms */, &pobj->m_auto_event);

        OSA_assert(OSA_SOK == status);

        pobj->m_started = TRUE;
    }

    DBG(DBG_INFO, "auto_discover: Started.\n");

    return status;
}

static status_t
__codecs_auto_discover_stop(codecs_auto_discover_object_t *pobj)
{
    status_t status = OSA_SOK;

    if (pobj->m_started) {
        /* Unregister codecs auto discover event */
        status |= osa_timer_unregister(pobj->m_auto_id);

        OSA_assert(OSA_SOK == status);

        pobj->m_started = FALSE;
    }

    DBG(DBG_INFO, "auto_discover: Stop.\n");

    return status;
}

static status_t
__codecs_auto_discover_event_handler(void *ud)
{
    return task_mgr_synchronize((task_object_t *)ud, CODECS_AUTO_DISCOVER_CMD_PROC, NULL, 0, MSG_FLAGS_WAIT_ACK);
}

#if defined(__cplusplus)
}
#endif
/** ============================================================================
 *
 *  Copyright (C), 1987 - 2014, xiong-kaifang Tech. Co.,, Ltd.
 *
 *  @File name:	osa_console.c
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2014-01-08
 *
 *  @Description:   The description of this file.
 *	
 *	                The template format for source file.
 *
 *  @Version:	    v1.0
 *
 *  @Function List:  //	主要函数及功能
 *	    1.  －－－－－
 *	    2.  －－－－－
 *
 *  @History:	     //	历史修改记录
 *
 *	<author>	    <time>	     <version>	    <desc>
 *  xiong-kaifang   2014-01-04     v1.0	        write this module.
 *
 *
 *  ============================================================================
 */

/*  --------------------- Include system headers ---------------------------- */
#include <signal.h>

/*  --------------------- Include user headers   ---------------------------- */
#include "osa.h"
#include "osa_debugger.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 *  --------------------- Macro definition -------------------------------------
 */

/** ============================================================================
 *  @Macro:         Macro name
 *
 *  @Description:   Description of this macro.
 *  ============================================================================
 */
#define OSA_CONSOLE_SESSIONS_MAX    (4)

/*
 *  --------------------- Structure definition ---------------------------------
 */

/** ----------------------------------------------------------------------------
 *  @Name:          Structure name
 *
 *  @Description:   Description of the structure.
 *
 *  @Field:         Field1 member
 *
 *  @Field          Field2 member
 *  ----------------------------------------------------------------------------
 */
struct __osa_console_session_t;
typedef struct __osa_console_session_t osa_console_session_t;
struct __osa_console_session_t
{
    unsigned int    m_reserved[2];

    unsigned int    m_id;

    int             m_sockfd;

    __be32          m_addr_be;

    osa_cmd_req_t   m_req;

    unsigned char   m_name[32];
    task_object_t   m_task_obj;
};

struct __osa_console_object_t;
typedef struct __osa_console_object_t osa_console_object_t;
struct __osa_console_object_t
{
    int             m_listen_fd;
    int             m_listen_port;

    osa_console_session_t
                    m_sessions[OSA_CONSOLE_SESSIONS_MAX];
    dlist_t         m_free_list;
    dlist_t         m_busy_list;
};

/*
 *  --------------------- Global variable definition ---------------------------
 */

/** ----------------------------------------------------------------------------
 *  @Name:          Variable name
 *
 *  @Description:   Description of the variable.
 * -----------------------------------------------------------------------------
 */

/*
 *  --------------------- Local function forward declaration -------------------
 */

/** ============================================================================
 *
 *  @Function:	    Local function forward declaration.
 *
 *  @Description:   //	函数功能、性能等的描述
 *
 *  @Calls:	        //	被本函数调用的函数清单
 *
 *  @Called By:	    //	调用本函数的函数清单
 *
 *  @Table Accessed://	被访问的表（此项仅对于牵扯到数据库操作的程序）
 *
 *  @Table Updated: //	被修改的表（此项仅对于牵扯到数据库操作的程序）
 *
 *  @Input:	        //	对输入参数的说明
 *
 *  @Output:	    //	对输出参数的说明
 *
 *  @Return:	    //	函数返回值的说明
 *
 *  @Enter          //  Precondition
 *
 *  @Leave          //  Postcondition
 *
 *  @Others:	    //	其它说明
 *
 *  ============================================================================
 */
static void debug_test_signal_handler(int sig)
{
    if (sig == SIGINT) {
        fprintf(stderr, "SIGINT signal caught, system shutdown now...\n");
        glb_tsk_mgr_exit = 1;
    } else {
        fprintf(stderr, "Invalid signal caught\n");
    }
}

/*
 *  --------------------- Public function definition ---------------------------
 */

/** ============================================================================
 *
 *  @Function:	    Public function definition.
 *
 *  @Description:   //	函数功能、性能等的描述
 *
 *  @Calls:	        //	被本函数调用的函数清单
 *
 *  @Called By:	    //	调用本函数的函数清单
 *
 *  @Table Accessed://	被访问的表（此项仅对于牵扯到数据库操作的程序）
 *
 *  @Table Updated: //	被修改的表（此项仅对于牵扯到数据库操作的程序）
 *
 *  @Input:	        //	对输入参数的说明
 *
 *  @Output:	    //	对输出参数的说明
 *
 *  @Return:	    //	函数返回值的说明
 *
 *  @Enter          //  Precondition
 *
 *  @Leave          //  Postcondition
 *
 *  @Others:	    //	其它说明
 *
 *  ============================================================================
 */
status_t osa_snet_init(const osa_snetcmd_prm_t *prm);

status_t osa_snet_deinit(void);

status_t osa_cnet_init(unsigned int ip_be, short int port);

typedef status_t (*OSA_CNET_CALLBACK_FXN)(void *buf, size_t size, void *ud);

status_t osa_cnet_send(unsigned short cmd, void *prm, unsigned int size, OSA_CNET_CALLBACK_FXN fxn, void *ud)
{
    status_t status = OSA_SOK;
    osa_cnet_object_t * pobj = &glb_osa_cnet_obj;

    if (size >= OSA_CNET_PT_SIZE) {
        return OSA_EINVAL;
    }

    mutex_lock(&pobj->m_mutex);

    msg_init(&pobj->m_msg)
    msg_set_cmd(&pobj->m_msg, cmd);
    msg_set_payload_ptr(&pobj->m_msg, prm);
    msg_set_payload_size(&pobj->m_msg, size);
    msg_set_flags(&pobj->m_msg, MSG_FLAGS_WAIT_ACK);
    msg_set_msg_size(&pobj->m_msg, sizeof(*msg));
    msg_set_msg_id(&pobj->m_msg, id);

    retval = send(pobj->m_sockfd, &pobj->m_msg, sizeof(msg_t), 0);

    if (prm != NULL && size > 0) {
        retval = send(pobj->m_sockfd, prm, size, 0);
    }

    retval = recv(pobj->m_sockfd, &pobj->m_msg, sizeof(msg_t), 0);

    size = msg_get_payload_size(&pobj->m_msg);

    if (size > 0) {
        int blk_num = size / OSA_CNET_BLK_SIZE;
        int blk_rem = size % OSA_CNET_BLK_SIZE;

        for (i = 0; i < blk_num; i++) {
            retval = recv(pobj->m_sockfd, pobj->m_payload, OSA_CNET_BLK_SIZE, 0);

            if (fxn != NULL) {
                (*fxn)(pobj->m_payload, OSA_CNET_BLK_SIZE, ud);
            }
        }

        if (blk_rem) {

            retval = recv(pobj->m_sockfd, pobj->m_payload, blk_rem, 0);

            if (fxn != NULL) {
                (*fxn)(pobj->m_payload, blk_rem, ud);
            }
        }

    }

    mutex_unlock(&pobj->m_mutex);

    return status;
}

typedef status_t (*OSA_SNET_CALLBACK_FXN)(msg_t *msg, void *ud);

struct __osa_msg_data_t;
struct __osa_msg_data_t;
struct __osa_msg_data_t
{
    msg_t           m_msg;

    union {
        struct {
            size_t  m_size;
            char    m_buf[OSA_MSG_PAYLOAD_SIZE - sizeof(size_t)];
        } m_in;

        struct {
            size_t  m_size;
            char    m_buf[OSA_MSG_PAYLOAD_SIZE - sizeof(size_t)];
        } m_out;

        char        __padding[OSA_MSG_PAYLOAD_SIZE];
    } u;
};

status_t osa_snet_process()
{
    status_t status = OSA_SOK;
    osa_snet_object_t * pobj = &glb_osa_snet_obj;

    retval = recv(pobj->m_sockfd, &pobj->m_msg, sizeof(pobj->m_msg), 0);

    size = msg_get_payload_size(&pobj->m_msg);

    if (size > 0) {
        OSA_assert(size < OSA_SNET_PAYLOAD_SIZE);

        retval = recv(pobj->m_sockfd, &pobj->m_payload, size, 0);
    }

    msg_set_payload_ptr(&pobj->m_msg, pobj->m_msg.u.m_in.m_buf);
    msg_set_payload_size(&pobj->m_msg, pobj->m_msg.u.m_in.m_size);
    msg_set_flags(&pobj->m_msg, MSG_FLAGS_WAIT_ACK);
    msg_set_msg_size(&pobj->m_msg, sizeof(msg_t));

    if (pobj->m_fxn != NULL) {
        status = (*pobj->m_fxn)(&pobj->m_msg, pobj->m_msg.u.m_out.m_buf, pobj->m_ud);
    }

    status = task_mgr_synchronize2(msg_get_cmd(&pobj->m_msg));

    return status;
}

status_t osa_cnet_deinit(void);
/*
 *  --------------------- Local function definition ----------------------------
 */

/** ============================================================================
 *
 *  @Function:	    Local function definition.
 *
 *  @Description:   //	函数功能、性能等的描述
 *
 *  ============================================================================
 */
static status_t __osa_console_init(const osa_console_prm_t *prm)
{
    

    /* Initialize osa console task */
    snprintf(pobj->m_name, sizeof (pobj->m_name) - 1, "%s", "OSA_CONSOLE_TASK");

    pobj->m_tsk_obj.m_name = pobj->m_name;
    pobj->m_tsk_obj.m_main = __osa_console_external_main;
    pobj->m_tsk_obj.m_find = NULL;
    pobj->m_tsk_obj.m_pri = 0;
    pobj->m_tsk_obj.m_stack_size = 0;
    pobj->m_tsk_obj.m_init_state = 0;
    pobj->m_tsk_obj.m_userdata = (void *)pobj;
    pobj->m_tsk_obj.m_task = TASK_INVALID_TSK;

    /* Register osa console task */
    task_object_t *ptsk = &pobj->m_tsk_obj;
    status = task_mgr_register(ptsk);
    if (OSA_ISERROR(status)) {
        return status;
    }

    /* Start osa console task */
    status = task_mgr_start(ptsk);
    if (OSA_ISERROR(status)) {
        status |= task_mgr_unregister(ptsk);
        return status;
    }

    /* Register server event */
    pobj->m_event.m_fxn    = __osa_console_event_handler;
    pobj->m_event.m_ud     = (void *)pobj;
    pobj->m_event.m_delete = FALSE;

    status = osa_timer_register(&pobj->m_id, 1000/* ms */, &pobj->m_event);

    OSA_assert(OSA_SOK == status);

    return status;
}

static status_t 
__osa_console_external_main(void *ud, task_t tsk, msg_t **msg)
{
    status_t status = OSA_SOK;
    
    codecs_net_server_object_t * pobj = NULL;

    pobj = (codecs_net_server_object_t *)ud;

    switch (msg_get_cmd((*msg)))
    {
        case TASK_CMD_INIT:
            status |= task_set_state(tsk, TASK_STATE_INIT);
            status |= __osa_console_do_initialize(pobj);

            break;

        case TASK_CMD_EXIT:
            status |= task_set_state(tsk, TASK_STATE_EXIT);
            status |= __osa_console_do_deinitialize(pobj);

            break;

        case TASK_CMD_PROC:
            status |= task_set_state(tsk, TASK_STATE_PROC);

            break;

        case OSA_CONSOLE_CMD_PROC:
            status |= __osa_console_do_process(pobj);

            break;

        default:
            break;
    }

    return status;
}

static status_t __osa_console_do_initialize(osa_console_object_t *pobj)
{
    int i;
    status_t status = OSA_SOK;

    osa_console_object_t * pobj = &glb_osa_console_obj;
    osa_console_session_t * pss = NULL;

    memset(pobj, 0, sizeof(*pobj));

    status |= mutex_create(&pobj->m_mutex);

    status |= dlist_init(&pobj->m_free_list);
    status |= dlist_init(&pobj->m_busy_list);

    OSA_assert(OSA_SOK == status)

    for (i = 0; i < OSA_ARRAYSIZE(pobj->m_sessions); i++) {
        pss = &pobj->m_sessions[i];
        pss->m_id = i;
        pss->m_sockfd = -1;

        status  = dlist_initialize_element((dlist_element_t *)pss);
        status |= dlist_put_tail(&pobj->m_free_list, (dlist_element_t *)pss);
    }

    pobj->m_listen_port = 8888;

    pobj->m_addr.sin_family = AF_INET;
    pobj->m_addr.sin_addr.s_addr = INADDR_ANY;
    pobj->m_addr.sin_port = htons(pobj->m_listen_port);

    /* Initialize server socket */
    pobj->m_listen_fd = ssocket_init(&pobj->m_addr, sizeof(pobj->m_addr));

    if (pobj->m_listen_fd < 0) {
        mutex_delete(&pobj->m_mutex);
        return OSA_EFAIL;
    }

    /* Set the server socket no blocking */
    socket_set_noblocking(pobj->m_listen_fd);

    /* Register osa console task */
    pobj->m_initialized = TRUE;

    return status;
}

static status_t __osa_console_do_process(osa_console_object_t *pobj)
{
    int fd;
    status_t status = OSA_SOK;
    struct sockaddr_in client_addr;
    socklen_t socklen = sizeof(struct sockaddr_in);
    osa_console_session_t *pss = NULL;
    osa_console_session_t *pcurr_ss = NULL;
    osa_console_session_t *pnext_ss = NULL;

    /* Free the connection disconnected */
    status = dlist_first(&pobj->m_busy_list, (dlist_element_t **)&pcurr_ss);
    while (!OSA_ISERROR(status) && pcurr_ss != NULL) {

        status = dlist_next(&pobj->m_busy_list, (dlist_element_t *)pcurr_ss, (dlist_element_t **)&pnext_ss);

        if (!pcurr_ss->m_connected) {
            status |= dlist_remove_element(&pobj->m_busy_list, (dlist_element_t *)pcurr_ss);
            status |= dlist_put_tail(&pobj->m_free_list, (dlist_element_t *)pcurr_ss);
        }

        pcurr_ss = pnext_ss;
    }

    /* Accept clinet connection */
    fd = accept(pobj->m_listen_fd, (struct sockaddr *)&client_addr, &socklen);

    if (fd > 0) {

        status = dlist_get_head(&pobj->m_free_list, (dlist_element_t **)&pss);

        if (!OSA_ISERROR(status) && pss != NULL) {
            pss->m_sockfd = fd;
            pss->m_addr  = client_addr;
            pss->m_connected = TRUE;

            status = __osa_console_create_session(pss);

            if (OSA_ISERROR(status)) {
                status = dlist_put_tail(&pobj->m_busy_list, (dlist_element_t *)pss);

                DBG(DBG_INFO, "osa console: new connection: fd = %d, ip: %d.%d.%d.%d.\n",
                        fd, 
                        (client_addr.sin_addr.s_addr >> 0 ) & 0xff,
                        (client_addr.sin_addr.s_addr >> 8 ) & 0xff,
                        (client_addr.sin_addr.s_addr >> 16) & 0xff,
                        (client_addr.sin_addr.s_addr >> 24) & 0xff
                   );
            } else {
                memset(pss, 0, sizeof(*pss));
                pss->m_sockfd = -1;
                status = dlist_put_tail(&pobj->m_free_list, (dlist_element_t *)pss);
                close(fd);
            }

        } else {
            close(fd);
        }
    }

    return status;
}

static status_t
__osa_console_event_handler(void *ud)
{
    return task_mgr_synchronize(&((osa_console_object_t *)ud)->m_tsk_obj, 
                                OSA_CONSOLE_CMD_PROC, NULL, 0, 0);
}

static int
__osa_console_deinit_apply_fxn(dlist_element_t * elem, void * data)
{
    osa_console_session_t *pss = (osa_console_session_t *)elem;

    if (pss->m_sockfd > 0) {
        close(pss->m_sockfd);
        pss->m_sockfd = -1;
        pss->m_connected = FALSE;
    }

    return 0;
}

static status_t __osa_console_do_deinitialize(osa_console_object_t *pobj)
{
    status_t status = OSA_SOK;

    status = dlist_map(&pobj->m_busy_list, __osa_console_deinit_apply_fxn, NULL);

    close(pobj->m_listen_fd);
    pobj->m_listen_fd = -1;

    pobj->m_initialized = FALSE;

    return status;
}

static status_t __osa_console_create_session(osa_console_session_t *pss)
{
    status_t status = OSA_SOK;

    /* Initialize osa console session task */
    snprintf(pobj->m_name, sizeof (pobj->m_name) - 1, "%s", "OSA_CONSOLE_SESSION%d", pss->m_id);

    pobj->m_tsk_obj.m_name = pobj->m_name;
    pobj->m_tsk_obj.m_main = __osa_console_session_external_main;
    pobj->m_tsk_obj.m_find = NULL;
    pobj->m_tsk_obj.m_pri = 0;
    pobj->m_tsk_obj.m_stack_size = 0;
    pobj->m_tsk_obj.m_init_state = 0;
    pobj->m_tsk_obj.m_userdata = (void *)pss;
    pobj->m_tsk_obj.m_task = TASK_INVALID_TSK;

    /* Register osa console task */
    task_object_t *ptsk = &pobj->m_tsk_obj;
    status = task_mgr_register(ptsk);
    if (OSA_ISERROR(status)) {
        return status;
    }

    /* Start osa console session task */
    status = task_mgr_start(ptsk);
    if (OSA_ISERROR(status)) {
        status |= task_mgr_unregister(ptsk);
        return status;
    }

    return status;
}

static status_t 
__osa_console_session_external_main(void *ud, task_t tsk, msg_t **msg)
{
    status_t status = OSA_SOK;
    
    osa_console_session_t * pobj = NULL;

    pobj = (osa_console_session_t *)ud;

    switch (msg_get_cmd((*msg)))
    {
        case TASK_CMD_INIT:
            status |= task_set_state(tsk, TASK_STATE_INIT);
            status |= __osa_console_session_do_initialize(pobj);

            break;

        case TASK_CMD_EXIT:
            status |= task_set_state(tsk, TASK_STATE_EXIT);
            status |= __osa_console_sesion_do_deinitialize(pobj);

            break;

        case TASK_CMD_PROC:
            status |= task_set_state(tsk, TASK_STATE_PROC);
            status |= __osa_console_session_do_process(pobj);

            break;

        default:
            break;
    }

    return status;
}

static status_t __osa_console_session_do_initialize(osa_console_session_t *pobj)
{
    status_t status = OSA_SOK;

    return status;
}

static int __osa_console_session_readline(osa_console_session_t *pobj, char *str)
{
	int fd = pobj->fd;
	char* p;
	int r;
	int i = 0;
	p = str;

	while (i < MAX_LINEBUFF_SIZE - 1) {
		r = recv(fd, p, 1, 0);
		if (r < 0) {
			printf("Failed to read from tcp socket. errno=%d\n", errno);
			return r;
		}

		if (r == 0) {
			printf(DBG_INFO, "Socket is closed by peer.\n");
			return -1;
		}

		if (*p == '\r') continue;
		if (*p == '\n') break;

		p += r;
		i += r;
	}

	*p = 0;

	return 0;
}

static int __osa_console_session_writeline(osa_console_session_t *pobj, const char *str)
{
	int fd = pobj->fd;
	const char* p = str;
	int size_to_write = strlen(str);
	int r;

	while (size_to_write) {
		r = send(fd, p, size_to_write, 0);
		if(r <= 0) {
			printf("Failed to write to tcp socket. errno=%d\n", errno);
			return r;
		}

		p += r;
		size_to_write -= r;
	}

	return 0;
}

static status_t __osa_console_do_process(osa_console_session_t *pobj)
{
    status_t status = OSA_SOK;
    task_state_t tsk_state = TASK_STATE_PROC;

	char console_msg[] = "[OSA]$ ";

    /*
     *  Note: if we do not return from this routine right now, ack the msg first.
     */
    msg_set_status(*msg, OSA_SOK);
    status = task_ack_free_msg(tsk, *msg);

    (*msg) = NULL;

    while (!OSA_ISERROR(task_get_state(tsk, &tsk_state))
            && tsk_state != TASK_STATE_EXIT) {

        /* Synchronize task */
        status = task_synchronize((void *)pobj, tsk, __codecs_audio_cap_synchronize, 0);
        
		if (__osa_console_session_writeline(pobj, console_msg)) {
			break;
		}

		if (__osa_console_session_readline(pobj, pobj->m_linebuf)) {
			break;
		}

		DBG(DBG_INFO, "Command received: %s\n", pobj->m_linebuf);

		__osa_console_session_cmdshell_process(pobj);
    }

    return status;
}

static status_t __osa_console_do_deinitialize(osa_console_object_t *pobj)
{
    return OSA_SOK;
}

static int __osa_console_session_cmdshell_process(osa_console_session_t *pobj)
{
	int r;
	int i, j, k;
	char*	arg_tags[IPNC_CMD_MAX_ARGS];
	char*	args[IPNC_CMD_MAX_ARGS];
	char	arg_set[IPNC_CMD_MAX_ARGS];
	int		nargs;
	char*	split_strs[IPNC_CMD_MAX_ARGS * 2 + 1];
	int 	split_max	= IPNC_CMD_MAX_ARGS * 2 + 1;
	char* 	cmd_txt;
	int		is_arg;
	int 	arg_found;
	char*	empty_str		= 	"";
	int		written;
	char* 	linebuff = pobj->m_linebuf;
	osa_cmd_req_t * req = &pobj->m_req;
	int		access = session->cur_user.access_rights;

	req->m_id = -1;

	if (split_string(linebuff, split_strs, &split_max, 0)) {
		req->m_status = IPNC_RCMD_EPARSE;
		strcpy(req->m_msg, "Too many arguments.");
		__osa_console_cmdshell_write_response(linebuff, req);
		__osa_console_session_writeline(pobj, linebuff);
		return -EINVAL;
	}

	if (split_max == 0) {
		req->m_status = IPNC_RCMD_EMPTYCMDLINE;
		req->m_msg[0] = 0;
		__osa_console_cmdshell_write_response(linebuff, req);
		__osa_console_session_writeline(pobj, linebuff);
		return -EINVAL;
	}

	nargs = 0;
	cmd_txt = split_strs[0];
	is_arg = 0;

	for(i = 1; i < split_max; i ++) {
		if(is_arg) {
			is_arg = 0;
			args[nargs++] = split_strs[i];
		}
        else {
            if(split_strs[i][0] == TAG_PREFIX) {
                if(nargs >= IPNC_CMD_MAX_ARGS) {
                    req->m_status = IPNC_RCMD_EPARSE;
                    strcpy(req->m_msg, "Too many arguments.");
                    __osa_console_cmdshell_write_response(linebuff, req);
                    __osa_console_session_writeline(pobj, linebuff);
                    return -EINVAL;
                }
				arg_tags[nargs] = split_strs[i] + 1;
				args[nargs] = empty_str;
				is_arg = 1;
			}
			else {

                req->m_status = IPNC_RCMD_EPARSE;
                strcpy(req->m_msg, "No space is allowed in a single argument.");
                __osa_console_cmdshell_write_response(linebuff, req);
                __osa_console_session_writeline(pobj, linebuff);
                return -EINVAL;
			}
		}
	}

	/* Help */
	if (strcasecmp(cmd_txt, "help") == 0) {
		req->m_status = IPNC_RCMD_REMARK_CONT;
		sprintf(req->m_msg, "%s", "Command list: [mandatory arg] <optional arg>");
		__osa_console_cmdshell_write_response(linebuff, req);
		__osa_console_session_writeline(pobj, linebuff);

		req->detailed_msg[0] = 0; // blank remark line
		__osa_console_cmdshell_write_response(linebuff, req);
		__osa_console_session_writeline(pobj, linebuff);

		req->m_status = IPNC_RCMD_INFO_CONT;
        for (i = 0; i < IPNC_NUM_CMDS; i ++) {
            struct ipnc_cmd_description* cmd = &gbl_ipnc_cmds[i];
            if(cmd->m_access <= access) {
                get_cmd_helpmsg(req->m_msg, cmd);
                __osa_console_cmdshell_write_response(linebuff, req);
                __osa_console_session_writeline(pobj, linebuff);
            }
        }
		req->m_status = IPNC_RCMD_REMARK;
		strcpy(req->m_msg, "Ends.");
		__osa_console_cmdshell_write_response(linebuff, req);
		__osa_console_session_writeline(pobj, linebuff);

		return 0;
	}

	/* Looking up commands */
	for (i = 0; i < IPNC_NUM_CMDS; i ++) {
		osa_cmd_t * cmd = &gbl_ipnc_cmds[i];
		if (cmd->m_access <= access && strcasecmp(cmd->m_cmd, cmd_txt) == 0) {
			req->m_id = cmd->m_id;
			memset(req->m_args, 0, sizeof(osa_cmd_req_arg_t ) * OSA_CMD_ARGS_MAX);
			memset(arg_set, 0, OSA_CMD_ARGS_MAX);
			for (j = 0; j < nargs; j ++) {
				arg_found = 0;
				for (k = 0; k < cmd->m_nargs; k ++) {
					osa_cmd_arg_t * cmd_arg = &cmd->m_args[k];
					if(strcasecmp(cmd_arg->m_tag, arg_tags[j]) == 0) {
						arg_found = 1;
                        if (set_argument(&req->m_args[k], args[j], cmd_arg->m_type)) {
                            req->m_status = OSA_EINVAL;
                            sprintf(req->m_msg, "-%s = \"%s\"", cmd_arg->m_tag, args[j]);

                            __osa_console_cmdshell_write_response(linebuff, req);
                            __osa_console_session_writeline(pobj, linebuff);

                            return -EINVAL;
                        }

						arg_set[k] = 1;
						break; // argument found
					}
				}

                if (!arg_found) {
                    req->m_status = OSA_EINVAL;
                    sprintf(req->m_msg, "-%s = \"%s\"", arg_tags[j], args[j]);

                    __osa_console_cmdshell_write_response(linebuff, req);
                    __osa_console_session_writeline(pobj, linebuff);

                    return -EINVAL;
                }
			}

			for (k = 0; k < cmd->m_nargs; k ++) {
				if (!arg_set[k]) {
					osa_cmd_arg_t * cmd_arg = &cmd->m_args[k];
                    if (cmd_arg->m_required) {
                        req->m_status = OSA_EINVAL;
                        sprintf(req->m_msg, "Required argument \"-%s\" not set.", cmd_arg->m_tag);
                        __osa_console_cmdshell_write_response(linebuff, req);
                        __osa_console_session_writeline(pobj, linebuff);
                        return -EINVAL;
                    } else {
						req->m_args[k].pVal = cmd_arg->m_default_value;
					}
				}
			}

			break; // command found
		}
	}

	if (req->m_id >= 0) {
		/* Call the ioctl after the command shell is parsed */

        mutex_lock(&glb_osa_console_obj.m_mutex);

		req->m_status = OSA_SOK;
		req->m_msg[0] = 0;
		written       = 0;

		r = __osa_console_ioctl(pobj, &written);

        mutex_lock(&glb_osa_console_obj.m_mutex);

        if (!written) {
            __osa_console_cmdshell_write_response(linebuff, req);
            __osa_console_session_writeline(pobj, linebuff);
        }

	} else {
		req->m_status = IPNC_RCMD_EBADCMD;
		sprintf(req->m_msg, "cmd=\'%s\'", cmd_txt);
		__osa_console_cmdshell_write_response(linebuff, req);
		__osa_console_session_writeline(pobj, linebuff);

		return -EINVAL;
	}

	return r;
}

static status_t __osa_console_ioctl(osa_console_session_t *pobj, int *written)
{
    (*written) = 1;

    return osa_cmd_request(&pobj->m_req);
}

static int set_argument(osa_cmd_req_arg_t *arg, const char *argstr, osa_cmd_arg_type_t arg_type)
{
    int nVal = 0;

    switch(arg_type)
    {
        case OSA_CMD_ARG_STRING:
            arg->m_strVal = (char *) argstr;
            break;

        case OSA_CMD_ARG_SIGNED:
            if (sscanf(argstr, "%d", &nVal) != 1) {
                return -EINVAL;
            }

            arg->m_iVal = nVal;
            break;

        case OSA_CMD_ARG_UNSIGNED:
            if (sscanf(argstr, "%d", &nVal) != 1) {
                return -EINVAL;
            }

            if (nVal < 0) {
                return -EINVAL;
            }
            arg->m_uVal = nVal;

            break;

        case OSA_CMD_ARG_BOOLEAN:
            if (strcasecmp(argstr, "yes") == 0 || strcasecmp(argstr, "true") == 0 ||
                    strcmp(argstr, "1") == 0) {
                arg->m_iVal = 1;
            } else if(strcasecmp(argstr, "no") == 0 || strcasecmp(argstr, "false") == 0 ||
                    strcmp(argstr, "0") == 0) {
                arg->m_iVal = 0;
            } else {
                return -EINVAL;
            }
            break;
    }

    return 0;
}

#if defined(__cplusplus)
}
#endif
