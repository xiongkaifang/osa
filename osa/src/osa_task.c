/** ============================================================================
 *
 *  Copyright (C), 1987 - 2015, xiong-kaifang Tech. Co.,, Ltd.
 *
 *  @File name:	osa_task.c
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2013-04-05
 *
 *  @Description:   The osa task.
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
 *  xiong-kaifang   2013-12-11     v1.1         Add msgs pool.
 *
 *  xiong-kaifang   2015-09-19     v1.2         1. Dynamically create and delete
 *                                                 task object(task_t).
 *                                              2. Delete msgs pool, all msgs
 *                                                 allocated and deallocated
 *                                                 using mailbox_alloc/free.
 *                                              3. Add mutex to protect task
 *                                                 state.
 *                                              4. Add task_check_arguments to
 *                                                 check the arguments.
 *                                              5. Add task_check_state routine.
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
#define TASK_IS_VALID(tsk)                  (HANDLE_IS_VALID(tsk))

#define TASKLIST_TASK_MIN                   (3)
#define TASKLIST_TASK_MAX                   (32)
#define TASKLIST_TASK_LINGER                (30 * 60)

#define task_check_arguments(arg)           osa_check_arguments(arg)
#define task_check_arguments2(arg1, arg2)   osa_check_arguments2(arg1, arg2)
#define task_check_arguments3(arg1, arg2, arg3) \
        osa_check_arguments3(arg1, arg2, arg3)

#define task_msg_swap_dst(msg)              \
    do {                                    \
        task_t tmp;                         \
        tmp = msg->u.m_tsk_msg.m_to;        \
        msg->u.m_tsk_msg.m_to = msg->u.m_tsk_msg.m_frm; \
        msg->u.m_tsk_msg.m_frm = tmp;       \
    } while (0)

/** ============================================================================
 *  @Macro:         TASK_CHECK_STATE
 *
 *  @Description:   Description of this macro.
 *  ============================================================================
 */
//#define TASK_CHECK_STATE

#if defined(TASK_CHECK_STATE)

#define task_check_valid_state(ptsk)        \
    do {                                    \
        task_state_t state;                 \
        osa_mutex_lock  (ptsk->m_mutex);    \
        state = ptsk->m_tsk_state;          \
        osa_mutex_unlock(ptsk->m_mutex);    \
        if (TASK_STATE_EXIT == state)       \
        { return OSA_EINVAL; }              \
    } while (0)
#else

#define task_check_valid_state(ptsk)        ((void)ptsk)
#endif
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
struct __task_t
{
    DLIST_ELEMENT_RESERVED;

    unsigned char   m_tsk_name[32];

    mailbox_params_t
                    m_mbx_prm;

    mailbox_t       m_mbx;

	TASK_MAIN		m_tsk_main;
	unsigned int	m_tsk_pri;
	unsigned int	m_stack_size;
    volatile
    unsigned int	m_tsk_state;
	void          * m_userdata;

    osa_mutex_t     m_mutex;

	task_token_t	m_tsk_token;
    task_data_t     m_tsk_data;
};

struct __tasklist_t
{
    DLIST_ELEMENT_RESERVED;

    bool_t          m_initialized;

    unsigned int    m_id;

    threadpool_t    m_thdpool;

