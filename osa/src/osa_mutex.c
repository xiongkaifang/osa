/** ============================================================================
 *
 *  Copyright (C), 1987 - 2015, xiong-kaifang Tech. Co.,, Ltd.
 *
 *  @File name:	osa_mutex.c
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2013-04-04
 *
 *  @Description:   The osa mutex.
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
 *  xiong-kaifang   2015-09-19     v2.0         1. Implement osa_cond_t related
 *                                                 routines.
 *                                              2. Dynamically create and delete
 *                                                 osa_mutex_t and osa_cond_t.
 *                                              3. Add codes to check arguments.
 *
 *  ============================================================================
 */

/*  --------------------- Include system headers ---------------------------- */
#include <time.h>
#include <errno.h>
#include <pthread.h>

/*  --------------------- Include user headers   ---------------------------- */
#include "osa.h"
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
#define MILLIONS    (1000000)
#define BILLIONS    (1000000000)

#define osa_mutex_check_arguments(arg)          osa_check_arguments(arg)
#define osa_mutex_check_arguments2(arg1, arg2)  osa_check_arguments2(arg1, arg2)

#define osa_cond_check_arguments(arg)           osa_check_arguments(arg)
#define osa_cond_check_arguments2(arg1, arg2)   osa_check_arguments2(arg1, arg2)

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
struct __osa_mutex_t
{
    pthread_mutex_t m_mutex;
};

struct __osa_cond_t
{
    pthread_cond_t  m_cond;
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
 *  @Output:        //	对输出参数的说明
 *
 *  @Return:        //	函数返回值的说明
 *
 *  @Enter          //  Precondition
 *
 *  @Leave          //  Postcondition
 *
 *  @Others:        //	其它说明
 *
 *  ============================================================================
 */

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
status_t osa_mutex_create(osa_mutex_t *pmutex)
{
    status_t               status = OSA_SOK;
    pthread_mutexattr_t    mutex_attr;
    struct __osa_mutex_t * pmtx   = NULL;

    osa_mutex_check_arguments(pmutex);

    (*pmutex) = INVALID_HANDLE;

    status = OSA_memAlloc(sizeof(struct __osa_mutex_t), &pmtx);
    if (OSA_ISERROR(status) || pmtx == NULL) {
        return status;
    }

    status |= pthread_mutexattr_init(&mutex_attr);
    status |= pthread_mutex_init(&pmtx->m_mutex, &mutex_attr);

    pthread_mutexattr_destroy(&mutex_attr);

    (*pmutex) = (osa_mutex_t)pmtx;

    return status;
}

status_t osa_mutex_lock  (osa_mutex_t mutex)
{
    status_t               status = OSA_SOK;
    struct __osa_mutex_t * pmutex = (struct __osa_mutex_t *)mutex;

    osa_mutex_check_arguments(pmutex);

    pthread_mutex_lock(&pmutex->m_mutex);

    return status;
}

status_t osa_mutex_unlock(osa_mutex_t mutex)
{
    status_t               status = OSA_SOK;
    struct __osa_mutex_t * pmutex = (struct __osa_mutex_t *)mutex;

    osa_mutex_check_arguments(pmutex);

    pthread_mutex_unlock(&pmutex->m_mutex);

    return OSA_SOK;
}

status_t osa_mutex_delete(osa_mutex_t *pmutex)
{
    status_t               status = OSA_SOK;
    struct __osa_mutex_t * pmtx   = (struct __osa_mutex_t *)(*pmutex);

    osa_mutex_check_arguments2(pmutex, pmtx);

    pthread_mutex_destroy(&pmtx->m_mutex);

    status |= OSA_memFree(sizeof(struct __osa_mutex_t), pmtx);

    (*pmutex) = INVALID_HANDLE;

    return status;
}

/* osa cond */
status_t osa_cond_create(osa_cond_t *pcond)
{
    status_t              status = OSA_SOK;
    struct __osa_cond_t * pcnd   = NULL;

    osa_cond_check_arguments(pcond);

    (*pcond) = INVALID_HANDLE;

    status = OSA_memAlloc(sizeof(struct __osa_cond_t), &pcnd);
    if (OSA_ISERROR(status) || pcnd == NULL) {
        return status;
    }

    pthread_cond_init(&pcnd->m_cond, NULL);

    (*pcond) = (osa_cond_t)pcnd;

    return status;
}

status_t osa_cond_wait  (osa_cond_t cond, osa_mutex_t mutex)
{
    status_t               status = OSA_SOK;
    struct __osa_cond_t  * pcond  = (struct __osa_cond_t  *)cond;
    struct __osa_mutex_t * pmutex = (struct __osa_mutex_t *)mutex;

    osa_cond_check_arguments2(pcond, pmutex);

    pthread_cond_wait(&pcond->m_cond, &pmutex->m_mutex);

    return status;
}

status_t osa_cond_timedwait(osa_cond_t cond, osa_mutex_t mutex, unsigned int timeout)
{
    int                    retval;
    status_t               status = OSA_SOK;
    struct timespec        abstime;
    struct __osa_cond_t  * pcond  = (struct __osa_cond_t  *)cond;
    struct __osa_mutex_t * pmutex = (struct __osa_mutex_t *)mutex;

    osa_cond_check_arguments2(pcond, pmutex);

    /*
     *  According to the Pthreads specification, we should use the absolute
     *  timeout.
     */
#if 0
    abstime.tv_sec   = timeout / 1000;
    abstime.tv_nsec  = (timeout % 1000) * 1000000;
#else
    clock_gettime(CLOCK_REALTIME, &abstime);

    abstime.tv_sec  += timeout / 1000;
    abstime.tv_nsec += (timeout % 1000) * MILLIONS;
    if (abstime.tv_nsec >= BILLIONS) {
        abstime.tv_sec  -= BILLIONS;
        abstime.tv_nsec += 1;
    }
#endif

    retval = pthread_cond_timedwait(&pcond->m_cond, &pmutex->m_mutex, &abstime);

    if (retval == 0) {
        status = OSA_SOK;
    } else if (retval == ETIMEDOUT) {
        status = OSA_ETIMEOUT;
    } else {
        status = OSA_EINVAL;
    }

    return status;
}

status_t osa_cond_signal(osa_cond_t cond)
{
    status_t              status = OSA_SOK;
    struct __osa_cond_t * pcond  = (struct __osa_cond_t *)cond;

    osa_cond_check_arguments(pcond);

    pthread_cond_signal(&pcond->m_cond);

    return status;
}

status_t osa_cond_broadcast(osa_cond_t cond)
{
    status_t              status = OSA_SOK;
    struct __osa_cond_t * pcond  = (struct __osa_cond_t *)cond;

    osa_cond_check_arguments(pcond);

    pthread_cond_broadcast(&pcond->m_cond);

    return status;
}

status_t osa_cond_delete(osa_cond_t *pcond)
{
    status_t              status = OSA_SOK;
    struct __osa_cond_t * pcnd   = (struct __osa_cond_t *)(*pcond);

    osa_cond_check_arguments2(pcond, pcnd);

    pthread_cond_destroy(&pcnd->m_cond);

    status |= OSA_memFree(sizeof(struct __osa_cond_t), pcnd);

    (*pcond) = INVALID_HANDLE;

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
