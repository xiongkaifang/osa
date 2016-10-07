/** ============================================================================
 *
 *  Copyright (C), 1987 - 2015, xiong-kaifang Tech. Co, Ltd.
 *
 *  @File Name:	osa_queue.h
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2013-04-04
 *
 *  @Description:   // 用于详细说明此程序文件完成的主要功能，与其它模块
 *                  // 或函数的接口，输出值，取值范围、含义及参数间的控
 *                  // 制、顺序、独立或依赖等关系
 *                  //
 *
 *                  The header file for osa queue(using dynamic array).
 *
 *  @Others:        // 其它内容说明
 *
 *  @Function List: // 主要函数列表，每条记录就包括函数名及功能简要说明
 *      1.  ...
 *      2.  ...
 *
 *  @History:       // 修改历史记录列表，每条修改记录就包括修改日期、修改
 *                  // 时间及修改内容简述
 *
 *  <author>        <time>       <version>      <description>
 *  xiong-kaifang   2013-04-04     v1.0	        Write this module.
 *
 *  xiong-kaifang   2015-09-19     v1.1         1. Using opaque type for queue.
 *                                              2. Tweak queue's prototype.
 *
 *  xiong-kaifang   2016-10-07     v1.2         1. Tweak queue's prototype.
 *                                              2. Add callback to clean up
 *                                              queue.
 *
 *  ============================================================================
 */

#if !defined (__OSA_QUEUE_H)
#define __OSA_QUEUE_H

/*  --------------------- Include system headers ---------------------------- */

/*  --------------------- Include user headers   ---------------------------- */
#include "std_defs.h"
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
enum __queue_state_t; typedef enum __queue_state_t queue_state_t;
enum __queue_state_t
{
    QUEUE_STATE_INIT = 0,
    QUEUE_STATE_EXIT = 1,
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
typedef HANDLE      queue_t;

typedef void     (* QUE_CLEANUP)(void * value, void * userdata);

/*
 *  --------------------- Public function declaration --------------------------
 */

/** =============================================================================
 *
 *  @Function:      // 函数名称
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
status_t queue_create  (queue_t * pque, unsigned int max_len);
status_t queue_put     (queue_t   que , void * value, unsigned int timeout);
status_t queue_get     (queue_t   que , void ** pvalue, unsigned int timeout);
status_t queue_peek    (queue_t   que , void ** pvalue);
status_t queue_count   (queue_t   que , unsigned int * pcount);
bool_t   queue_is_empty(queue_t   que);
bool_t   queue_is_full (queue_t   que);
status_t queue_exit    (queue_t   que , QUE_CLEANUP cleanup, void * userdata);
status_t queue_delete  (queue_t * pque);

#if defined(__cplusplus)
}
#endif  /* defined(__cplusplus) */

#endif  /* if !defined (__OSA_QUEUE_H) */
