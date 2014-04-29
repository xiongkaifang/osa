/** ============================================================================
 *
 *  Copyright (C), 1987 - 2013, xiong-kaifang Tech. Co.,, Ltd.
 *
 *  @File name:	osa_queue2.c
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2014-04-25
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
 *  xiong-kaifang   2014-04-25     v1.0	        write this module.
 *
 *
 *  ============================================================================
 */

/*  --------------------- Include system headers ---------------------------- */

/*  --------------------- Include user headers   ---------------------------- */
#include "osa_queue2.h"
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
#define queue_is_exit(queue)    ((queue)->m_state == OSA_QUEUE2_STATE_EXIT)

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
status_t osa_queue2_create(osa_queue2_t *queue)
{
    status_t status = OSA_SOK;

    pthread_mutexattr_t mutex_attr;
    pthread_condattr_t cond_attr;

    status |= pthread_mutexattr_init(&mutex_attr);
    status |= pthread_condattr_init(&cond_attr);

    status |= pthread_mutex_init(&queue->m_mutex, &mutex_attr);
    status |= pthread_cond_init(&queue->m_rd_cond, &cond_attr);
    status |= pthread_cond_init(&queue->m_wr_cond, &cond_attr);

    pthread_condattr_destroy(&cond_attr);
    pthread_mutexattr_destroy(&mutex_attr);

    queue->m_state = OSA_QUEUE2_STATE_INIT;

    return status;
}

status_t osa_queue2_put(osa_queue2_t *queue, unsigned int value, unsigned int timeout)
{
    status_t status = OSA_EFAIL;

    pthread_mutex_lock(&queue->m_mutex);

    while (!osa_queue2_is_exit(queue)) {
        status = dlist_initialize_element((dlist_element_t *)value);
        status = dlist_put_tail(&queue->m_queue, (dlist_element_t *)value);
        pthread_cond_signal(&queue->m_rd_cond);
    }

    pthread_mutex_unlock(&queue->m_mutex);

    return status;
}

status_t osa_queue2_get(osa_queue2_t *queue, unsigned int *value, unsigned int timeout)
{
    status_t status = OSA_EFAIL;

    pthread_mutex_lock(&queue->m_mutex);

    while (!osa_queue2_is_exit(queue)) {
        if (!dlist_is_empty(&queue->m_queue)) {

            if (value != NULL) {
                status = dlist_get_head(&queue->m_queue, (dlist_element_t **)value);
            }

            status = OSA_SOK;
            pthread_cond_signal(&queue->m_wr_cond);
            break;
        } else {
            if (timeout == OSA_TIMEOUT_NONE) {
                break;
            }

            status |= pthread_cond_wait(&queue->m_rd_cond, &queue->m_mutex);
        }
    }

    pthread_mutex_unlock(&queue->m_mutex);

    return status;
}

status_t osa_queue2_peek(osa_queue2_t *queue, unsigned int *value)
{
    status_t status = OSA_ENOENT;

    pthread_mutex_lock(&queue->m_mutex);

    if (!dlist_is_empty(&queue->m_queue)) {
        if (value != NULL) {
            status = dlist_first(&queue->m_queue, (dlist_element_t **)value);
        }
    }

    pthread_mutex_unlock(&queue->m_mutex);

    return status;
}

status_t osa_queue2_count(osa_queue2_t *queue, unsigned int *count)
{
    status_t status = OSA_SOK;

    pthread_mutex_lock(&queue->m_mutex);

    if (count != NULL) {
        status = dlist_count(&queue->m_queue, count);
    }

    pthread_mutex_unlock(&queue->m_mutex);

    return status;
}

Bool     osa_queue2_is_empty(osa_queue2_t *queue)
{
    Bool is_empty;

    pthread_mutex_lock(&queue->m_mutex);

    if (dlist_is_empty(&queue->m_queue)) {
        is_empty = TRUE;
    } else {
        is_empty = FALSE;
    }

    pthread_mutex_unlock(&queue->m_mutex);

    return is_empty;
}

status_t osa_queue2_exit(osa_queue2_t *queue)
{
    return osa_queue2_set_state(queue, OSA_QUEUE2_STATE_EXIT);
}

status_t osa_queue2_set_state(osa_queue2_t *queue, osa_queue2_state_t state)
{
    status_t status = OSA_SOK;

    pthread_mutex_lock(&queue->m_mutex);

    queue->m_state = state;

    if (oas_queue2_is_exit(queue)) {
        pthread_cond_broadcast(&queue->m_wr_cond);
        pthread_cond_broadcast(&queue->m_rd_cond);
    }

    pthread_mutex_unlock(&queue->m_mutex);

    return status;
}

status_t osa_queue2_reset(osa_queue2_t *queue)
{
    status_t status = OSA_SOK;

    pthread_mutex_lock(&queue->m_mutex);

    status = dlist_init(&queue->m_queue);

    pthread_mutex_unlock(&queue->m_mutex);

    return status;
}

status_t osa_queue2_delete(osa_queue2_t *queue)
{
    status_t status = OSA_SOK;

    pthread_cond_destroy(&queue->m_rd_cond);
    pthread_cond_destroy(&queue->m_wr_cond);
    pthread_mutex_destroy(&queue->m_mutex);

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
