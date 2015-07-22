/** ============================================================================
 *
 *  osa_console_test.c
 *
 *  Author     : xkf
 *
 *  Date       : May 21, 2013
 *
 *  Description: 
 *  ============================================================================
 */

/*  --------------------- Include system headers ---------------------------- */
#include <signal.h>

/*  --------------------- Include user headers   ---------------------------- */
//#include "module_cmds.h"
#include "vcs_cmds.h"

#include "osa.h"
#include "dlist.h"
#include "osa_task_mgr.h"
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
struct __osa_console_object_t;
typedef struct __osa_console_object_t osa_console_object_t;
struct __osa_console_object_t
{
    char            m_name[32];
    task_object_t   m_task_obj;
    dlist_t         m_cmds_list;
};

/*
 *  --------------------- Global variable definition ---------------------------
 */

/** ----------------------------------------------------------------------------
 *  @Name:          Variable name
 *
 *  @Description:   Description of the variable.
 * -----------------------------------------------------------------------------
 */
static int glb_tsk_mgr_exit = 0;

static osa_console_object_t glb_console_obj;

static const char * const GT_NAME = "main";
/*
 *  --------------------- Local function forward declaration -------------------
 */

static void osa_console_test_signal_handler(int sig)
{
    if (sig == SIGINT) {
        task_mgr_synchronize(&glb_console_obj.m_task_obj, TASK_CMD_EXIT, NULL, 0, 0);
        fprintf(stderr, "SIGINT signal caught, system shutdown now...\n");
    } else {
        fprintf(stderr, "Invalid signal caught\n");
    }
}

/*
 *  --------------------- Public function definition ---------------------------
 */