    osa_mutex_t     m_mutex;
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
static tasklist_params_t   glb_tsklist_prm  = {
    .m_min_tsk_nums = TASKLIST_TASK_MIN,
    .m_max_tsk_nums = TASKLIST_TASK_MAX,
    .m_max_linger   = TASKLIST_TASK_LINGER
};

static struct __tasklist_t glb_tsklist_obj = {
    .m_initialized  = FALSE,
};

static unsigned int glb_cur_init = 0;

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
 *  @Others:        //其它说明
 *
 *  ============================================================================
 */
static status_t
__tasklist_initialize(struct __tasklist_t *ptsklist, tasklist_params_t *prm);

static unsigned int __tasklist_get_task_id(struct __tasklist_t *ptsklist);

static status_t
__tasklist_deinitialize(struct __tasklist_t *ptsklist);

static int __task_internal_main(void *userdata);

static int __task_internal_exit(void *userdata);

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
status_t tasklist_init(tasklist_params_t *prm)
{
    status_t              status   = OSA_SOK;
    struct __tasklist_t * ptsklist = &glb_tsklist_obj;

    if (glb_cur_init++ == 0) {

        if (prm == NULL) {
            prm = &glb_tsklist_prm;
        }

        status = __tasklist_initialize(ptsklist, prm);
    }

    return status;
}

status_t tasklist_instruments(void)
{
    return threadpool_instruments(glb_tsklist_obj.m_thdpool);
}

status_t tasklist_deinit(void)
{
    status_t              status   = OSA_SOK;
    struct __tasklist_t * ptsklist = &glb_tsklist_obj;

    if (--glb_cur_init == 0) {
        status = __tasklist_deinitialize(ptsklist);
    }

    return status;
}

status_t task_create(const char *name, TASK_MAIN main,
        unsigned int pri, unsigned int stack_size,
        unsigned int init_state, void *userdata,
        task_t *tsk)
{
    int               i;
    unsigned int      id;
    status_t          status = OSA_SOK;
    struct __task_t * ptsk   = NULL;

    task_check_arguments2(name, tsk);

    (*tsk) = INVALID_HANDLE;

    status = OSA_memAlloc(sizeof(struct __task_t), &ptsk);
    if (OSA_ISERROR(status) || ptsk == NULL) {
        return status;
    }

    snprintf(ptsk->m_tsk_name, sizeof(ptsk->m_tsk_name) - 1, "%s", name);

    id = __tasklist_get_task_id(&glb_tsklist_obj);
    snprintf(ptsk->m_mbx_prm.m_name, sizeof(ptsk->m_mbx_prm.m_name) - 1, "%s:%02d", "TSK", id);

    status = mailbox_open(&ptsk->m_mbx, &ptsk->m_mbx_prm);
    if (OSA_ISERROR(status)) {
        OSA_memFree(sizeof(struct __task_t), ptsk);
        return status;
    }

    status = osa_mutex_create(&ptsk->m_mutex);
    if (OSA_ISERROR(status)) {
        mailbox_close(&ptsk->m_mbx, &ptsk->m_mbx_prm);
        OSA_memFree(sizeof(struct __task_t), ptsk);
        return status;
    }

    ptsk->m_tsk_main        = main;
    ptsk->m_tsk_pri         = pri;
    ptsk->m_stack_size      = stack_size;
    ptsk->m_tsk_state       = init_state;
    ptsk->m_userdata        = userdata;

    (*tsk) = (task_t)ptsk;

    if (main == NULL) {
        return status;
    }

    ptsk->m_tsk_data.m_name = (char *)ptsk->m_tsk_name;
    ptsk->m_tsk_data.m_main = (Fxn)__task_internal_main;
    ptsk->m_tsk_data.m_exit = (Fxn)__task_internal_exit;

    for (i = 0; i < OSA_ARRAYSIZE(ptsk->m_tsk_data.m_args); i++) {
        ptsk->m_tsk_data.m_args[i] = (Arg)NULL;
    }
    ptsk->m_tsk_data.m_args[0] = (Arg)ptsk;

    status = threadpool_add_task(glb_tsklist_obj.m_thdpool, &ptsk->m_tsk_data, &ptsk->m_tsk_token);
    if (OSA_ISERROR(status)) {
        osa_mutex_delete(&ptsk->m_mutex);
        mailbox_close(&ptsk->m_mbx, &ptsk->m_mbx_prm);
        OSA_memFree(sizeof(struct __task_t), ptsk);

        (*tsk) = INVALID_HANDLE;
    }

    return status;
}

status_t task_send_msg(task_t to, task_t frm, msg_t *msg, msg_type_t msgt)
{
    struct __task_t * ptsk_to   = (struct __task_t *)to;
    struct __task_t * ptsk_frm  = (struct __task_t *)frm;

    task_check_arguments3(ptsk_to, ptsk_frm, msg);

    task_check_valid_state(ptsk_to);

    msg->u.m_tsk_msg.m_to  = to;
    msg->u.m_tsk_msg.m_frm = frm;

    return mailbox_send_msg(ptsk_to->m_mbx, ptsk_frm->m_mbx, msg, msgt, OSA_TIMEOUT_FOREVER);
}

status_t task_recv_msg(task_t tsk, msg_t **msg, msg_type_t msgt)
{
    struct __task_t * ptsk = (struct __task_t *)tsk;

    task_check_arguments2(ptsk, msg);

    task_check_valid_state(ptsk);

    return mailbox_recv_msg(ptsk->m_mbx, msg, msgt, OSA_TIMEOUT_FOREVER);
}

status_t task_broadcast(task_t tolists[], task_t frm,
        unsigned short cmd, void *prm, unsigned int size, unsigned int flags)
{
    int               i;
    msg_t           * msg    = NULL;
    status_t          status = OSA_SOK;
    struct __task_t * ptsk   = NULL;

    for (i = 0; TASK_IS_VALID(tolists[i]); i++) {

        status = task_alloc_msg(sizeof(*msg), &msg);

        if (OSA_ISERROR(status) || msg == NULL) {
            break;
        }

        //msg_init(msg);
        msg_set_cmd(msg, cmd);
        msg_set_payload_ptr(msg, prm);
        msg_set_payload_size(msg, size);
        msg_set_flags(msg, flags);
        msg_set_msg_size(msg, sizeof(*msg));

        msg->u.m_tsk_msg.m_to  = tolists[i];
        msg->u.m_tsk_msg.m_frm = frm;

        ptsk = (struct __task_t *)tolists[i];

        status = task_send_msg(tolists[i], frm, msg, MSG_TYPE_CMD);
        if (OSA_ISERROR(status)) {
            task_free_msg(sizeof(*msg), msg);
        }
    }

    return status;
}

status_t task_synchronize(task_t tsk, TASK_SYNC fxn, unsigned int nums, void *userdata)
{
    int        i;
    int        retval = 0;
    status_t   status = OSA_SOK;
    msg_t    * msg    = NULL;
    bool_t     bexit  = FALSE;

    task_check_arguments2(HANDLE_TO_POINTER(tsk), fxn);

    status = task_get_msg_count(tsk, &nums, MSG_TYPE_CMD);

    if (OSA_ISERROR(status)) {
        return status;
    }

    for (i = 0; i < nums; i++) {

        status = task_check_msg(tsk, &msg, MSG_TYPE_CMD);

        if (OSA_ISERROR(status)) {
            continue;
        }

        if (TASK_CMD_EXIT == (msg_get_cmd(msg))) {
            bexit = TRUE;
        }

        retval = (*fxn)(tsk, msg, userdata);

        msg_set_status(msg, retval);

        status = task_ack_free_msg(tsk, msg);

    }

    if (bexit) {
        ((struct __task_t *)tsk)->m_tsk_state = TASK_STATE_EXIT;
    }

    return status;
}

status_t task_alloc_msg(unsigned short size, msg_t **msg)
{
    task_check_arguments(msg);

#if 1
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
    task_check_arguments(msg);

#if 1
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

status_t task_wait_msg(task_t tsk, msg_t **msg, msg_type_t msgt)
{
    struct __task_t * ptsk = (struct __task_t *)tsk;

    task_check_arguments2(ptsk, msg);

    task_check_valid_state(ptsk);

    return mailbox_wait_msg(ptsk->m_mbx, msg, msgt);
}

status_t task_check_msg(task_t tsk, msg_t **msg, msg_type_t msgt)
{
    struct __task_t * ptsk = (struct __task_t *)tsk;

    task_check_arguments2(ptsk, msg);

    task_check_valid_state(ptsk);

    return mailbox_check_msg(ptsk->m_mbx, msg, msgt);
}

status_t task_wait_cmd(task_t tsk, msg_t **msg, unsigned short cmd)
{
    struct __task_t * ptsk = (struct __task_t *)tsk;

    task_check_arguments2(ptsk, msg);

    task_check_valid_state(ptsk);

    return mailbox_wait_cmd(ptsk->m_mbx, msg, cmd);
}

status_t task_wait_ack(task_t tsk, msg_t **msg, unsigned int id)
{
    struct __task_t * ptsk = (struct __task_t *)tsk;

    task_check_arguments2(ptsk, msg);

    //task_check_valid_state(ptsk);

    return mailbox_wait_ack(ptsk->m_mbx, msg, id);
}

status_t task_flush(task_t tsk)
{
    struct __task_t * ptsk = (struct __task_t *)tsk;

    task_check_arguments(ptsk);

    task_check_valid_state(ptsk);

    return mailbox_flush(ptsk->m_mbx);
}

status_t task_ack_free_msg(task_t tsk, msg_t *msg)
{
    status_t          status   = OSA_SOK;

    task_check_arguments(msg);

    if (msg_get_flags(msg) & MSG_FLAGS_WAIT_ACK) {

        task_msg_swap_dst(msg);
        msg->u.m_tsk_msg.m_frm = tsk;

        msg_clear_flags(msg, MSG_FLAGS_WAIT_ACK);

        status = task_send_msg(msg->u.m_tsk_msg.m_to,
                msg->u.m_tsk_msg.m_frm, msg, MSG_TYPE_ACK);
    } else {

        if (msg_get_flags(msg) & MSG_FLAGS_FREE_PRM) {
            OSA_memFree(msg_get_payload_size(msg), msg_get_payload_ptr(msg));
        }

        status = task_free_msg(msg_get_msg_size(msg), msg);
    }

    return status;
}

status_t task_get_msg_count(task_t tsk, unsigned int *cnt, msg_type_t msgt)
{
    struct __task_t * ptsk = (struct __task_t *)tsk;

    task_check_arguments2(ptsk, cnt);

    return mailbox_get_msg_count(ptsk->m_mbx, cnt, msgt);
}

status_t task_get_state(task_t tsk, task_state_t *state)
{
    struct __task_t * ptsk = (struct __task_t *)tsk;

    task_check_arguments2(ptsk, state);

    osa_mutex_lock  (ptsk->m_mutex);
    (*state) = ptsk->m_tsk_state;
    osa_mutex_unlock(ptsk->m_mutex);

    return OSA_SOK;
}

status_t task_set_state(task_t tsk, task_state_t  state)
{
    struct __task_t * ptsk = (struct __task_t *)tsk;

    task_check_arguments(ptsk);

    osa_mutex_lock  (ptsk->m_mutex);
    ptsk->m_tsk_state = state;
    osa_mutex_unlock(ptsk->m_mutex);

    return OSA_SOK;
}

bool_t   task_check_state(task_t tsk)
{
    bool_t            state;
    struct __task_t * ptsk = (struct __task_t *)tsk;

    if (ptsk == NULL) {
        return FALSE;
    }

    osa_mutex_lock  (ptsk->m_mutex);
    state = (TASK_STATE_EXIT != ptsk->m_tsk_state);
    osa_mutex_unlock(ptsk->m_mutex);

    return (state);
}

status_t task_delete(task_t *tsk)
{
    status_t              status   = OSA_SOK;
    struct __task_t     * ptsk     = (struct __task_t *)(*tsk);
    struct __tasklist_t * ptsklist = &glb_tsklist_obj;

    task_check_arguments2(tsk, ptsk);

    if (ptsk->m_tsk_main != NULL) {

        /*
         *  TODO: We should improve this!!!
         */
        status = threadpool_cancel_task(ptsklist->m_thdpool, &ptsk->m_tsk_token);
    }


    status |= osa_mutex_delete(&ptsk->m_mutex);

    status |= mailbox_close(&ptsk->m_mbx, &ptsk->m_mbx_prm);

    status |= OSA_memFree(sizeof(struct __task_t), ptsk);

    (*tsk) = INVALID_HANDLE;

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
static status_t
__tasklist_initialize(struct __tasklist_t *ptsklist, tasklist_params_t *prm)
{
    status_t            status = OSA_SOK;
    threadpool_params_t thdpool_params;

    /* Initialize tasklist object */
    memset(ptsklist, 0, sizeof(*ptsklist));

    status = osa_mutex_create(&ptsklist->m_mutex);
    if (OSA_ISERROR(status)) {
        return status;
    }

    thdpool_params.m_min_thd_nums = prm->m_min_tsk_nums;
    thdpool_params.m_max_thd_nums = prm->m_max_tsk_nums;
    thdpool_params.m_max_linger   = prm->m_max_linger;
    status = threadpool_create(&ptsklist->m_thdpool, &thdpool_params);
    if (OSA_ISERROR(status)) {
        osa_mutex_delete(&ptsklist->m_mutex);
        return status;
    }

    status |= dlist_init(&ptsklist->m_list);

    ptsklist->m_id          = 0;
    ptsklist->m_initialized = TRUE;

    return status;
}

static unsigned int __tasklist_get_task_id(struct __tasklist_t *ptsklist)
{
    unsigned int id = 0;

    osa_mutex_lock  (ptsklist->m_mutex);

    id = ptsklist->m_id++;

    osa_mutex_unlock(ptsklist->m_mutex);

    return id;
}

static status_t
__tasklist_deinitialize(struct __tasklist_t *ptsklist)
{
    status_t status = OSA_SOK;

    /* Deinitialize tasklist object */
    status |= threadpool_delete(&ptsklist->m_thdpool);

    status |= osa_mutex_delete(&ptsklist->m_mutex);

    status |= dlist_init(&ptsklist->m_list);

    ptsklist->m_initialized = FALSE;

    return status;
}

static int __task_internal_main(void *userdata)
{
    msg_t           * pmsg;
    status_t          status;
    struct __task_t * ptsk = (struct __task_t *)userdata;

    OSA_assert(ptsk != NULL);

    while (task_check_state((task_t)ptsk)) {

        pmsg   = NULL;
        status = task_wait_msg((task_t)ptsk, &pmsg, MSG_TYPE_CMD);

        if (OSA_ISERROR(status) || pmsg == NULL) {
            break;
        }

        //OSA_assert(OSA_SOK == status && pmsg != NULL);

        if (TASK_CMD_INIT == msg_get_cmd(pmsg)) {
            task_set_state((task_t)ptsk, TASK_STATE_INIT);
        } else if (TASK_CMD_PROC == msg_get_cmd(pmsg)) {
            task_set_state((task_t)ptsk, TASK_STATE_PROC);
        } else if (TASK_CMD_EXIT == msg_get_cmd(pmsg)) {
            task_set_state((task_t)ptsk, TASK_STATE_EXIT);
        }

        /* Call the user's main function */
        status = ptsk->m_tsk_main((task_t)ptsk, &pmsg, ptsk->m_userdata);

        if (pmsg != NULL) {
            msg_set_status(pmsg, status);

            task_ack_free_msg((task_t)ptsk, pmsg);
        }
    }

    return status;
}

static int __task_internal_exit(void *userdata)
{
    msg_t           * pmsg = NULL;
    msg_t           * ptmp = NULL;
    status_t          status;
    task_state_t      state;
    struct __task_t * ptsk = (struct __task_t *)userdata;

    OSA_assert(ptsk != NULL);

    status = task_get_state((task_t)ptsk, &state);

    if (TASK_STATE_EXIT != state) {
        status = task_alloc_msg(sizeof(*pmsg), &pmsg);

        if (!OSA_ISERROR(status) && pmsg != NULL) {
            unsigned int msg_id;
            msg_set_payload_ptr(pmsg, NULL);
            msg_set_payload_size(pmsg, 0);
            msg_set_cmd(pmsg, TASK_CMD_EXIT);
            msg_set_flags(pmsg, MSG_FLAGS_WAIT_ACK);
            msg_set_status(pmsg, OSA_SOK);

            msg_id = msg_get_msg_id(pmsg);

            status |= task_send_msg((task_t)ptsk, (task_t)ptsk, pmsg, MSG_TYPE_CMD);
            status |= task_wait_ack((task_t)ptsk, &ptmp, msg_id);

            OSA_assert(ptmp == pmsg);

            task_free_msg(sizeof(*pmsg), pmsg);
        }
    }

    return status;
}

#if defined(__cplusplus)
}
#endif
