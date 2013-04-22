/** ============================================================================
 *
 *  Copyright (C), 1987 - 2013, xiong-kaifang Tech. Co.,, Ltd.
 *
 *  @File name:	tsk_drv_test3.c
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2013-04-08
 *
 *  @Description:   The description of this file.
 *	
 *	                The template format for source file.
 *
 *  @Version:	    v1.0
 *
 *  @Function List:  //	主要函数及功能
 *	    1.  －－－－－
 *	    2.  －－－－－
 *
 *  @History:	     //	历史修改记录
 *
 *	<author>	    <time>	     <version>	    <desc>
 *  xiong-kaifang   2013-04-04     v1.0	        write this module.
 *
 *
 *  ============================================================================
 */

/*  --------------------- Include system headers ---------------------------- */

/*  --------------------- Include user headers   ---------------------------- */
#include "osa.h"
#include "tsk_drv_test3.h"
#include "debug.h"

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
 *  --------------------- Structure definition ---------------------------------
 */

/** ----------------------------------------------------------------------------
 *  @Name:          Structure name
 *
 *  @Description:   Description of the structure.
 *
 *  @Field:         Field1 member
 *
 *  @Field          Field2 member
 *  ----------------------------------------------------------------------------
 */
struct __task_driver_test3_object_t
{
    const char   * m_name;
};

typedef struct __task_driver_test3_object_t task_driver_test3_object_t;

/*
 *  --------------------- Global variable definition ---------------------------
 */
static task_driver_test3_object_t glb_tsk_drv_test3 = {
    .m_name       = "TSK_TEST_OBJECT3"
};

task_object_t glb_tsk_obj3 = {
    .m_name       = "TSK_TEST_OBJECT3",
    .m_main       = tsk_drv_test3_main,
    .m_pri        = 0,
    .m_stack_size = 0,
    .m_init_state = 0,
    .m_userdata   = &glb_tsk_drv_test3
};

/** ----------------------------------------------------------------------------
 *  @Name:          Variable name
 *
 *  @Description:   Description of the variable.
 * -----------------------------------------------------------------------------
 */

/*
 *  --------------------- Local function forward declaration -------------------
 */

/** ============================================================================
 *
 *  @Function:	    Local function forward declaration.
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
static status_t task_do_initialize(HANDLE hdl, task_t tsk, msg_t *msg);

static status_t task_do_exit(HANDLE hdl, task_t tsk, msg_t *msg);

static status_t task_do_synchronize(void *ud, task_t tsk, msg_t *msg);

static status_t task_do_process(HANDLE hdl, task_t tsk, msg_t **msg);

static status_t task_external_main(void *ud, task_t tsk, msg_t **msg);

/*
 *  --------------------- Public function definition ---------------------------
 */

/** ============================================================================
 *
 *  @Function:	    Public function definition.
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
status_t tsk_drv_test3_main(void *ud, task_t tsk, msg_t **msg)
{
    return task_external_main(ud, tsk, msg);
}

/*
 *  --------------------- Local function definition ----------------------------
 */

/** ============================================================================
 *
 *  @Function:	    Local function definition.
 *
 *  @Description:   //	函数功能、性能等的描述
 *
 *  ============================================================================
 */
static status_t task_do_initialize(HANDLE hdl, task_t tsk, msg_t *msg)
{
    status_t status = OSA_SOK;
    task_driver_test3_object_t * tsk_hdl = NULL;

    tsk_hdl = (task_driver_test3_object_t *)hdl;

    DBG(DBG_INFO, "task_do_initialize: %s do initialized.\n", tsk_hdl->m_name);

    return status;
}

static status_t task_do_exit(HANDLE hdl, task_t tsk, msg_t *msg)
{
    status_t status = OSA_SOK;
    task_driver_test3_object_t * tsk_hdl = NULL;

    tsk_hdl = (task_driver_test3_object_t *)hdl;

    DBG(DBG_INFO, "task_do_deinitialize: %s do deinitialized.\n", tsk_hdl->m_name);

    return status;
}

static status_t task_do_synchronize(void *ud, task_t tsk, msg_t *msg)
{
    status_t status = OSA_SOK;
    task_driver_test3_object_t * tsk_hdl = NULL;

    tsk_hdl = (task_driver_test3_object_t *)ud;

    DBG(DBG_INFO, "task_do_synchronize: %s do synchronized.\n", tsk_hdl->m_name);

    switch (msg_get_cmd(msg))
    {
        case TASK_CMD_EXIT:
            status |= task_do_exit((HANDLE)tsk_hdl, tsk, msg);
            status |= task_set_state(tsk, TASK_CMD_EXIT);
            break;

#if 0
        case 'TASK_USR_CMD1':
            break;

        case 'TASK_USR_CMD2':
            break;
#endif

        default:
            break;
    }

    return status;
}

static status_t task_do_process(HANDLE hdl, task_t tsk, msg_t **msg)
{
    status_t status = OSA_SOK;
    unsigned int msg_cnt = 0;
    task_state_t tsk_state = TASK_STATE_PROC;

    task_driver_test3_object_t * tsk_hdl = NULL;

    tsk_hdl = (task_driver_test3_object_t *)hdl;

    OSA_assert((void *)tsk_hdl != NULL);

    DBG(DBG_INFO, "task_do_process: %s do processed.\n", tsk_hdl->m_name);

    /*
     *  Note: if we do not return from this routine, ack the msg first.
     */
    msg_set_status(*msg, OSA_SOK);
    status = task_ack_free_msg(tsk, *msg);

    (*msg) = NULL;

    while (!OSA_ISERROR(task_get_state(tsk, &tsk_state))
            && tsk_state != TASK_STATE_EXIT) {

        /*
         *   Do the task.
         */
#if defined(TASK_MGR_DEBUG)
        fprintf(stderr, "task_do_process: TASK[0x%x]: %s is running...\n", tsk, tsk_hdl->m_name);
#endif  // if defined TASK_MGR_DEBUG

        sleep(3);

        status |= task_synchronize((void *)hdl, tsk, task_do_synchronize, msg_cnt);
    }

    return status;
}

static status_t task_external_main(void *ud, task_t tsk, msg_t **msg)
{
    status_t status = OSA_SOK;
    HANDLE hdl = (HANDLE)ud;

    switch (msg_get_cmd((*msg)))
    {
        case TASK_CMD_INIT:
            status |= task_set_state(tsk, TASK_STATE_INIT);
            status |= task_do_initialize(hdl, tsk, *msg);
            break;

        case TASK_CMD_EXIT:
            status |= task_set_state(tsk, TASK_STATE_EXIT);
            status |= task_do_exit(hdl, tsk, *msg);
            break;

        case TASK_CMD_PROC:
            status |= task_set_state(tsk, TASK_STATE_PROC);
            status |= task_do_process(hdl, tsk, msg);
            break;

        default:
            break;
    }

    return status;
}

#if defined(__cplusplus)
}
#endif
