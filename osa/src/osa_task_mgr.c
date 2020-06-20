/** ============================================================================
 *
 *  Copyright (C), 1987 - 2015, xiong-kaifang Tech. Co.,, Ltd.
 *
 *  @File name:	osa_task_mgr.c
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2013-04-16
 *
 *  @Description:   The osa task manager.
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
 *  xiong-kaifang   2013-04-16     v1.0	        Write this module.
 *
 *  xiong-kaifang   2015-09-19     v1.2         1. Delete msgs pool.
 *                                              2. Add task_mgr_check_arguments
 *                                                 to check arguments.
 *
 *  ============================================================================
 */

/*  --------------------- Include system headers ---------------------------- */
#include <stdio.h>

/*  --------------------- Include user headers   ---------------------------- */
#include "osa.h"
#include "osa_task_mgr.h"
#include "osa_msgq_mgr.h"
#include "osa_mailbox.h"
#include "osa_task.h"
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
#define TASK_MGR_TASK_NAME      ("TASK_MGR_TSK")

#define TASK_MGR_TASK_MIN       (3)
#define TASK_MGR_TASK_MAX       (32)
#define TASK_MGR_TASK_LINGER    (30 * 60)

#define task_mgr_check_arguments(arg)   osa_check_arguments(arg)

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
enum __task_mgr_cmd_t; typedef enum __task_mgr_cmd_t task_mgr_cmd_t;
enum __task_mgr_cmd_t
{
    TASK_MGR_CMD_BASE                = TASK_CMD_MAX      + 1,
    TASK_MGR_CMD_REGISTER_TASK       = TASK_MGR_CMD_BASE + 1,
    TASK_MGR_CMD_START_TASK          = TASK_MGR_CMD_BASE + 2,
    TASK_MGR_CMD_STOP_TASK           = TASK_MGR_CMD_BASE + 3,
    TASK_MGR_CMD_UNREGISTER_TASK     = TASK_MGR_CMD_BASE + 4,
    TASK_MGR_CMD_FIND_TASK           = TASK_MGR_CMD_BASE + 5,
    TASK_MGR_CMD_TASK_INSTRUMENTS    = TASK_MGR_CMD_BASE + 6,
    TASK_MGR_CMD_THDPOOL_INSTRUMENTS = TASK_MGR_CMD_BASE + 7,
};

struct __task_mgr_find_prm_t;
typedef struct __task_mgr_find_prm_t task_mgr_find_prm_t;
struct __task_mgr_find_prm_t
{
    unsigned char * m_name;     //  [IN]
    task_object_t * m_task;     //  [OUT]
};

struct __task_mgr_t; typedef struct __task_mgr_t task_mgr_t;
struct __task_mgr_t
{
    DLIST_ELEMENT_RESERVED;

