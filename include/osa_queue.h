/** ============================================================================
 *
 *  Copyright (C), 1987 - 2013, xiong-kaifang Tech. Co, Ltd.
 *
 *  @File Name:	osa_queue.h
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2013-04-04
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

#if !defined (__OSA_QUEUE_H)
#define __OSA_QUEUE_H

/*  --------------------- Include system headers ---------------------------- */
#include <pthread.h>

/*  --------------------- Include user headers   ---------------------------- */
#include "osa.h"
#include "osa_mutex.h"
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
struct __queue_t; typedef struct __queue_t queue_t;
struct __queue_t
{
	unsigned int	m_rd_idx;
	unsigned int	m_wr_idx;
	unsigned int	m_len;
	unsigned int	m_count;
	unsigned int  * m_queue;
	
	//mutex_t			m_mutex;
    pthread_mutex_t m_mutex;
	pthread_cond_t	m_rd_cond;
	pthread_cond_t	m_wr_cond;
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
status_t queue_create(queue_t *queue, unsigned int max_len);
status_t queue_put(queue_t *queue, unsigned int value, unsigned int timeout);
status_t queue_get(queue_t *queue, unsigned int *value, unsigned int timeout);
status_t queue_peek(queue_t *queue, unsigned int *value);
status_t queue_count(queue_t *queue, unsigned int *count);
Bool     queue_is_empty(queue_t *queue);
status_t queue_delele(queue_t *queue);

#if defined(__cplusplus)
}
#endif  /* defined(__cplusplus) */

#endif  /* if !defined (__OSA_QUEUE_H) */
