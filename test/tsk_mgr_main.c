/** ============================================================================
 *
 *  Copyright (C), 1987 - 2013, xiong-kaifang Tech. Co.,, Ltd.
 *
 *  @File name:	tsk_mgr_main.c
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
#include <signal.h>

/*  --------------------- Include user headers   ---------------------------- */
#include "osa.h"
#include "tsk_mgr_main.h"
#include "tsk_drv_test1.h"
#include "tsk_drv_test2.h"
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

/*
 *  --------------------- Global variable definition ---------------------------
 */

/** ----------------------------------------------------------------------------
 *  @Name:          Variable name
 *
 *  @Description:   Description of the variable.
 * -----------------------------------------------------------------------------
 */
task_mgr_object_t   glb_tsk_mgr_obj;

static unsigned int glb_tsk_mgr_exit = 0;

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
status_t system_init(task_mgr_handle hdl);
status_t system_deinit(task_mgr_handle hdl);

status_t task_daemon_init(task_mgr_handle hdl);
status_t task_daemon_start(task_mgr_handle hdl);
status_t task_daemon_run(task_mgr_handle hdl);
status_t task_daemon_stop(task_mgr_handle hdl);
status_t task_daemon_exit(task_mgr_handle hdl);
status_t task_daemon_task_create(task_mgr_handle hdl);
status_t task_daemon_task_delete(task_mgr_handle hdl);

static status_t
tsk_mgr_daemon_process_msg(task_mgr_handle hdk, task_t tsk, msg_t *msg);

status_t tsk_mgr_daemon(task_mgr_handle hdl);

