/** ============================================================================
 *
 *  Copyright (C), 1987 - 2015, xiong-kaifang Tech. Co.,, Ltd.
 *
 *  @File name:	osa_queue2.c
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2014-04-25
 *
 *  @Description:   The osa queue2(using double link list).
 *
 *
 *  @Version:       v1.0
 *
 *  @Function List: // 主要函数及功能
 *      1.  －－－－－
 *      2.  －－－－－
 *
 *  @History:       //	历史修改记录
 *
 *  <author>        <time>       <version>      <description>
 *
 *  xiong-kaifang   2014-04-25     v1.0	        Write this module.
 *
 *  xiong-kaifang   2015-09-19     v1.1         1. Dynamically create and delete
 *                                                 queue2 object(queue2_t).
 *                                              2. Add codes to check arguments.
 *
 *  ============================================================================
 */

/*  --------------------- Include system headers ---------------------------- */

/*  --------------------- Include user headers   ---------------------------- */
#include "osa.h"
#include "osa_queue2.h"
#include "osa_mutex.h"
#include "osa_mem.h"
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
#define queue2_is_exit(queue)   ((queue)->m_state == QUEUE2_STATE_EXIT)

#define queue2_check_arguments(arg)         osa_check_arguments(arg)

#define queue2_check_arguments2(arg1, arg2) osa_check_arguments2(arg1, arg2)

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
struct __queue2_t
{
    volatile
    queue2_state_t  m_state;

    osa_mutex_t     m_mutex;
    osa_cond_t      m_rd_cond;
#if 0
    osa_cond_t      m_exit_cond;
#endif