int main(int argc, char *argv[])
{
    status_t status;

    system_init(&glb_console_obj);

    status = tsk_mgr_daemon(&glb_console_obj);

    system_deinit(&glb_console_obj);

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
status_t system_init(osa_console_object_t *pobj)
{
    status_t status = OSA_SOK;

    /*
     *  Initialize debug module.
     */
    osa_debugger_prm_t debug_prm;

    debug_prm.m_debug_level = DBG_INFO;
    debug_prm.m_out = stderr;
    debug_prm.m_name = "osa_console";
    debug_prm.m_folder = "/home/xiaoxiong/workspace/osa.base";

    osa_debugger_init(&debug_prm);
    //debugger_setlevel(DBG_DETAILED);
    //osa_debugger_setlevel(DBG_DETAILED);
    osa_debugger_setlevel(DBG_INFO);

    /*
     *  Initialize tasklist.
     */
    task_mgr_prm_t tsk_mgr_prm;
    fprintf(stderr, "system_init: Initialize task manager.\n");
    tsk_mgr_prm.m_msg_cnt = 10;
    tsk_mgr_prm.m_tsk_cnt = 10;
    status |= task_mgr_init(&tsk_mgr_prm);

    /*
     *  Initialize osa timer.
     */
    status |= osa_timer_init();

    OSA_assert(OSA_SOK == status);

    DBG(DBG_INFO, GT_NAME, "system initialized.\n");

    return status;
}

status_t system_deinit(osa_console_object_t *pobj)
{
    status_t status = OSA_SOK;

    DBG(DBG_INFO, GT_NAME, "system de-initialized ...\n");

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
    DBG(DBG_INFO, GT_NAME, "system de-initialized.\n");

    osa_debugger_deinit();

    OSA_assert(OSA_SOK == status);

    return status;
}

status_t tsk_mgr_daemon(osa_console_object_t *pobj)
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

static bool
__osa_console_cmd_find_match_fxn(dlist_element_t *elem, void *data)
{
    return ((((osa_cmd_t *)elem)->m_id) == (((unsigned int)data) - 1));
}

static status_t __osa_console_cmd_find(void *ud, unsigned short cmd, void **ptsk)
{
    status_t status = OSA_ENOENT;
    osa_cmd_t *pcmd = NULL;
    osa_console_object_t *pobj = (osa_console_object_t *)ud;

    status = dlist_search_element(&pobj->m_cmds_list, (void *)(cmd + 1),
             (dlist_element_t **)&pcmd, __osa_console_cmd_find_match_fxn);

#if 0
    if (!OSA_ISERROR(status)) {
        (*ptsk) = &pobj->m_task_obj;
    }
#endif

    return status;
}

status_t task_daemon_init(osa_console_object_t *pobj)
{
    int i;
    status_t status = OSA_SOK;

    status |= dlist_init(&pobj->m_cmds_list);

    //for (i = 0; i < OSA_ARRAYSIZE(glb_module_cmds); i++) {
    for (i = 0; i < OSA_ARRAYSIZE(glb_vcs_cmds); i++) {
        status |= dlist_initialize_element((dlist_element_t *)&glb_vcs_cmds[i]);

        status |= dlist_put_tail(&pobj->m_cmds_list, (dlist_element_t *)&glb_vcs_cmds[i]);
    }

    snprintf((char *)pobj->m_name, sizeof(pobj->m_name), "%s", "MAIN_TSK");
    pobj->m_task_obj.m_name = (unsigned char *)pobj->m_name;
    pobj->m_task_obj.m_main = NULL;
    pobj->m_task_obj.m_find = __osa_console_cmd_find;
    pobj->m_task_obj.m_pri = 0;
    pobj->m_task_obj.m_stack_size = 0;
    pobj->m_task_obj.m_init_state = 0;
    pobj->m_task_obj.m_userdata = (void *)pobj;
    pobj->m_task_obj.m_task = TASK_INVALID_TSK;

    status = task_mgr_register(&pobj->m_task_obj);

    status = osa_console_init(NULL);

    return status;
}

status_t task_daemon_start(osa_console_object_t *pobj)
{
    Bool init_succeeded = TRUE;
    status_t status = OSA_SOK;

    return status;
}

static status_t
__vcs_daemon_do_process(osa_console_object_t *pobj, task_t tsk, msg_t *msg)
{
    int cmd = 0;
    status_t status = OSA_SOK;
    osa_cmd_req_t *preq = NULL;

    preq = (osa_cmd_req_t *)msg_get_payload_ptr(msg);

    switch(msg_get_cmd(msg))
    {
        case TASK_CMD_EXIT:
            status |= task_set_state(tsk, TASK_STATE_EXIT);
            break;

        case TASK_CMD_PROC:
            status |= task_set_state(tsk, TASK_STATE_PROC);
            break;

#if 0
#define VCS_VENC_CMD_SET_BITRATE_ARG_id                    0
#define VCS_VENC_CMD_SET_BITRATE_ARG_b                     1
#define VCS_VDEC_CMD_CREATE_CHANNEL_ARG_cid                0
#define VCS_VDEC_CMD_CREATE_CHANNEL_ARG_wid                1
#define VCS_VDEC_CMD_CREATE_CHANNEL_ARG_dev                2
#define VCS_VDEC_CMD_CREATE_CHANNEL_ARG_w                  3
#define VCS_VDEC_CMD_CREATE_CHANNEL_ARG_h                  4
#define VCS_VDEC_CMD_CREATE_CHANNEL_ARG_r                  5
#define VCS_VDEC_CMD_CREATE_CHANNEL_ARG_b                  6
#define VCS_VDEC_CMD_CREATE_CHANNEL_ARG_t                  7
#define VCS_VDEC_CMD_CREATE_CHANNEL_ARG_port               8
#define VCS_VDEC_CMD_CREATE_CHANNEL_ARG_ip                 9
#define VCS_VDEC_CMD_CREATE_CHANNEL_ARG_url               10
#define VCS_VDEC_CMD_DELETE_CHANNEL_ARG_id                 0
#define VCS_VDEC_CMD_DISABLE_CHANNEL_ARG_id                0
#define VCS_VDEC_CMD_ENABLE_CHANNEL_ARG_id                 0
#define VCS_VDEC_CMD_CHANGE_SOURCE_ARG_id                  0
#define VCS_VDEC_CMD_CHANGE_SOURCE_ARG_t                   1
#define VCS_VDEC_CMD_CHANGE_SOURCE_ARG_port                2
#define VCS_VDEC_CMD_CHANGE_SOURCE_ARG_ip                  3
#define VCS_VDEC_CMD_CHANGE_SOURCE_ARG_url                 4
#endif
        /* Commands from osa console */
        case VCS_VDEC_CMD_CREATE_CHANNEL:
            DBG(DBG_INFO, GT_NAME, "createchn: cid=%d,win=%d,dev=%d,w=%d,h=%d,r=%d,b=%d,t=%d,port=%d,ip=%s,url=%s.\n",
                    preq->m_args[VCS_VDEC_CMD_CREATE_CHANNEL_ARG_cid],
                    preq->m_args[VCS_VDEC_CMD_CREATE_CHANNEL_ARG_wid],
                    preq->m_args[VCS_VDEC_CMD_CREATE_CHANNEL_ARG_dev],
                    preq->m_args[VCS_VDEC_CMD_CREATE_CHANNEL_ARG_w],
                    preq->m_args[VCS_VDEC_CMD_CREATE_CHANNEL_ARG_h],
                    preq->m_args[VCS_VDEC_CMD_CREATE_CHANNEL_ARG_r],
                    preq->m_args[VCS_VDEC_CMD_CREATE_CHANNEL_ARG_b],
                    preq->m_args[VCS_VDEC_CMD_CREATE_CHANNEL_ARG_t],
                    preq->m_args[VCS_VDEC_CMD_CREATE_CHANNEL_ARG_port],
                    preq->m_args[VCS_VDEC_CMD_CREATE_CHANNEL_ARG_ip],
                    preq->m_args[VCS_VDEC_CMD_CREATE_CHANNEL_ARG_url]
                    );

            preq->m_status = OSA_SOK;
            snprintf(preq->m_msgs, sizeof(preq->m_msgs) - 1, "Success.");

            break;

        case VCS_VDEC_CMD_DELETE_CHANNEL:
            DBG(DBG_INFO, GT_NAME, "deletechn: chn_id = %d.\n", preq->m_args[VCS_VDEC_CMD_DELETE_CHANNEL_ARG_id]);
            preq->m_status = OSA_SOK;
            snprintf(preq->m_msgs, sizeof(preq->m_msgs) - 1, "Success.");
            break;

        case VCS_VENC_CMD_SET_BITRATE:
            DBG(DBG_INFO, GT_NAME, "setbitrate: chn_id = %d, bitrate = %d.\n",
                    preq->m_args[VCS_VENC_CMD_SET_BITRATE_ARG_id], preq->m_args[VCS_VENC_CMD_SET_BITRATE_ARG_b]);
            preq->m_status = OSA_SOK;
            //snprintf(preq->m_msgs, sizeof(preq->m_msgs) - 1, "Success.");
            break;

        case VCS_VDEV_CMD_GET_NETPARAMS:
            DBG(DBG_INFO, GT_NAME, "getnetparams.\n");
            sprintf(preq->m_msgs, "%d %d %d", 1234, 2345, 3456);
            break;

        default:
            break;
    }

    msg_set_status(msg, status);

    return status;
}

status_t task_daemon_run(osa_console_object_t *pobj)
{
    msg_t *msg = NULL;
    status_t status   = OSA_SOK;
    task_state_t state = TASK_STATE_PROC;

    glb_tsk_mgr_exit = 0;
    signal(SIGINT, osa_console_test_signal_handler);

    while (!glb_tsk_mgr_exit) {

        status = task_wait_msg(pobj->m_task_obj.m_task, &msg, MSG_TYPE_CMD);

        if (OSA_ISERROR(status)) {
            continue;
        }

        status |= __vcs_daemon_do_process(pobj, pobj->m_task_obj.m_task, msg);

        status |= task_ack_free_msg(pobj->m_task_obj.m_task, msg);

        if (!OSA_ISERROR(task_get_state(pobj->m_task_obj.m_task, &state))
                && state == TASK_STATE_EXIT) {
            break;
        }
    }

    return status;
}

status_t task_daemon_stop(osa_console_object_t *pobj)
{
    status_t status = OSA_SOK;

    return status;
}

status_t task_daemon_exit(osa_console_object_t *pobj)
{
    status_t status = OSA_SOK;
    
    DBG(DBG_INFO, GT_NAME, "de-initialized ...\n");

    status = osa_console_deinit();

    status |= task_mgr_unregister(&pobj->m_task_obj);

    DBG(DBG_INFO, GT_NAME, "de-initialized.\n");

    return status;
}

#if defined(__cplusplus)
}
#endif
