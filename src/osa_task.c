/** ============================================================================
 *
 *  Copyright (C), 1987 - 2013, xiong-kaifang Tech. Co.,, Ltd.
 *
 *  @File name:	osa_task.c
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2013-04-05
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
 *  xiong-kaifang   2013-04-05     v1.0	        write this module.
 *
 *  xiong-kaifang   2013-12-11     v1.1         Add msgs pool.
 *
 *  ============================================================================
 */

/*  --------------------- Include system headers ---------------------------- */
#include <stdio.h>

/*  --------------------- Include user headers   ---------------------------- */
#include "osa_task.h"
#include "threadpool.h"
#include "osa_mailbox.h"
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
#define TASKLIST_MSG_NUM_MAX                (100)

#define TASKLIST_CRITICAL_ENTER()           \
    do {                                    \
        mutex_lock(&glb_tsklist_mutex);     \
    } while (0)

#define TASKLIST_CRITICAL_LEAVE()           \
    do {                                    \
        mutex_unlock(&glb_tsklist_mutex);   \
    } while (0)

#define TASK_GET_TSK_HANDLE(tsk)            ((task_handle)tsk)

#define TASK_IS_INVALID_TSK(tsk)            (TASK_INVALID_TSK == (tsk))

#define TASK_MSG_SWAP_DST(msg)              \
    do {                                    \
        task_t tmp;                         \
        tmp = msg->u.m_tsk_msg.m_to;        \
        msg->u.m_tsk_msg.m_to = msg->u.m_tsk_msg.m_frm; \
        msg->u.m_tsk_msg.m_frm = tmp;       \
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
struct __task_node_t; typedef struct __task_node_t task_node_t;

struct __task_t;
struct __task_t
{
	unsigned int	m_reserved[2];

    unsigned char   m_name[32];
	mailbox_t		m_tsk_mbx;
	
	TASK_MAIN		m_tsk_main;
	unsigned int	m_tsk_pri;
	unsigned int	m_stack_size;
	unsigned int	m_tsk_state;
	void          * m_userdata;
	task_token_t	m_tsk_token;
    task_node_t   * m_tsk_node;
};

struct __task_node_t
{
    unsigned int    m_reserved[2];

    unsigned char   m_name[32];
    struct __task_t m_tsk;
};

struct __tasklist_t; typedef struct __tasklist_t tasklist_t;
struct __tasklist_t {
    Bool            m_initialized;
    Bool            m_paddings;
	unsigned int	m_tsk_cnt;
	unsigned int	m_cur_cnt;
	
	mutex_t			m_mutex;
	threadpool_t	m_thdpool;
	task_node_t   * m_tsklist;

    msg_t         * m_msgs;

    dlist_t         m_busy_list;
    dlist_t         m_free_list;
    dlist_t         m_msgs_list;
};

typedef struct __task_t * task_handle;

/*
 *  --------------------- Global variable definition ---------------------------
 */

/** ----------------------------------------------------------------------------
 *  @Name:          Variable name
 *
 *  @Description:   Description of the variable.
 * -----------------------------------------------------------------------------
 */
static tasklist_params_t glb_tsklist_prm  = {
    .m_tsk_cnt    = TASKLIST_TSK_MAX
};

static tasklist_t glb_tsklist_obj = {
    .m_initialized = FALSE,
};

static unsigned int glb_cur_init = 0;

OSA_DECLARE_AND_INIT_MUTEX(glb_tsklist_mutex);

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
static status_t
__tasklist_initialize(tasklist_t *tsklist, tasklist_params_t *prm);

static status_t
__tasklist_alloc_task(tasklist_t *tsklist, task_node_t **tsk);

static status_t
__tasklist_free_task(tasklist_t *tsklist, task_node_t *tsk);

static status_t
__tasklist_deinitialize(tasklist_t *tsklist);

static status_t
task_init(task_handle hdl, const char *name, unsigned int id);

static status_t
task_deinit(task_handle hdl);

static void * __task_internal_main(void *userdata);

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
status_t tasklist_init(tasklist_params_t *prm)
{
	status_t status = OSA_SOK;
	
	/* Create and initialize thdpool object */
    TASKLIST_CRITICAL_ENTER();

	if (glb_cur_init++ == 0) {
        
        if (prm == NULL) {
            prm = &glb_tsklist_prm;
        }

		status = __tasklist_initialize(&glb_tsklist_obj, prm);
	}

    TASKLIST_CRITICAL_LEAVE();
	
	return status;
}

status_t tasklist_deinit(void)
{
	status_t status = OSA_SOK;
	
	/* Deintialize and delete thdpool object */
    TASKLIST_CRITICAL_ENTER();

	if (--glb_cur_init == 0) {
		status = __tasklist_deinitialize(&glb_tsklist_obj);
	}

    TASKLIST_CRITICAL_LEAVE();
	
	return status;
}

status_t tasklist_instruments(void)
{
    return threadpool_instruments(glb_tsklist_obj.m_thdpool);
}

status_t task_create(const char *name, TASK_MAIN main,
					 unsigned int pri, unsigned int stack_size,
					 unsigned int init_state, void *userdata,
					 task_t *tsk)
{
	status_t status = OSA_SOK;
    task_node_t *tsk_node = NULL;
	task_handle tsk_hdl = NULL;
	task_operation_t tsk_ops;
	
	(*tsk) = TASK_INVALID_TSK;
	status = __tasklist_alloc_task(&glb_tsklist_obj, &tsk_node);
	if (OSA_ISERROR(status)) {
		return status;
	}

    snprintf(tsk_node->m_name, sizeof(tsk_node->m_name) - 1, "%s", name);

    tsk_hdl = &tsk_node->m_tsk;
	(*tsk) = (task_t)tsk_hdl;
	
	tsk_hdl->m_tsk_main   = main;
	tsk_hdl->m_tsk_pri    = pri;
	tsk_hdl->m_stack_size = stack_size;
    tsk_hdl->m_tsk_state  = init_state;
	tsk_hdl->m_userdata   = userdata;
	
    if (main == NULL) {
        return status;
    }

	tsk_ops.m_main    = (Fxn)__task_internal_main;
	tsk_ops.m_args[0] = (unsigned int)tsk_node;
	tsk_ops.m_args[1] = (unsigned int)NULL;
	tsk_ops.m_args[2] = (unsigned int)NULL;
	tsk_ops.m_args[3] = (unsigned int)NULL;
	tsk_ops.m_args[4] = (unsigned int)NULL;
	tsk_ops.m_args[5] = (unsigned int)NULL;
	tsk_ops.m_args[6] = (unsigned int)NULL;
	tsk_ops.m_args[7] = (unsigned int)NULL;
	
	status = threadpool_add_task(glb_tsklist_obj.m_thdpool, &tsk_ops, &tsk_hdl->m_tsk_token);
	if (OSA_ISERROR(status)) {
		__tasklist_free_task(&glb_tsklist_obj, tsk_node);
		return status;
	}
	
	(*tsk) = (task_t)tsk_hdl;
	

	return status;
}

#if 0
status_t task_get_mailbox(task_t tsk, mailbox_t *mbx)
{
	status_t status = OSA_SOK;
	task_handle tsk_hdl = (task_handle)tsk;
	
	OSA_assert(tsk_hdl != NULL && mbx != NULL);
	
	(*mbx) = tsk_hdl->m_tsk_mbx;

	return status;
}

status_t task_set_mailbox(task_t tsk, mailbox_t mbx)
{
	status_t status = OSA_SOK;
	task_handle tsk_hdl = (task_handle)tsk;
	
	OSA_assert(tsk_hdl != NULL && !mailbox_is_invalid(mbx)
				&& mailbox_is_invalid(tsk_hdl->m_tsk_mbx));
	
	tsk_hdl->m_tsk_mbx = mbx;

	return status;
}
#endif

status_t task_send_msg(task_t to, task_t frm, msg_t *msg, msg_type_t msgt)
{
	status_t status = OSA_SOK;
	task_handle to_hdl = (task_handle)to;
	task_handle frm_hdl = (task_handle)frm;
	
	msg->u.m_tsk_msg.m_to  = to;
	msg->u.m_tsk_msg.m_frm = frm;
	status = mailbox_send_msg(to_hdl->m_tsk_mbx, frm_hdl->m_tsk_mbx,
				msg, msgt, OSA_TIMEOUT_FOREVER);
	
	return status;
}

#if 0
status_t task_broadcast(task_t to_lists[], task_t frm, msg_t *msg)
{
	int i = 0;
	int	msg_cnt = 0;
	status_t status = OSA_SOK;
	task_handle tsk_hdl = NULL;
	mailbox_t mbx_lists[OSA_MBX_BROADCAST_MAX];
	mailbox_t frm_mbx;
	mailbox_handle mbx_hdl = (mailbox_handle)from;
	
	OSA_assert(to_lists != NULL && msg != NULL);
	
	for (i = 0; i < OSA_MBX_BROADCAST_MAX; i++) {
		mbx_lists[i] = NULL;
	}
	
	if ((task_handle)from == NULL) {
		frm_mbx = MAILBOX_INVALID_MBX;
	} else {
		frm_mbx = ((task_handl)from)->m_tsk_mbx;
	}
	
	for (i = 0; (tsk_hdl = (task_handle)to_lists[i]) != NULL; i++) {
		msg_cnt++;
		mbx_lists[i] = tsk_hdl->m_tsk_mbx;
		
		if (msg_cnt >= OSA_MBX_BROADCAST_MAX) {
			OSA_assert(0);
		}
	}
	
	if (msg_cnt == 0) {
		return status;
	}
	
	msg->u.m_tsk_msg.m_to  = OSA_TASK_INVALID_TSK;
	msg->u.m_tsk_msg.m_frm = frm;
	status = mailbox_broadcast(mbx_lists, frm_mbx, msg);
	
	return status;
}
#endif

status_t task_broadcast(task_t tolists[], task_t frm,
        unsigned short cmd, void *prm, unsigned int size, unsigned int flags)
{
    int i;
    msg_t *msg = NULL;
    status_t status = OSA_SOK;
    unsigned int msg_cnt = 0;
    mailbox_t to_mbx_lists[MAILBOX_BROADCAST_MAX];
    mailbox_t frm_mbx;
    task_handle tsk_hdl = TASK_GET_TSK_HANDLE(frm);

    if (tsk_hdl != NULL) {
        frm_mbx = tsk_hdl->m_tsk_mbx;
    } else {
        frm_mbx = MAILBOX_INVALID_MBX;
    }

    for (i = 0; i < OSA_ARRAYSIZE(to_mbx_lists); i++) {
        to_mbx_lists[i] = MAILBOX_INVALID_MBX;
    }

    for (i = 0; !TASK_IS_INVALID_TSK(tolists[i]); i++) {
        tsk_hdl = TASK_GET_TSK_HANDLE(tolists[i]);
        to_mbx_lists[i] = tsk_hdl->m_tsk_mbx;
        msg_cnt++;

        if (msg_cnt >= MAILBOX_BROADCAST_MAX) {
            OSA_assert(0);
        }
    }

    if (msg_cnt > 0) {
        for (i = 0; i < msg_cnt; i++) {
            status = task_alloc_msg(sizeof(*msg), &msg);

            if (OSA_ISERROR(status)) {
                break;
            }
            msg_init(msg);
            msg_set_cmd(msg, cmd);
            msg_set_payload_ptr(msg, prm);
            msg_set_payload_size(msg, size);
            msg_set_flags(msg, flags);
            msg_set_msg_size(msg, sizeof(*msg));
            //msg_set_msg_id(msg, id);

            msg->u.m_tsk_msg.m_to  = tolists[i];
            msg->u.m_tsk_msg.m_frm = frm;
            status |= mailbox_send_msg(to_mbx_lists[i], frm_mbx, msg, MSG_TYPE_CMD, OSA_TIMEOUT_FOREVER);

            msg = NULL;
        }
    }

    return status;
}

status_t task_synchronize(void *ud, task_t tsk, TASK_SYNC_FXN fxn, unsigned int nums)
{
    int i;
    status_t status = OSA_SOK;
    msg_t *msg = NULL;
    int retval = 0;

    status = task_get_msg_count(tsk, &nums, MSG_TYPE_CMD);

    for (i = 0; i < nums; i++) {

        status = task_check_msg(tsk, &msg, MSG_TYPE_CMD);
        
        OSA_assert(OSA_SOK == status);
        
        retval = (*fxn)(ud, tsk, msg);

        msg_set_status(msg, retval);

        status |= task_ack_free_msg(tsk, msg);

    }

    return status;
}

status_t task_alloc_msg(unsigned short size, msg_t **msg)
{
#if 0
    return mailbox_alloc_msg(size, msg);
#else

    /*
     *  Modified by: xiong-kaifang.
     *
     *  Date       : Dec 11, 2013.
     *
     *  Description:
     *
     *      Allocate memory for msg from pool.
     */
    status_t status = OSA_ENOENT;
    tasklist_t *tsklist = &glb_tsklist_obj;

    mutex_lock(&tsklist->m_mutex);

    if (!dlist_is_empty(&tsklist->m_msgs_list)) {
        status = dlist_get_head(&tsklist->m_msgs_list, (dlist_element_t **)msg);

        OSA_assert(OSA_SOK == status);
    }

    mutex_unlock(&tsklist->m_mutex);

    return status;

#endif
}

status_t task_free_msg(unsigned short size, msg_t *msg)
{
#if 0
    return mailbox_free_msg(size, msg);
#else

    /*
     *  Modified by: xiong-kaifang.
     *
     *  Date       : Dec 11, 2013.
     *
     *  Description:
     *
     *      De-allocate memory for msg to pool.
     */
    status_t status = OSA_SOK;
    tasklist_t *tsklist = &glb_tsklist_obj;

    //OSA_assert(TRUE == OSA_ARRAYISVALIDENTRY(msg, tsklist->m_msgs));

    mutex_lock(&tsklist->m_mutex);

    status = dlist_put_tail(&tsklist->m_msgs_list, (dlist_element_t *)msg);

    mutex_unlock(&tsklist->m_mutex);

    return status;

#endif
}

status_t task_recv_msg(task_t tsk, msg_t **msg, msg_type_t msgt)
{
	status_t status = OSA_SOK;
	task_handle tsk_hdl = TASK_GET_TSK_HANDLE(tsk);
	
	status = mailbox_recv_msg(tsk_hdl->m_tsk_mbx, msg, msgt, OSA_TIMEOUT_FOREVER);
	
	return status;
}

status_t task_wait_msg(task_t tsk, msg_t **msg, msg_type_t msgt)
{
	status_t status = OSA_SOK;
	task_handle tsk_hdl = TASK_GET_TSK_HANDLE(tsk);
	
	status = mailbox_wait_msg(tsk_hdl->m_tsk_mbx, msg, msgt);
	
	return status;
}

status_t task_wait_ack(task_t tsk, msg_t **msg, unsigned int id)
{
    status_t status = OSA_SOK;
    task_handle tsk_hdl = TASK_GET_TSK_HANDLE(tsk);

    status = mailbox_wait_ack(tsk_hdl->m_tsk_mbx, msg, id);

    return status;
}

status_t task_check_msg(task_t tsk, msg_t **msg, msg_type_t msgt)
{
	status_t status = OSA_SOK;
	task_handle tsk_hdl = TASK_GET_TSK_HANDLE(tsk);
	
	status = mailbox_check_msg(tsk_hdl->m_tsk_mbx, msg, msgt);
	
	return status;
}

status_t task_wait_cmd(task_t tsk, msg_t **msg, unsigned short cmd)
{
	status_t status = OSA_SOK;
	task_handle tsk_hdl = TASK_GET_TSK_HANDLE(tsk);
	
	status = mailbox_wait_cmd(tsk_hdl->m_tsk_mbx, msg, cmd);
	
	return status;
}

status_t task_flush(task_t tsk)
{
    task_handle hdl = TASK_GET_TSK_HANDLE(tsk);

    return mailbox_flush(hdl->m_tsk_mbx);
}

status_t task_ack_free_msg(task_t tsk, msg_t *msg)
{
    status_t status = OSA_SOK;
    task_handle to_hdl  = NULL;
    task_handle frm_hdl = NULL;

    if (msg_get_flags(msg) & MSG_FLAGS_WAIT_ACK) {
        TASK_MSG_SWAP_DST(msg);
        msg->u.m_tsk_msg.m_frm = tsk;

        msg_clear_flags(msg, MSG_FLAGS_WAIT_ACK);

        status |= task_send_msg(msg->u.m_tsk_msg.m_to,
                    msg->u.m_tsk_msg.m_frm, msg, MSG_TYPE_ACK);
    } else {

        if (msg_get_flags(msg) & MSG_FLAGS_FREE_PRM) {
            OSA_memFree(msg_get_payload_size(msg), msg_get_payload_ptr(msg));
        }

        //status |= mailbox_free_msg(msg_get_msg_size(msg), msg);
        status |= task_free_msg(msg_get_msg_size(msg), msg);
    }

    return status;
}

#if 0
status_t task_ack_msg(task_t tsk, task_msg_t *msg)
{
    status_t status = OSA_SOK;
    task_handle to_hdl  = NULL;
    task_handle frm_hdl = NULL;

    task_msg_swap_dst(msg);
    msg->m_frm = tsk;

    to_hdl  = (task_handle)msg->m_to;
    frm_hdl = (task_handle)msg->m_frm;
    status |= mailbox_send_msg(to_hdl->m_mbx, frm_hdl->m_mbx, MAILBOX_MSGQ_ACK, (mailbox_msg_t *)msg);

    return status;
}
#endif

status_t task_get_msg_count(task_t tsk, unsigned int *cnt, msg_type_t msgt)
{
    status_t status = OSA_SOK;
	task_handle tsk_hdl = TASK_GET_TSK_HANDLE(tsk);

    return mailbox_get_msg_count(tsk_hdl->m_tsk_mbx, cnt, msgt);
}

status_t task_get_state(task_t tsk, task_state_t *state)
{
    status_t status = OSA_SOK;
	task_handle tsk_hdl = TASK_GET_TSK_HANDLE(tsk);
	
    (*state) = tsk_hdl->m_tsk_state;

	return status;
}

status_t task_set_state(task_t tsk, task_state_t  state)
{
    status_t status = OSA_SOK;
	task_handle tsk_hdl = TASK_GET_TSK_HANDLE(tsk);
	
    tsk_hdl->m_tsk_state = state;

	return status;
}

status_t task_delete(task_t *tsk)
{
	status_t status = OSA_SOK;
	task_state_t state;
	task_handle tsk_hdl = TASK_GET_TSK_HANDLE(*tsk);
	
	OSA_assert(tsk_hdl != NULL);
	
	status = task_get_state((*tsk), &state);
	if (state != TASK_STATE_EXIT) {
        msg_t *msg = NULL;
        msg_t *rcv_msg = NULL;

        status = task_alloc_msg(sizeof(*msg), &msg);
        if (!OSA_ISERROR(status) && msg != NULL) {
            msg_init(msg);
            msg_set_payload_ptr(msg, NULL);
            msg_set_payload_size(msg, 0);
            msg_set_cmd(msg, TASK_CMD_EXIT);
            msg_set_flags(msg, MSG_FLAGS_WAIT_ACK);
            msg_set_status(msg, OSA_SOK);

            status |= task_send_msg((*tsk), (*tsk), msg, MSG_TYPE_CMD);
            status |= task_recv_msg((*tsk), &rcv_msg, MSG_TYPE_CMD);
            OSA_assert(rcv_msg == msg);

            task_free_msg(sizeof(*msg), rcv_msg);
        }
	}

    /*
     *  TODO: !!!!
     */
	status |= __tasklist_free_task(&glb_tsklist_obj, tsk_hdl->m_tsk_node);

    (*tsk) = TASK_INVALID_TSK;

	return status;
}

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
static status_t
__tasklist_initialize(tasklist_t *tsklist, tasklist_params_t *prm)
{
	int i = 0;
	status_t status = OSA_SOK;
    unsigned int mem_size = 0;
    task_node_t *tsk_node = NULL;
	threadpool_params_t thdpool_params;
	
	memset(tsklist, 0, sizeof(tsklist));
	
	tsklist->m_tsk_cnt = prm->m_tsk_cnt;
	tsklist->m_cur_cnt = 0;

    mem_size = sizeof(task_node_t) * tsklist->m_tsk_cnt + sizeof(msg_t) * TASKLIST_MSG_NUM_MAX;
	status = OSA_memAlloc(mem_size, (void **)&tsklist->m_tsklist);
	if (OSA_ISERROR(status) || tsklist->m_tsklist == NULL) {
		return OSA_EMEM;
	}
	
	status = mutex_create(&tsklist->m_mutex);
	if (OSA_ISERROR(status)) {
		OSA_memFree(mem_size, (void *)tsklist->m_tsklist);
		tsklist->m_tsklist = NULL;
		return status;
	}
	
	thdpool_params.m_min_thd_nums = 3;
	thdpool_params.m_max_thd_nums = tsklist->m_tsk_cnt;
	thdpool_params.m_max_tsk_nums = tsklist->m_tsk_cnt;
	status = threadpool_create(&tsklist->m_thdpool, &thdpool_params);
	if (OSA_ISERROR(status)) {
		mutex_delete(&tsklist->m_mutex);
		OSA_memFree(mem_size, (void *)tsklist->m_tsklist);
		tsklist->m_tsklist = NULL;
        return status;
	}
	
    status |= dlist_init(&tsklist->m_busy_list);
    status |= dlist_init(&tsklist->m_free_list);

	/* Initialize task object */
	for (i = 0; i < tsklist->m_tsk_cnt; i++) {
        tsk_node = tsklist->m_tsklist + i;
        tsk_node->m_tsk.m_tsk_node = tsk_node;

        status |= task_init(&tsk_node->m_tsk, "TSK", i + 1);
        
        if (OSA_ISERROR(status)) {
            break;
        }

        status |= dlist_initialize_element((dlist_element_t *)tsk_node);
        status |= dlist_put_tail(&tsklist->m_free_list, (dlist_element_t *)tsk_node);
	}

    /* Initialize msg pool */
    status |= dlist_init(&tsklist->m_msgs_list);
    tsklist->m_msgs = (msg_t *)(tsklist->m_tsklist + tsklist->m_tsk_cnt);

	for (i = 0; i < TASKLIST_MSG_NUM_MAX; i++) {
        status |= dlist_initialize_element((dlist_element_t *)&tsklist->m_msgs[i]);
        status |= dlist_put_tail(&tsklist->m_msgs_list, (dlist_element_t *)&tsklist->m_msgs[i]);
	}

	return status;
}

static status_t
__tasklist_alloc_task(tasklist_t *tsklist, task_node_t **tsk)
{
    status_t status = OSA_ENOENT;

    mutex_lock(&tsklist->m_mutex);

    if (!dlist_is_empty(&tsklist->m_free_list)) {
        status = dlist_get_head(&tsklist->m_free_list, (dlist_element_t **)tsk);
        status = dlist_put_tail(&tsklist->m_busy_list, (dlist_element_t *)(*tsk));
        OSA_assert(OSA_SOK == status);
    }

    mutex_unlock(&tsklist->m_mutex);

    return status;
}

static status_t
__tasklist_free_task(tasklist_t *tsklist, task_node_t *tsk)
{
    status_t status = OSA_SOK;

    mutex_lock(&tsklist->m_mutex);

    status |= dlist_remove_element(&tsklist->m_busy_list, (dlist_element_t *)tsk);
    status |= dlist_put_tail(&tsklist->m_free_list, (dlist_element_t *)tsk);
    OSA_assert(OSA_SOK == status);

    mutex_unlock(&tsklist->m_mutex);

    return status;
}

static status_t
__tasklist_deinitialize(tasklist_t *tsklist)
{
	int i = 0;
    unsigned int mem_size = 0;
    task_node_t *tsk_node = NULL;
	status_t status = OSA_SOK;
	
	/* Deinitialize task object */
	for (i = 0; i < tsklist->m_tsk_cnt; i++) {
        tsk_node = tsklist->m_tsklist + i;

		status  |= task_deinit(&tsk_node->m_tsk);
	}
	
	status |= threadpool_delete(&tsklist->m_thdpool);
	status |= mutex_delete(&tsklist->m_mutex);
	
    mem_size = sizeof(task_node_t) * tsklist->m_tsk_cnt + sizeof(msg_t) * TASKLIST_MSG_NUM_MAX;

	if (tsklist->m_tsklist != NULL) {
		OSA_memFree(mem_size, (void *)tsklist->m_tsklist);
		tsklist->m_tsklist = NULL;
	}
	
	return status;
}

static status_t
task_init(task_handle hdl, const char *name, unsigned int id)
{
    status_t status = OSA_SOK;

    snprintf(hdl->m_name, sizeof(hdl->m_name) - 1, "%s:%02d", name, id);

    status |= mailbox_system_register(hdl->m_name, &hdl->m_tsk_mbx);

    return status;
}

static status_t
task_deinit(task_handle hdl)
{
    status_t status = OSA_SOK;

    status |= mailbox_system_unregister(hdl->m_name, hdl->m_tsk_mbx);

    return status;
}

static void * __task_internal_main(void *userdata)
{
	msg_t        * msg = NULL;
	status_t	   status;
    task_state_t   state;
    task_node_t  * tsk_node = (task_node_t *)userdata;
	task_handle    tsk_hdl = NULL;
	
	OSA_assert(tsk_node != NULL);

    tsk_hdl = &tsk_node->m_tsk;

    while (1) {

        status = task_wait_msg((task_t)tsk_hdl, &msg, MSG_TYPE_CMD);

        OSA_assert(OSA_SOK == status);

        status = tsk_hdl->m_tsk_main(tsk_hdl->m_userdata, (task_t)tsk_hdl, &msg);

        if (msg != NULL) {
            msg_set_status(msg, status);
            status |= task_ack_free_msg((task_t)tsk_hdl, msg);
        }

        if (!OSA_ISERROR(task_get_state((task_t)tsk_hdl, &state))
                && state == TASK_STATE_EXIT) {
            break;
        }
    }
	
	return ((void *)tsk_hdl);
}

#if defined(__cplusplus)
}
#endif
