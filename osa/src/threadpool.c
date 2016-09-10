/** ============================================================================
 *
 *  Copyright (C), 1987 - 2015, xiong-kaifang Tech. Co.,, Ltd.
 *
 *  @File name:	threadpool.c
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2012-09-12
 *
 *  @Description:   The thread pool.
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
 *  xiong-kaifang   2012-09-12     v1.0	        Write this module.
 *
 *  xiong-kaifang   2015-08-21     V1.1         Add
 *                                              threadpool_cancel_task routine.
 *
 *  xiong-kaifang   2015-09-18     v1.2         1. Add m_max_linger parameter,
 *                                                 idle threads will exit if the
 *                                                 time of waiting for task
 *                                                 beyond this number.
 *                                              2. Use osa_mutex_t and
 *                                                 osa_cond_t.
 *                                              3. Allocate and deallocate
 *                                                 thdpool task dynamically.
 *                                              4. Delete all debug messages in
 *                                                 case of thread cancellation.
 *                                              5. Others misc tweak.
 *
 *  xiong-kaifang   2015-10-04     v1.3         Using 'INVALID_HANDLE' to
 *                                              initialize related variables.
 *
 *  ============================================================================
 */

/*  --------------------- Include system headers ---------------------------- */
#include <stdio.h>
#include <string.h>
#include <errno.h>

#if defined(_WIN32)
#else
#include <pthread.h>
#endif

/*  --------------------- Include user headers   ---------------------------- */
#include "osa.h"
#include "threadpool.h"
#include "osa_mem.h"
#include "osa_mutex.h"
#include "thread.h"
#include "dlist.h"
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
#define THREADPOOL_THREAD_MIN               (2)
#define THREADPOOL_THREAD_MAX               (32)
#define THREADPOOL_LINGER_MAX               (30 * 60)

#define THREADPOOL_CANCEL_SHRESHOLD         (1000)

#define thdpool_check_arguments(arg)         osa_check_arguments(arg)

#define thdpool_check_arguments2(arg1, arg2) osa_check_arguments2(arg1, arg2)

#define thdpool_check_arguments3(arg1, arg2, arg3)  \
        osa_check_arguments3(arg1, arg2, arg3)

/** ============================================================================
 *  @Macro:         THREADPOOL_DEBUG
 *
 *  @Description:   Description of this macro.
 *  ============================================================================
 */
#define THREADPOOL_DEBUG

#if !defined(THREADPOOL_DEBUG)

#ifdef DBG
#undef DBG
#define DBG(level, tag, arg...)
#endif

#endif  /* if defined THREADPOOL_DEBUG */

/** ============================================================================
 *  @Macro:         CANCELLATION_DEBUG
 *
 *  @Description:   Description of this macro.
 *  ============================================================================
 */
//#define CANCELLATION_DEBUG

#if defined(CANCELLATION_DEBUG)
#define CAN_DBG(level, tag, arg...) osa_debugger((level), (tag), ##arg);
#else
#define CAN_DBG(level, tag, arg...)
#endif  /* if defined CANCELLATION_DEBUG */

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
enum __threadpool_state_t;
typedef enum __threadpool_state_t threadpool_state_t;
enum __threadpool_state_t
{
    THREADPOOL_STATE_WAIT    = 0x01,
    THREADPOOL_STATE_DESTROY = 0x02,
};

struct __thdpool_task_t;
typedef struct __thdpool_task_t thdpool_task_t;
struct __thdpool_task_t
{
    DLIST_ELEMENT_RESERVED;
    threadpool_task_state_t m_tsk_state;
    task_data_t             m_tsk_ops;
    void                  * m_userdata;
};

struct __threadpool_t
{
    DLIST_ELEMENT_RESERVED;

    osa_mutex_t             m_mutex;
    osa_cond_t              m_work_cond;
    osa_cond_t              m_wait_cond;
    osa_cond_t              m_exit_cond;
    unsigned int            m_task_exit;

    dlist_t                 m_busy_list;
    dlist_t                 m_idle_list;

    dlist_t                 m_task_list;
    dlist_t                 m_runn_list;
    dlist_t                 m_fini_list;
    dlist_t                 m_cell_list;

    unsigned int            m_min_thd_nums;
    unsigned int            m_max_thd_nums;
    unsigned int            m_cur_thd_nums;
    unsigned int            m_idl_thd_nums;
    unsigned int            m_max_linger;

