/** ============================================================================
 *
 *  Copyright (C), 1987 - 2013, xiong-kaifang Tech. Co, Ltd.
 *
 *  @File Name:	osa_queue2.h
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2014-04-25
 *
 *  @Description:   //	用于详细说明此程序文件完成的主要功能，与其它模块
 *		            //	或函数的接口，输出值，取值范围、含义及参数间的控
 *		            //	制、顺序、独立或依赖等关系
 *		            //
 *
 *	                The format for header file.
 *
 *  @Others:	    //	其它内容说明
 *
 *  @Function List: //	主要函数列表，每条记录就包括函数名及功能简要说明
 *	    1.  ...
 *	    2.  ...
 *
 *  @History:	    //	修改历史记录列表，每条修改记录就包括修改日期、修改
 *	        	    //	时间及修改内容简述
 *	    1.  Date:
 *	        Author:
 *	        Modification:
 *	    2.  ...
 *
 *  ============================================================================
 */

#if !defined (__OSA_QUEUE2_H)
#define __OSA_QUEUE2_H

/*  --------------------- Include system headers ---------------------------- */
#include <pthread.h>

/*  --------------------- Include user headers   ---------------------------- */
#include "osa.h"
#include "dlist.h"
#include "osa_status.h"

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
enum __osa_queue2_state_t; typedef enum __osa_queue2_state_t osa_queue2_state_t;
enum __osa_queue2_state_t
{
    OSA_QUEUE2_STATE_INIT = 0,
    OSA_QUEUE2_STATE_EXIT = 1
};

/*
 *  --------------------- Data type definition ---------------------------------
 */

/** ----------------------------------------------------------------------------
 *  @Name:          Structure name
 *
 *  @Description:   Description of the structure.
 *
 *  @Field:         Field1 member
 *
 *  @Field:         Field2 member
 *  ----------------------------------------------------------------------------
 */
struct __osa_queue2_t; typedef struct __osa_queue2_t osa_queue2_t;
struct __osa_queue2_t
{
    volatile 
    unsigned int    m_state;
    pthread_mutex_t m_mutex;
    pthread_cond_t  m_rd_cond;
    pthread_cond_t  m_wr_cond;
    dlist_t         m_queue;
};

/*
 *  --------------------- Public function declaration --------------------------
 */

/** =============================================================================
 *
 *  @Function:	    //	函数名称
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
status_t osa_queue2_create(osa_queue2_t *queue);
status_t osa_queue2_put(osa_queue2_t *queue, unsigned int value, unsigned int timeout);
status_t osa_queue2_get(osa_queue2_t *queue, unsigned int *value, unsigned int timeout);
status_t osa_queue2_peek(osa_queue2_t *queue, unsigned int *value);
status_t osa_queue2_count(osa_queue2_t *queue, unsigned int *count);
Bool     osa_queue2_is_empty(osa_queue2_t *queue);
status_t osa_queue2_exit(osa_queue2_t *queue);
status_t osa_queue2_set_state(osa_queue2_t *queue, osa_queue2_state_t state);
status_t osa_queue2_reset(osa_queue2_t *queue);
status_t osa_queue2_delete(osa_queue2_t *queue);

#if defined(__cplusplus)
}
#endif  /* defined(__cplusplus) */

#endif  /* if !defined (__OSA_QUEUE_H) */