/** ============================================================================
 *
 *  Copyright (C), 1987 - 2015, xiong-kaifang Tech. Co.,, Ltd.
 *
 *  @File name:	osa_mailbox.c
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2013-04-05
 *
 *  @Description:   The osa mailbox.
 *
 *
 *  @Version:       v1.0
 *
 *  @Function List: // 主要函数及功能
 *      1.  －－－－－
 *      2.  －－－－－
 *
 *  @History:       // 历史修改记录
 *
 *  <author>        <time>       <version>      <description>
 *
 *  xiong-kaifang   2013-04-05     v1.0	        Write this module.
 *
 *  xiong-kaifang   2015-09-19     v1.1         1. Add codes to check arguments.
 *                                              1. Misc codes updated.
 *
 *  ============================================================================
 */

/*  --------------------- Include system headers ---------------------------- */
#include <string.h>

/*  --------------------- Include user headers   ---------------------------- */
#include "osa.h"
#include "osa_mailbox.h"
#include "osa_msgq.h"
#include "osa_mem.h"
#include "osa_mutex.h"
#include "dlist.h"

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
#define MBX_IS_VALID(mbx)                   (HANDLE_IS_VALID(mbx))

#define MAILBOX_SYSTEM_MBX_NUMS             (10)

#define mbx_check_arguments(arg)            osa_check_arguments(arg)
#define mbx_check_arguments2(arg1, arg2)    osa_check_arguments2(arg1, arg2)

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
struct __mailbox_t
{
    DLIST_ELEMENT_RESERVED;

    unsigned char   m_name[32];
    unsigned int    m_mbx_id;
    unsigned char   m_names[MSG_TYPE_MAX][32];
	msgq_t			m_msgqs[MSG_TYPE_MAX];
};

struct __mailbox_system_t
{
    DLIST_ELEMENT_RESERVED;

    bool_t          m_initialized;