    unsigned int    m_initialized;
    unsigned int    m_tsk_cnt;
    osa_mutex_t     m_mutex;
    dlist_t         m_tsklists;
    task_object_t   m_tsk_obj;
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
static task_mgr_prm_t glb_tsk_mgr_prm  = {
    .m_min_tsk_nums = TASK_MGR_TASK_MIN,
    .m_max_tsk_nums = TASK_MGR_TASK_MAX,
    .m_max_linger   = TASK_MGR_TASK_LINGER
};

static task_mgr_t     glb_tsk_mgr_obj = {
    .m_initialized  = FALSE,
    .m_tsk_obj = {
        .m_name = TASK_MGR_TASK_NAME,
    }
};

static unsigned int glb_cur_init = 0;

static const char * const STATE_STR[TASK_STATE_PROC + 2] = {
    "Regi",
    "Init",
    "Exit",
    "N/A ",
    "Proc",
    "NULL"
};

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
__task_mgr_env_initialize(task_mgr_prm_t *prm);

static status_t
__task_mgr_env_deinitialize(void);

static status_t
__task_mgr_initialize(task_mgr_t *ptskmgr, task_mgr_prm_t *prm);

static status_t
__task_mgr_register_task(task_mgr_t *ptskmgr, task_object_t *ptsk);

static status_t
__task_mgr_start_task(task_mgr_t *ptskmgr, task_object_t *ptsk);

static status_t
__task_mgr_stop_task(task_mgr_t *ptskmgr, task_object_t *ptsk);

static status_t
__task_mgr_unregister_task(task_mgr_t *ptskmgr, task_object_t *ptsk);

static status_t
__task_mgr_deinitialize(task_mgr_t *ptskmgr);

static status_t __task_mgr_internal_main(task_t tsk, msg_t **msg, void *userdata);

static status_t
__task_mgr_broadcast(task_mgr_t *ptskmgr, unsigned short cmd, void *prm, unsigned int size, unsigned int flags);

static status_t
__task_mgr_synchronize(task_t to, task_t frm, unsigned short cmd, void *prm, unsigned int size, unsigned int flags);

static status_t
__task_mgr_task_instruments(task_mgr_t *ptskmgr);

static status_t
__task_mgr_thdpool_instruments(task_mgr_t *ptskmgr);

static status_t
__task_mgr_find_task(task_mgr_t *ptskmgr, task_mgr_find_prm_t *prm);

static bool
__task_mgr_find_match_fxn2(dlist_element_t *elem, void *data);

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
 *  @Called By:	    //调用本函数的函数清单
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
status_t task_mgr_init(task_mgr_prm_t *prm)
{
    status_t     status = OSA_SOK;
    task_mgr_t * ptskmgr = (task_mgr_t *)&glb_tsk_mgr_obj;

    if (glb_cur_init++ == 0) {

        if (prm == NULL) {
            prm = &glb_tsk_mgr_prm;
        }

        status = __task_mgr_initialize(ptskmgr, prm);
    }

    return status;
}

status_t task_mgr_register(task_object_t *ptsk)
{
    task_mgr_t * ptskmgr = &glb_tsk_mgr_obj;

    task_mgr_check_arguments(ptsk);

    return __task_mgr_synchronize(ptskmgr->m_tsk_obj.m_task,
                                  ptskmgr->m_tsk_obj.m_task, TASK_MGR_CMD_REGISTER_TASK,
                                  ptsk, sizeof(*ptsk), MSG_FLAGS_WAIT_ACK);
}

status_t task_mgr_start(task_object_t *ptsk)
{
    task_mgr_t * ptskmgr = &glb_tsk_mgr_obj;

    task_mgr_check_arguments(ptsk);

#if 0
    return __task_mgr_synchronize(ptskmgr->m_tsk_obj.m_task,
                                  ptskmgr->m_tsk_obj.m_task, TASK_MGR_CMD_START_TASK,
                                  ptsk, sizeof(*ptsk), MSG_FLAGS_WAIT_ACK);
#else
    /*
     *  Modified by: xiong-kaifang.
     *
     *  Date       : Nov 20, 2013.
     *
     *  Description:
     *
     *      At present, we don't use task manager to start or stop other task,
     *      because task manager will be blocked if we do that recursion.
     *
     */

    if (!task_check_state(ptsk->m_task)) {
        return OSA_SOK;
    }

    return __task_mgr_start_task(ptskmgr, ptsk);
#endif
}

status_t task_mgr_stop(task_object_t *ptsk)
{
    task_mgr_t * ptskmgr = &glb_tsk_mgr_obj;

    task_mgr_check_arguments(ptsk);

#if 0
    return __task_mgr_synchronize(ptskmgr->m_tsk_obj.m_task,
                                  ptskmgr->m_tsk_obj.m_task, TASK_MGR_CMD_STOP_TASK,
                                  tsk, sizeof(*tsk), MSG_FLAGS_WAIT_ACK);
#else
    /*
     *  Modified by: xiong-kaifang.
     *
     *  Date       : Nov 20, 2013.
     *
     *  Description:
     *
     *      At present, we don't use task manager to start or stop other task,
     *      because task manager will be blocked if we do that recursion.
     *
     */

    if (!task_check_state(ptsk->m_task)) {
        return OSA_SOK;
    }

    return __task_mgr_stop_task(ptskmgr, ptsk);
#endif
}

status_t task_mgr_broadcast(unsigned short cmd, void *prm, unsigned int size, unsigned int flags)
{
    task_mgr_t * ptskmgr = &glb_tsk_mgr_obj;

    return __task_mgr_broadcast(ptskmgr, cmd, prm, size, flags);
}

status_t task_mgr_synchronize(task_object_t *ptsk, unsigned short cmd, void *prm, unsigned int size, unsigned int flags)
{
    task_mgr_t *ptskmgr = &glb_tsk_mgr_obj;

    task_mgr_check_arguments(ptsk);

    return __task_mgr_synchronize(ptsk->m_task, ptskmgr->m_tsk_obj.m_task, cmd, prm, size, flags);
}

status_t task_mgr_synchronize3(task_object_t *pto, task_object_t * pfrm,
        unsigned short cmd, void *prm, unsigned int size, unsigned int flags)
{
    task_mgr_t *ptskmgr = &glb_tsk_mgr_obj;

    task_mgr_check_arguments(pto);

    if (!pfrm) {
        pfrm = &ptskmgr->m_tsk_obj;
    }

    return __task_mgr_synchronize(pto->m_task, pfrm->m_task, cmd, prm, size, flags);
}

status_t task_mgr_synchronize2(unsigned short cmd, void *prm, unsigned int size, unsigned int flags)
{
    status_t        status  = OSA_SOK;
    task_mgr_t    * ptskmgr = &glb_tsk_mgr_obj;
    task_object_t * ptsk    = NULL;

    osa_mutex_lock(ptskmgr->m_mutex);

    status = dlist_search_element(&ptskmgr->m_tsklists, (void *)&cmd,
                                 (dlist_element_t **)&ptsk, __task_mgr_find_match_fxn2);
    if (!OSA_ISERROR(status)) {
        status = OSA_SOK;
    } else {
        status = OSA_ENOENT;
    }

    osa_mutex_unlock(ptskmgr->m_mutex);

    if (!OSA_ISERROR(status)) {
        status = task_mgr_synchronize(ptsk, cmd, prm, size, flags);
    }

    return status;
}

status_t task_mgr_unregister(task_object_t *ptsk)
{
    task_mgr_t *ptskmgr = &glb_tsk_mgr_obj;

    task_mgr_check_arguments(ptsk);

    return __task_mgr_synchronize(ptskmgr->m_tsk_obj.m_task,
                                  ptskmgr->m_tsk_obj.m_task, TASK_MGR_CMD_UNREGISTER_TASK,
                                  ptsk, sizeof(*ptsk), MSG_FLAGS_WAIT_ACK);
}

status_t task_mgr_find(const char *name, task_object_t **ptsk)
{
    status_t     status  = OSA_SOK;
    task_mgr_t * ptskmgr = &glb_tsk_mgr_obj;

    (*ptsk) = NULL;

    /* Find the task in the task mgr */
    task_mgr_find_prm_t prm;

    prm.m_name = (unsigned char *)name;
    prm.m_task = NULL;

    status = __task_mgr_synchronize(ptskmgr->m_tsk_obj.m_task,
                                    ptskmgr->m_tsk_obj.m_task, TASK_MGR_CMD_FIND_TASK,
                                    &prm, sizeof(prm), MSG_FLAGS_WAIT_ACK);

    if (!OSA_ISERROR(status)) {
        (*ptsk) = prm.m_task;
    }

    return status;
}

status_t task_mgr_task_instruments(void)
{
    task_mgr_t * ptskmgr = &glb_tsk_mgr_obj;

    return __task_mgr_synchronize(ptskmgr->m_tsk_obj.m_task,
                                  ptskmgr->m_tsk_obj.m_task, TASK_MGR_CMD_TASK_INSTRUMENTS,
                                  NULL, 0, MSG_FLAGS_WAIT_ACK);
}

status_t task_mgr_thdpool_instruments(void)
{
    task_mgr_t * ptskmgr = &glb_tsk_mgr_obj;

    return __task_mgr_synchronize(ptskmgr->m_tsk_obj.m_task,
                                  ptskmgr->m_tsk_obj.m_task, TASK_MGR_CMD_THDPOOL_INSTRUMENTS,
                                  NULL, 0, MSG_FLAGS_WAIT_ACK);
}

status_t task_mgr_deinit(void)
{
    status_t     status  = OSA_SOK;
    task_mgr_t * ptskmgr = &glb_tsk_mgr_obj;

    if (--glb_cur_init == 0) {
        status = __task_mgr_deinitialize(ptskmgr);
    }

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
__task_mgr_env_initialize(task_mgr_prm_t *prm)
{
    status_t             status = OSA_SOK;
    msgq_mgr_prm_t       msgq_mgr_prm;
    mailbox_system_prm_t mbx_sys_prm;
    tasklist_params_t    tsklist_prm;

    msgq_mgr_prm.m_msgq_cnt    = prm->m_max_tsk_nums * 2;
    mbx_sys_prm.m_mbx_cnt      = prm->m_max_tsk_nums;
    tsklist_prm.m_min_tsk_nums = prm->m_min_tsk_nums;
    tsklist_prm.m_max_tsk_nums = prm->m_max_tsk_nums;
    tsklist_prm.m_max_linger   = prm->m_max_linger;

    status |= msgq_mgr_init(&msgq_mgr_prm);

    status |= mailbox_system_init(&mbx_sys_prm);

    status |= tasklist_init(&tsklist_prm);
    
    OSA_assert(OSA_SOK == status);

    return status;
}

static status_t
__task_mgr_env_deinitialize(void)
{
    status_t status = OSA_SOK;

    status |= tasklist_deinit();

    status |= mailbox_system_deinit();

    status |= msgq_mgr_deinit();

    return status;
}

static status_t
__task_mgr_initialize(task_mgr_t *ptskmgr, task_mgr_prm_t *prm)
{
    int      i;
	status_t status = OSA_SOK;

    /*
     *  Initialize task manager env.
     */
    status = __task_mgr_env_initialize(prm);
    if (OSA_ISERROR(status)) {
        return status;
    }

    status = osa_mutex_create(&ptskmgr->m_mutex);

    status = dlist_init(&ptskmgr->m_tsklists);

    /* Register task manager */
    ptskmgr->m_tsk_obj.m_name       = TASK_MGR_TASK_NAME;
    ptskmgr->m_tsk_obj.m_main       = __task_mgr_internal_main;
    ptskmgr->m_tsk_obj.m_pri        = 0;
    ptskmgr->m_tsk_obj.m_stack_size = 0;
    ptskmgr->m_tsk_obj.m_init_state = 0;
    ptskmgr->m_tsk_obj.m_userdata   = (void *)ptskmgr;

    status = __task_mgr_register_task(ptskmgr, &ptskmgr->m_tsk_obj);

    if (OSA_ISERROR(status)) {
        osa_mutex_delete(&ptskmgr->m_mutex);
        __task_mgr_env_deinitialize();
        return status;
    }

    /* Send INIT cmd */
    status = __task_mgr_synchronize(ptskmgr->m_tsk_obj.m_task,
                                    ptskmgr->m_tsk_obj.m_task, TASK_CMD_INIT,
                                    NULL, 0, MSG_FLAGS_WAIT_ACK);
    if (OSA_ISERROR(status)) {
        __task_mgr_synchronize(ptskmgr->m_tsk_obj.m_task,
                               ptskmgr->m_tsk_obj.m_task, TASK_CMD_EXIT,
                               NULL, 0, MSG_FLAGS_WAIT_ACK);

        __task_mgr_unregister_task(ptskmgr, &ptskmgr->m_tsk_obj);

        osa_mutex_delete(&ptskmgr->m_mutex);
        __task_mgr_env_deinitialize();

        return status;
    }

    /* Send PROC cmd */
    status = __task_mgr_synchronize(ptskmgr->m_tsk_obj.m_task,
            ptskmgr->m_tsk_obj.m_task, TASK_CMD_PROC, NULL, 0, MSG_FLAGS_WAIT_ACK);
    if (OSA_ISERROR(status)) {
        __task_mgr_synchronize(ptskmgr->m_tsk_obj.m_task,
                               ptskmgr->m_tsk_obj.m_task, TASK_CMD_EXIT,
                               NULL, 0, MSG_FLAGS_WAIT_ACK);

        __task_mgr_unregister_task(ptskmgr, &ptskmgr->m_tsk_obj);

        osa_mutex_delete(&ptskmgr->m_mutex);
        __task_mgr_env_deinitialize();
        return status;
    }

    ptskmgr->m_initialized = TRUE;

    OSA_assert(OSA_SOK == status);

	return status;
}

static status_t
__task_mgr_deinitialize(task_mgr_t *ptskmgr)
{
    status_t        status       = OSA_SOK;
    task_object_t * cur_tsk_node = NULL;
    task_object_t * nex_tsk_node = NULL;

	/* 
     * Deinitialize task manager object.
     *
     */

    /* Delete the tasks from the tasklists */
    status = dlist_first(&ptskmgr->m_tsklists, (dlist_element_t **)&cur_tsk_node);

    while (!OSA_ISERROR(status) && (cur_tsk_node != NULL)) {

        status = dlist_next(&ptskmgr->m_tsklists,
                            (dlist_element_t *) cur_tsk_node,
                            (dlist_element_t **)&nex_tsk_node
                            );

        if (OSA_ISERROR(status) || nex_tsk_node == NULL) {
            break;
        }

        /* Send UNREGISTER_TASK cmd to delete task from tasklists */
         __task_mgr_synchronize(ptskmgr->m_tsk_obj.m_task,
                                ptskmgr->m_tsk_obj.m_task, TASK_MGR_CMD_UNREGISTER_TASK,
                                (void *)nex_tsk_node, sizeof(*nex_tsk_node), MSG_FLAGS_WAIT_ACK);
        
        status = dlist_first(&ptskmgr->m_tsklists, (dlist_element_t **)&cur_tsk_node);
    }

    /* Send EXIT cmd to task manager itself */
    status |= __task_mgr_synchronize(ptskmgr->m_tsk_obj.m_task,
                                     ptskmgr->m_tsk_obj.m_task, TASK_CMD_EXIT,
                                     NULL, 0, MSG_FLAGS_WAIT_ACK);

    /* Delete task manager itself from tasklists */
    status |= __task_mgr_unregister_task(ptskmgr, &ptskmgr->m_tsk_obj);

    status |= osa_mutex_delete(&ptskmgr->m_mutex);

    /*
     *  Deinitialize task mansger env.
     */
    status |= __task_mgr_env_deinitialize();

    ptskmgr->m_initialized = FALSE;

	return status;
}

static status_t
__task_mgr_broadcast(task_mgr_t *ptskmgr, unsigned short cmd, void *prm, unsigned int size, unsigned int flags)
{
    int             i;
    int             msg_cnt = 0;

    msg_t         * msg = NULL;
    status_t        status = OSA_SOK;
    task_t          tsklists[TASK_MGR_TASK_MAX];
    task_state_t    tsk_state;
    task_object_t * cur_tsk_node = NULL;
    task_object_t * nex_tsk_node = NULL;

    for (i = 0; i < OSA_ARRAYSIZE(tsklists); i++) {
        tsklists[i] = TASK_INVALID_TSK;
    }

    osa_mutex_lock(ptskmgr->m_mutex);

    status = dlist_first(&ptskmgr->m_tsklists, (dlist_element_t **)&cur_tsk_node);
    OSA_assert(OSA_SOK == status);

    do {
        status = dlist_next(&ptskmgr->m_tsklists,
                            (dlist_element_t *) cur_tsk_node,
                            (dlist_element_t **)&nex_tsk_node
                           );
        if (!OSA_ISERROR(status) && nex_tsk_node != NULL) {
            status = task_get_state(nex_tsk_node->m_task, &tsk_state);
            OSA_assert(OSA_SOK == status);
            if ((nex_tsk_node->m_main != NULL) && (tsk_state != TASK_STATE_EXIT)) {
                tsklists[msg_cnt++] = nex_tsk_node->m_task;
            }
            cur_tsk_node = nex_tsk_node;
        }
    } while (!OSA_ISERROR(status) && (nex_tsk_node != NULL));

    osa_mutex_unlock(ptskmgr->m_mutex);

    if (msg_cnt == 0) {
        return OSA_SOK;
    }

    for (i = 0; i < msg_cnt; i++) {
        status |= __task_mgr_synchronize(tsklists[i], ptskmgr->m_tsk_obj.m_task, cmd, prm, size, flags);
    }

    return status;
}

static status_t
__task_mgr_synchronize(task_t to, task_t frm, unsigned short cmd, void *prm, unsigned int size, unsigned int flags)
{
    msg_t      * msg = NULL;
    msg_t      * rcv_msg = NULL;
    status_t     status = OSA_SOK;
    unsigned int id;
	
    /* Synchronize task */
    status = task_alloc_msg(sizeof(*msg), &msg);

    if (OSA_ISERROR(status)) {
        return status;
    }

    id = msg_get_msg_id(msg);

    msg_set_cmd(msg, cmd);
    msg_set_payload_ptr(msg, prm);
    msg_set_payload_size(msg, size);

    /*
     *  Added by   : xiong-kaifang.
     *
     *  Date       : Oct 06, 2016.
     *
     *  Description:
     *
     *      If user doesn't set msg priority, using the default value.
     *
     */
    msg_clear_flags(msg, MSG_FLAGS_PRI_MASK);
    if (!(flags & MSG_FLAGS_PRI_MASK)) {
        flags |= MSG_FLAGS_DEFAULT_PRI;
    }
    msg_set_flags(msg, flags);
    msg_set_msg_size(msg, sizeof(*msg));

    status = task_send_msg(to, frm, msg, MSG_TYPE_CMD);
    if (OSA_ISERROR(status)) {
        task_free_msg(sizeof(*msg), msg);
        return status;
    }

    if (flags & MSG_FLAGS_WAIT_ACK) {
        status = task_wait_ack(frm, &rcv_msg, id);
        if (OSA_ISERROR(status)) {
            return status;
        }

        OSA_assert(msg == rcv_msg);

        status = msg_get_status(rcv_msg);

        task_free_msg(sizeof(*msg), msg);
    }

	return status;
}

static status_t
__task_mgr_register_task(task_mgr_t *ptskmgr, task_object_t *ptsk)
{
    status_t            status = OSA_SOK;
    task_mgr_find_prm_t prm;

    /*
     *  Check the task wheter it is already registered.
     */
    prm.m_name = (unsigned char *)ptsk->m_name;
    prm.m_task = NULL;
    status = __task_mgr_find_task(ptskmgr, &prm);

    if (!OSA_ISERROR(status)) {
        OSA_assert(prm.m_task == ptsk);
        return status;
    }

    /* Create task */
    status = task_create(ptsk->m_name,
                         ptsk->m_main,
                         ptsk->m_pri,
                         ptsk->m_stack_size,
                         ptsk->m_init_state,
                         ptsk->m_userdata,
                         &ptsk->m_task
                        );

    if (OSA_ISERROR(status)) {
        return status;
    }

    /* Register task object */
    osa_mutex_lock(ptskmgr->m_mutex);

    status = dlist_initialize_element((dlist_element_t *)ptsk);
    status = dlist_put_tail(&ptskmgr->m_tsklists, (dlist_element_t *)ptsk);

    ptskmgr->m_tsk_cnt += 1;

    osa_mutex_unlock(ptskmgr->m_mutex);

    return status;
}

static status_t
__task_mgr_start_task(task_mgr_t *ptskmgr, task_object_t *ptsk)
{
    status_t            status = OSA_SOK;
    task_mgr_find_prm_t prm;

    /*
     *  Check the task wheter it is already registered.
     */
    prm.m_name = (unsigned char *)ptsk->m_name;
    prm.m_task = NULL;
    status = __task_mgr_find_task(ptskmgr, &prm);

    if (OSA_ISERROR(status)) {
        return status;
    }

    status = __task_mgr_synchronize(ptsk->m_task, ptskmgr->m_tsk_obj.m_task, TASK_CMD_INIT, NULL, 0, MSG_FLAGS_WAIT_ACK);
    if (OSA_ISERROR(status)) {
        return status;
    }

    return __task_mgr_synchronize(ptsk->m_task, ptskmgr->m_tsk_obj.m_task, TASK_CMD_PROC, NULL, 0, MSG_FLAGS_WAIT_ACK);
}

static status_t
__task_mgr_stop_task(task_mgr_t *ptskmgr, task_object_t *ptsk)
{
    status_t            status = OSA_SOK;
    task_mgr_find_prm_t prm;

    /*
     *  Check the task wheter it is already registered.
     */
    prm.m_name = (unsigned char *)ptsk->m_name;
    prm.m_task = NULL;
    status = __task_mgr_find_task(ptskmgr, &prm);

    if (OSA_ISERROR(status)) {
        return status;
    }

    return __task_mgr_synchronize(ptsk->m_task, ptskmgr->m_tsk_obj.m_task, TASK_CMD_EXIT, NULL, 0, MSG_FLAGS_WAIT_ACK);
}

static status_t
__task_mgr_unregister_task(task_mgr_t *ptskmgr, task_object_t *ptsk)
{
    status_t            status = OSA_SOK;
    task_mgr_find_prm_t prm;

    /*
     *  Check the task wheter it is already registered.
     */
    prm.m_name = (unsigned char *)ptsk->m_name;
    prm.m_task = NULL;
    status = __task_mgr_find_task(ptskmgr, &prm);

    if (OSA_ISERROR(status)) {
        return status;
    }

    /* Delete task */
    status = task_delete(&ptsk->m_task);

    /* Unregister task object */
    osa_mutex_lock(ptskmgr->m_mutex);

    status = dlist_remove_element(&ptskmgr->m_tsklists, (dlist_element_t *)ptsk);

    osa_mutex_unlock(ptskmgr->m_mutex);

    return status;
}

static status_t
__task_mgr_instruments_apply_fxn(dlist_element_t *elem, void *data)
{
    static int      cnt      = 0;
    status_t        status   = OSA_SOK;
    task_state_t    state;
    task_mgr_t    * ptskmgr  = NULL;
    task_object_t * tsk_node = NULL;

    tsk_node = (task_object_t *)elem;
    ptskmgr  = (task_mgr_t    *)data;

    status = task_get_state(tsk_node->m_task, &state);
    OSA_assert(OSA_SOK == status);

    fprintf(stdout, "    [%02d]    | [0x%08x] |    [%s]    | [%-22s]\n",
            cnt++, tsk_node->m_task, STATE_STR[state], tsk_node->m_name);

    if (cnt >= ptskmgr->m_tsk_cnt) {
        cnt = 0;
    }

    return status;
}

static status_t
__task_mgr_task_instruments(task_mgr_t *ptskmgr)
{
    status_t status = OSA_SOK;

    fprintf(stdout, "\nTASK_MGR: Satatistics.\n"
                    "\n   INDEX    |     TASK     |     STATE    |     NAME"
                    "\n--------------------------------------------------------------------\n"
                    );

    osa_mutex_lock(ptskmgr->m_mutex);

    status = dlist_map(&ptskmgr->m_tsklists, __task_mgr_instruments_apply_fxn, (void *)ptskmgr);

    osa_mutex_unlock(ptskmgr->m_mutex);

    return status;
}

static status_t
__task_mgr_thdpool_instruments(task_mgr_t *ptskmgr)
{
    return tasklist_instruments();
}

static bool
__task_mgr_find_match_fxn(dlist_element_t *elem, void *data)
{
    return (strcmp(((task_object_t *)elem)->m_name,
                   ((task_mgr_find_prm_t *)data)->m_name) == 0);
}

static status_t
__task_mgr_find_task(task_mgr_t *ptskmgr, task_mgr_find_prm_t *prm)
{
    status_t        status   = OSA_SOK;
    task_object_t * tsk_node = NULL;

    prm->m_task = NULL;

    osa_mutex_lock(ptskmgr->m_mutex);

    status = dlist_search_element(&ptskmgr->m_tsklists, (void *)prm,
                                 (dlist_element_t **)&tsk_node, __task_mgr_find_match_fxn);
    if (!OSA_ISERROR(status)) {
        prm->m_task = tsk_node;
        status = OSA_SOK;
    } else {
        status = OSA_ENOENT;
    }

    osa_mutex_unlock(ptskmgr->m_mutex);

    return status;
}

static bool
__task_mgr_find_match_fxn2(dlist_element_t *elem, void *data)
{
    bool            is_found = false;
    status_t        status   = OSA_ENOENT;
    task_object_t * ptsk     = (task_object_t *)elem;

    if (ptsk->m_find != NULL) {
        status = (*ptsk->m_find)(ptsk->m_userdata, *(unsigned short *)data, NULL);
    }

    if (!OSA_ISERROR(status)) {
        is_found = true;
    }

    return is_found;
}

static status_t
__task_mgr_internal_main(task_t tsk, msg_t **msg, void *userdata)
{
	status_t	  status  = OSA_SOK;
    task_mgr_t  * ptskmgr = (task_mgr_t *)userdata;
	
    OSA_assert(ptskmgr != NULL);

    switch(msg_get_cmd((*msg)))
    {
        case TASK_CMD_INIT:
            status = task_set_state(tsk, TASK_STATE_INIT);
            break;

        case TASK_CMD_PROC:
            status = task_set_state(tsk, TASK_STATE_PROC);
            break;

        case TASK_CMD_EXIT:
            status = task_set_state(tsk, TASK_STATE_EXIT);
            break;

        /* Cmds for task manager */
        case TASK_MGR_CMD_REGISTER_TASK:
            status = __task_mgr_register_task(ptskmgr, (task_object_t *)msg_get_payload_ptr((*msg)));
            break;

        case TASK_MGR_CMD_START_TASK:
            status = __task_mgr_start_task(ptskmgr, (task_object_t *)msg_get_payload_ptr((*msg)));
            break;

        case TASK_MGR_CMD_STOP_TASK:
            status = __task_mgr_stop_task(ptskmgr, (task_object_t *)msg_get_payload_ptr((*msg)));
            break;

        case TASK_MGR_CMD_UNREGISTER_TASK:
            status = __task_mgr_unregister_task(ptskmgr, (task_object_t *)msg_get_payload_ptr((*msg)));
            break;

        case TASK_MGR_CMD_TASK_INSTRUMENTS:
            status = __task_mgr_task_instruments(ptskmgr);
            break;

        case TASK_MGR_CMD_THDPOOL_INSTRUMENTS:
            status = __task_mgr_thdpool_instruments(ptskmgr);
            break;

        case TASK_MGR_CMD_FIND_TASK:
            status = __task_mgr_find_task(ptskmgr, (task_mgr_find_prm_t *)msg_get_payload_ptr((*msg)));
            break;

        default:
            break;
    }

	return (status);
}

#if defined(__cplusplus)
}
#endif
