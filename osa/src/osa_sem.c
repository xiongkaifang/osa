/** ============================================================================
 *
 *  Copyright (C), 1987 - 2015, xiong-kaifang Tech. Co.,, Ltd.
 *
 *  @File name:	osa_sem.c
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2013-04-04
 *
 *  @Description:   The osa semaphore.
 *
 *
 *  @Version:       v1.0
 *
 *  @Function List:  //	主要函数及功能
 *      1.  －－－－－
 *      2.  －－－－－
 *
 *  @History:        //	历史修改记录
 *
 *  <author>        <time>       <version>      <description>
 *
 *  xiong-kaifang   2013-04-04     v1.0	        Write this module.
 *
 *
 *  ============================================================================
 */

/*  --------------------- Include system headers ---------------------------- */

/*  --------------------- Include user headers   ---------------------------- */
#include "osa.h"
#include "osa_mem.h"
#include "osa_sem.h"
#include "osa_mutex.h"

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
#define osa_sem_is_exit(sem)    ((sem)->m_state == OSA_SEM_STATE_EXIT)

#define osa_sem_check_arguments(arg)            osa_check_arguments(arg)
#define osa_sem_check_arguments2(arg1, arg2)    osa_check_arguments2(arg1, arg2)

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
enum __osa_sem_state_t; typedef enum __osa_sem_state_t osa_sem_state_t;
enum __osa_sem_state_t
{
    OSA_SEM_STATE_INIT = 0,
    OSA_SEM_STATE_EXIT = 1,
};

struct __osa_sem_t
{
	unsigned int	m_count;
	unsigned int	m_max_count;
	osa_mutex_t	    m_mutex;
	osa_cond_t	    m_cond;
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
status_t osa_sem_create(osa_sem_t *psem, unsigned int max_cnt, unsigned int init_value)
{
    status_t          status = OSA_SOK;
    struct __osa_sem_t * ps  = NULL;

    osa_sem_check_arguments(psem);

    (*psem) = INVALID_HANDLE;

    status = OSA_memAlloc(sizeof(struct __osa_sem_t), &ps);
    if (OSA_ISERROR(status) || ps == NULL) {
        return status;
    }

    status = osa_mutex_create(&ps->m_mutex);
    if (OSA_ISERROR(status)) {
        status |= OSA_memFree(sizeof(struct __osa_sem_t), ps);
        return status;
    }
    status = osa_cond_create(&ps->m_cond);
    if (OSA_ISERROR(status)) {
        status |= osa_mutex_delete(&ps->m_mutex);
        status |= OSA_memFree(sizeof(struct __osa_sem_t), ps);
        return status;
    }

    ps->m_count     = init_value;
    ps->m_max_count = max_cnt;

    if (ps->m_max_count == 0) {
        ps->m_max_count = 1;
    }

    if (ps->m_count > ps->m_max_count) {
        ps->m_count = ps->m_max_count;
    }

    ps->m_state = OSA_SEM_STATE_INIT;

    (*psem) = (HANDLE)ps;

    return status;
}

status_t osa_sem_wait  (osa_sem_t sem, unsigned int timeout)
{
    status_t           status = OSA_EFAIL;
    struct __osa_sem_t * psem = (struct __osa_sem_t *)sem;

    osa_sem_check_arguments(psem);

    osa_mutex_lock(psem->m_mutex);

    while (!osa_sem_is_exit(psem)) {
        if (psem->m_count > 0) {
            psem->m_count--;
            status = OSA_SOK;
            break;
        } else {
            if (OSA_TIMEOUT_NONE == timeout) {
                status = OSA_ETIMEOUT;
            } else if (OSA_TIMEOUT_FOREVER == timeout) {
                status = osa_cond_wait(psem->m_cond, psem->m_mutex);

            } else {
                status = osa_cond_timedwait(psem->m_cond, psem->m_mutex, timeout);
            }

            if (OSA_ISERROR(status)) {
                break;
            }
        }
    }

    osa_mutex_unlock(psem->m_mutex);

    return status;
}

status_t osa_sem_signal(osa_sem_t sem)
{
    status_t           status = OSA_SOK;
    struct __osa_sem_t * psem = (struct __osa_sem_t *)sem;

    osa_sem_check_arguments(psem);

    osa_mutex_lock(psem->m_mutex);

    if (psem->m_count < psem->m_max_count) {
        psem->m_count++;
        status |= osa_cond_signal(psem->m_cond);
    }

    osa_mutex_unlock(psem->m_mutex);

    return status;
}

status_t osa_sem_exit  (osa_sem_t sem)
{
    status_t           status = OSA_SOK;
    struct __osa_sem_t * psem = (struct __osa_sem_t *)sem;

    osa_sem_check_arguments(psem);

    osa_mutex_lock(psem->m_mutex);

    psem->m_state = OSA_SEM_STATE_EXIT;

    osa_cond_broadcast(psem->m_cond);

    osa_mutex_unlock(psem->m_mutex);

    return status;
}

status_t osa_sem_delete(osa_sem_t *psem)
{
    status_t         status = OSA_SOK;
    struct __osa_sem_t * ps = (struct __osa_sem_t *)(*psem);

    osa_sem_check_arguments2(psem, ps);

    status |= osa_cond_delete(&ps->m_cond);
    status |= osa_mutex_delete(&ps->m_mutex);
    status |= OSA_memFree(sizeof(struct __osa_sem_t), ps);

    (*psem) = INVALID_HANDLE;

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
