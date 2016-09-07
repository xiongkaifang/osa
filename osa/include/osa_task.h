/** ============================================================================
 *
 *  Copyright (C), 1987 - 2015, xiong-kaifang Tech. Co, Ltd.
 *
 *  @File Name:	osa_task.h
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2013-04-05
 *
 *  @Description:   // 用于详细说明此程序文件完成的主要功能，与其它模块
 *                  // 或函数的接口，输出值，取值范围、含义及参数间的控
 *                  // 制、顺序、独立或依赖等关系
 *                  //
 *
 *                  The header file for osa task.
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
 *  xiong-kaifang   2013-04-05     v1.0	        Write this module.
 *
 *  xiong-kaifang   2015-09-19     v1.2         1. Add routine:
 *                                                 task_check_state
 *                                              2. Tweak TASK_SYNC and TASK_MAIN
 *                                                 prototype.
 *
 *  xiong-kaifang   2016-09-07     v1.3         Add macro 'TASK_MSG_ACK()'.
 *
 *  ============================================================================
 */

#if !defined (__OSA_TASK_H)
#define __OSA_TASK_H

/*  --------------------- Include system headers ---------------------------- */

/*  --------------------- Include user headers   ---------------------------- */
#include "osa_msg.h"
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
#define TASK_INVALID_TSK    (~(0u))

#define TASK_MSG_ACK(tsk, pmsg, status) \
    do {                                \
        msg_set_status(*pmsg, status);  \
        task_ack_free_msg(tsk, *pmsg);  \
        (*pmsg) = NULL;                 \
    } while (0)

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
typedef HANDLE      task_t;

enum __task_cmd_t; typedef enum __task_cmd_t task_cmd_t;
enum __task_cmd_t
{
    TASK_CMD_INIT   = 1 << 0,
    TASK_CMD_EXIT   = 1 << 1,
    TASK_CMD_PROC   = 1 << 2,
    TASK_CMD_MAX    = 1 << 7,
};

enum __task_state_t; typedef enum __task_state_t task_state_t;
enum __task_state_t
{
    TASK_STATE_INIT = 1 << 0,
    TASK_STATE_EXIT = 1 << 1,
    TASK_STATE_PROC = 1 << 2,
};

typedef status_t (*TASK_SYNC)(task_t tsk, msg_t *msg , void *userdata);

typedef status_t (*TASK_MAIN)(task_t tsk, msg_t **msg, void *userdata);

struct __tasklist_params_t;
typedef struct __tasklist_params_t tasklist_params_t;
struct __tasklist_params_t
{
    unsigned int    m_min_tsk_nums;
    unsigned int    m_max_tsk_nums;
    unsigned int    m_max_linger;
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
status_t tasklist_init(tasklist_params_t *prm);
status_t tasklist_instruments(void);
status_t tasklist_deinit(void);

status_t task_create(const char *name, TASK_MAIN main,
					 unsigned int pri, unsigned int stack_size,
					 unsigned int init_state, void *userdata,
					 task_t *tsk);

status_t task_send_msg(task_t to, task_t frm, msg_t *msg, msg_type_t msgt);
status_t task_recv_msg(task_t tsk, msg_t **msg, msg_type_t msgt);

//status_t task_broadcast(task_t to_lists[], task_t frm, msg_t *msg);
status_t task_broadcast(task_t tolists[], task_t frm,
        unsigned short cmd, void *prm, unsigned int size, unsigned int flags);

status_t task_synchronize(task_t tsk, TASK_SYNC fxn, unsigned int nums, void *userdata);

status_t task_alloc_msg(unsigned short size, msg_t **msg);
status_t task_free_msg(unsigned short size, msg_t *msg);

status_t task_wait_msg(task_t tsk, msg_t **msg, msg_type_t msgt);
status_t task_check_msg(task_t tsk, msg_t **msg, msg_type_t msgt);
status_t task_wait_cmd(task_t tsk, msg_t **msg, unsigned short cmd);
status_t task_wait_ack(task_t tsk, msg_t **msg, unsigned int id);
status_t task_flush(task_t tsk);

status_t task_ack_free_msg(task_t tsk, msg_t *msg);

status_t task_get_msg_count(task_t tsk, unsigned int *cnt, msg_type_t msgt);

status_t task_get_state(task_t tsk, task_state_t *state);
status_t task_set_state(task_t tsk, task_state_t  state);
bool_t   task_check_state(task_t tsk);

status_t task_delete(task_t *tsk);

#if defined(__cplusplus)
}
#endif  /* defined(__cplusplus) */

#endif  /* if !defined (__OSA_TASK_H) */