    dlist_t         m_queue;
    int             m_exit;
    int             m_read;
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
status_t queue2_create(queue2_t *pque)
{
    status_t             status = OSA_SOK;
    struct __queue2_t  * pqueue = NULL;

    queue2_check_arguments(pque);

    (*pque) = INVALID_HANDLE;

    status = OSA_memAlloc(sizeof(struct __queue2_t), &pqueue);
    if (OSA_ISERROR(status) || pqueue == NULL) {
        return status;
    }

    status = osa_mutex_create(&pqueue->m_mutex);
    if (OSA_ISERROR(status)) {
        OSA_memFree(sizeof(struct __queue2_t), pqueue);
        return status;
    }

    status = osa_cond_create (&pqueue->m_rd_cond);
#if 0
    status = osa_cond_create (&pqueue->m_exit_cond);
#endif
    if (OSA_ISERROR(status)) {
        osa_mutex_delete(&pqueue->m_mutex);
        OSA_memFree(sizeof(struct __queue2_t), pqueue);
        return status;
    }

    status |= dlist_init(&pqueue->m_queue);

    pqueue->m_state = QUEUE2_STATE_INIT;
    pqueue->m_exit  = 0;
    pqueue->m_read  = 0;

    (*pque) = (queue2_t)pqueue;

    return status;
}

status_t queue2_put(queue2_t que, void *pvalue , unsigned int timeout)
{
    status_t            status = OSA_EFAIL;
    struct __queue2_t * pque   = (struct __queue2_t *)que;

    queue2_check_arguments(pque);

    osa_mutex_lock  (pque->m_mutex);

    if (!queue2_is_exit(pque)) {
        status  = OSA_SOK;
        status |= dlist_initialize_element((dlist_element_t *)pvalue);
        status |= dlist_put_tail(&pque->m_queue, (dlist_element_t *)pvalue);
        status |= osa_cond_signal(pque->m_rd_cond);
    }

    osa_mutex_unlock(pque->m_mutex);

    return status;
}

status_t queue2_get(queue2_t que, void **ppvalue, unsigned int timeout)
{
    status_t            status = OSA_EFAIL;
    struct __queue2_t * pque   = (struct __queue2_t *)que;

    queue2_check_arguments(pque);

    osa_mutex_lock(pque->m_mutex);

#if 0
    pque->m_read = 1;
#endif

    while (!queue2_is_exit(pque)) {

        if (!dlist_is_empty(&pque->m_queue)) {

            if (ppvalue != NULL) {
                status = dlist_get_head(&pque->m_queue, (dlist_element_t **)ppvalue);
            }

            status = OSA_SOK;
            break;
        } else {
            if (timeout == OSA_TIMEOUT_NONE) {
                break;
            }

            status = osa_cond_wait(pque->m_rd_cond, pque->m_mutex);
        }
    }

#if 0
    if (pque->m_exit) {
        pque->m_exit = 0;
        osa_cond_signal(pque->m_exit_cond);
    }

    pque->m_read = 0;
#endif
    osa_mutex_unlock(pque->m_mutex);

    return status;
}

status_t queue2_peek(queue2_t que, void **ppvalue)
{
    status_t            status = OSA_ENOENT;
    struct __queue2_t * pque   = (struct __queue2_t *)que;

    queue2_check_arguments(pque);

    osa_mutex_lock  (pque->m_mutex);

    if (!dlist_is_empty(&pque->m_queue)) {
        if (ppvalue != NULL) {
            status = dlist_first(&pque->m_queue, (dlist_element_t **)ppvalue);
        }
    }

    osa_mutex_unlock(pque->m_mutex);

    return status;
}

status_t queue2_count(queue2_t que, unsigned int *pcount)
{
    status_t            status = OSA_EFAIL;
    struct __queue2_t * pque   = (struct __queue2_t *)que;

    queue2_check_arguments2(pque, pcount);

    osa_mutex_lock  (pque->m_mutex);

    status = dlist_count(&pque->m_queue, pcount);

    osa_mutex_unlock(pque->m_mutex);

    return status;
}

bool_t   queue2_is_empty(queue2_t que)
{
    bool_t              is_empty;
    struct __queue2_t * pque = (struct __queue2_t *)que;

#if 0

    queue2_check_arguments(pque);

#else

    if (pque == NULL) {
        return FALSE;
    }

#endif

    osa_mutex_lock  (pque->m_mutex);

    if (dlist_is_empty(&pque->m_queue)) {
        is_empty = TRUE;
    } else {
        is_empty = FALSE;
    }

    osa_mutex_unlock(pque->m_mutex);

    return is_empty;
}

status_t queue2_exit(queue2_t que)
{
    return queue2_set_state(que, QUEUE2_STATE_EXIT);
}

status_t queue2_set_state(queue2_t que, queue2_state_t state)
{
    status_t            status = OSA_SOK;
    struct __queue2_t * pque   = (struct __queue2_t *)que;

    queue2_check_arguments(pque);

    osa_mutex_lock  (pque->m_mutex);

    pque->m_state = state;

    if (queue2_is_exit(pque)) {
        osa_cond_broadcast(pque->m_rd_cond);
    }

    osa_mutex_unlock(pque->m_mutex);

    return status;
}

status_t queue2_reset(queue2_t que)
{
    status_t            status = OSA_SOK;
    struct __queue2_t * pque   = (struct __queue2_t *)que;

    queue2_check_arguments(pque);

    osa_mutex_lock  (pque->m_mutex);

    status = dlist_init(&pque->m_queue);

    osa_mutex_unlock(pque->m_mutex);

    return status;
}

status_t queue2_delete(queue2_t *pque)
{
    status_t            status = OSA_SOK;
    struct __queue2_t * pqueue = (struct __queue2_t *)(*pque);

    queue2_check_arguments2(pque, pqueue);

    /*
     *  TODO:
     */
#if 0
    fprintf(stderr, "queue2 get mutex.\n");
    osa_mutex_lock  (pqueue->m_mutex);
    pqueue->m_state = QUEUE2_STATE_EXIT;
    pqueue->m_exit = 1;
    fprintf(stderr, "queue2 cond broadcast.\n");
    osa_cond_broadcast(pqueue->m_rd_cond);
    while (pqueue->m_exit && pqueue->m_read) {
        osa_cond_wait(pqueue->m_exit_cond, pqueue->m_mutex);
    }
    osa_mutex_unlock(pqueue->m_mutex);
    status |= osa_cond_delete (&pqueue->m_exit_cond);

#endif

    status |= osa_cond_delete (&pqueue->m_rd_cond);
    status |= osa_mutex_delete(&pqueue->m_mutex);
    status |= dlist_init(&pqueue->m_queue);

    OSA_memFree(sizeof(struct __queue2_t), pqueue);

    (*pque) = INVALID_HANDLE;

    return status;
}

/*
 *  --------------------- Local function definition ----------------------------
 */

/** ============================================================================
 *
 *  @Function:      Local function definition.
 *
 *  @Description:   //  函数功能、性能等的描述
 *
 *  ============================================================================
 */

#if defined(__cplusplus)
}
#endif
