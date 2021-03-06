/** ============================================================================
 *
 *  Copyright (C), 1987 - 2015, xiong-kaifang Tech. Co, Ltd.
 *
 *  @File Name:	osa_mutex.h
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2012-02-24
 *
 *  @Description:   // 用于详细说明此程序文件完成的主要功能，与其它模块
 *                  // 或函数的接口，输出值，取值范围、含义及参数间的控
 *                  // 制、顺序、独立或依赖等关系
 *                  //
 *
 *                  The header file for osa mutex and osa cond.
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
 *  xiong-kaifang   2012-02-24     v1.0	        Write this module.
 *
 *  xiong-kaifang   2015-09-19     v2.0         1. Add osa_cond_t related
 *                                                  routines.
 *                                              2. Using opaque type for
 *                                                 osa_mutex_t and osa_cond_t.
 *
 *  ============================================================================
 */

#if !defined (__OSA_MUTEX_H)
#define __OSA_MUTEX_H

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
typedef HANDLE      osa_mutex_t;

typedef HANDLE      osa_cond_t;

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
status_t osa_mutex_create(osa_mutex_t *pmutex);
status_t osa_mutex_lock  (osa_mutex_t  mutex);
status_t osa_mutex_unlock(osa_mutex_t  mutex);
status_t osa_mutex_delete(osa_mutex_t *pmutex);

status_t osa_cond_create (osa_cond_t *pcond);
status_t osa_cond_wait   (osa_cond_t cond, osa_mutex_t mutex);
status_t osa_cond_timedwait(osa_cond_t cond, osa_mutex_t mutex, unsigned int timeout);
status_t osa_cond_signal (osa_cond_t cond);
status_t osa_cond_broadcast(osa_cond_t cond);
status_t osa_cond_delete (osa_cond_t *pcond);

#if defined(__cplusplus)
}
#endif  /* defined(__cplusplus) */

#endif  /* if !defined (__OSA_MUTEX_H) */