    osa_mutex_t     m_mutex;
    unsigned int    m_mbx_id;
    dlist_t         m_list;
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
static mailbox_system_prm_t      glb_mbx_sys_prm  = {
    .m_mbx_cnt    = MAILBOX_SYSTEM_MBX_NUMS,
};

static struct __mailbox_system_t glb_mbx_sys_obj = {
    .m_initialized = FALSE,
};

static unsigned int glb_cur_init = 0;

/*
 *  --------------------- Local function forward declaration -------------------
 */

/** ============================================================================
 *
 *  @Function:     Local function forward declaration.
 *
 *  @Description:   // 函数功能、性能等的描述
 *
 *  @Calls:	        // 被本函数调用的函数清单
 *
 *  @Called By:	    // 调用本函数的函数清单
 *
 *  @Table Accessed:// 被访问的表（此项仅对于牵扯到数据库操作的程序）
 *
 *  @Table Updated: // 被修改的表（此项仅对于牵扯到数据库操作的程序）
 *
 *  @Input:	        // 对输入参数的说明
 *
 *  @Output:        // 对输出参数的说明
 *
 *  @Return:        // 函数返回值的说明
 *
 *  @Enter          // Precondition
 *
 *  @Leave          // Postcondition
 *
 *  @Others:        // 其它说明
 *
 *  ============================================================================
 */
static unsigned int
__mailbox_system_get_mbx_id(struct __mailbox_system_t *pmbxs);

static status_t
__mailbox_system_internal_find(struct __mailbox_system_t *pmbxs,
        const char *name, struct __mailbox_t **ppmbx);

static status_t
__mailbox_system_register(const char *name, struct __mailbox_t *pmbx);

static status_t
__mailbox_system_unregister(const char *name, struct __mailbox_t *pmbx);

/*
 *  --------------------- Public function definition ---------------------------
 */

/** ============================================================================
 *
 *  @Function:    Public function definition.
 *
 *  @Description:   // 函数功能、性能等的描述
 *
 *  @Calls:	        // 被本函数调用的函数清单
 *
 *  @Called By:	    // 调用本函数的函数清单
 *
 *  @Table Accessed:// 被访问的表（此项仅对于牵扯到数据库操作的程序）
 *
 *  @Table Updated: // 被修改的表（此项仅对于牵扯到数据库操作的程序）
 *
 *  @Input:	        // 对输入参数的说明
 *
 *  @Output:        // 对输出参数的说明
 *
 *  @Return:        // 函数返回值的说明
 *
 *  @Enter          // Precondition
 *
 *  @Leave          // Postcondition
 *
 *  @Others:        // 其它说明
 *
 *  ============================================================================
 */
status_t mailbox_system_init(mailbox_system_prm_t *prm)
{
    status_t                    status = OSA_SOK;
    struct __mailbox_system_t * pmbxs  = &glb_mbx_sys_obj;

    if (glb_cur_init++ == 0) {

        status = osa_mutex_create(&pmbxs->m_mutex);
        if (OSA_ISERROR(status)) {
            return status;
        }

        pmbxs->m_mbx_id = 0;

        status |= dlist_init(&pmbxs->m_list);

        pmbxs->m_initialized = TRUE;
    }

    return status;
}

status_t mailbox_system_find(const char *name, mailbox_t *mbx)
{
    struct __mailbox_system_t * pmbxs  = &glb_mbx_sys_obj;

    mbx_check_arguments2(name, mbx);

    return __mailbox_system_internal_find(pmbxs, name, (struct __mailbox_t **)mbx);
}

status_t mailbox_system_deinit(void)
{
    status_t                    status = OSA_SOK;
    struct __mailbox_system_t * pmbxs  = &glb_mbx_sys_obj;

    if (--glb_cur_init == 0) {

        status |= osa_mutex_delete(&pmbxs->m_mutex);
        status |= dlist_init(&pmbxs->m_list);

        pmbxs->m_initialized = FALSE;
    }

    return status;
}

status_t mailbox_open(mailbox_t *mbx, mailbox_params_t *prm)
{
    int                  i;
    status_t             status = OSA_SOK;
    struct __mailbox_t * pmbx   = NULL;

    mbx_check_arguments2(mbx, prm);

    (*mbx) = INVALID_HANDLE;

    status = OSA_memAlloc(sizeof(struct __mailbox_t), &pmbx);
    if (OSA_ISERROR(status) || pmbx == NULL) {
        return status;
    }

    pmbx->m_mbx_id = __mailbox_system_get_mbx_id(&glb_mbx_sys_obj);

    for (i = 0; i < OSA_ARRAYSIZE(pmbx->m_msgqs); i++) {
        pmbx->m_msgqs[i] = INVALID_HANDLE;
    }

    for (i = 0; i < OSA_ARRAYSIZE(pmbx->m_msgqs); i++) {
        snprintf(pmbx->m_names[i], sizeof(pmbx->m_names[i]) - 1,
                "%s%02d:%s", prm->m_name,
                pmbx->m_mbx_id, i == MAILBOX_MSGQ_CMD ? "CMD" : "ACK");

        status = msgq_open(pmbx->m_names[i], &pmbx->m_msgqs[i], NULL);

        if (OSA_ISERROR(status)) {

            for (i = 0; i < OSA_ARRAYSIZE(pmbx->m_msgqs); i++) {
                msgq_close(pmbx->m_names[i], &pmbx->m_msgqs[i]);
            }
            return status;
        }
    }

    snprintf(pmbx->m_name, sizeof(pmbx->m_name) - 1, "%s", prm->m_name);

    status = __mailbox_system_register(pmbx->m_name, pmbx);

    (*mbx) = (mailbox_t)pmbx;

    return status;
}

status_t mailbox_recv_msg(mailbox_t mbx, msg_t **msg, msg_type_t msgt, unsigned int timeout)
{
    struct __mailbox_t * pmbx = (struct __mailbox_t *)mbx;

    mbx_check_arguments2(pmbx, msg);

    return msgq_recv(pmbx->m_msgqs[msgt], msg, timeout);
}

status_t mailbox_send_msg(mailbox_t to, mailbox_t frm, msg_t *msg, msg_type_t msgt, unsigned int timeout)
{
    struct __mailbox_t * pmbx = (struct __mailbox_t *)to;

    mbx_check_arguments2(pmbx, msg);

	msg->u.m_mbx_msg.m_to  = to;
	msg->u.m_mbx_msg.m_frm = frm;

    return msgq_send(pmbx->m_msgqs[msgt], msg, timeout);
}

status_t mailbox_broadcast(mailbox_t mbx_lists[], mailbox_t frm, msg_t *msg)
{
    int        i;
    msg_t    * pmsg   = NULL;
    status_t   status = OSA_SOK;

    mbx_check_arguments(msg);

    for (i = 0; MBX_IS_VALID(mbx_lists[i]); i++) {

        status = mailbox_alloc_msg(sizeof(msg_t), &pmsg);
        if (OSA_ISERROR(status)|| pmsg == NULL) {
            break;
        }

        *pmsg = *msg;

        status = mailbox_send_msg(mbx_lists[i], frm, pmsg, MSG_TYPE_CMD, OSA_TIMEOUT_NONE);
        if (OSA_ISERROR(status)) {
            mailbox_free_msg(sizeof(msg_t), pmsg);
        }
    }

    return status;
}

status_t mailbox_alloc_msg(unsigned short size, msg_t **msg)
{
    return msgq_alloc(size, msg);
}

status_t mailbox_free_msg (unsigned short size, msg_t *msg)
{
    return msgq_free(size, msg);
}

status_t mailbox_wait_msg(mailbox_t mbx, msg_t **msg, msg_type_t msgt)
{
    struct __mailbox_t * pmbx = (struct __mailbox_t *)mbx;

    mbx_check_arguments2(pmbx, msg);

	return msgq_recv(pmbx->m_msgqs[msgt], msg, OSA_TIMEOUT_FOREVER);
}

status_t mailbox_check_msg(mailbox_t mbx, msg_t **msg, msg_type_t msgt)
{
    struct __mailbox_t * pmbx = (struct __mailbox_t *)mbx;

    mbx_check_arguments2(pmbx, msg);

	return msgq_recv(pmbx->m_msgqs[msgt], msg, OSA_TIMEOUT_NONE);
}

status_t mailbox_get_msg_count(mailbox_t mbx, unsigned int *cnt, msg_type_t msgt)
{
    struct __mailbox_t * pmbx = (struct __mailbox_t *)mbx;

    mbx_check_arguments2(pmbx, cnt);

	return msgq_count(pmbx->m_msgqs[msgt], cnt);
}

status_t mailbox_wait_cmd(mailbox_t mbx, msg_t **msg, unsigned short cmd)
{
	status_t             status = OSA_EFAIL;
    struct __mailbox_t * pmbx   = (struct __mailbox_t *)mbx;

    mbx_check_arguments2(pmbx, msg);

	do {
		status = msgq_recv(pmbx->m_msgqs[MAILBOX_MSGQ_CMD], msg, OSA_TIMEOUT_FOREVER);

		if (OSA_ISERROR(status) || (msg_get_cmd(*msg) == cmd)) {
			break;
		}

		status = msgq_send(pmbx->m_msgqs[MAILBOX_MSGQ_CMD], *msg, OSA_TIMEOUT_NONE);

	} while (1);

    return status;
}

status_t mailbox_wait_ack(mailbox_t mbx, msg_t **msg, unsigned int id)
{
	status_t             status = OSA_EFAIL;
    struct __mailbox_t * pmbx   = (struct __mailbox_t *)mbx;

    mbx_check_arguments2(pmbx, msg);

	do {
		status = msgq_recv(pmbx->m_msgqs[MAILBOX_MSGQ_ACK], msg, OSA_TIMEOUT_FOREVER);

		if (OSA_ISERROR(status) || (msg_get_msg_id(*msg) == id)) {
			break;
		}

		status = msgq_send(pmbx->m_msgqs[MAILBOX_MSGQ_ACK], *msg, OSA_TIMEOUT_NONE);

	} while (1);

    return status;
}

status_t mailbox_flush(mailbox_t mbx)
{
    msg_t              * msg    = NULL;
	status_t             status = OSA_EFAIL;
    struct __mailbox_t * pmbx   = (struct __mailbox_t *)mbx;

    mbx_check_arguments(pmbx);

    do {
        status = mailbox_check_msg(mbx, &msg, MSG_TYPE_CMD);
        if (!OSA_ISERROR(status)) {
            mailbox_free_msg(msg_get_msg_size(msg), msg);
        }
    } while (!OSA_ISERROR(status));

    do {
        status = mailbox_check_msg(mbx, &msg, MSG_TYPE_ACK);
        if (!OSA_ISERROR(status)) {
            mailbox_free_msg(msg_get_msg_size(msg), msg);
        }
    } while (!OSA_ISERROR(status));

    return status;
}

status_t mailbox_close(mailbox_t *mbx, mailbox_params_t *prm)
{
    int                  i;
	status_t             status = OSA_SOK;
    struct __mailbox_t * pmbx   = (struct __mailbox_t *)(*mbx);

    mbx_check_arguments2(mbx, pmbx);

    status |= __mailbox_system_unregister(pmbx->m_name, pmbx);

    for (i = 0; i < OSA_ARRAYSIZE(pmbx->m_msgqs); i++) {
        status |= msgq_close(pmbx->m_names[i], &pmbx->m_msgqs[i]);
    }

    status |= OSA_memFree(sizeof(struct __mailbox_t), pmbx);

    (*mbx) = INVALID_HANDLE;

    return status;
}

/*
 *  --------------------- Local function definition ----------------------------
 */

/** ============================================================================
 *
 *  @Function:      Local function definition.
 *
 *  @Description:   // 函数功能、性能等的描述
 *
 *  ============================================================================
 */
static unsigned int
__mailbox_system_get_mbx_id(struct __mailbox_system_t *pmbxs)
{
    unsigned int id;

    osa_mutex_lock  (pmbxs->m_mutex);

    id = pmbxs->m_mbx_id++;

    osa_mutex_unlock(pmbxs->m_mutex);

    return id;
}

static bool
mailbox_system_find_match_fxn(dlist_element_t *elem, void *data)
{
    return (strncmp(((struct __mailbox_t *)elem)->m_name,
                    (const char *)data,
                    sizeof(((struct __mailbox_t *)elem)->m_name)) == 0);
}

static status_t
__mailbox_system_internal_find(struct __mailbox_system_t *pmbxs,
        const char *name, struct __mailbox_t **ppmbx)
{
    status_t status = OSA_ENOENT;

    (*ppmbx) = NULL;

    status = dlist_search_element(&pmbxs->m_list, (void *)name,
                                  (dlist_element_t **)ppmbx, mailbox_system_find_match_fxn);

    if (!OSA_ISERROR(status)) {
        status = OSA_SOK;
    }

    return status;
}

static status_t
__mailbox_system_register(const char *name, struct __mailbox_t *pmbx)
{
    status_t                    status = OSA_SOK;
    struct __mailbox_system_t * pmbxs  = &glb_mbx_sys_obj;

    osa_mutex_lock  (pmbxs->m_mutex);

    status |= dlist_put_tail(&pmbxs->m_list, (dlist_element_t *)pmbx);

    osa_mutex_unlock(pmbxs->m_mutex);

    return status;


    return status;
}

static status_t
__mailbox_system_unregister(const char *name, struct __mailbox_t *pmbx)
{
    status_t                    status = OSA_SOK;
    struct __mailbox_system_t * pmbxs  = &glb_mbx_sys_obj;

    osa_mutex_lock  (pmbxs->m_mutex);

    status |= dlist_remove_element(&pmbxs->m_list, (dlist_element_t *)pmbx);

    osa_mutex_unlock(pmbxs->m_mutex);

    return status;
}

#if defined(__cplusplus)
}
#endif
