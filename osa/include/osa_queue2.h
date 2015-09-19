/** ============================================================================
 *
 *  Copyright (C), 1987 - 2015, xiong-kaifang Tech. Co, Ltd.
 *
 *  @File Name:	osa_queue2.h
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2014-04-25
 *
 *  @Description:   // 用于详细说明此程序文件完成的主要功能，与其它模块
 *                  // 或函数的接口，输出值，取值范围、含义及参数间的控
 *                  // 制、顺序、独立或依赖等关系
 *                  //
 *
 *                  The header file for osa queue2(using double link list).
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
 *
 *  xiong-kaifang   2014-04-25     v1.0	        Write this module.
 *
 *  xiong-kaifang   2015-09-19     v1.1         1. Using opaque type for queue2.
 *                                              2. Tweak queue2's prototypes.
 *
 *  ============================================================================
 */

#if !defined (__OSA_QUEUE2_H)
#define __OSA_QUEUE2_H

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
enum __queue2_state_t; typedef enum __queue2_state_t queue2_state_t;
enum __queue2_state_t
{
    QUEUE2_STATE_INIT = 0,
    QUEUE2_STATE_EXIT = 1
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
typedef HANDLE      queue2_t;

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
status_t queue2_create(queue2_t *pque);

status_t queue2_put(queue2_t que, void *pvalue , unsigned int timeout);

status_t queue2_get(queue2_t que, void **ppvalue, unsigned int timeout);

status_t queue2_peek(queue2_t que, void **ppvalue);

status_t queue2_count(queue2_t que, unsigned int *pcount);

bool_t   queue2_is_empty(queue2_t que);

status_t queue2_exit(queue2_t que);

status_t queue2_set_state(queue2_t que, queue2_state_t state);

status_t queue2_reset(queue2_t que);

status_t queue2_delete(queue2_t *pque);

#if defined(__cplusplus)
}
#endif  /* defined(__cplusplus) */

#endif  /* if !defined (__OSA_QUEUE_H) */
