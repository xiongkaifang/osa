/** ============================================================================
 *
 *  Copyright (C), 1987 - 2015, xiong-kaifang Tech. Co.,, Ltd.
 *
 *  @File name:	osa_queue.c
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2013-04-04
 *
 *  @Description:   The osa queue(using dynamic array).
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
 *  xiong-kaifang   2015-09-19     v1.1         1. Dynamically create and delete
 *                                                 queue object(queue_t).
 *                                              2. Add codes to check arguments.
 *
 *  xiong-kaifang   2016-10-07     v1.2         1. Tweak queue's prototype.
 *                                              2. Add callback to clean up
 *                                              queue.
 *
 *  ============================================================================
 */

/*  --------------------- Include system headers ---------------------------- */

/*  --------------------- Include user headers   ---------------------------- */
#include "osa.h"
#include "osa_queue.h"
#include "osa_mutex.h"
#include "osa_mem.h"

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
#define queue_is_exit(queue)    ((queue)->m_state == QUEUE_STATE_EXIT)

#define queue_check_arguments(arg)          osa_check_arguments(arg)

#define queue_check_arguments2(arg1, arg2)  osa_check_arguments2(arg1, arg2)
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
struct __queue_t
{
	unsigned int	m_rd_idx;
	unsigned int	m_wr_idx;
	unsigned int	m_len;
	unsigned int	m_count;
	void **         m_queue;