    threadpool_state_t      m_state;
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
static threadpool_params_t default_thdp_params = {
    .m_min_thd_nums = THREADPOOL_THREAD_MIN,
    .m_max_thd_nums = THREADPOOL_THREAD_MAX,
    .m_max_linger   = THREADPOOL_LINGER_MAX
};

static unsigned int cur_cnt_index  = 0;

static const char * const GT_NAME  = "thdpool";

static const char * const THD_NAME = "N/A";
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
static bool
__threadpool_find_match_fxn(dlist_element_t *elem, void *data);

static status_t task_common_main(const task_data_t *ops, void *arg);

static status_t
__threadpool_init(struct __threadpool_t *pthdp, const threadpool_params_t *params);

static status_t
__threadpool_exit(struct __threadpool_t *pthdp);

static status_t __threadpool_run_stub(void);

static int
__threadpool_task_dispatch(struct __threadpool_t *pthdp, thdpool_task_t **pptsk, bool_t *ptimedout);

static void __threadpool_cleanup_task(thdpool_task_t *ptsk);

static void __threadpool_notify_waiters(struct __threadpool_t *pthdp);

static void __threadpool_cleanup_worker(struct __threadpool_t *pthdp);

static status_t
__threadpool_alloc_task_cell(struct __threadpool_t *pthdp, thdpool_task_t **pptsk);

static status_t
__threadpool_free_task_cell(struct __threadpool_t *pthdp, thdpool_task_t *ptsk);

static status_t
__threadpool_instruments(struct __threadpool_t *pthdp, FILE *out);

static status_t
__threadpool_task_exit_madatory(thdpool_task_t *ptsk);

static status_t
__threadpool_task_cell_apply_fxn(dlist_element_t *elem, void *data);

static status_t
__threadpool_cancel_task(struct __threadpool_t *pthdp, task_token_t token);

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
status_t threadpool_create(threadpool_t *thdp, const threadpool_params_t *prm)
{
    status_t                status = OSA_SOK;
    struct __threadpool_t * pthdp  = NULL;

    DBG(DBG_DETAILED, GT_NAME, "threadpool_create: Enter(thdp=0x%x, prm=0x%x).\n", thdp, prm);

    thdpool_check_arguments(thdp);

    (*thdp) = INVALID_HANDLE;

    if (prm == NULL) {
        prm = &default_thdp_params;
    }

    if ((prm->m_min_thd_nums > prm->m_max_thd_nums) || (prm->m_max_thd_nums < 1)) {
        return OSA_EARGS;
    }

    status = OSA_memAlloc(sizeof(struct __threadpool_t), &pthdp);
    if (!OSA_ISERROR(status) && pthdp != NULL) {
        status = __threadpool_init(pthdp, prm);

        if (OSA_ISERROR(status)) {
            OSA_memFree(sizeof(struct __threadpool_t), pthdp);
            pthdp = NULL;
            return status;
        }
    }

    DBG(DBG_INFO, GT_NAME,
            "threadpool_create: Thread pool (min=%d, max=%d, linger=%d) created successfully.\n",
            pthdp->m_min_thd_nums, pthdp->m_max_thd_nums, pthdp->m_max_linger);

    (*thdp) = (threadpool_t)pthdp;

    DBG(DBG_DETAILED, GT_NAME, "threadpool_create: Exit(status=0x%x).\n", status);

    return status;
}

status_t threadpool_add_task(threadpool_t thdp,
        const task_data_t *tsk_data, task_token_t *token)
{
    status_t                status;
    thread_t                thd;
    thread_attrs_t          thd_attrs;
    task_token_t            tsk_token    = INVALID_HANDLE;
    thdpool_task_t        * ptsk         = NULL;
    struct __threadpool_t * pthdp = (struct __threadpool_t *)thdp;

    DBG(DBG_DETAILED, GT_NAME, "threadpool_add_task: Enter (thdp=0x%x, "
        "task_data=0x%x).\n", thdp, tsk_data);

    thdpool_check_arguments3(pthdp, tsk_data, token);

    (*token) = INVALID_HANDLE;

    status = __threadpool_alloc_task_cell(pthdp, &ptsk);
    if (OSA_ISERROR(status)) {
        return status;
    }

    ptsk->m_tsk_state = THREADPOOL_TASK_PENDING;
    ptsk->m_userdata  = NULL;
    memcpy(&ptsk->m_tsk_ops, tsk_data, sizeof(task_data_t));

    /* Get threadpool mutex */
    osa_mutex_lock(pthdp->m_mutex);

    status = dlist_initialize_element((dlist_element_t *)ptsk);
    status = dlist_put_tail(&pthdp->m_task_list, (dlist_element_t *)ptsk);

    if (pthdp->m_idl_thd_nums > 0) {
        DBG(DBG_DETAILED, GT_NAME, "threadpool_add_task: Using idle thread to process this task.\n", ptsk);
        osa_cond_signal(pthdp->m_work_cond);
    } else if (pthdp->m_cur_thd_nums < pthdp->m_max_thd_nums) {
        thd_attrs         = default_thd_attrs;
        thd_attrs.name    = (char *)THD_NAME;
        thd_attrs.environ = (void *)pthdp;

        DBG(DBG_DETAILED, GT_NAME, "threadpool_add_task: Create a new thread to process this task.\n");
        status = thread_create(&thd, (Fxn)__threadpool_run_stub, &thd_attrs);
        if (!OSA_ISERROR(status)) {
            status = dlist_initialize_element((dlist_element_t *)thd);
            status = dlist_put_tail(&pthdp->m_idle_list, (dlist_element_t *)thd);

            pthdp->m_cur_thd_nums++;
        }
    }

    /* Release threadpool mutex */
    osa_mutex_unlock(pthdp->m_mutex);

    (*token) = (task_token_t)ptsk;

    DBG(DBG_DETAILED, GT_NAME, "threadpool_add_task: Exit (token=0x%x).\n", ptsk);

    return status;
}

#if 0
int  threadpool_sync_call(threadpool_handle hdl, 
        task_token_t tsk_token, int cmd, void *arg)
{
    int                      status = 0;
    threadpool_task_data_t * tsk_p  = NULL;

    DBG(DBG_DETAILED, "threadpool_sync_call: Enter (hdl=0x%x, tsk_token=0x%x "
            "cmd=0x%x, arg=0x%x)\n", hdl, tsk_token, cmd, arg);

    if (hdl == NULL || tsk_token == NULL) {
        DBG(DBG_ERROR, "threadpool_sync_task: Invalid arguments.\n");
        return -EINVAL;
    }

    tsk_p = (threadpool_task_data_t *)tsk_token;

    status = threadsync_call(hdl->mbx_id, tsk_p->mbx_id, cmd, arg);

    DBG(DBG_DETAILED, "threadpool_sync_call: Exit (status=0x%x)\n", status);

    return status;
}

void threadpool_sync(threadpool_handle hdl, task_token_t tsk_token, 
        THREADPOOLSYNC_FXN sync_fxn, int nmsgs)
{
    int                      status = 0;
    threadpool_task_data_t * tsk_p  = NULL;

    DBG(DBG_DETAILED, "threadpool_sync: Enter (hdl=0x%x, tsk_token=0x%x "
            "sync_fxn=0x%x, nmsgs=0x%x)\n", hdl, tsk_token, sync_fxn, nmsgs);

    if (hdl == NULL || tsk_token == NULL || sync_fxn == NULL || nmsgs <= 0) {
        DBG(DBG_ERROR, "threadpool_sync: Invalid arguments.\n");
        return ;
    }

    tsk_p = (threadpool_task_data_t *)tsk_token;

    threadsync(tsk_p, tsk_p->mbx_id, (THREADSYNC_FXN)sync_fxn, nmsgs);

    DBG(DBG_DETAILED, "threadpool_sync: Exit (status=0x%x)\n", status);
}
#endif

status_t threadpool_cancel_task(threadpool_t thdp, task_token_t *ptoken)
{
    status_t                status      = OSA_SOK;
    task_token_t            token       = (task_token_t)(*ptoken);
    struct __threadpool_t * pthdp       = (struct __threadpool_t *)thdp;

    DBG(DBG_DETAILED, GT_NAME, "threadpool_cancel_task: Enter (thdp=0x%x).\n", thdp);

    thdpool_check_arguments3(pthdp, ptoken, HANDLE_TO_POINTER(token));

    (*ptoken) = INVALID_HANDLE;

    status = __threadpool_cancel_task(pthdp, token);

    DBG(DBG_DETAILED, GT_NAME, "threadpool_cancel_task: Leave (status=0x%x).\n", status);

    return status;
}

status_t threadpool_wait(threadpool_t thdp)
{
    status_t                status = OSA_SOK;
    struct __threadpool_t * pthdp  = (struct __threadpool_t *)thdp;

    DBG(DBG_DETAILED, GT_NAME, "threadpool_wait: Enter (thdp=0x%x).\n", thdp);

    thdpool_check_arguments(pthdp);

    /* Get threadpool mutex */
    status |= osa_mutex_lock(pthdp->m_mutex);

    while (!dlist_is_empty(&pthdp->m_task_list)
            || !dlist_is_empty(&pthdp->m_busy_list)) {
        pthdp->m_state |= THREADPOOL_STATE_WAIT;
        osa_cond_wait(pthdp->m_wait_cond, pthdp->m_mutex);
    }

    /* Release threadpool mutex */
    status |= osa_mutex_unlock(pthdp->m_mutex);

    DBG(DBG_DETAILED, GT_NAME, "threadpool_wait: Exit (status=0x%x).\n", status);

    return status;
}

status_t threadpool_delete(threadpool_t *thdp)
{
    status_t                status = OSA_SOK;
    struct __threadpool_t * pthdp  = (struct __threadpool_t *)(*thdp);

    DBG(DBG_DETAILED, GT_NAME, "threadpool_delete: Enter (thdp=0x%x).\n", thdp);

    thdpool_check_arguments2(thdp, pthdp);

    status = __threadpool_exit(pthdp);

    /*
     *  Free the threadpool object.
     */
    OSA_memFree(sizeof(struct __threadpool_t), pthdp);

    (*thdp) = INVALID_HANDLE;

    DBG(DBG_DETAILED, GT_NAME, "threadpool_delete: Exit(status=0x%x).\n", status);

    return status;
}

status_t threadpool_instruments(threadpool_t thdp)
{
    status_t                status = OSA_SOK;
    struct __threadpool_t * pthdp  = (struct __threadpool_t *)thdp;

    DBG(DBG_DETAILED, GT_NAME, "threadpool_instruments: Enter (thdp=0x%x).\n", thdp);

    thdpool_check_arguments(pthdp);

    /* Get threadpool mutex */
    status |= osa_mutex_lock(pthdp->m_mutex);

    status = __threadpool_instruments(pthdp, stdout); 

    /* Release threadpool mutex */
    status |= osa_mutex_unlock(pthdp->m_mutex);

    DBG(DBG_DETAILED, GT_NAME, "threadpool_instruments: Exit (status=0x%x).\n", status);

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
__threadpool_find_match_fxn(dlist_element_t *elem, void *data)
{
    return (elem == (dlist_element_t *)data);
}

static status_t
task_common_main(const task_data_t *ops, void *arg)
{
    status_t status = OSA_SOK;

    OSA_assert(ops != NULL && ops->m_main != NULL);

    //((thdpool_task_t *)arg)->m_tsk_state = THREADPOOL_TASK_RUNNING;

    status |= ops->m_main(ops->m_args[0], ops->m_args[1],
                          ops->m_args[2], ops->m_args[3]);

    //((thdpool_task_t *)arg)->m_tsk_state = THREADPOOL_TASK_FINISHED;

    return (status);
}

static status_t
__threadpool_init(struct __threadpool_t *pthdp, const threadpool_params_t *params)
{
    int      i;
    status_t status = OSA_SOK;

    DBG(DBG_DETAILED, GT_NAME, "__threadpool_init: Enter (hdl=0x%x, params=0x%x).\n",
            pthdp, params);

    /* Initialize thread module */
    thread_init();

    /* Initialize dlist object */
    status = dlist_init(&pthdp->m_busy_list);
    if (OSA_ISERROR(status)) {
        goto exit_from_error;
    }
    status = dlist_init(&pthdp->m_idle_list);
    if (OSA_ISERROR(status)) {
        goto exit_from_error;
    }

    status = dlist_init(&pthdp->m_task_list);
    if (OSA_ISERROR(status)) {
        goto exit_from_error;
    }
    status = dlist_init(&pthdp->m_runn_list);
    if (OSA_ISERROR(status)) {
        goto exit_from_error;
    }
    status = dlist_init(&pthdp->m_fini_list);
    if (OSA_ISERROR(status)) {
        goto exit_from_error;
    }
    status = dlist_init(&pthdp->m_cell_list);
    if (OSA_ISERROR(status)) {
        goto exit_from_error;
    }

    /* Initialize mutex, cond, etc. */
    status = osa_mutex_create(&pthdp->m_mutex);
    if (OSA_ISERROR(status)) {
        goto exit_from_error;
    }
    status = osa_cond_create(&pthdp->m_work_cond);
    if (OSA_ISERROR(status)) {
        goto exit_from_error;
    }
    status = osa_cond_create(&pthdp->m_wait_cond);
    if (OSA_ISERROR(status)) {
        goto exit_from_error;
    }
    status = osa_cond_create(&pthdp->m_exit_cond);
    if (OSA_ISERROR(status)) {
        goto exit_from_error;
    }

    pthdp->m_min_thd_nums = params->m_min_thd_nums;
    pthdp->m_max_thd_nums = params->m_max_thd_nums;

    pthdp->m_cur_thd_nums = 0;
    pthdp->m_idl_thd_nums = 0;

    pthdp->m_max_linger   = params->m_max_linger;

    pthdp->m_state        = 0;
    pthdp->m_task_exit    = FALSE;

    DBG(DBG_DETAILED, GT_NAME, "__threadpool_init: Exit.\n");

    return status;

exit_from_error:

    /* Finalize thread module */
    if (HANDLE_IS_VALID( pthdp->m_exit_cond)) {
        osa_cond_delete(&pthdp->m_exit_cond);
    }
    if (HANDLE_IS_VALID( pthdp->m_wait_cond)) {
        osa_cond_delete(&pthdp->m_wait_cond);
    }
    if (HANDLE_IS_VALID( pthdp->m_work_cond)) {
        osa_cond_delete(&pthdp->m_work_cond);
    }
    if (HANDLE_IS_VALID( pthdp->m_mutex)) {
        osa_mutex_delete(&pthdp->m_mutex);
    }

    thread_exit();

    return status;
}

static int  __threadpool_exit(struct __threadpool_t *pthdp)
{
    status_t         status  = OSA_SOK;
    thread_t         thd_cur = INVALID_HANDLE;
    thread_t         thd_nex = INVALID_HANDLE;
    thdpool_task_t * ptsk    = NULL;

    DBG(DBG_DETAILED, GT_NAME, "__threadpool_exit: Enter (pthdp=0x%x).\n", pthdp);

    /* Get threadpool mutex */
    osa_mutex_lock(pthdp->m_mutex);

    /* Wakeup idle workers */
    pthdp->m_state |= THREADPOOL_STATE_DESTROY;
    osa_cond_broadcast(pthdp->m_work_cond);

    /* Cancel all active work thread */
    int cnt = 0;
    status = dlist_count(&pthdp->m_busy_list, &cnt);
    DBG(DBG_INFO, GT_NAME, "__threadpool_exit: Cancel all working thread:%d.\n", cnt);
    status = dlist_first(&pthdp->m_busy_list, (dlist_element_t **)&thd_cur);
    while (!OSA_ISERROR(status) && (HANDLE_IS_VALID(thd_cur))) {
        status = thread_cancel(thd_cur);

        status = dlist_next(&pthdp->m_busy_list,
                            (dlist_element_t *)  thd_cur,
                            (dlist_element_t **)&thd_nex
                            );
        if (!OSA_ISERROR(status)) {
            thd_cur = thd_nex;
        } else {
            /* TODO Error process */
            break;
        }
    }

    /* Wait all active task workers to finish */
    DBG(DBG_INFO, GT_NAME, "__threadpool_exit: Wait for all threads to finish.\n");
    while (!dlist_is_empty(&pthdp->m_busy_list)) {
        pthdp->m_state |= THREADPOOL_STATE_WAIT;
        osa_cond_wait(pthdp->m_wait_cond, pthdp->m_mutex);
    }

    /* Wait all task workers to terminate */
    DBG(DBG_INFO, GT_NAME, "__threadpool_exit: Wait for all threads to terminate:%d.\n", pthdp->m_cur_thd_nums);
    while (pthdp->m_cur_thd_nums != 0) {
        osa_cond_wait(pthdp->m_exit_cond, pthdp->m_mutex);
    }

    /* Release threadpool mutex */
    osa_mutex_unlock(pthdp->m_mutex);

    /* Delete all task workers */
    DBG(DBG_INFO, GT_NAME, "__threadpool_exit: Delete all thread.\n");
    status = OSA_SOK;
    while (!dlist_is_empty(&pthdp->m_idle_list) && !OSA_ISERROR(status)) {
        status = 
            dlist_get_head(&pthdp->m_idle_list, (dlist_element_t **)&thd_cur);
        if (!OSA_ISERROR(status)) {
            thread_delete(&thd_cur);
        }
    }

    /* Finalize thread module */
    thread_exit();

    /* Free all the finished task cells */
    status |= dlist_map2(&pthdp->m_fini_list, __threadpool_task_cell_apply_fxn, NULL);
    status |= dlist_map2(&pthdp->m_cell_list, __threadpool_task_cell_apply_fxn, NULL);

    /* Destroy threadpool object */
    status |= osa_cond_delete(&pthdp->m_exit_cond);
    status |= osa_cond_delete(&pthdp->m_wait_cond);
    status |= osa_cond_delete(&pthdp->m_work_cond);

    status |= osa_mutex_delete(&pthdp->m_mutex);

    DBG(DBG_DETAILED, GT_NAME, "__threadpool_exit: Exit.\n");

    return status;
}

static status_t __threadpool_run_stub(void)
{
    int                     status;
    bool                    tsk_exit;
    bool_t                  timedout = FALSE;
    thread_t                thd      = INVALID_HANDLE;
    thdpool_task_t        * ptsk     = NULL;
    struct __threadpool_t * pthdp    = NULL;;

    CAN_DBG(DBG_DETAILED, GT_NAME, "__threadpool_run_stub: Enter.\n");

    thd   = thread_self();
    pthdp = (struct __threadpool_t *)thread_get_env(thd);

    //DBG(DBG_INFO, "__threadpool_run_stub: thd=0x%x, pthdp=0x%x\n", thd, pthdp);

    /* Get threadpool mutex */
    osa_mutex_lock(pthdp->m_mutex);

    /* Register worker cleanup routine */
#if defined(_WIN32)
#else
    pthread_cleanup_push(
            (void (*)(void *))__threadpool_cleanup_worker, (void *)pthdp);
#endif

    for ( ; ; ) {

        pthdp->m_idl_thd_nums++;

        if (pthdp->m_state & THREADPOOL_STATE_WAIT) {
            __threadpool_notify_waiters(pthdp);
        }

        /* Wait for task */
        status = __threadpool_task_dispatch(pthdp, &ptsk, &timedout);

        pthdp->m_idl_thd_nums--;

        /* Case1: The thread pool will be destroyed */
        if (pthdp->m_state & THREADPOOL_STATE_DESTROY) {
            break;
        }

        /* Case2: We timed out */
        if (timedout && (pthdp->m_cur_thd_nums > pthdp->m_min_thd_nums)) {
            CAN_DBG(DBG_DETAILED, GT_NAME," __threadpool_run_stub: Thread[0x%x] exit due to timed out.\n", thd);
            break;
        }

        /* Release threadpool mutex */
        osa_mutex_unlock(pthdp->m_mutex);

        /*
         *  Case3: There is new task in the task list.
         *
         */
        /* Register thread cleanup routine */
#if defined(_WIN32)
#else
        pthread_cleanup_push(
                (void (*)(void *))__threadpool_cleanup_task, (void *)ptsk);
#endif

        /* Process the task */
        status = task_common_main(&ptsk->m_tsk_ops, ptsk);

        /* Clean up task: Call ====> __threadpool_cleanup_task(thdp_hdl) */
#if defined(_WIN32)
        __threadpool_cleanup_task(ptsk);
#else
        pthread_cleanup_pop(1);
#endif

        ptsk = NULL;

        /*
         *  TODO: Bug here. We need get threadpool mutex again, rigth???.
         *
         *  Answer: No, we need not, we have get the mutex in task cleanup
         *          routine.
         */
        //osa_mutex_lock(&thdp_hdl->mutex);

    }

    /* Clean up worker: Call ====> __threadpool_cleanup_worker(thdp_hdl) */
#if defined(_WIN32)
    __threadpool_cleanup_worker(pthdp);
#else
    pthread_cleanup_pop(1);
#endif

    CAN_DBG(DBG_DETAILED, GT_NAME, "__threadpool_run_stub: Exit(status=0x%x).\n", status);

    return status;
}

static int
__threadpool_task_dispatch(struct __threadpool_t *pthdp, thdpool_task_t **pptsk, bool_t *ptimedout)
{
    status_t status = OSA_SOK;
    thread_t thd    = INVALID_HANDLE;

    CAN_DBG(DBG_DETAILED, GT_NAME, "__threadpool_task_dispatch: Enter (pthdp=0x%x, "
        "pptsk=0x%x, ptimedout=0x%x).\n", pthdp, pptsk, ptimedout);

    thd          = thread_self();
    (*pptsk)     = NULL;
    (*ptimedout) = FALSE;

    /*
     *  Note: When we get here, we have already lock the mutex.
     */
    //osa_mutex_lock(&pthdp->m_mutex);

    while (dlist_is_empty(&pthdp->m_task_list)
            && !(pthdp->m_state & THREADPOOL_STATE_DESTROY)) {
        CAN_DBG(DBG_DETAILED, GT_NAME, "__threadpool_task_dispatch: Thread[0x%x] wait for "
                "task.\n", thd);

        if (pthdp->m_cur_thd_nums < pthdp->m_min_thd_nums) {
            status = osa_cond_wait(pthdp->m_work_cond, pthdp->m_mutex);
        } else {

            if (pthdp->m_max_linger == 0) {
                (*ptimedout) = TRUE;
                break;
            }

            status = osa_cond_timedwait(pthdp->m_work_cond,
                                        pthdp->m_mutex,
                                        pthdp->m_max_linger * 1000);
            if (OSA_ETIMEOUT == status) {
                (*ptimedout) = TRUE;
                break;
            }
        }
    }


    /*
     *  Case1: The thread pool will be destroyed.
     *
     *  Case2: We timed out.
     */
    if ((pthdp->m_state & THREADPOOL_STATE_DESTROY) || (*ptimedout)) {
        return status;
    }

    /* Move cur thread from idle list to busy list */
    status = dlist_remove_element(&pthdp->m_idle_list, (dlist_element_t *)thd);
    status = dlist_put_tail(&pthdp->m_busy_list, (dlist_element_t *)thd);

    /* Fetch the task from task data list */
    status = dlist_get_head(&pthdp->m_task_list, (dlist_element_t **)pptsk);
    (*pptsk)->m_userdata  = (void *)thd;
    (*pptsk)->m_tsk_state = THREADPOOL_TASK_RUNNING;

    /* Set the thread name to task name */
    thread_set_name(thd, (*pptsk)->m_tsk_ops.m_name);

    status = dlist_put_tail(&pthdp->m_runn_list, (dlist_element_t *)(*pptsk));

    /*
     *  Note: We have to unlock the mutex outside of this routine.
     */
    //osa_mutex_unlock(&pthdp->m_mutex);

    CAN_DBG(DBG_DETAILED, GT_NAME, "__threadpool_task_dispatch: Exit (status=0x%x).\n", status);

    return status;
}

static void __threadpool_cleanup_task(thdpool_task_t * ptsk)
{
    status_t                status = OSA_SOK;
    bool_t                  bexit  = FALSE;
    thread_t                thd    = INVALID_HANDLE;
    struct __threadpool_t * pthdp  = NULL;;

    CAN_DBG(DBG_DETAILED, GT_NAME, "__threadpool_cleanup_task: Enter (ptsk=0x%x).\n", ptsk);

    thd   = thread_self();
    pthdp = (struct __threadpool_t *)thread_get_env(thd);

    /* Get threadpool mutex */
    osa_mutex_lock(pthdp->m_mutex);

    /* Reset the thread mame to default name: THD_NAME */
    thread_set_name(thd, (char *)THD_NAME);

    /* Move cur thread from busy list to idle list */
    status = dlist_remove_element(&pthdp->m_busy_list, (dlist_element_t *)thd);
    status = dlist_put_tail(&pthdp->m_idle_list, (dlist_element_t *)thd);

    /* Free task cell */
    status = dlist_remove_element(&pthdp->m_runn_list, (dlist_element_t *)ptsk);
    ptsk->m_tsk_state = THREADPOOL_TASK_FINISHED;
    ptsk->m_userdata  = NULL;

    /*
     *  Note: TODO.
     */
    if (pthdp->m_task_exit) {
        pthdp->m_task_exit = FALSE;
        osa_cond_signal(pthdp->m_exit_cond);
    }

    status = dlist_put_tail(&pthdp->m_fini_list, (dlist_element_t *)ptsk);

    /* Check THREADPOOL_WAIT flag */
    bexit  = pthdp->m_state & THREADPOOL_STATE_WAIT;
    if (bexit) {

        /* Notity other task to exit */
        __threadpool_notify_waiters(pthdp);
    }

    /* Release threadpool mutex */
    //osa_mutex_unlock(thdp_hdl->mutex);

    CAN_DBG(DBG_DETAILED, GT_NAME, "__threadpool_cleanup_task: Exit.\n");
}

static void __threadpool_notify_waiters(struct __threadpool_t *pthdp)
{
    CAN_DBG(DBG_DETAILED, GT_NAME, "__threadpool_notify_waiters: Enter (pthdp=0x%x).\n", pthdp);

    if (dlist_is_empty(&pthdp->m_task_list) && dlist_is_empty(&pthdp->m_busy_list)) {
        pthdp->m_state &= ~THREADPOOL_STATE_WAIT;
        CAN_DBG(DBG_INFO, GT_NAME, "__threadpool_notify_waiters: Signal wait condition variable.\n");
        osa_cond_broadcast(pthdp->m_wait_cond);
    }

    CAN_DBG(DBG_DETAILED, GT_NAME, "__threadpool_notify_waiters: Exit.\n");
}

static void __threadpool_cleanup_worker(struct __threadpool_t *pthdp)
{
    int            status;
    thread_t       thd  = INVALID_HANDLE;
    thread_attrs_t thd_attrs;

    CAN_DBG(DBG_DETAILED, GT_NAME, "__threadpool_cleanup_worker: Enter (pthdp=0x%x).\n", pthdp);

    pthdp->m_cur_thd_nums--;

    CAN_DBG(DBG_DETAILED, GT_NAME, "__threadpool_cleanup_worker: Remaining %d thread left.\n", pthdp->m_cur_thd_nums);

    if (pthdp->m_state & THREADPOOL_STATE_DESTROY) {
        if (pthdp->m_cur_thd_nums == 0) {
            CAN_DBG(DBG_DETAILED, GT_NAME, "__threadpool_cleanup_worker: The last thread leave.\n");
            osa_cond_broadcast(pthdp->m_exit_cond);
        }
    }
    else if (!dlist_is_empty(&pthdp->m_task_list)
            && (pthdp->m_cur_thd_nums < pthdp->m_max_thd_nums)) {
        /* TODO: BUG here, current thread need exit, do we need create new one??? */
        CAN_DBG(DBG_DETAILED, GT_NAME, "__threadpool_cleanup_worker: Current thread eixt, create a new thread to process task.\n");
        thd_attrs         = default_thd_attrs;
        thd_attrs.environ = (void *)pthdp;
        status = thread_create(&thd, (Fxn)__threadpool_run_stub, &thd_attrs);
        if (!OSA_ISERROR(status)) {

            /*
             *  Modified by: xiong-kaifang.
             *
             *  Date       : May 16, 2014.
             *
             *  Description:
             *
             *               Add the new created thread to thdpool idle list.
             */
            status = dlist_initialize_element((dlist_element_t *)thd);
            status = dlist_put_tail(&pthdp->m_idle_list, (dlist_element_t *)thd);

            pthdp->m_cur_thd_nums++;
        }
    }

    thd     = thread_self();
    status |= dlist_remove_element(&pthdp->m_idle_list, (dlist_element_t *)thd);

    /* Release threadpool mutex */
    osa_mutex_unlock(pthdp->m_mutex);

    /* TODO: Delete current thread object */
    status |= thread_delete(&thd);

    CAN_DBG(DBG_DETAILED, GT_NAME, "__threadpool_cleanup_worker: Leave: Thread[0x%x] exit.\n", tmp);
}

static status_t
__threadpool_alloc_task_cell(struct __threadpool_t *pthdp, thdpool_task_t **pptsk)
{
    status_t status = OSA_ERES;

    DBG(DBG_DETAILED, GT_NAME, "__threadpool_alloc_task_cell: Enter (pthdp=0x%x, pptsk=0x%x).\n",
            pthdp, pptsk);

    (*pptsk) = NULL;

    /* Get threadpool task mutex */
    osa_mutex_lock(pthdp->m_mutex);

    if (!dlist_is_empty(&pthdp->m_cell_list)) {
        status = dlist_get_head(&pthdp->m_cell_list, (dlist_element_t **)pptsk);
        OSA_assert(OSA_SOK == status);
        status = OSA_SOK;
    }

    /* Release threadpool task mutex */
    osa_mutex_unlock(pthdp->m_mutex);

    if (OSA_ISERROR(status)) {
        status = OSA_memAlloc(sizeof(thdpool_task_t), pptsk);

    }

    DBG(DBG_DETAILED, GT_NAME, "__threadpool_alloc_task_cell: Exit "
            "(status=0x%x).\n", status);

    return status;
}

static status_t
__threadpool_free_task_cell(struct __threadpool_t *pthdp, thdpool_task_t *ptsk)
{
    status_t status = OSA_SOK;

    DBG(DBG_DETAILED, GT_NAME, "__threadpool_free_task_cell: Enter (pthdp=0x%x, "
            "ptsk=0x%x).\n", pthdp, ptsk);
#if 1
    /*
     *  Note: TODO.
     */

    /* Get threadpool task mutex */
    osa_mutex_lock(pthdp->m_mutex);

    status |= dlist_put_tail(&pthdp->m_cell_list, (dlist_element_t *)ptsk);

    /* Release threadpool task mutex */
    osa_mutex_unlock(pthdp->m_mutex);

#else

    status = OSA_memFree(sizeof(thdpool_task_t), ptsk);

#endif

    DBG(DBG_DETAILED, GT_NAME, "__threadpool_free_task_cell: Exit.\n");

    return status;
}

static status_t
__threadpool_instruments_apply_fxn(dlist_element_t *elem, void *data)
{
    status_t status = OSA_SOK;
    thread_t thd    = INVALID_HANDLE;

    thd = (thread_t)elem;

    OSA_assert(HANDLE_IS_VALID(thd));

    /*
     *  TODO: Where should we print this messages to???.
     */

    fprintf(stdout, "    [%02d]    | [0x%08x] |    [%s]    | [%-22s]\n",
            cur_cnt_index++, thd, (const char *)data, thread_get_name(thd));

    return status;
}

static status_t
__threadpool_instruments(struct __threadpool_t *pthdp, FILE *out)
{
    status_t status = OSA_SOK;

    fprintf(out, "\nTHDPOOL: Statistics.\n"
                 "\nTotal Count |  Busy Count  |  Idle Count  |  Max Count  |  Min Count"
                 "\n--------------------------------------------------------------------\n"
           );
    fprintf(out, "    [%02d]    |     [%02d]     |     [%02d]     |     [%02d]    |     [%02d]\n",
                  pthdp->m_cur_thd_nums, pthdp->m_cur_thd_nums - pthdp->m_idl_thd_nums,
                  pthdp->m_idl_thd_nums, pthdp->m_max_thd_nums, pthdp->m_min_thd_nums);

    fprintf(out, "\n   INDEX    |    THREAD    |     STATE    |    NAME"
                 "\n--------------------------------------------------------------------\n");

    status |= dlist_map(&pthdp->m_busy_list, __threadpool_instruments_apply_fxn, (void *)"Busy");
    status |= dlist_map(&pthdp->m_idle_list, __threadpool_instruments_apply_fxn, (void *)"Idle");

    /* Reset cur_cnt_index value */
    cur_cnt_index = 0;

    return status;
}

static status_t
__threadpool_task_exit_madatory(thdpool_task_t *ptsk)
{
    return thread_cancel((thread_t)ptsk->m_userdata);
}

static status_t
__threadpool_task_cell_apply_fxn(dlist_element_t *elem, void *data)
{
    return OSA_memFree(sizeof(thdpool_task_t), (thdpool_task_t *)elem);
}

static status_t
__threadpool_cancel_task(struct __threadpool_t *pthdp, task_token_t token)
{
    bool_t           be_canceled = FALSE;
    status_t         status      = OSA_SOK;
    thdpool_task_t * ptsk        = NULL;

    /* Get threadpool mutex */
    osa_mutex_lock(pthdp->m_mutex);

    /*
     *  Step1: Look up the task in the task list. If we find it, we just remove
     *         it from the task list.
     */
    status = dlist_search_element(&pthdp->m_task_list, (void *)token,
                                 (dlist_element_t **)&ptsk, __threadpool_find_match_fxn);
    if (!OSA_ISERROR(status) && ptsk != NULL) {
        status = dlist_remove_element(&pthdp->m_task_list, (dlist_element_t *)ptsk);
        OSA_assert(OSA_SOK == status);
        be_canceled = TRUE;

        goto return_from_search;
    }

    /*
     *  Step2: Look up the task in the running  list. If not, we just call the 
     *         'exit' callback function to tell the task to exit.
     */
    status = dlist_search_element(&pthdp->m_runn_list, (void *)token,
                                 (dlist_element_t **)&ptsk, __threadpool_find_match_fxn);
    if (!OSA_ISERROR(status) && ptsk != NULL) {

        if (ptsk->m_tsk_ops.m_exit != NULL) {
#if 1
            status = ptsk->m_tsk_ops.m_exit(
                     ptsk->m_tsk_ops.m_args[0], ptsk->m_tsk_ops.m_args[1],
                     ptsk->m_tsk_ops.m_args[2], ptsk->m_tsk_ops.m_args[3]);
#else
            __threadpool_task_exit_madatory(ptsk);
#endif
            status             = OSA_SOK;
            pthdp->m_task_exit = TRUE;
            //DBG(DBG_DETAILED, GT_NAME, "__threadpool_cancel_task: wait.\n");
            while (pthdp->m_task_exit && !OSA_ISERROR(status)) {
                status = osa_cond_timedwait(pthdp->m_exit_cond,
                                            pthdp->m_mutex,
                                            THREADPOOL_CANCEL_SHRESHOLD/*pthdp->m_max_linger*/);
            }
            //DBG(DBG_DETAILED, GT_NAME, "__threadpool_cancel_task: waite finished:%s.\n", osa_status_get_description(status));
        }

        /*
         *  Note: TODO.
         */
        if (ptsk->m_tsk_ops.m_exit == NULL || OSA_ETIMEOUT == status) {
            status = dlist_search_element(&pthdp->m_runn_list, (void *)token,
                                          (dlist_element_t **)&ptsk, __threadpool_find_match_fxn);
            if (!OSA_ISERROR(status) && ptsk != NULL) {
                DBG(DBG_DETAILED, GT_NAME, "__threadpool_cancel_task: madatory.\n");
                __threadpool_task_exit_madatory(ptsk);

                status             = OSA_SOK;
                pthdp->m_task_exit = TRUE;
                while (pthdp->m_task_exit && !OSA_ISERROR(status)) {
                    status = osa_cond_timedwait(pthdp->m_exit_cond,
                                                pthdp->m_mutex,
                                                THREADPOOL_CANCEL_SHRESHOLD/*pthdp->m_max_linger*/);
                }
                //DBG(DBG_DETAILED, GT_NAME, "__threadpool_cancel_task: madatory:%s.\n", osa_status_get_description(status));
            }
        }
    }
    pthdp->m_task_exit = FALSE;

    /*
     *  Step3: Look up the task in the fini list. If we find it, we just remove
     *         it from the finished list.
     */
    status = dlist_search_element(&pthdp->m_fini_list, (void *)token,
                                 (dlist_element_t **)&ptsk, __threadpool_find_match_fxn);
    if (!OSA_ISERROR(status) && ptsk != NULL) {
        status = dlist_remove_element(&pthdp->m_fini_list, (dlist_element_t *)ptsk);
        OSA_assert(OSA_SOK == status);
        be_canceled = TRUE;

        goto return_from_search;
    }

    /* Release threadpool mutex */
    osa_mutex_unlock(pthdp->m_mutex);

    return status;

return_from_search:

    /* Release threadpool mutex */
    osa_mutex_unlock(pthdp->m_mutex);

    if (be_canceled) {
        __threadpool_free_task_cell(pthdp, ptsk);

        status = OSA_SOK;
    }

    return status;
}

#if defined(__cplusplus)
}
#endif
