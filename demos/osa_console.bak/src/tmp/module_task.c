/** ============================================================================
 *
 *  Copyright (C), 1987 - 2014, xiong-kaifang Tech. Co.,, Ltd.
 *
 *  @File name:	module_task.c
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2014-01-08
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
 *  xiong-kaifang   2014-01-04     v1.0	        write this module.
 *
 *
 *  ============================================================================
 */

/*  --------------------- Include system headers ---------------------------- */
#include <signal.h>

/*  --------------------- Include user headers   ---------------------------- */
#include "osa.h"
#include "osa_debugger.h"

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
typedef void * task_mgr_handle;
/*
 *  --------------------- Global variable definition ---------------------------
 */

/** ----------------------------------------------------------------------------
 *  @Name:          Variable name
 *
 *  @Description:   Description of the variable.
 * -----------------------------------------------------------------------------
 */
static int glb_tsk_exit = 0;

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
static void debug_test_signal_handler(int sig)
{
    if (sig == SIGINT) {
        fprintf(stderr, "SIGINT signal caught, system shutdown now...\n");
        glb_tsk_mgr_exit = 1;
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

    system_init(hdl);

    status = tsk_mgr_daemon(hdl);

    system_deinit(hdl);

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
status_t system_init(void)
{
    status_t status = OSA_SOK;

    /*
     *  Initialize debug module.
     */
    osa_debugger_prm_t debug_prm;

    debug_prm.m_debug_level = DBG_INFO;
    debug_prm.m_out = stderr;
    debug_prm.m_name = "main";
    debug_prm.m_folder = "/home/xiaoxiong/workspace/osa.base";

    osa_debugger_init(&debug_prm);
    //debugger_setlevel(DBG_DETAILED);
    osa_debugger_setlevel(DBG_INFO);

    DBG(DBG_INFO, "main", "system initialized.\n");

    return status;
}

status_t system_deinit(void)
{
    status_t status = OSA_SOK;

    DBG(DBG_INFO, "main", "system de-initialized ...\n");

    /*
     *  Finalize osa timer.
     */
    status |= osa_timer_deinit();

    /*
     *  Finialize tasklist.
     */
    status |= task_mgr_deinit();

    /*
     *  Finalize debug module.
     */
    DBG(DBG_INFO, "main", "system de-initialized.\n");

    osa_debugger_deinit();

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

    DBG(DBG_INFO, "main", "codecs system initialized ...\n");

    usleep(500000);

    DBG(DBG_INFO, "main", "codecs system initialized.\n");

    return status;
}

status_t task_daemon_start(task_mgr_handle hdl)
{
    Bool init_succeeded = TRUE;
    status_t status = OSA_SOK;

    return status;
}

status_t task_daemon_run(task_mgr_handle hdl)
{
    int ch;
    status_t status = OSA_SOK;

    glb_tsk_mgr_exit = 0;
    signal(SIGINT, debug_test_signal_handler);

    while (!glb_tsk_mgr_exit) {

        sleep(1);

        DBG(DBG_INFO, "main", "tsmux: tsmux task is running ...\n");
    }

    return OSA_SOK;
}

status_t task_daemon_stop(task_mgr_handle hdl)
{
    status_t status = OSA_SOK;

    return status;
}

status_t task_daemon_exit(task_mgr_handle hdl)
{
    status_t status = OSA_SOK;
    
    DBG(DBG_INFO, "main", "codecs system de-initialized ...\n");

    usleep(500000);

    DBG(DBG_INFO, "main", "codecs system de-initialized.\n");

    return status;
}

#if defined(__cplusplus)
}
#endif