    osa_mutex_t     m_mutex;
	osa_cond_t	    m_rd_cond;
	osa_cond_t	    m_wr_cond;
    volatile
    unsigned int    m_state;
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
status_t queue_create  (queue_t * pque, unsigned int max_len)
{
    int                size;
    status_t           status = OSA_SOK;
    struct __queue_t * que    = NULL;

    queue_check_arguments(pque);

    (*pque) = INVALID_HANDLE;

    size = sizeof(struct __queue_t) + sizeof(void *) * max_len;
    status = OSA_memAlloc(size, &que);
    if (OSA_ISERROR(status) || que == NULL) {
        return status;
    }

    status = osa_mutex_create(&que->m_mutex);
    if (OSA_ISERROR(status)) {
        OSA_memFree(size, que);
        return status;
    }
    status = osa_cond_create (&que->m_rd_cond);
    if (OSA_ISERROR(status)) {
        osa_mutex_delete(&que->m_mutex);
        OSA_memFree(size, que);
        return status;
    }
    status = osa_cond_create (&que->m_wr_cond);
    if (OSA_ISERROR(status)) {
        osa_mutex_delete(&que->m_mutex);
        osa_cond_delete(&que->m_rd_cond);
        OSA_memFree(size, que);
        return status;
    }

    que->m_rd_idx = 0;
    que->m_wr_idx = 0;
    que->m_count  = 0;
    que->m_len    = max_len;
    que->m_queue  = (void **)(que + 1);

    que->m_state  = QUEUE_STATE_INIT;

    (*pque) = (queue_t)que;

    return status;
}

status_t queue_put     (queue_t   que , void * value, unsigned int timeout)
{
    status_t           status = OSA_EFAIL;
    struct __queue_t * pque   = (struct __queue_t *)que;

    queue_check_arguments(pque);

    osa_mutex_lock(pque->m_mutex);

    while (!queue_is_exit(pque)) {
        if (pque->m_count < pque->m_len) {
            pque->m_queue[pque->m_wr_idx] = value;
            pque->m_wr_idx = (pque->m_wr_idx + 1) % pque->m_len;
            pque->m_count++;
            status = OSA_SOK;
            osa_cond_signal(pque->m_rd_cond);
            break;
        } else {
            if (OSA_TIMEOUT_NONE == timeout) {
                break;
            } else if (OSA_TIMEOUT_FOREVER == timeout) {
                status = osa_cond_wait(pque->m_rd_cond, pque->m_mutex);
            } else {
                status = osa_cond_timedwait(pque->m_rd_cond, pque->m_mutex, timeout);
            }

            if (queue_is_exit(pque)) {
                status = OSA_EFAIL;
            }
            if (OSA_ISERROR(status)) {
                break;
            }
        }
    }

    osa_mutex_unlock(pque->m_mutex);

    return status;
}

status_t queue_get     (queue_t   que , void ** pvalue, unsigned int timeout)
{
    status_t           status = OSA_EFAIL;
    struct __queue_t * pque   = (struct __queue_t *)que;

    queue_check_arguments(pque);

    osa_mutex_lock(pque->m_mutex);

    while (!queue_is_exit(pque)) {
        if (pque->m_count > 0) {

            if (pvalue != NULL) {
                (*pvalue) = pque->m_queue[pque->m_rd_idx];
            }

            pque->m_rd_idx = (pque->m_rd_idx + 1) % pque->m_len;
            pque->m_count--;
            status = OSA_SOK;
            osa_cond_signal(pque->m_wr_cond);
            break;
        } else {
            if (OSA_TIMEOUT_NONE == timeout) {
                break;
            } else if (OSA_TIMEOUT_FOREVER == timeout) {
                status = osa_cond_wait(pque->m_rd_cond, pque->m_mutex);
            } else {
                status = osa_cond_timedwait(pque->m_rd_cond, pque->m_mutex, timeout);
            }

            if (queue_is_exit(pque)) {
                status = OSA_EFAIL;
            }
            if (OSA_ISERROR(status)) {
                break;
            }
        }
    }

    osa_mutex_unlock(pque->m_mutex);

    return status;
}

status_t queue_peek    (queue_t   que , void ** pvalue)
{
    status_t           status = OSA_SOK;
    struct __queue_t * pque   = (struct __queue_t *)que;

    queue_check_arguments(pque);

    osa_mutex_lock(pque->m_mutex);

    if (pque->m_count > 0) {
        if (pvalue != NULL) {
            (*pvalue) = pque->m_queue[pque->m_rd_idx];
        }
    }

    osa_mutex_unlock(pque->m_mutex);

    return status;
}

status_t queue_count   (queue_t   que , unsigned int * pcount)
{
    status_t           status = OSA_SOK;
    struct __queue_t * pque   = (struct __queue_t *)que;

    queue_check_arguments(pque);

    osa_mutex_lock(pque->m_mutex);

    if (pcount != NULL) {
        (*pcount) = pque->m_count;
    }

    osa_mutex_unlock(pque->m_mutex);

    return status;
}

bool_t   queue_is_empty(queue_t   que)
{
    bool_t             is_empty = FALSE;
    struct __queue_t * pque     = (struct __queue_t *)que;

    if (pque == NULL) {
        return is_empty;
    }

    osa_mutex_lock(pque->m_mutex);

    if (pque->m_count == 0) {
        is_empty = TRUE;
    } else {
        is_empty = FALSE;
    }

    osa_mutex_unlock(pque->m_mutex);

    return is_empty;
}

bool_t   queue_is_full (queue_t   que)
{
    bool_t             is_full = FALSE;
    struct __queue_t * pque    = (struct __queue_t *)que;

    if (pque == NULL) {
        return is_full;
    }

    osa_mutex_lock(pque->m_mutex);

    if (pque->m_count == pque->m_len) {
        is_full = TRUE;
    } else {
        is_full = FALSE;
    }

    osa_mutex_unlock(pque->m_mutex);

    return is_full;
}

status_t queue_exit    (queue_t   que , QUE_CLEANUP cleanup, void * userdata)
{
    void *             elem   = NULL;
    status_t           status = OSA_SOK;
    struct __queue_t * pque   = (struct __queue_t *)que;

    queue_check_arguments(pque);

    osa_mutex_lock(pque->m_mutex);

    pque->m_state = QUEUE_STATE_EXIT;

    /* Clean up all elements */
    if (cleanup) {
        while (pque->m_count) {
            elem = pque->m_queue[pque->m_rd_idx];
            pque->m_rd_idx = (pque->m_rd_idx + 1) % pque->m_len;
            pque->m_count--;

            (*cleanup)(elem, userdata);
        }
    }

    /* Notify others */
    osa_cond_broadcast(pque->m_wr_cond);
    osa_cond_broadcast(pque->m_rd_cond);

    osa_mutex_unlock(pque->m_mutex);

    return status;
}

status_t queue_delete  (queue_t * pque)
{
    int                size;
    status_t           status = OSA_SOK;
    struct __queue_t * que    = (struct __queue_t *)(*pque);

    queue_check_arguments2(pque, que);

    status |= osa_cond_delete (&que->m_wr_cond);
    status |= osa_cond_delete (&que->m_rd_cond);
    status |= osa_mutex_delete(&que->m_mutex);

    size = sizeof(struct __queue_t) + sizeof(void *) * que->m_len;
    status |= OSA_memFree(size, que);

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
 *  @Description:   // 函数功能、性能等的描述
 *
 *  ============================================================================
 */

#if defined(__cplusplus)
}
#endif
