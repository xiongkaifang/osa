/** ============================================================================
 *
 *  Copyright (C), 1987 - 2013, xiong-kaifang Tech. Co, Ltd.
 *
 *  @File Name:	osa_task.h
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2013-04-05
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

#define TASKLIST_TSK_MAX    (10)

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
typedef unsigned int task_t;

#if 0
struct __task_msg_t; typedef struct __task_msg_t task_msg_t;
struct __task_msg_t
{
    mailbox_msg_t   m_header;
    task_t          m_to;
    task_t          m_frm;
};
#endif

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

typedef status_t (*TASK_SYNC_FXN)(void *ud, task_t tsk, msg_t *msg);

#if 0
typedef enum task_msg_type_t {
    TASK_MSG_CMD = MAILBOX_MSGQ_CMD,
    TASK_MSG_ACK = MAILBOX_MSGQ_ACK,
} task_msg_type_t;
#endif

typedef status_t (*TASK_MAIN)(void *userdata, task_t tsk, msg_t **msg);

struct __tasklist_params_t;
typedef struct __tasklist_params_t tasklist_params_t;
struct __tasklist_params_t
{
    unsigned short  m_tsk_cnt;
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
status_t tasklist_init(tasklist_params_t *prm);
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

status_t task_synchronize(void *ud, task_t tsk, TASK_SYNC_FXN fxn, unsigned int nums);

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

status_t task_delete(task_t *tsk);

#if defined(__cplusplus)
}
#endif  /* defined(__cplusplus) */

#endif  /* if !defined (__OSA_TASK_H) */
