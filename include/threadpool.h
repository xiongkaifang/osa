/*
 *  threadpool.h
 *
 *  Created on: Sep 12, 2012
 *  Author    : xkf
 */

#if !defined (THREADPOOL_H_)
#define THREADPOOL_H_

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
#define THREADPOOL_TASK_ARGS_MAX    (8)

/*
 *  --------------------- Data type definition ---------------------------------
 */
typedef int threadpool_t;

typedef void * task_token_t;

typedef int    (*THREADPOOLSYNC_FXN)(void *hdl, int cmd, void *arg);

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
struct __threadpool_params_t;
typedef struct __threadpool_params_t threadpool_params_t;
struct __threadpool_params_t
{
    unsigned short  m_min_thd_nums;
    unsigned short  m_max_thd_nums;
    unsigned int    m_max_tsk_nums;
};

enum __threadpool_state_t;
typedef enum __threadpool_state_t threadpool_state_t;
enum __threadpool_state_t
{
    THREADPOOL_STATE_WAIT    = 0x01,
    THREADPOOL_STATE_DESTROY = 0x02,
};

enum __threadpool_task_state_t;
typedef enum __threadpool_task_state_t threadpool_task_state_t;
enum __threadpool_task_state_t
{
    THREADPOOL_TASK_PENDING  = 0x01,
    THREADPOOL_TASK_RUNNING  = 0x02,
    THREADPOOL_TASK_FINISHED = 0x04,
};

struct __task_operation_t;
typedef struct __task_operation_t task_operation_t;
struct __task_operation_t
{
    Fxn             m_main;
    unsigned int    m_args[THREADPOOL_TASK_ARGS_MAX];
};

struct __task_common_operation_t;
typedef struct __task_common_operation_t task_common_operation_t;
struct __task_common_operation_t
{
    task_operation_t    m_tsk_ops;
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
status_t threadpool_create(threadpool_t *thdp, const threadpool_params_t *prm);

status_t threadpool_add_task(threadpool_t thdp, const task_operation_t *tsk_ops, task_token_t *token);

status_t threadpool_sync_task(threadpool_t thdp, task_token_t task, int cmd, void *arg);

status_t threadpool_cancel_task(threadpool_t thdp, task_token_t task);

status_t threadpool_wait(threadpool_t thdp);

status_t threadpool_delete(threadpool_t *thdp);

status_t threadpool_instruments(threadpool_t thdp);

#if defined(__cplusplus)
}
#endif  /* defined(__cplusplus) */

#endif  /* if !defined (THREADPOOL_H_) */
