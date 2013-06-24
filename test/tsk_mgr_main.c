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
#include "tsk_drv_test3.h"
#include "tsk_drv_test4.h"
#include "tsk_drv_test5.h"
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

task_object_t glb_tsk_mgr = {
    .m_name       = "TSK_MGR_DAEMON",
    .m_main       = NULL,
    .m_pri        = 0,
    .m_stack_size = 0,
    .m_init_state = 0,
    .m_userdata   = NULL,
    .m_task       = TASK_INVALID_TSK
};

static const char * menu[] = {
    "\nChoice | Description\n",
    "---------------------------------\n",
    "   1     Print Task Manamger instruments.\n",
    "   2     Not used\n",
    "   3     Not used\n",
    "   4     Not used\n",
    "   5     Not used\n",
    "   t     Task operate dynamically\n",
    "   q     Quit from this task daemon.\n"
};

static const char * submenu[] = {
    "\nChoice | Description\n",
    "---------------------------------\n",
    "  'c'    Create task5.\n",
    "  'd'    Delete task5\n",
    "  's'    Start task5\n",
    "  'p'    Stop task5\n",
};

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

static int
task_mgr_daemon_print_menu(task_mgr_handle hdl);

static status_t
task_mgr_daemon_dynamic(task_mgr_handle hdl);

status_t tsk_mgr_daemon(task_mgr_handle hdl);

