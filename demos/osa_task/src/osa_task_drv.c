/** ============================================================================
 *
 *  Copyright (C), 1987 - 2013, xiong-kaifang Tech. Co.,, Ltd.
 *
 *  @File name:	osa_task_drv.c
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
 *  xiong-kaifang   2015-07-23     v1.0	        write this module.
 *
 *
 *  ============================================================================
 */

/*  --------------------- Include system headers ---------------------------- */
#include <signal.h>

/*  --------------------- Include user headers   ---------------------------- */
#include "osa.h"
#include "osa_task.h"
#include "osa_timer.h"
#include "osa_debugger.h"

#include "osa_task_drv.h"

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
struct __osa_task_object_t
{
    task_mgr_params_t       m_params;

    int                     m_event1_id;
    osa_event_t             m_event1;
    int                     m_event2_id;
    osa_event_t             m_event2;

    task_object_t           m_tsk_obj;
};

typedef struct __osa_task_object_t   osa_task_object_t;

static osa_task_object_t glb_osa_task_obj;

const char * const GT_NAME  = "osa_task_drv";

const char * const TSK_NAME = "OSA_TASK_DRV";
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
static status_t system_init(osa_task_object_t *pobj);
static status_t system_deinit(osa_task_object_t *pobj);

static status_t task_daemon_init(osa_task_object_t *pobj);
static status_t task_daemon_start(osa_task_object_t *pobj);
static status_t task_daemon_run(osa_task_object_t *pobj);
static status_t task_daemon_stop(osa_task_object_t *pobj);
static status_t task_daemon_exit(osa_task_object_t *pobj);

static status_t
osa_task_daemon_process_msg(osa_task_object_t *pobj, task_t tsk, msg_t *msg);

status_t osa_task_daemon(osa_task_object_t *pobj);

static void osa_task_daemon_signal_handler(int sig)
{
    if (sig == SIGINT) {
        fprintf(stderr, "SIGINT signal caught, system shutdown now...\n");
        task_mgr_synchronize(&glb_osa_task_obj.m_tsk_obj, TASK_CMD_EXIT, NULL, 0, 0);
    } else if (sig == SIGUSR1) {
        task_mgr_synchronize(&glb_osa_task_obj.m_tsk_obj, OSA_TASK_CMD_PRINT_TSKSTATS, NULL, 0, 0);
    } else if (sig == SIGUSR2) {
        task_mgr_synchronize(&glb_osa_task_obj.m_tsk_obj, OSA_TASK_CMD_PRINT_THDSTATS, NULL, 0, 0);
    } else {
        fprintf(stderr, "Invalid signal caught\n");
    }
}

static status_t __osa_task_event1_handler(void *);

static status_t __osa_task_event2_handler(void *);

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

    system_init(&glb_osa_task_obj);

    status = osa_task_daemon(&glb_osa_task_obj);

    system_deinit(&glb_osa_task_obj);

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
status_t system_init(osa_task_object_t *pobj)
{
    status_t status = OSA_SOK;

    /*
     *  Initialize debug module.
     */
    osa_debugger_prm_t debug_prm;

    debug_prm.m_debug_level = DBG_INFO;
    debug_prm.m_out = stderr;
    debug_prm.m_name = (char *)TSK_NAME;
    debug_prm.m_folder = NULL;

    osa_debugger_init(&debug_prm);
    osa_debugger_setlevel(DBG_DETAILED);
    //osa_debugger_setlevel(DBG_INFO);

    /*
     *  Initialize tasklist.
     */
    DBG(DBG_INFO, GT_NAME, "Initiailze task manager.\n");
    pobj->m_params.m_tsk_mgr_prm.m_min_tsk_nums = 3;
    pobj->m_params.m_tsk_mgr_prm.m_max_tsk_nums = 10;
    pobj->m_params.m_tsk_mgr_prm.m_max_linger   = 10 * 60;
    status |= task_mgr_init(&pobj->m_params.m_tsk_mgr_prm);

    /*
     *  Initialize osa timer.
     */
    DBG(DBG_INFO, GT_NAME, "Initiailze osa timer.\n");
    status |= osa_timer_init();

    OSA_assert(OSA_SOK == status);

    return status;
}

