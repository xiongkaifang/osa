/** ============================================================================
 *
 *  Copyright (C), 1987 - 2015, xiong-kaifang Tech. Co, Ltd.
 *
 *  @File Name:	osa_task_mgr.h
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2013-04-16
 *
 *  @Description:   //	用于详细说明此程序文件完成的主要功能，与其它模块
 *		            //	或函数的接口，输出值，取值范围、含义及参数间的控
 *		            //	制、顺序、独立或依赖等关系
 *		            //
 *
 *	                The header file for osa task manager.
 *
 *  @Others:	    //	其它内容说明
 *
 *  @Function List: //	主要函数列表，每条记录就包括函数名及功能简要说明
 *	    1.  ...
 *	    2.  ...
 *
 *  @History:	    //	修改历史记录列表，每条修改记录就包括修改日期、修改
 *	        	    //	时间及修改内容简述
 *
 *	<author>	    <time>	     <version>	    <desc>
 *  xiong-kaifang   2013-04-16     v1.0	        Write this module.
 *
 *  ============================================================================
 */

#if !defined (__OSA_TASK_MGR_H)
#define __OSA_TASK_MGR_H

/*  --------------------- Include system headers ---------------------------- */

/*  --------------------- Include user headers   ---------------------------- */
#include "osa_msg.h"
#include "osa_task.h"
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
#define TASK_MGR_TSK_NAME   ("TASK_MGR_TSK")

#define TASK_MGR_TSK_MAX    (10)

#define TASK_MGR_MSG_MAX    (TASK_MGR_TSK_MAX)
/*
 *  --------------------- Data type definition ---------------------------------
 */
typedef status_t (*TASK_CMD_FIND)(void *ud, unsigned short cmd, void **ptsk);

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
struct __task_mgr_prm_t;
typedef struct __task_mgr_prm_t task_mgr_prm_t;
struct __task_mgr_prm_t
{
    unsigned short  m_msg_cnt;
    unsigned short  m_tsk_cnt;
};

struct __task_object_t; typedef struct __task_object_t task_object_t;
struct __task_object_t
{
    DLIST_ELEMENT_RESERVED;

    unsigned char * m_name;
    TASK_MAIN       m_main;
    TASK_CMD_FIND   m_find;
    unsigned int    m_pri;
    unsigned int    m_stack_size;
    unsigned int    m_init_state;
    void          * m_userdata;
    task_t          m_task;
};

enum __task_mgr_cmd_t; typedef enum __task_mgr_cmd_t task_mgr_cmd_t;
enum __task_mgr_cmd_t
{
    TASK_MGR_CMD_BASE        = TASK_CMD_MAX + 1,
    TASK_MGR_CMD_CREATE_TASK = TASK_MGR_CMD_BASE + 1,
    TASK_MGR_CMD_START_TASK  = TASK_MGR_CMD_BASE + 2,
    TASK_MGR_CMD_STOP_TASK   = TASK_MGR_CMD_BASE + 3,
    TASK_MGR_CMD_DELETE_TASK = TASK_MGR_CMD_BASE + 4,
    TASK_MGR_CMD_FIND_TASK   = TASK_MGR_CMD_BASE + 5,
    TASK_MGR_CMD_TASK_INSTRUMENTS    = TASK_MGR_CMD_BASE + 6,
    TASK_MGR_CMD_THDPOOL_INSTRUMENTS = TASK_MGR_CMD_BASE + 7,
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
status_t task_mgr_init(task_mgr_prm_t *prm);

status_t task_mgr_register(task_object_t *tsk);

status_t task_mgr_start(task_object_t *tsk);

status_t task_mgr_stop(task_object_t *tsk);

status_t task_mgr_broadcast(unsigned short cmd, void *prm, unsigned int size, unsigned int flags);

status_t task_mgr_synchronize(task_object_t *tsk, unsigned short cmd, void *prm, unsigned int size, unsigned int flags);

status_t task_mgr_synchronize2(unsigned short cmd, void *prm, unsigned int size, unsigned int flags);

status_t task_mgr_unregister(task_object_t *tsk);

status_t task_mgr_find(const char *name, task_object_t **ptsk);

status_t task_mgr_task_instruments(void);

status_t task_mgr_thdpool_instruments(void);

status_t task_mgr_deinit(void);

#if defined(__cplusplus)
}
#endif  /* defined(__cplusplus) */

#endif  /* if !defined (__OSA_TASK_MGR_H) */