static void tsk_mgr_daemon_signal_handler(int sig)
{
    if (sig == SIGINT) {
        fprintf(stderr, "SIGINT signal caught, system shutdown now...\n");

        task_mgr_synchronize(glb_tsk_mgr_obj.m_tsklists[TASK_MGR_TSK0], TASK_CMD_EXIT, NULL, 0, 0);
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
     *  Initialize tasklist.
     */
    fprintf(stderr, "system_init: Initiailze task manager.\n");
    hdl->m_params.m_tsk_mgr_prm.m_msg_cnt = 10;
    hdl->m_params.m_tsk_mgr_prm.m_tsk_cnt = 10;
    status |= task_mgr_init(&hdl->m_params.m_tsk_mgr_prm);

    OSA_assert(OSA_SOK == status);

    return status;
}

status_t system_deinit(task_mgr_handle hdl)
{
    status_t status = OSA_SOK;

    /*
     *  Finialize tasklist.
     */
    status |= task_mgr_deinit();
    fprintf(stderr, "system_deinit: De-initialize task manager.\n");

    /*
     *  Finalize debug module.
     */
    fprintf(stderr, "system_deinit: De-initialize debug module.\n");
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
    hdl->m_cur_cnt = 5;
    
    hdl->m_tsklists[TASK_MGR_TSK0] = &glb_tsk_mgr;
    hdl->m_tsklists[TASK_MGR_TSK1] = &glb_tsk_obj1;
    hdl->m_tsklists[TASK_MGR_TSK2] = &glb_tsk_obj2;
    hdl->m_tsklists[TASK_MGR_TSK3] = &glb_tsk_obj3;
    hdl->m_tsklists[TASK_MGR_TSK4] = &glb_tsk_obj4;
    hdl->m_tsklists[TASK_MGR_TSK5] = &glb_tsk_obj5;

    status = task_daemon_task_create(hdl);

    return status;
}

status_t task_daemon_start(task_mgr_handle hdl)
{
    Bool init_succeeded = TRUE;
    status_t status = OSA_SOK;

    status = task_mgr_broadcast(TASK_CMD_INIT, NULL, 0, 0);
    if (OSA_ISERROR(status)) {
        status |= task_mgr_broadcast(TASK_CMD_EXIT, NULL, 0, 0);
    }

    return status;
}

status_t task_daemon_run(task_mgr_handle hdl)
{
    int ch;
    msg_t *msg = NULL;
    status_t status = OSA_SOK;
    task_state_t state = TASK_STATE_PROC;

    status = task_mgr_broadcast(TASK_CMD_PROC, NULL, 0, 0);

    glb_tsk_mgr_exit = 0;
    signal(SIGINT, tsk_mgr_daemon_signal_handler);

    while (!glb_tsk_mgr_exit) {

#if defined(TASM_MGR_DEBUG)
        sleep(1);

        DBG(DBG_INFO, "task_daemon_run: task daemon system is running ...\n");
#endif  // if defined TASK_MGR_DEBUG

        ch = task_mgr_daemon_print_menu(hdl);

        if (ch == '\n') {
            continue;
        }

        switch(ch)
        {
            case '1':
                status = task_mgr_task_instruments();
                break;

            case '2':
                status = task_mgr_thdpool_instruments();
                break;

            case '3':
                DBG(DBG_WARNING, "task_daemon_run: Not used.\n");
                break;

            case '4':
                DBG(DBG_WARNING, "task_daemon_run: Not used.\n");
                break;

            case '5':
                DBG(DBG_WARNING, "task_daemon_run: Not used.\n");
                break;

            case 't':
                status = task_mgr_daemon_dynamic(hdl);
                break;

            case 'q':
                DBG(DBG_WARNING, "task_daemon_run: Quit from this task daemon.\n");
                glb_tsk_mgr_exit = 1;
                break;

            default:
                DBG(DBG_WARNING, "task_daemon_run: Invalid choice.\n");
                break;
        }
#if 0
        status = task_check_msg(hdl->m_tsklists[TASK_MGR_TSK0]->m_task, &msg, MSG_TYPE_CMD);

        if (OSA_ISERROR(status)) {
            continue;
        }

        status |= tsk_mgr_daemon_process_msg(hdl, hdl->m_tsklists[TASK_MGR_TSK0]->m_task, msg);

        status |= task_ack_free_msg(hdl->m_tsklists[TASK_MGR_TSK0]->m_task, msg);

        if (!OSA_ISERROR(task_get_state(hdl->m_tsklists[TASK_MGR_TSK0]->m_task, &state))
                && state == TASK_STATE_EXIT) {
            break;
        }
#endif
    }

    return OSA_SOK;
}

status_t task_daemon_stop(task_mgr_handle hdl)
{
    status_t status = OSA_SOK;

    status = task_mgr_broadcast(TASK_CMD_EXIT, NULL, 0, MSG_FLAGS_WAIT_ACK);

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

    //status = task_create("TSK_MGR_MAIN", NULL, 0, 0, 0, NULL, &hdl->m_cur_tsk);

    for (i = 0; i < hdl->m_cur_cnt; i++) {
        status = task_mgr_register(hdl->m_tsklists[i]); 

        OSA_assert(OSA_SOK == status);
    }

    return status;
}

status_t task_daemon_task_delete(task_mgr_handle hdl)
{
    int i;
    status_t status = OSA_SOK;

    for (i = 0; i < hdl->m_cur_cnt; i++) {

        status = task_mgr_unregister(hdl->m_tsklists[i]);

        OSA_assert(OSA_SOK == status);
    }

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

static int
task_mgr_daemon_print_menu(task_mgr_handle hdl)
{
    int i;
    static char input[32];

    for (i = 0; i < OSA_ARRAYSIZE(menu); i++) {
        fprintf(stdout, "%s", menu[i]);
    }

    fprintf(stdout, "Please enter the choice: ");

    fgets(input, sizeof(input) - 1, stdin);

    return input[0];
}

static int
task_mgr_daemon_print_submenu(void)
{
    int i;
    static char input[32];

    for (i = 0; i < OSA_ARRAYSIZE(submenu); i++) {
        fprintf(stdout, "%s", submenu[i]);
    }

    fprintf(stdout, "Please enter the choice: ");

    fgets(input, sizeof(input) - 1, stdin);

    return input[0];
}

static status_t
task_mgr_daemon_dynamic(task_mgr_handle hdl)
{
    int ch;

    ch = task_mgr_daemon_print_submenu();

    switch(ch)
    {
        case 'c':
            task_mgr_register(hdl->m_tsklists[TASK_MGR_TSK5]);
            break;

        case 'd':
            task_mgr_unregister(hdl->m_tsklists[TASK_MGR_TSK5]);
            break;

        case 's':
            task_mgr_start(hdl->m_tsklists[TASK_MGR_TSK5]);
            break;

        case 'p':
            task_mgr_stop(hdl->m_tsklists[TASK_MGR_TSK5]);
            break;

        default:
            break;
    }
}

#if defined(__cplusplus)
}
#endif