static void tsk_mgr_daemon_signal_handler(int sig)
{
    if (sig == SIGINT) {
        fprintf(stderr, "SIGINT signal caught, system shutdown now...\n");
        //glb_tsk_mgr_exit = 1; 

        int i;
        int retval = 0;
        status_t status = OSA_SOK;
        task_t tsklists[TASKLIST_TSK_MAX];

        for (i = 0; i < OSA_ARRAYSIZE(tsklists); i++) {
            tsklists[i] = TASK_INVALID_TSK;
        }

        tsklists[0] = glb_tsk_mgr_obj.m_cur_tsk;

        task_broadcast(tsklists, (task_t)NULL, TASK_CMD_EXIT, NULL, 0, 0);

    } else {
        fprintf(stderr, "Invalid signal caught\n");
    }
}

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
int main(int argc, char *argv[])
{
    status_t status;

    system_init(&glb_tsk_mgr_obj);

    status = tsk_mgr_daemon(&glb_tsk_mgr_obj);

    system_deinit(&glb_tsk_mgr_obj);

    return status;
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
status_t system_init(task_mgr_handle hdl)
{
    status_t status = OSA_SOK;

    /*
     *  Initialize debug module.
     */
    fprintf(stderr, "system_init: Initiailze debug module.\n");
    debugger_init(stderr, NULL);
    //debugger_setlevel(DBG_DETAILED);
    debugger_setlevel(DBG_INFO);

    /*
     *  Initialize msgq manager.
     */
    hdl->m_params.m_msgq_mgr_prm.m_msgq_cnt = 20;

    fprintf(stderr, "system_init: Initiailze msgq manager.\n");
    status |= msgq_mgr_init(&hdl->m_params.m_msgq_mgr_prm);

    /*
     *  Initialize mailbox system.
     */
    fprintf(stderr, "system_init: Initiailze mailbox system.\n");
    hdl->m_params.m_mbx_sys_prm.m_mbx_cnt = 5;
    status |= mailbox_system_init(&hdl->m_params.m_mbx_sys_prm);

    /*
     *  Initialize tasklist.
     */
    fprintf(stderr, "system_init: Initiailze task lists.\n");
    hdl->m_params.m_tsklist_prm.m_tsk_cnt = 5;
    status |= tasklist_init(&hdl->m_params.m_tsklist_prm);

    OSA_assert(OSA_SOK == status);

    return status;
}

status_t system_deinit(task_mgr_handle hdl)
{
    status_t status = OSA_SOK;

    /*
     *  Finialize tasklist.
     */
    fprintf(stderr, "system_deinit: De-initialize task lists\n");
    status |= tasklist_deinit();

    /*
     *  Finalize mailbox system.
     */
    
    fprintf(stderr, "system_deinit: De-initialize mailbox system\n");
    status |= mailbox_system_deinit();

    /*
     *  Finalize msgq manager.
     */
    fprintf(stderr, "system_deinit: De-initialize msgq manager\n");
    status |= msgq_mgr_deinit();

    /*
     *  Finalize debug module.
     */
    fprintf(stderr, "system_deinit: De-initialize debug module\n");
    debugger_destroy();

    OSA_assert(OSA_SOK == status);

    return status;
}

status_t tsk_mgr_daemon(task_mgr_handle hdl)
{
    status_t status = OSA_SOK;

    status = task_daemon_init(hdl);
    if (OSA_ISERROR(status)) {
        return status;
    }

    status = task_daemon_start(hdl);
    if (OSA_ISERROR(status)) {
        return status;
    }

    status = task_daemon_run(hdl);
    if (OSA_ISERROR(status)) {
        return status;
    }

    status = task_daemon_stop(hdl);
    status = task_daemon_exit(hdl);

    return status;
}

status_t task_daemon_init(task_mgr_handle hdl)
{
    status_t status = OSA_SOK;

    status |= mutex_create(&hdl->m_mutex);

    hdl->m_tsk_cnt = TASK_MGR_TSK_MAX;
    hdl->m_cur_cnt = 2;
    
    hdl->m_tsklists[TASK_MGR_TSK1].m_tsk_obj = &glb_tsk_obj1;
    hdl->m_tsklists[TASK_MGR_TSK1].m_tsk     = TASK_INVALID_TSK;

    hdl->m_tsklists[TASK_MGR_TSK2].m_tsk_obj = &glb_tsk_obj2;
    hdl->m_tsklists[TASK_MGR_TSK2].m_tsk     = TASK_INVALID_TSK;

    status = task_daemon_task_create(hdl);

    return status;
}

status_t task_daemon_start(task_mgr_handle hdl)
{
    int i, j;
    int retval = 0;
    Bool init_succeeded = TRUE;
    msg_t *msg = NULL;
    status_t status = OSA_SOK;
    task_t tsklists[TASKLIST_TSK_MAX];

    for (i = 0; i < OSA_ARRAYSIZE(tsklists); i++) {
        tsklists[i] = TASK_INVALID_TSK;
    }

    for (i = 0; i < hdl->m_cur_cnt; i++) {
        tsklists[i] = hdl->m_tsklists[i].m_tsk;
    }

    /*
     *  Broadcast INIT cmd.
     */
    sleep(1);
    fprintf(stderr, "task_daemon_start: Send INIT cmd.\n");
    status = task_broadcast(tsklists, hdl->m_cur_tsk, TASK_CMD_INIT, NULL, 0, MSG_FLAGS_WAIT_ACK);

    /*
     *  Wait for ack.
     */
    fprintf(stderr, "task_daemon_start: Wait INIT ack.\n");
    for (i = 0; i < hdl->m_cur_cnt; i++) {
        status |= task_recv_msg(hdl->m_cur_tsk, &msg, MSG_TYPE_ACK);

        /*
         *  Checkout the status.
         */
        retval = msg_get_status(msg);

        //msg_set_flags(msg, 0);

        status |= task_ack_free_msg(hdl->m_cur_tsk, msg);

        if (OSA_ISERROR(retval)) {
            /*
             *  Error.
             */
            //break;
            init_succeeded = FALSE;
        }
    }

    if (!init_succeeded) {

        DBG(DBG_ERROR, "task_daemon_start: Not all task initiailzed successfully, shut down system...\n");
        status = task_broadcast(tsklists, hdl->m_cur_tsk, TASK_CMD_EXIT, NULL, 0, MSG_FLAGS_WAIT_ACK);

        /*
         *  Wait for ack.
         */
        for (i = 0; i < hdl->m_cur_cnt; i++) {
            status |= task_recv_msg(hdl->m_cur_tsk, &msg, MSG_TYPE_ACK);

            /*
             *  Checkout the status.
             */
            retval = msg_get_status(msg);

            //msg_set_flags(msg, 0);

            status |= task_ack_free_msg(hdl->m_cur_tsk, msg);
        }
    }

    return (init_succeeded ? OSA_SOK : OSA_EFAIL);
}

status_t task_daemon_run(task_mgr_handle hdl)
{
    int i;
    int retval = 0;
    msg_t *msg = NULL;
    status_t status = OSA_SOK;
    task_state_t state = TASK_STATE_PROC;
    task_t tsklists[TASKLIST_TSK_MAX];

    for (i = 0; i < OSA_ARRAYSIZE(tsklists); i++) {
        tsklists[i] = TASK_INVALID_TSK;
    }

    for (i = 0; i < hdl->m_cur_cnt; i++) {
        tsklists[i] = hdl->m_tsklists[i].m_tsk;
    }

    /*
     *  Broadcast PROCESS cmd.
     */
    fprintf(stderr, "task_daemon_run: Send PROC cmd\n");
    status = task_broadcast(tsklists, hdl->m_cur_tsk, TASK_CMD_PROC, NULL, 0, 0);

    OSA_assert(OSA_SOK == status);

    glb_tsk_mgr_exit = 0;
    signal(SIGINT, tsk_mgr_daemon_signal_handler);

    while (!glb_tsk_mgr_exit) {

        sleep(1);

        DBG(DBG_INFO, "task_daemon_run: task daemon system is running...\n");

        status = task_check_msg(hdl->m_cur_tsk, &msg, MSG_TYPE_CMD);

        if (OSA_ISERROR(status)) {
            continue;
        }

        status |= tsk_mgr_daemon_process_msg(hdl, hdl->m_cur_tsk, msg);

        status |= task_ack_free_msg(hdl->m_cur_tsk, msg);

        if (!OSA_ISERROR(task_get_state(hdl->m_cur_tsk, &state))
                && state == TASK_STATE_EXIT) {
            break;
        }
    }

    return OSA_SOK;
}

status_t task_daemon_stop(task_mgr_handle hdl)
{
    int i;
    int retval = 0;
    msg_t *msg = NULL;
    status_t status = OSA_SOK;
    task_t tsklists[TASKLIST_TSK_MAX];

    for (i = 0; i < OSA_ARRAYSIZE(tsklists); i++) {
        tsklists[i] = TASK_INVALID_TSK;
    }

    for (i = 0; i < hdl->m_cur_cnt; i++) {
        tsklists[i] = hdl->m_tsklists[i].m_tsk;
    }

    /*
     *  Broadcast EXIT cmd.
     */
    fprintf(stderr, "task_daemon_stop: Send EXIT cmd\n");
    status = task_broadcast(tsklists, hdl->m_cur_tsk, TASK_CMD_EXIT, NULL, 0, MSG_FLAGS_WAIT_ACK);

    for (i = 0; i < hdl->m_cur_cnt; i++) {

        status |= task_recv_msg(hdl->m_cur_tsk, &msg, MSG_TYPE_ACK);

        /*
         *  Checkout the status.
         */
        retval = msg_get_status(msg);

        //msg_set_flags(msg, 0);

        status |= task_ack_free_msg(hdl->m_cur_tsk, msg);

    }

    return status;
}

status_t task_daemon_exit(task_mgr_handle hdl)
{
    status_t status = OSA_SOK;
    
    status |= task_daemon_task_delete(hdl);

    status |= mutex_delete(&hdl->m_mutex);

    return status;
}

status_t task_daemon_task_create(task_mgr_handle hdl)
{
    int i;
    status_t status = OSA_SOK;

    status = task_create("TSK_MGR_MAIN", NULL, 0, 0, 0, NULL, &hdl->m_cur_tsk);

    for (i = 0; i < hdl->m_cur_cnt; i++) {
        status = task_create(
                hdl->m_tsklists[i].m_tsk_obj->m_name,
                hdl->m_tsklists[i].m_tsk_obj->m_main,
                hdl->m_tsklists[i].m_tsk_obj->m_pri,
                hdl->m_tsklists[i].m_tsk_obj->m_stack_size,
                hdl->m_tsklists[i].m_tsk_obj->m_init_state,
                hdl->m_tsklists[i].m_tsk_obj->m_userdata,
                &hdl->m_tsklists[i].m_tsk
                );

        OSA_assert(OSA_SOK == status);
    }

    return status;
}

status_t task_daemon_task_delete(task_mgr_handle hdl)
{
    int i;
    status_t status = OSA_SOK;

    for (i = 0; i < hdl->m_cur_cnt; i++) {

        status = task_delete(&hdl->m_tsklists[i].m_tsk);

        OSA_assert(OSA_SOK == status);
    }

    status = task_delete(&hdl->m_cur_tsk);

    return status;
}

static status_t
tsk_mgr_daemon_process_msg(task_mgr_handle hdl, task_t tsk, msg_t *msg)
{
    status_t status = OSA_SOK;

    switch(msg_get_cmd(msg))
    {
        case TASK_CMD_EXIT:
            DBG(DBG_INFO, "tsk_mgr_daemon_process_msg: EXIT cmd received\n");
            task_set_state(tsk, TASK_STATE_EXIT);
            break;

        default:
            break;
    }

    return status;
}

#if defined(__cplusplus)
}
#endif
