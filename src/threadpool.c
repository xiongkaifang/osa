/*
 *  threadpool.c
 *
 *  Created on: Sep 12, 2012
 *  Author    : xkf
 *
 */

/*  --------------------- Include system headers ---------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

/*  --------------------- Include user headers   ---------------------------- */
#include "osa.h"
#include "threadpool.h"
#include "osa_mem.h"
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
#define THREADPOOL_GET_THDPOOL_HANDLE(thdp) ((threadpool_handle)thdp)

#define THREADPOOL_THREAD_MIN               (2)
#define THREADPOOL_THREAD_MAX               (10)
#define THREADPOOL_TASK_MAX                 (32)

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
struct __thdpool_task_t;
typedef struct __thdpool_task_t thdpool_task_t;
struct __thdpool_task_t
{
    unsigned int            m_reserved[2];
    threadpool_task_state_t m_tsk_state;
    task_common_operation_t m_tsk_ops;
    void                  * m_userdata;
};

struct __threadpool_t
{
    unsigned int            m_reserved[2];

    pthread_mutex_t         m_mutex;

    pthread_cond_t          m_work_cond;
    pthread_cond_t          m_wait_cond;
    pthread_cond_t          m_exit_cond;

    pthread_mutex_t         m_tsk_mutex;
    thdpool_task_t          m_tsklists[THREADPOOL_TASK_MAX];

    dlist_t                 m_busy_list;
    dlist_t                 m_free_list;
    dlist_t                 m_task_list;
    dlist_t                 m_cell_list;

    unsigned int            m_min_thd_nums;
    unsigned int            m_max_thd_nums;
    unsigned int            m_cur_thd_nums;
    unsigned int            m_idl_thd_nums;

    threadpool_state_t      m_state;
};

typedef struct __threadpool_t * threadpool_handle;

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
    .m_max_tsk_nums = THREADPOOL_TASK_MAX
};

static unsigned int cur_cnt_index = 0;

static const char * const GT_NAME = "thdpool";
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
static status_t task_common_main(const task_common_operation_t *ops, void *arg);

static status_t
__threadpool_init(threadpool_handle hdl, const threadpool_params_t *params);

static status_t
__threadpool_exit(threadpool_handle hdl);

static void __threadpool_run_stub(void);

static int
__threadpool_task_dispatch(threadpool_handle hdl, thdpool_task_t **tsk, int timeout);

static void __threadpool_cleanup_task(thdpool_task_t *tsk);

static void __threadpool_notify_waiters(threadpool_handle hdl);

static void __threadpool_cleanup_worker(threadpool_handle hdl);

static status_t
__threadpool_alloc_task_cell(threadpool_handle hdl, thdpool_task_t **tsk);

static status_t
__threadpool_free_task_cell(threadpool_handle hdl, thdpool_task_t *tsk);

static status_t
__threadpool_instruments(threadpool_handle hdl, FILE *out);

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
status_t threadpool_create(threadpool_t *thdp, const threadpool_params_t *prm)
{
    status_t status = OSA_SOK;
    threadpool_handle thdp_hdl = NULL;

    DBG(DBG_DETAILED, GT_NAME, "threadpool_create: Enter(thdp=0x%x, prm=0x%x)\n", thdp, prm);

    if (prm == NULL) {
        prm = &default_thdp_params;
    }

    status = OSA_memAlloc(sizeof(*thdp_hdl), (void **)&thdp_hdl);
    if (!OSA_ISERROR(status) && thdp_hdl != NULL) {
        status = __threadpool_init(thdp_hdl, prm);

        if (OSA_ISERROR(status)) {
            OSA_memFree(sizeof(*thdp_hdl), (void *)thdp_hdl);
            thdp_hdl = NULL;
        }
    }

    (*thdp) = (threadpool_t)thdp_hdl;

    DBG(DBG_DETAILED, GT_NAME, "threadpool_create: Exit(status=0x%x)\n", status);

    return status;
}

