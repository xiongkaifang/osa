/** ============================================================================
 *
 *  Copyright (C), 1987 - 2015, xiong-kaifang Tech. Co.,, Ltd.
 *
 *  @File name:	osa_msgq.c
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2013-04-04
 *
 *  @Description:   The osa message queue(using osa_queue).
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
 *  xiong-kaifang   2013-04-04     v1.0	        Write this module.
 *
 *  xiong-kaifang   2015-09-19     v1.1         1. Using datatype 'queue2_t' to
 *                                                 implement msg queue.
 *                                              2. Add codes to reuse the msg to
 *                                                 be freed.
 *                                              3. Add codes to check arguments.
 *
 *  ============================================================================
 */

/*  --------------------- Include system headers ---------------------------- */
#include <string.h>

/*  --------------------- Include user headers   ---------------------------- */
#include "osa.h"
#include "osa_msgq.h"
#include "osa_mutex.h"
#include "osa_mem.h"
#include "osa_rbtree.h"
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
#define MSGQ_MGR_MSGQ_MAX                   (32)

#define MSGQ_MSG_GET_SRC_QUEUE(msg)         (msg_get_src(msg))
#define MSGQ_MSG_SET_SRC_QUEUE(msg, src)    (msg_set_src(msg, src))
#define MSGQ_MSG_GET_DST_QUEUE(msg)         (msg_get_dst(msg))
#define MSGQ_MSG_SET_DST_QUEUE(msg, dst)    (msg_set_dst(msg, dst))

#define msgq_check_arguments(arg)           osa_check_arguments(arg)
#define msgq_check_arguments2(arg1, arg2)   osa_check_arguments2(arg1, arg2)

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
struct __msgq_t
{
    DLIST_ELEMENT_RESERVED;

    unsigned char   m_name[32];

    osa_mutex_t     m_mutex;
    osa_cond_t      m_cond;
    unsigned int    m_mcount;

    struct rb_root  m_rbroot;
};

struct __msgq_mgr_t; typedef struct __msgq_mgr_t msgq_mgr_t;
struct __msgq_mgr_t
{
    DLIST_ELEMENT_RESERVED;

    bool_t          m_initialized;

    unsigned int    m_msg_id;

