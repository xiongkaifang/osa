/** ============================================================================
 *
 *  Copyright (C), 1987 - 2013, xiong-kaifang Tech. Co.,, Ltd.
 *
 *  @File name:	osa_sem.c
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2013-04-04
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
 *  xiong-kaifang   2013-04-04     v1.0	        write this module.
 *
 *
 *  ============================================================================
 */

/*  --------------------- Include system headers ---------------------------- */

/*  --------------------- Include user headers   ---------------------------- */
#include "osa.h"
#include "osa_sem.h"

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
status_t sem2_create(sem2_t *sem, unsigned int max_cnt, unsigned int init_value)
{
    status_t status = OSA_SOK;
    pthread_mutexattr_t mutex_attr;
    pthread_condattr_t cond_attr;

    status |= pthread_mutexattr_init(&mutex_attr);
    status |= pthread_condattr_init(&cond_attr);

    status |= pthread_mutex_init(&sem->m_mutex, &mutex_attr);
    status |= pthread_cond_init(&sem->m_cond, &cond_attr);
    
    sem->m_count     = init_value;
    sem->m_max_count = max_cnt;

    if (sem->m_max_count == 0) {
        sem->m_max_count = 1;
    }

    if (sem->m_count > sem->m_max_count) {
        sem->m_count = sem->m_max_count;
    }

    pthread_mutexattr_destroy(&mutex_attr);
    pthread_condattr_destroy(&cond_attr);

    return status;
}

status_t sem2_wait  (sem2_t *sem, unsigned int timeout)
{
    status_t status = OSA_EFAIL;

    pthread_mutex_lock(&sem->m_mutex);

    while (1) {
        if (sem->m_count > 0) {
            sem->m_count--;
            status = OSA_SOK;
            break;
        } else {
            if (timeout == OSA_TIMEOUT_NONE) {
                break;
            }

            status |= pthread_cond_wait(&sem->m_cond, &sem->m_mutex);
        }
    }

    pthread_mutex_unlock(&sem->m_mutex);

    return status;
}

status_t sem2_signal(sem2_t *sem)
{
    status_t status = OSA_SOK;

    pthread_mutex_lock(&sem->m_mutex);

    if (sem->m_count < sem->m_max_count) {
        sem->m_count++;
        status |= pthread_cond_signal(&sem->m_cond);
    }

    pthread_mutex_unlock(&sem->m_mutex);

    return status;
}

status_t sem2_delete(sem2_t *sem)
{
    status_t status = OSA_SOK;

    pthread_cond_destroy(&sem->m_cond);
    pthread_mutex_destroy(&sem->m_mutex);

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

#if defined(__cplusplus)
}
#endif