status_t threadpool_add_task(threadpool_t thdp,
        const task_operation_t *tsk_ops, task_token_t *token)
{
    int             status;
    thread_attrs_t  thd_attrs;
    thread_handle   thd_hdl   = NULL;
    task_token_t    tsk_token = NULL;
    thdpool_task_t *tsk_p     = NULL;
    threadpool_handle hdl = NULL;

    hdl = THREADPOOL_GET_THDPOOL_HANDLE(thdp);

    DBG(DBG_DETAILED, GT_NAME, "threadpool_add_task: Enter (hdl=0x%x, "
        "task_ops=0x%x)\n", hdl, tsk_ops);

    if (hdl == NULL || tsk_ops == NULL || token == NULL) {
        DBG(DBG_ERROR, GT_NAME, "threadpool_add_task: Invalid arguments.\n");
        return OSA_EARGS;
    }

    (*token) = NULL;

    status = __threadpool_alloc_task_cell(hdl, &tsk_p);
    if (OSA_ISERROR(status)) {
        DBG(DBG_ERROR, GT_NAME, "threadpool_add_task: No unused task cell.\n");
        return status;
    }

    tsk_p->m_tsk_state = THREADPOOL_TASK_PENDING;
    tsk_p->m_userdata  = NULL;
    memcpy(&tsk_p->m_tsk_ops, tsk_ops, sizeof(task_operation_t));

    /* Get threadpool mutex */
    pthread_mutex_lock(&hdl->m_mutex);

    status = dlist_initialize_element((dlist_element_t *)tsk_p);
    status = dlist_put_tail(&hdl->m_task_list, (dlist_element_t *)tsk_p);

    if (hdl->m_idl_thd_nums > 0) {
        DBG(DBG_DETAILED, GT_NAME, "threadpool_add_task: Using idle thread to process this task.\n", tsk_p);
        pthread_cond_signal(&hdl->m_work_cond);
    } else if (hdl->m_cur_thd_nums < hdl->m_max_thd_nums) {
        thd_attrs = default_thd_attrs;
        thd_attrs.environ = (void *)hdl;

        DBG(DBG_DETAILED, GT_NAME, "threadpool_add_task: Create a new thread to process this task.\n");
        thd_hdl = thread_create((Fxn)__threadpool_run_stub, &thd_attrs);
        if (thd_hdl != NULL) {
            status = dlist_initialize_element((dlist_element_t *)thd_hdl);
            status = dlist_put_tail(&hdl->m_free_list, (dlist_element_t *)thd_hdl);

            hdl->m_cur_thd_nums++;
        }
    }

    /* Release threadpool mutex */
    pthread_mutex_unlock(&hdl->m_mutex);

    (*token) = tsk_p;

    DBG(DBG_DETAILED, GT_NAME, "threadpool_add_task: Exit (token=0x%x)\n", tsk_p);

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

status_t threadpool_wait(threadpool_t thdp)
{
    status_t status = OSA_SOK;
    threadpool_handle hdl = THREADPOOL_GET_THDPOOL_HANDLE(thdp);

    DBG(DBG_DETAILED, GT_NAME, "threadpool_wait: Enter (hdl=0x%x)\n", hdl);

    if (hdl == NULL) {
        DBG(DBG_ERROR, GT_NAME, "threadpool_wait: Invalid arguments.\n");
        return -EINVAL;
    }

    /* Get threadpool mutex */
    pthread_mutex_lock(&hdl->m_mutex);

    while (!dlist_is_empty(&hdl->m_task_list) 
            || !dlist_is_empty(&hdl->m_busy_list)) {
        hdl->m_state |= THREADPOOL_STATE_WAIT;
        pthread_cond_wait(&hdl->m_wait_cond, &hdl->m_mutex);
    }

    /* Release threadpool mutex */
    pthread_mutex_unlock(&hdl->m_mutex);

    DBG(DBG_DETAILED, GT_NAME, "threadpool_wait: Exit (status=0x%x)\n", status);

    return status;
}

status_t threadpool_delete(threadpool_t *thdp)
{
    status_t status = OSA_SOK;
    threadpool_handle hdl = THREADPOOL_GET_THDPOOL_HANDLE(*thdp);

    DBG(DBG_DETAILED, GT_NAME, "threadpool_delete: Enter (hdl=0x%x)\n", hdl);

    if (hdl == NULL) {
        DBG(DBG_ERROR, GT_NAME, "threadpool_delete: Invalid arguments.\n");
        return OSA_EARGS;
    }

    status = __threadpool_exit(hdl);

    /*
     *  Free the threadpool object.
     */
    OSA_memFree(sizeof(*hdl), (void *)hdl);

    (*thdp) = (threadpool_t)NULL;

    DBG(DBG_DETAILED, GT_NAME, "threadpool_delete: Exit(status=0x%x)\n", status);

    return status;
}

status_t threadpool_instruments(threadpool_t thdp)
{
    status_t status = OSA_SOK;
    threadpool_handle hdl = THREADPOOL_GET_THDPOOL_HANDLE(thdp);

    DBG(DBG_DETAILED, GT_NAME, "threadpool_instruments: Enter (hdl=0x%x)\n", hdl);

    if (hdl == NULL) {
        DBG(DBG_ERROR, GT_NAME, "threadpool_instruments: Invalid arguments.\n");
        return -EINVAL;
    }

    /* Get threadpool mutex */
    pthread_mutex_lock(&hdl->m_mutex);

    status = __threadpool_instruments(hdl, stdout); 

    /* Release threadpool mutex */
    pthread_mutex_unlock(&hdl->m_mutex);

    DBG(DBG_DETAILED, GT_NAME, "threadpool_instruments: Exit (status=0x%x)\n", status);

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
task_common_main(const task_common_operation_t *ops, void *arg)
{
    status_t status = OSA_SOK;

    OSA_assert(ops != NULL && ops->m_tsk_ops.m_main != NULL);

    status |= ops->m_tsk_ops.m_main(
                ops->m_tsk_ops.m_args[0], ops->m_tsk_ops.m_args[1],
                ops->m_tsk_ops.m_args[2], ops->m_tsk_ops.m_args[3],
                ops->m_tsk_ops.m_args[4], ops->m_tsk_ops.m_args[5],
                ops->m_tsk_ops.m_args[6], ops->m_tsk_ops.m_args[7]);

    return (status);
}

static status_t
__threadpool_init(threadpool_handle hdl, const threadpool_params_t *params)
{
    int i;
    status_t status = OSA_SOK;

    DBG(DBG_DETAILED, GT_NAME, "__threadpool_init: Enter (hdl=0x%x, params=0x%x)\n",
            hdl, params);

    /* Initialize dlist object */
    status = dlist_init(&hdl->m_busy_list);
    if (FAILED(status)) {
        goto exit_from_error;
    }
    status = dlist_init(&hdl->m_free_list);
    if (FAILED(status)) {
        goto exit_from_error;
    }
    status = dlist_init(&hdl->m_task_list);
    if (FAILED(status)) {
        goto exit_from_error;
    }
    status = dlist_init(&hdl->m_cell_list);
    if (FAILED(status)) {
        goto exit_from_error;
    }

    /* Initialize thread module */
    thread_init();

    pthread_mutex_init(&hdl->m_mutex, NULL);

    pthread_cond_init(&hdl->m_work_cond, NULL);
    pthread_cond_init(&hdl->m_wait_cond, NULL);
    pthread_cond_init(&hdl->m_exit_cond, NULL);

    pthread_mutex_init(&hdl->m_tsk_mutex, NULL);

    hdl->m_min_thd_nums = params->m_min_thd_nums;
    hdl->m_max_thd_nums = params->m_max_thd_nums;

    hdl->m_cur_thd_nums = 0;
    hdl->m_idl_thd_nums = 0;

    hdl->m_state        = 0;

    for (i = 0; i < OSA_ARRAYSIZE(hdl->m_tsklists); i++) {
        status |= dlist_initialize_element((dlist_element_t *)&hdl->m_tsklists[i]);
        status |= dlist_put_tail(&hdl->m_cell_list, (dlist_element_t *)&hdl->m_tsklists[i]);
        OSA_assert(OSA_SOK == status);
    }

    DBG(DBG_DETAILED, GT_NAME, "__threadpool_init: Exit\n");

    return status;

exit_from_error:

    /* Finalize thread module */
    thread_exit();

    return status;
}

static int  __threadpool_exit(threadpool_handle hdl)
{
    int           status = 0;
    thread_handle cur_thd_hdl = NULL;
    thread_handle nex_thd_hdl = NULL;

    DBG(DBG_DETAILED, GT_NAME, "__threadpool_exit: Enter (hdl=0x%x)\n", hdl);

    usleep(10000);

    /* Get threadpool mutex */
    pthread_mutex_lock(&hdl->m_mutex);

    /* Wakeup idle workers */
    hdl->m_state |= THREADPOOL_STATE_DESTROY;
    pthread_cond_broadcast(&hdl->m_work_cond);

    /* Cancel all active work thread */
    DBG(DBG_DETAILED, GT_NAME, "__threadpool_exit: Cancel all working thread.\n");
    status = dlist_first(&hdl->m_busy_list, (dlist_element_t **)&cur_thd_hdl);
    while ((cur_thd_hdl != NULL) && SUCCEEDED(status)) {
        status = thread_cancel(cur_thd_hdl);

        status = dlist_next(&hdl->m_busy_list,
                            (dlist_element_t *) cur_thd_hdl,
                            (dlist_element_t **)&nex_thd_hdl
                            );
        if (SUCCEEDED(status)) {
            cur_thd_hdl = nex_thd_hdl;
        } else {
            /* TODO Error process */
            break;
        }
    }

    /* Wait all active task workers to finish */
    while (!dlist_is_empty(&hdl->m_busy_list)) {
        hdl->m_state |= THREADPOOL_STATE_WAIT;
        DBG(DBG_DETAILED, GT_NAME, "__threadpool_exit: Wait for all thread to finish.\n");
        pthread_cond_wait(&hdl->m_wait_cond, &hdl->m_mutex);
    }

    /* Wait all task workers to terminate */
    while (hdl->m_cur_thd_nums != 0) {
        DBG(DBG_DETAILED, GT_NAME, "__threadpool_exit: Wait for all thread to terminate.\n");
        pthread_cond_wait(&hdl->m_exit_cond, &hdl->m_mutex);
    }

    /* Release threadpool mutex */
    pthread_mutex_unlock(&hdl->m_mutex);

    /* Delete all task workers */
    DBG(DBG_DETAILED, GT_NAME, "__threadpool_exit: Delete all thread.\n");
    status = 0;
    while (!dlist_is_empty(&hdl->m_free_list) && SUCCEEDED(status)) {
        status = 
            dlist_get_head(&hdl->m_free_list, (dlist_element_t **)&cur_thd_hdl);
        if (SUCCEEDED(status)) {
            thread_delete(cur_thd_hdl);
        }
    }

    /* Finalize thread module */
    thread_exit();

    /* Destroy threadpool object */
    pthread_mutex_destroy(&hdl->m_mutex);

    pthread_cond_destroy(&hdl->m_work_cond);
    pthread_cond_destroy(&hdl->m_wait_cond);
    pthread_cond_destroy(&hdl->m_exit_cond);

    pthread_mutex_destroy(&hdl->m_tsk_mutex);

    DBG(DBG_DETAILED, GT_NAME, "__threadpool_exit: Exit\n");

    return status;
}

static void __threadpool_run_stub(void)
{
    int                 status;
    bool                tsk_exit;
    int                 timeout  = -1;
    thread_handle       thd_hdl  = NULL;
    threadpool_handle   thdp_hdl = NULL;
    thdpool_task_t    * tsk_p    = NULL;

    DBG(DBG_DETAILED, GT_NAME, "__threadpool_run_stub: Enter\n");

    thd_hdl  = thread_self();
    thdp_hdl = thread_get_env(thd_hdl);
    //DBG(DBG_INFO, "__threadpool_run_stub: thd_hdl=0x%x, thdp_hdl=0x%x\n", thd_hdl, thdp_hdl);

    /* Get threadpool mutex */
    pthread_mutex_lock(&thdp_hdl->m_mutex);

    /* Register worker cleanup routine */
    pthread_cleanup_push(
            (void (*)(void *))__threadpool_cleanup_worker, (void *)thdp_hdl);

    for ( ; ; ) {

        thdp_hdl->m_idl_thd_nums++;

        if (thdp_hdl->m_state & THREADPOOL_STATE_WAIT) {
            __threadpool_notify_waiters(thdp_hdl);
        }

        /* Wait for task */
        status = __threadpool_task_dispatch(thdp_hdl, &tsk_p, timeout);

        thdp_hdl->m_idl_thd_nums--;

        if (thdp_hdl->m_state & THREADPOOL_STATE_DESTROY) {
            break;
        }

        /* Release threadpool mutex */
        pthread_mutex_unlock(&thdp_hdl->m_mutex);

        /* Register thread cleanup routine */
        pthread_cleanup_push(
                (void (*)(void *))__threadpool_cleanup_task, (void *)tsk_p);

        /* Process the task */
        status = task_common_main(&tsk_p->m_tsk_ops, tsk_p);

        /* Clean up task: Call ====> __threadpool_cleanup_task(thdp_hdl) */
        pthread_cleanup_pop(1);

        tsk_p = NULL;

#if 0
        /*
         *  TODO: Bug here. We need get threadpool mutex again, rigth???.
         */
        pthread_mutex_lock(&thdp_hdl->mutex);
#endif

    }

    /* Clean up worker: Call ====> __threadpool_cleanup_worker(thdp_hdl) */
    pthread_cleanup_pop(1);

    DBG(DBG_DETAILED, GT_NAME, "__threadpool_run_stub: Exit\n");
}

static int
__threadpool_task_dispatch(threadpool_handle hdl, 
        thdpool_task_t **tsk, int timeout)
{
    int             status  = 0;
    thread_handle   thd_hdl = NULL;

    DBG(DBG_DETAILED, GT_NAME, "__threadpool_task_dispatch: Enter (hdl=0x%x, "
        "tsk=0x%x, timeout=0x%x)\n", hdl, tsk, timeout);

    thd_hdl = thread_self();

    //pthread_mutex_lock(&hdl->mutex);

    /* Check THREADPOOL_WAIT flag */

    while (dlist_is_empty(&hdl->m_task_list)
            && !(hdl->m_state & THREADPOOL_STATE_DESTROY)) {
        DBG(DBG_DETAILED, GT_NAME, "__threadpool_task_dispatch: thread[0x%x] wait for "
                "task.\n", thd_hdl);

        pthread_cond_wait(&hdl->m_work_cond, &hdl->m_mutex);
    }

    (*tsk) = NULL;

    /* Check THREADPOOL_WAIT flag */
    if (hdl->m_state & THREADPOOL_STATE_DESTROY) {
        return status;
    }

    /* Move cur thread from free list to busy list */
    status = dlist_remove_element(&hdl->m_free_list, (dlist_element_t *)thd_hdl);
    status = dlist_put_tail(&hdl->m_busy_list, (dlist_element_t *)thd_hdl);

    /* Fetch the task from task data list */
    status = dlist_get_head(&hdl->m_task_list, (dlist_element_t **)tsk);

    //pthread_mutex_unlock(&hdl->mutex);

    DBG(DBG_DETAILED, GT_NAME, "__threadpool_task_dispatch: Exit (status=0x%x)\n", status);

    return status;
}

static void __threadpool_cleanup_task(thdpool_task_t * tsk)
{
    int                 status   = 0;
    unsigned short      bexit    = 0;
    thread_handle       thd_hdl  = NULL;
    threadpool_handle   thdp_hdl = NULL;

    DBG(DBG_DETAILED, GT_NAME, "__threadpool_cleanup_task: Enter (tsk=0x%x)\n", tsk);

    thd_hdl  = thread_self();
    thdp_hdl = thread_get_env(thd_hdl);

    /* Get threadpool mutex */
    pthread_mutex_lock(&thdp_hdl->m_mutex);

    /* Move cur thread from busy list to free list */
    status = dlist_remove_element(&thdp_hdl->m_busy_list, (dlist_element_t *)thd_hdl);
    status = dlist_put_tail(&thdp_hdl->m_free_list, (dlist_element_t *)thd_hdl);

    /* Free task cell */
    __threadpool_free_task_cell(thdp_hdl, tsk);

    /* Check THREADPOOL_WAIT flag */
    bexit  = thdp_hdl->m_state & THREADPOOL_STATE_WAIT;
    if (bexit) {

        /* Notity other task to exit */
        __threadpool_notify_waiters(thdp_hdl);
    }

    /* Release threadpool mutex */
    //pthread_mutex_unlock(&thdp_hdl->mutex);

    DBG(DBG_DETAILED, GT_NAME, "__threadpool_cleanup_task: Exit\n");
}

static void __threadpool_notify_waiters(threadpool_handle hdl)
{
    DBG(DBG_DETAILED, GT_NAME, "__threadpool_notify_waiters: Enter (hdl=0x%x)\n", hdl);

    if (dlist_is_empty(&hdl->m_task_list) && dlist_is_empty(&hdl->m_busy_list)) {
        hdl->m_state &= ~THREADPOOL_STATE_WAIT;
        pthread_cond_broadcast(&hdl->m_wait_cond);
    }

    DBG(DBG_DETAILED, GT_NAME, "__threadpool_notify_waiters: Exit\n");
}

static void __threadpool_cleanup_worker(threadpool_handle hdl)
{
    int             status;
    thread_handle   thd_hdl = NULL;
    thread_attrs_t  thd_attrs;

    DBG(DBG_DETAILED, GT_NAME, "__threadpool_cleanup_worker: Enter (hdl=0x%x)\n", hdl);

    hdl->m_cur_thd_nums--;

    DBG(DBG_DETAILED, GT_NAME, "__threadpool_cleanup_worker: Remaining %d thread left.\n", hdl->m_cur_thd_nums);

    if (hdl->m_state & THREADPOOL_STATE_DESTROY) {
        if (hdl->m_cur_thd_nums == 0) {
            DBG(DBG_DETAILED, GT_NAME, "__threadpool_cleanup_worker: The last thread left.\n");
            pthread_cond_broadcast(&hdl->m_exit_cond);
        }
    }
    else if (!dlist_is_empty(&hdl->m_task_list)
            && (hdl->m_cur_thd_nums < hdl->m_max_thd_nums)) {
        /* TODO: BUG here, current thread need exit, do we need create new one??? */
        DBG(DBG_DETAILED, GT_NAME, "__threadpool_cleanup_worker: Current thread eixt, create a new thread to process task.\n");
        thd_attrs         = default_thd_attrs;
        thd_attrs.environ = (void *)hdl;
        thd_hdl = thread_create((Fxn)__threadpool_run_stub, &thd_attrs);
        if (thd_hdl != NULL) {
            hdl->m_cur_thd_nums++;
        }
    }

    /* TODO: Delete current thread object */
    //thread_delete(thread_self());

    /* Release threadpool mutex */
    pthread_mutex_unlock(&hdl->m_mutex);

    DBG(DBG_DETAILED, GT_NAME, "__threadpool_cleanup_worker: Exit\n");
}

static status_t
__threadpool_alloc_task_cell(threadpool_handle hdl, thdpool_task_t **tsk)
{
    status_t status = OSA_ENOENT;

    DBG(DBG_DETAILED, GT_NAME, "__threadpool_alloc_task_cell: Enter (hdl=0x%x)\n",
            hdl);

    (*tsk) = NULL;

    /* Get threadpool task mutex */
    pthread_mutex_lock(&hdl->m_tsk_mutex);

    if (!dlist_is_empty(&hdl->m_cell_list)) {
        status = dlist_get_head(&hdl->m_cell_list, (dlist_element_t **)tsk);
        OSA_assert(OSA_SOK == status);
        status = OSA_SOK;
    }

    /* Release threadpool task mutex */
    pthread_mutex_unlock(&hdl->m_tsk_mutex);

    DBG(DBG_DETAILED, GT_NAME, "__threadpool_alloc_task_cell: Exit "
            "(status=0x%x)\n", status);

    return status;
}

static status_t
__threadpool_free_task_cell(threadpool_handle hdl, thdpool_task_t *tsk)
{
    status_t status = OSA_SOK;

    DBG(DBG_DETAILED, GT_NAME, "__threadpool_free_task_cell: Enter (hdl=0x%x, "
            "tsk=0x%x)\n", hdl, tsk);

    /* Get threadpool task mutex */
    pthread_mutex_lock(&hdl->m_tsk_mutex);

    status |= dlist_put_tail(&hdl->m_cell_list, (dlist_element_t *)tsk);

    /* Release threadpool task mutex */
    pthread_mutex_unlock(&hdl->m_tsk_mutex);

    DBG(DBG_DETAILED, GT_NAME, "__threadpool_free_task_cell: Exit\n");

    return status;
}

static status_t
__threadpool_instruments_apply_fxn(dlist_element_t *elem, void *data)
{
    status_t        status  = OSA_SOK;
    thread_handle   thd_hdl = NULL;

    thd_hdl = (thread_handle)elem;
    OSA_assert(thd_hdl != NULL);

    fprintf(stdout, "    [%02d]    | [0x%08x] |    [%s]    | [%s]\n",
            cur_cnt_index++, thd_hdl, (const char *)data, "Not implementation");

    return status;
}

static status_t
__threadpool_instruments(threadpool_handle hdl, FILE *out)
{
    status_t status = OSA_SOK;

    fprintf(out, "\nTHDPOOL: Statistics.\n"
                 "\nTotal Count |  Busy Count  |  Idle Count  |  Max Count  |  Min Count"
                 "\n--------------------------------------------------------------------\n"
           );
    fprintf(out, "    [%02d]    |     [%02d]     |     [%02d]     |     [%02d]    |     [%02d]\n",
                  hdl->m_cur_thd_nums, hdl->m_cur_thd_nums - hdl->m_idl_thd_nums,
                  hdl->m_idl_thd_nums, hdl->m_max_thd_nums, hdl->m_min_thd_nums);

    fprintf(out, "\n     ID     |    THREAD    |     STATE    |    NAME"
                 "\n--------------------------------------------------------------------\n");

    status |= dlist_map(&hdl->m_busy_list, __threadpool_instruments_apply_fxn, (void *)"Busy");
    status |= dlist_map(&hdl->m_free_list, __threadpool_instruments_apply_fxn, (void *)"Idle");

    /* Reset cur_cnt_index value */
    cur_cnt_index = 0;

    return status;
}

#if defined(__cplusplus)
}
#endif
