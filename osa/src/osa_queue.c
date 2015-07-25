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
 *  @Version:	    v1.0
 *
 *  @Function List:  //	主要函数及功能
 *	    1.  －－－－－
 *	    2.  －－－－－
 *
 *  @History:	     //	历史修改记录
 *
 *	<author>	    <time>	     <version>	    <desc>
 *  xiong-kaifang   2013-04-04     v1.0	        Write this module.
 *
 *
 *  ============================================================================
 */

/*  --------------------- Include system headers ---------------------------- */

/*  --------------------- Include user headers   ---------------------------- */
#include "osa_queue.h"
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
#define queue_is_exit(queue)    ((queue)->m_state == OSA_QUEUE_STATE_EXIT)

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
status_t queue_create(queue_t *queue, unsigned int max_len)
{
    status_t status = OSA_SOK;

    pthread_mutexattr_t mutex_attr;
    pthread_condattr_t cond_attr;

    queue->m_rd_idx = 0;
    queue->m_wr_idx = 0;
    queue->m_count  = 0;
    queue->m_len    = max_len;

    status = OSA_memAlloc(sizeof(unsigned int) * queue->m_len, (void **)&queue->m_queue);
    if (OSA_ISERROR(status) || queue->m_queue == NULL) {
        return OSA_EMEM;
    }

    status |= pthread_mutexattr_init(&mutex_attr);
    status |= pthread_condattr_init(&cond_attr);

    status |= pthread_mutex_init(&queue->m_mutex, &mutex_attr);
    status |= pthread_cond_init(&queue->m_rd_cond, &cond_attr);
    status |= pthread_cond_init(&queue->m_wr_cond, &cond_attr);

    pthread_condattr_destroy(&cond_attr);
    pthread_mutexattr_destroy(&mutex_attr);

    queue->m_state = OSA_QUEUE_STATE_INIT;

    return status;
}

status_t queue_put(queue_t *queue, unsigned int value, unsigned int timeout)
{
    status_t status = OSA_EFAIL;

    pthread_mutex_lock(&queue->m_mutex);

    while (!queue_is_exit(queue)) {
        if (queue->m_count < queue->m_len) {
            queue->m_queue[queue->m_wr_idx] = value;
            queue->m_wr_idx = (queue->m_wr_idx + 1) % queue->m_len;
            queue->m_count++;
            status = OSA_SOK;
            pthread_cond_signal(&queue->m_rd_cond);
            break;
        } else {
            if (timeout == OSA_TIMEOUT_NONE) {
                break;
            }

            status |= pthread_cond_wait(&queue->m_wr_cond, &queue->m_mutex);
        }
    }

    pthread_mutex_unlock(&queue->m_mutex);

    return status;
}

status_t queue_get(queue_t *queue, unsigned int *value, unsigned int timeout)
{
    status_t status = OSA_EFAIL;

    pthread_mutex_lock(&queue->m_mutex);

    while (!queue_is_exit(queue)) {
        if (queue->m_count > 0) {

            if (value != NULL) {
                (*value) = queue->m_queue[queue->m_rd_idx];
            }

            queue->m_rd_idx = (queue->m_rd_idx + 1) % queue->m_len;
            queue->m_count--;
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

status_t queue_peek(queue_t *queue, unsigned int *value)
{
    status_t status = OSA_SOK;

    pthread_mutex_lock(&queue->m_mutex);

    if (queue->m_count > 0) {
        if (value != NULL) {
            (*value) = queue->m_queue[queue->m_rd_idx];
        }
    }

    pthread_mutex_unlock(&queue->m_mutex);

    return status;
}

status_t queue_count(queue_t *queue, unsigned int *count)
{
    status_t status = OSA_SOK;

    pthread_mutex_lock(&queue->m_mutex);

    if (count != NULL) {
        (*count) = queue->m_count;
    }

    pthread_mutex_unlock(&queue->m_mutex);

    return status;
}

Bool     queue_is_empty(queue_t *queue)
{
    Bool is_empty;

    pthread_mutex_lock(&queue->m_mutex);

    if (queue->m_count == 0) {
        is_empty = TRUE;
    } else {
        is_empty = FALSE;
    }

    pthread_mutex_unlock(&queue->m_mutex);

    return is_empty;
}

status_t queue_exit(queue_t *queue)
{
    return queue_set_state(queue, OSA_QUEUE_STATE_EXIT);
}

status_t queue_set_state(queue_t *queue, queue_state_t state)
{
    status_t status = OSA_SOK;

    pthread_mutex_lock(&queue->m_mutex);

    queue->m_state = state;

    if (queue_is_exit(queue)) {
        pthread_cond_broadcast(&queue->m_wr_cond);
        pthread_cond_broadcast(&queue->m_rd_cond);
    }

    pthread_mutex_unlock(&queue->m_mutex);

    return status;
}

status_t queue_reset(queue_t *queue)
{
    status_t status = OSA_SOK;

    pthread_mutex_lock(&queue->m_mutex);

    queue->m_rd_idx = 0;
    queue->m_wr_idx = 0;
    queue->m_count  = 0;

    pthread_mutex_unlock(&queue->m_mutex);

    return status;
}

status_t queue_delete(queue_t *queue)
{
    status_t status = OSA_SOK;

    if (queue->m_queue != NULL) {
        OSA_memFree(sizeof(unsigned int) * queue->m_len, queue->m_queue);
        queue->m_queue = NULL;
    }

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