status_t system_deinit(osa_task_object_t *pobj)
{
    status_t status = OSA_SOK;

    /*
     *  Finalize osa timer.
     */
    DBG(DBG_INFO, GT_NAME, "De-initiailze osa timer.\n");
    status |= osa_timer_deinit();

    /*
     *  Finialize tasklist.
     */
    DBG(DBG_INFO, GT_NAME, "De-initialize task manager.\n");
    status |= task_mgr_deinit();

    /*
     *  Finalize debug module.
     */
    DBG(DBG_INFO, GT_NAME, "De-initialize osa debug module.\n");
    osa_debugger_deinit();

    OSA_assert(OSA_SOK == status);

    return status;
}

status_t osa_task_daemon(osa_task_object_t *pobj)
{
    status_t status = OSA_SOK;

    status = task_daemon_init(pobj);
    if (OSA_ISERROR(status)) {
        return status;
    }

    status = task_daemon_start(pobj);
    if (OSA_ISERROR(status)) {
        return status;
    }

    status = task_daemon_run(pobj);
    if (OSA_ISERROR(status)) {
        return status;
    }

    status = task_daemon_stop(pobj);
    status = task_daemon_exit(pobj);

    return status;
}

status_t task_daemon_init(osa_task_object_t *pobj)
{
    status_t status = OSA_SOK;

    /*
     *  Register osa task driver task.
     */
    pobj->m_tsk_obj.m_name = (char *)TSK_NAME;
    pobj->m_tsk_obj.m_main = NULL;
    pobj->m_tsk_obj.m_find = NULL;

    status = task_mgr_register(&pobj->m_tsk_obj);

    return status;
}

status_t task_daemon_start(osa_task_object_t *pobj)
{
    status_t status = OSA_SOK;

    return status;
}

status_t task_daemon_run(osa_task_object_t *pobj)
{
    int ch;
    msg_t *msg = NULL;
    status_t status = OSA_SOK;
    task_state_t state = TASK_STATE_PROC;

    signal(SIGINT , osa_task_daemon_signal_handler);
    signal(SIGUSR1, osa_task_daemon_signal_handler);
    signal(SIGUSR2, osa_task_daemon_signal_handler);

    while (1) {
        status = task_wait_msg(pobj->m_tsk_obj.m_task, &msg, MSG_TYPE_CMD);

        if (OSA_ISERROR(status)) {
            continue;
        }

        status |= osa_task_daemon_process_msg(pobj, pobj->m_tsk_obj.m_task, msg);

        status |= task_ack_free_msg(pobj->m_tsk_obj.m_task, msg);

        if (!OSA_ISERROR(task_get_state(pobj->m_tsk_obj.m_task, &state))
                && state == TASK_STATE_EXIT) {
            break;
        }
    }

    return OSA_SOK;
}

status_t task_daemon_stop(osa_task_object_t *pobj)
{
    int i;
    status_t status = OSA_SOK;

    status |= task_mgr_unregister(&pobj->m_tsk_obj);

    return status;
}

status_t task_daemon_exit(osa_task_object_t *pobj)
{
    status_t status = OSA_SOK;
    
    return status;
}

static status_t
osa_task_daemon_process_msg(osa_task_object_t *pobj, task_t tsk, msg_t *msg)
{
    status_t status = OSA_SOK;

    switch(msg_get_cmd(msg))
    {
        case TASK_CMD_EXIT:
            DBG(DBG_INFO, GT_NAME, "Exit cmd received, exit...\n");
            task_set_state(tsk, TASK_STATE_EXIT);
            break;

        case OSA_TASK_CMD_PRINT_TSKSTATS:
            status = task_mgr_task_instruments();
            break;

        case OSA_TASK_CMD_PRINT_THDSTATS:
            status = task_mgr_thdpool_instruments();
            break;

        default:
            break;
    }

    return status;
}

#if defined(__cplusplus)
}
#endif
