/** ============================================================================
 *
 *  Copyright (C), 1987 - 2015, xiong-kaifang Tech. Co, Ltd.
 *
 *  @File Name:	threadpool.h
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2012-09-12
 *
 *  @Description:   // 用于详细说明此程序文件完成的主要功能，与其它模块
 *                  // 或函数的接口，输出值，取值范围、含义及参数间的控
 *                  // 制、顺序、独立或依赖等关系
 *                  //
 *
 *                  The header file for threadpool(a implementation for thread
 *                  pool).
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
 *  xiong-kaifang   2012-09-12     v1.0	        Write this module.
 *
 *  xiong-kaifang   2015-09-18     v1.1         1. Add m_max_linger parameter.
 *                                              2. Modify threadpool_cancel_task
 *                                                 prototype.
 *
 *  ============================================================================
 */

#if !defined (__OSA_THREADPOOL_H)
#define __OSA_THREADPOOL_H

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
#define THREADPOOL_TASK_ARGS_MAX    (4)

/*
 *  --------------------- Data type definition ---------------------------------
 */
typedef HANDLE  threadpool_t;

typedef HANDLE  task_token_t;

typedef int    (*THREADPOOLSYNC_FXN)(void *hdl, int cmd, void *arg);

/** ----------------------------------------------------------------------------
 *  @Name:          Structure name
 *
 *  @Description:   Description of the structure.
 *
 *  @Field:         m_min_thd_nums
 *                  Minimum number of worker threads.
 *
 *  @Field:         m_max_thd_nums
 *                  Maximum number of worker threads.
 *
 *  @Field:         m_max_linger
 *                  Maximum seconds before idle workers exit.
 *  ----------------------------------------------------------------------------
 */
struct __threadpool_params_t;
typedef struct __threadpool_params_t threadpool_params_t;
struct __threadpool_params_t
{
    unsigned short  m_min_thd_nums;
    unsigned short  m_max_thd_nums;
    unsigned int    m_max_linger;
};

enum __threadpool_task_state_t;
typedef enum __threadpool_task_state_t threadpool_task_state_t;
enum __threadpool_task_state_t
{
    THREADPOOL_TASK_PENDING  = 0x01,
    THREADPOOL_TASK_RUNNING  = 0x02,
    THREADPOOL_TASK_FINISHED = 0x04,
};

struct __task_data_t; typedef struct __task_data_t task_data_t;
struct __task_data_t
{
    char *          m_name;
    Fxn             m_main;
    Fxn             m_exit;
    Arg             m_args[THREADPOOL_TASK_ARGS_MAX];
};

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
status_t threadpool_create(threadpool_t *thdp, const threadpool_params_t *prm);

status_t threadpool_add_task(threadpool_t thdp, const task_data_t *tsk_data, task_token_t *token);

status_t threadpool_sync_task(threadpool_t thdp, task_token_t task, int cmd, void *arg);

status_t threadpool_cancel_task(threadpool_t thdp, task_token_t *task);

status_t threadpool_wait(threadpool_t thdp);

status_t threadpool_delete(threadpool_t *thdp);

status_t threadpool_instruments(threadpool_t thdp);

#if defined(__cplusplus)
}
#endif  /* defined(__cplusplus) */

#endif  /* if !defined (__OSA_THREADPOOL_H) */