    osa_mutex_t     m_mutex;
    dlist_t         m_list;
    dlist_t         m_msgs_pool;
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
static msgq_mgr_prm_t   glb_mgr_prm  = {
    .m_msgq_cnt    = MSGQ_MGR_MSGQ_MAX
};

static msgq_mgr_t       glb_msgq_mgr = {
    .m_initialized = FALSE,
};

static unsigned int     glb_cur_init = 0;

/*
 *  --------------------- Local function forward declaration -------------------
 */

/** ============================================================================
 *
 *  @Function:      Local function forward declaration.
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
static status_t
__msgq_mgr_internal_find(msgq_mgr_t *msgq_mgr, const char *name, msgq_t *msgq);

static status_t
__msgq_mgr_register(const char *name, msgq_t msgq, msgq_attrs_t *attrs);

static status_t
__msgq_mgr_find(const char *name, msgq_t *msgq);

static status_t
__msgq_mgr_unregister(const char *name, msgq_t msgq);

static unsigned int
__msgq_mgr_get_msg_id(void);

static bool
__msgq_mgr_find_match_fxn(dlist_element_t *elem, void *data);

static status_t
__msgq_mgr_msgs_pool_apply_fxn(dlist_element_t *elem, void *data);

static void __msgq_msg_insert(struct __msgq_t * pmsgq, msg_t * pmsg);

/*
 *  --------------------- Public function definition ---------------------------
 */

/** ============================================================================
 *
 *  @Function:      Public function definition.
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
status_t msgq_mgr_init(msgq_mgr_prm_t *prm)
{
    status_t              status = OSA_SOK;
    struct __msgq_mgr_t * pmgr   = &glb_msgq_mgr;

    if (glb_cur_init++ == 0) {

        status = osa_mutex_create(&pmgr->m_mutex);
        if (OSA_ISERROR(status)) {
            return status;
        }

        status |= dlist_init(&pmgr->m_list);
        status |= dlist_init(&pmgr->m_msgs_pool);

        pmgr->m_msg_id      = 0;
        pmgr->m_initialized = TRUE;
    }

    return status;
}

status_t msgq_mgr_deinit(void)
{
    status_t              status = OSA_SOK;
    struct __msgq_mgr_t * pmgr   = &glb_msgq_mgr;

    if (--glb_cur_init == 0) {

        status |= dlist_map2(&pmgr->m_msgs_pool, __msgq_mgr_msgs_pool_apply_fxn, NULL);

        status |= dlist_init(&pmgr->m_list);

        status |= osa_mutex_delete(&pmgr->m_mutex);

        pmgr->m_initialized = FALSE;
    }

    return status;
}

status_t msgq_open(const char *name, msgq_t *msgq, msgq_attrs_t *attrs)
{
    status_t          status = OSA_SOK;
    struct __msgq_t * pmsgq  = NULL;

    msgq_check_arguments(msgq);

    (*msgq) = INVALID_HANDLE;

    status = OSA_memAlloc(sizeof(struct __msgq_t), &pmsgq);
    if (OSA_ISERROR(status) || pmsgq == NULL) {
        return status;
    }

    status = osa_mutex_create(&pmsgq->m_mutex);
    if (OSA_ISERROR(status)) {
        OSA_memFree(sizeof(struct __msgq_t), pmsgq);
        return status;
    }
    status = osa_cond_create(&pmsgq->m_cond);
    if (OSA_ISERROR(status)) {
        osa_mutex_delete(&pmsgq->m_mutex);
        OSA_memFree(sizeof(struct __msgq_t), pmsgq);
        return status;
    }

    pmsgq->m_mcount = 0;

    snprintf(pmsgq->m_name, sizeof(pmsgq->m_name) - 1, "%s", name);

    status |= __msgq_mgr_register(name, (msgq_t)pmsgq, attrs);

    (*msgq) = (msgq_t)pmsgq;

    return status;
}

status_t msgq_locate(const char *name, msgq_t *msgq, msgq_locate_attrs_t *attrs)
{
    msgq_check_arguments2(name, msgq);

	return __msgq_mgr_find(name, msgq);
}

status_t msgq_release(msgq_t msgq)
{
	return OSA_SOK;
}

status_t msgq_alloc(unsigned short size, msg_t **msg)
{
    status_t              status = OSA_EFAIL;
    struct __msgq_mgr_t * pmgr   = &glb_msgq_mgr;

    msgq_check_arguments(msg);

    (*msg) = NULL;

    status |= osa_mutex_lock  (pmgr->m_mutex);
    if (!dlist_is_empty(&pmgr->m_msgs_pool)) {
        status = dlist_search_element(&pmgr->m_msgs_pool,
                                      (void *)((unsigned int)size),
                                      (dlist_element_t **)msg,
                                      __msgq_mgr_find_match_fxn);
        if (!OSA_ISERROR(status) && (*msg) != NULL) {
            status |= dlist_remove_element(&pmgr->m_msgs_pool, (dlist_element_t *)(*msg));
        }
    }
    status |= osa_mutex_unlock(pmgr->m_mutex);

    if (OSA_ISERROR(status) && (*msg) == NULL) {
        status = OSA_memAlloc(size, msg);
        if (!OSA_ISERROR(status) && (*msg) != NULL) {
            msg_init((*msg));
            msg_set_flags((*msg), MSG_FLAGS_DEFAULT_PRI);
            msg_set_msg_id((*msg), __msgq_mgr_get_msg_id());
        }
    }

    return status;
}

status_t msgq_free(unsigned short size, msg_t *msg)
{
    status_t              status = OSA_SOK;
    struct __msgq_mgr_t * pmgr   = &glb_msgq_mgr;

    msgq_check_arguments(msg);

    msg_clear_flags(msg, MSG_FLAGS_PRI_MASK);
    msg_set_flags(msg, MSG_FLAGS_DEFAULT_PRI);
    status |= osa_mutex_lock  (pmgr->m_mutex);
    status |= dlist_put_tail(&pmgr->m_msgs_pool, (dlist_element_t *)msg);
    status |= osa_mutex_unlock(pmgr->m_mutex);

    return status;
}

status_t msgq_send(msgq_t msgq, msg_t *msg, unsigned int timeout)
{
    struct __msgq_t * pmsgq  = (struct __msgq_t *)msgq;

    msgq_check_arguments2(pmsgq, msg);

    osa_mutex_lock  (pmsgq->m_mutex);

    __msgq_msg_insert(pmsgq, msg);

    pmsgq->m_mcount++;

    osa_cond_signal(pmsgq->m_cond);

    osa_mutex_unlock(pmsgq->m_mutex);

    return OSA_SOK;
}


status_t msgq_recv(msgq_t msgq, msg_t **msg, unsigned int timeout)
{
    status_t          status = OSA_EFAIL;
    struct rb_node  * pnode  = NULL;
    struct __msgq_t * pmsgq  = (struct __msgq_t *)msgq;

    msgq_check_arguments2(pmsgq, msg);

    osa_mutex_lock  (pmsgq->m_mutex);

    while (true) {
        if (pmsgq->m_mcount > 0) {

            pnode  = rb_last(&pmsgq->m_rbroot);

            OSA_assert(NULL != pnode);

            *msg   = rb_entry((osa_head_t *)pnode, msg_t, m_head);
            rb_erase((struct rb_node *)&(*msg)->m_head, &pmsgq->m_rbroot);

            pmsgq->m_mcount--;
            status = OSA_SOK;

            break;
        } else {
            if (OSA_TIMEOUT_NONE == timeout) {
                break;
            } else if (OSA_TIMEOUT_FOREVER == timeout) {
                status = osa_cond_wait(pmsgq->m_cond, pmsgq->m_mutex);
            } else {
                status = osa_cond_timedwait(pmsgq->m_cond, pmsgq->m_mutex, timeout);
            }

            if (OSA_ISERROR(status)) {
                break;
            }
        }
    }

    osa_mutex_unlock(pmsgq->m_mutex);

    return status;
}

status_t msgq_get_src_queue(msg_t *msg, msgq_t *msgq)
{
    status_t status = OSA_SOK;

    msgq_check_arguments2(msg, msgq);

	(*msgq) = (msgq_t)MSGQ_MSG_GET_SRC_QUEUE(msg);

    return status;
}

status_t msgq_set_src_queue(msg_t *msg, msgq_t msgq)
{
    status_t status = OSA_SOK;

    msgq_check_arguments(msg);

	MSGQ_MSG_SET_SRC_QUEUE(msg, msgq);

    return status;
}

status_t msgq_get_dst_queue(msg_t *msg, msgq_t *msgq)
{
    status_t status = OSA_SOK;

    msgq_check_arguments2(msg, msgq);

	(*msgq) = (msgq_t)MSGQ_MSG_GET_DST_QUEUE(msg);

    return status;
}

status_t msgq_set_dst_queue(msg_t *msg, msgq_t msgq)
{
    status_t status = OSA_SOK;

    msgq_check_arguments2(msg, HANDLE_TO_POINTER(msgq));

	MSGQ_MSG_SET_DST_QUEUE(msg, msgq);

    return status;
}

status_t msgq_count(msgq_t msgq, unsigned int *cnt)
{
    struct __msgq_t * pmsgq  = (struct __msgq_t *)msgq;

    msgq_check_arguments2(pmsgq, cnt);

    osa_mutex_lock  (pmsgq->m_mutex);
    (*cnt) = pmsgq->m_mcount;
    osa_mutex_unlock(pmsgq->m_mutex);

    return OSA_SOK;
}

status_t msgq_close(const char *name, msgq_t *msgq)
{
    status_t          status = OSA_SOK;
    struct __msgq_t * pmsgq  = (struct __msgq_t *)(*msgq);

    msgq_check_arguments2(msgq, pmsgq);

    status |= __msgq_mgr_unregister(name, (msgq_t)pmsgq);

    status |= osa_cond_delete (&pmsgq->m_cond);
    status |= osa_mutex_delete(&pmsgq->m_mutex);

    status |= OSA_memFree(sizeof(struct __msgq_t), pmsgq);

    (*msgq) = INVALID_HANDLE;

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
static bool
msgq_mgr_find_match_fxn(dlist_element_t *elem, void *data)
{
    return (strncmp(((struct __msgq_t *)elem)->m_name,
                    (const char *)data,
                    sizeof(((struct __msgq_t *)elem)->m_name)) == 0);
}

static status_t
__msgq_mgr_internal_find(msgq_mgr_t *msgq_mgr, const char *name, msgq_t *msgq)
{
    status_t status = OSA_ENOENT;

    (*msgq) = INVALID_HANDLE;

    status = dlist_search_element(&msgq_mgr->m_list, (void *)name,
                                  (dlist_element_t **)msgq, msgq_mgr_find_match_fxn);

    if (!OSA_ISERROR(status)) {
        status = OSA_SOK;
    }

    return status;
}

static status_t
__msgq_mgr_register(const char *name, msgq_t msgq, msgq_attrs_t *attrs)
{
    status_t              status = OSA_SOK;
    struct __msgq_mgr_t * pmgr   = &glb_msgq_mgr;

    msgq_check_arguments(HANDLE_TO_POINTER(msgq));

    osa_mutex_lock  (pmgr->m_mutex);

    status |= dlist_put_tail(&pmgr->m_list, (dlist_element_t *)msgq);

    osa_mutex_unlock(pmgr->m_mutex);

    return status;
}

static status_t
__msgq_mgr_find(const char *name, msgq_t *msgq)
{
    status_t              status = OSA_SOK;
    struct __msgq_mgr_t * pmgr   = &glb_msgq_mgr;

    msgq_check_arguments(msgq);

    osa_mutex_lock  (pmgr->m_mutex);

    status = __msgq_mgr_internal_find(pmgr, name, msgq);

    osa_mutex_unlock(pmgr->m_mutex);

    return status;
}

static status_t
__msgq_mgr_unregister(const char *name, msgq_t msgq)
{
    status_t              status = OSA_SOK;
    struct __msgq_mgr_t * pmgr   = &glb_msgq_mgr;

    msgq_check_arguments(HANDLE_TO_POINTER(msgq));

    osa_mutex_lock  (pmgr->m_mutex);

    status |= dlist_remove_element(&pmgr->m_list, (dlist_element_t *)msgq);

    osa_mutex_unlock(pmgr->m_mutex);

    return status;
}

static unsigned int
__msgq_mgr_get_msg_id(void)
{
    unsigned int          id;
    struct __msgq_mgr_t * pmgr = &glb_msgq_mgr;

    osa_mutex_lock  (pmgr->m_mutex);

    id = pmgr->m_msg_id++;

    osa_mutex_unlock(pmgr->m_mutex);

    return id;
}

static bool
__msgq_mgr_find_match_fxn(dlist_element_t *elem, void *data)
{
    return ((msg_get_msg_size(elem)) == ((unsigned int)data));
}

static status_t
__msgq_mgr_msgs_pool_apply_fxn(dlist_element_t *elem, void *data)
{
    return OSA_memFree(msg_get_msg_size(elem), elem);
}

static void __msgq_msg_insert(struct __msgq_t * pmsgq, msg_t * pmsg)
{
    msg_t           * pentry = NULL;
    struct rb_node  * parent = NULL;
    struct rb_node ** pnew   = &pmsgq->m_rbroot.rb_node;

    while (*pnew) {

        parent = *pnew;

        pentry = rb_entry((osa_head_t *)parent, msg_t, m_head);

        if (msg_get_priority(pmsg) < msg_get_priority(pentry)) {
            /* High priority msg insert into right child */
            pnew = &parent->rb_right;
        } else {
            /* Low or equal priority msg insert into left child */
            pnew = &parent->rb_left;
        }
    }

    rb_link_node((struct rb_node *)&pmsg->m_head, parent, pnew);
    rb_insert_color((struct rb_node *)&pmsg->m_head, &pmsgq->m_rbroot);
}

#if defined(__cplusplus)
}
#endif
