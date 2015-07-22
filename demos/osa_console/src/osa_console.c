/** ============================================================================
 *
 *  osa_console.c
 *
 *  Author     : xkf
 *
 *  Date       : May 21, 2013
 *
 *  Description: 
 *  ============================================================================
 */

/*  --------------------- Include system headers ---------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/*  --------------------- Include user headers   ---------------------------- */
#include "osa_console.h"
//#include "module_cmds.h"
#include "vcs_cmds.h"

#include "osa.h"
#include "dlist.h"
#include "osa_cmd.h"
#include "osa_mutex.h"
#include "osa_timer.h"
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
#define OSA_CONSOLE_SESSIONS_MAX        (4)

#define OSA_SESSION_LINEBUF_SIZE        (1024)

#define OSA_CONSOLE_TCP_PORT            (8888)

#define OSA_CONSOLE_CMD_PROC            (0x6000) 

#define OSA_CONSOLE_CMD_FREE_SESSION    (0x6001) 

#define OSA_CONSOLE_CRITICAL_ENTER()        \
    do {                                    \
        mutex_lock(&glb_osa_console_mutex); \
    } while (0)

#define OSA_CONSOLE_CRITICAL_LEAVE()        \
    do {                                    \
        mutex_unlock(&glb_osa_console_mutex);\
    } while (0)

#define TAG_PREFIX			            '-'

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
struct __osa_console_session_t;
typedef struct __osa_console_session_t osa_console_session_t;
struct __osa_console_session_t
{
    unsigned int    m_reserved[2];
    unsigned int    m_id;

    unsigned int    m_connected;
    struct sockaddr_in
                    m_addr;
    int             m_sockfd;
    unsigned int    m_addr_be;

    osa_cmd_req_t   m_req;

    unsigned char   m_name[32];
    task_object_t   m_tsk_obj;

    char            m_linebuf[OSA_SESSION_LINEBUF_SIZE];
};

struct __osa_console_object_t;
typedef struct __osa_console_object_t osa_console_object_t;
struct __osa_console_object_t
{
    unsigned int    m_initialized;

    int             m_listen_fd;
    int             m_listen_port;

    struct sockaddr_in
                    m_addr;

    mutex_t         m_mutex;
    osa_console_session_t
                    m_sessions[OSA_CONSOLE_SESSIONS_MAX];
    dlist_t         m_free_list;
    dlist_t         m_busy_list;

    int             m_id;
    osa_event_t     m_event;

    unsigned char   m_name[32];
    task_object_t   m_tsk_obj;
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
OSA_DECLARE_AND_INIT_MUTEX(glb_osa_console_mutex);

static unsigned int glb_cur_init = 0;

static osa_console_object_t glb_osa_console_obj;

static const char * const   GT_NAME = "osa_console";
/*
 *  --------------------- Local function forward declaration -------------------
 */

static status_t __osa_console_init(osa_console_object_t *pobj, const osa_console_prm_t *prm);

static status_t __osa_console_external_main(void *ud, task_t tsk, msg_t **msg);

static status_t __osa_console_event_handler(void *ud);

static status_t __osa_console_deinit(osa_console_object_t *pobj);
/*
 *  --------------------- Public function definition ---------------------------
 */

status_t osa_console_init(const osa_console_prm_t *prm)
{
    status_t status = OSA_SOK;
    osa_console_object_t * pobj = &glb_osa_console_obj;

    OSA_CONSOLE_CRITICAL_ENTER();

    if (glb_cur_init++ == 0) {
        status = __osa_console_init(pobj, prm);
    }

    OSA_CONSOLE_CRITICAL_LEAVE();

    return status;
}

status_t osa_console_deinit(void)
{
    status_t status = OSA_SOK;
    osa_console_object_t * pobj = &glb_osa_console_obj;

    OSA_CONSOLE_CRITICAL_ENTER();

    if (--glb_cur_init == 0) {
        status = __osa_console_deinit(pobj);
    }

    OSA_CONSOLE_CRITICAL_LEAVE();

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
static status_t __osa_console_init(osa_console_object_t *pobj, const osa_console_prm_t *prm)
{
    status_t status = OSA_SOK;

    /* Initialize osa console task */
    snprintf(pobj->m_name, sizeof (pobj->m_name) - 1, "%s", "OSA_CONSOLE_TASK");

    pobj->m_tsk_obj.m_name = pobj->m_name;
    pobj->m_tsk_obj.m_main = __osa_console_external_main;
    pobj->m_tsk_obj.m_find = NULL;
    pobj->m_tsk_obj.m_pri = 0;
    pobj->m_tsk_obj.m_stack_size = 0;
    pobj->m_tsk_obj.m_init_state = 0;
    pobj->m_tsk_obj.m_userdata = (void *)pobj;
    pobj->m_tsk_obj.m_task = TASK_INVALID_TSK;

    /* Register osa console task */
    task_object_t *ptsk = &pobj->m_tsk_obj;
    status = task_mgr_register(ptsk);
    if (OSA_ISERROR(status)) {
        return status;
    }

    /* Start osa console task */
    status = task_mgr_start(ptsk);
    if (OSA_ISERROR(status)) {
        status |= task_mgr_unregister(ptsk);
        return status;
    }

    /* Register server event */
    pobj->m_event.m_fxn    = __osa_console_event_handler;
    pobj->m_event.m_ud     = (void *)pobj;
    pobj->m_event.m_delete = FALSE;

    status = osa_timer_register(&pobj->m_id, 2000/* ms */, &pobj->m_event);

    DBG(DBG_INFO, GT_NAME, "osa console initialized.\n");

    OSA_assert(OSA_SOK == status);

    return status;
}

static status_t __osa_console_deinit(osa_console_object_t *pobj)
{
    status_t status = OSA_SOK;
    task_object_t *ptsk = NULL;
    osa_console_session_t *pss = NULL;

    status |= osa_timer_unregister(pobj->m_id);

    /* Free the session already connected */
    status = dlist_first(&pobj->m_busy_list, (dlist_element_t **)&pss);
    while (!OSA_ISERROR(status) && pss != NULL) {

        status = task_mgr_synchronize(&pobj->m_tsk_obj,
                                      OSA_CONSOLE_CMD_FREE_SESSION, (void *)pss->m_id, sizeof(pss->m_id), MSG_FLAGS_WAIT_ACK);

        status = dlist_first(&pobj->m_busy_list, (dlist_element_t **)&pss);
    }

    ptsk    = &pobj->m_tsk_obj;
    status |= task_mgr_stop(ptsk);

    status |= task_mgr_unregister(ptsk);

    return status;
}

static status_t __osa_console_do_initialize(osa_console_object_t *pobj)
{
    int i;
    status_t status = OSA_SOK;

    osa_console_session_t * pss = NULL;

    //memset(pobj, 0, sizeof(*pobj));

    status |= mutex_create(&pobj->m_mutex);

    status |= dlist_init(&pobj->m_free_list);
    status |= dlist_init(&pobj->m_busy_list);

    OSA_assert(OSA_SOK == status)

    for (i = 0; i < OSA_ARRAYSIZE(pobj->m_sessions); i++) {
        pss = &pobj->m_sessions[i];
        pss->m_id = i;
        pss->m_sockfd = -1;

        status  = dlist_initialize_element((dlist_element_t *)pss);
        status |= dlist_put_tail(&pobj->m_free_list, (dlist_element_t *)pss);
    }

    pobj->m_listen_port = OSA_CONSOLE_TCP_PORT;

    pobj->m_addr.sin_family = AF_INET;
    pobj->m_addr.sin_addr.s_addr = INADDR_ANY;
    pobj->m_addr.sin_port = htons(pobj->m_listen_port);

    /* Initialize server socket */
    pobj->m_listen_fd = ssocket_init(&pobj->m_addr, sizeof(pobj->m_addr));

    if (pobj->m_listen_fd < 0) {
        mutex_delete(&pobj->m_mutex);
        return OSA_EFAIL;
    }

    /* Set the server socket no blocking */
    socket_set_noblocking(pobj->m_listen_fd);

    /* Register osa console task */
    pobj->m_initialized = TRUE;

    return status;
}

static status_t __osa_console_create_session(osa_console_session_t *pss);

static status_t __osa_console_do_process(osa_console_object_t *pobj)
{
    int fd;
    status_t status = OSA_SOK;
    struct sockaddr_in client_addr;
    socklen_t socklen = sizeof(struct sockaddr_in);
    osa_console_session_t *pss = NULL;
    osa_console_session_t *pcurr_ss = NULL;
    osa_console_session_t *pnext_ss = NULL;

#if 0
    /* Free the connection disconnected */
    status = dlist_first(&pobj->m_busy_list, (dlist_element_t **)&pcurr_ss);
    while (!OSA_ISERROR(status) && pcurr_ss != NULL) {

        status = dlist_next(&pobj->m_busy_list, (dlist_element_t *)pcurr_ss, (dlist_element_t **)&pnext_ss);

        if (!pcurr_ss->m_connected) {
            DBG(DBG_INFO, "osa console", "Free session %s.\n", pcurr_ss->m_name);
            status |= dlist_remove_element(&pobj->m_busy_list, (dlist_element_t *)pcurr_ss);
            status |= dlist_put_tail(&pobj->m_free_list, (dlist_element_t *)pcurr_ss);
        }

        pcurr_ss = pnext_ss;
    }
#endif

    /* Accept clinet connection */
    fd = accept(pobj->m_listen_fd, (struct sockaddr *)&client_addr, &socklen);

    if (fd > 0) {

        status = dlist_get_head(&pobj->m_free_list, (dlist_element_t **)&pss);

        if (!OSA_ISERROR(status) && pss != NULL) {
            pss->m_sockfd = fd;
            pss->m_addr  = client_addr;
            pss->m_connected = TRUE;

            status = __osa_console_create_session(pss);

            if (!OSA_ISERROR(status)) {
                status = dlist_initialize_element((dlist_element_t *)pss);
                status = dlist_put_tail(&pobj->m_busy_list, (dlist_element_t *)pss);

                DBG(DBG_INFO, GT_NAME, "new connection: fd = %d, ip: %d.%d.%d.%d.\n",
                        fd, 
                        (client_addr.sin_addr.s_addr >> 0 ) & 0xff,
                        (client_addr.sin_addr.s_addr >> 8 ) & 0xff,
                        (client_addr.sin_addr.s_addr >> 16) & 0xff,
                        (client_addr.sin_addr.s_addr >> 24) & 0xff
                   );
            } else {
                //memset(pss, 0, sizeof(*pss));
                pss->m_sockfd = -1;
                status = dlist_initialize_element((dlist_element_t *)pss);
                status = dlist_put_tail(&pobj->m_free_list, (dlist_element_t *)pss);
                close(fd);
            }

        } else {
            close(fd);
        }
    }

    return status;
}

static status_t __osa_console_event_handler(void *ud)
{
    return task_mgr_synchronize(&((osa_console_object_t *)ud)->m_tsk_obj, 
                                OSA_CONSOLE_CMD_PROC, NULL, 0, 0);
}

static bool
__osa_console_session_find_match_fxn(dlist_element_t *elem, void *data)
{
    return ((((osa_console_session_t *)elem)->m_id) == (((unsigned int)data) - 1));
}

static bool
__osa_console_session_comp_match_fxn(dlist_element_t *elem, void *data)
{
    return ((((osa_console_session_t *)elem)->m_id) >= (((unsigned int)data) - 1));
}

static status_t __osa_console_delete_session(osa_console_session_t *pss);

static status_t __osa_console_free_session(osa_console_object_t *pobj, unsigned int id)
{
    status_t status = OSA_ENOENT;
    osa_console_session_t *pcurr_ss = NULL;
    osa_console_session_t *pnext_ss = NULL;

    status = dlist_search_element(&pobj->m_busy_list, (void *)(id + 1),
             (dlist_element_t **)&pcurr_ss, __osa_console_session_find_match_fxn);

    if (!OSA_ISERROR(status)) {
        status |= __osa_console_delete_session(pcurr_ss);

        status |= dlist_remove_element(&pobj->m_busy_list, (dlist_element_t *)pcurr_ss);

        status = dlist_search_element(&pobj->m_free_list, (void *)(pcurr_ss->m_id + 1),
                (dlist_element_t **)&pnext_ss, __osa_console_session_comp_match_fxn);

        if (!OSA_ISERROR(status) && pnext_ss != NULL) {
            status = dlist_insert_before(&pobj->m_free_list, (dlist_element_t *)pcurr_ss, (dlist_element_t *)pnext_ss);
        } else {
            status = dlist_put_tail(&pobj->m_free_list, (dlist_element_t *)pcurr_ss);
        }
    }

    return status;
}

static status_t __osa_console_do_deinitialize(osa_console_object_t *pobj)
{
    status_t status = OSA_SOK;
    
    status |= mutex_delete(&pobj->m_mutex);

    close(pobj->m_listen_fd);
    pobj->m_listen_fd = -1;

    pobj->m_initialized = FALSE;

    return status;
}

static status_t __osa_console_external_main(void *ud, task_t tsk, msg_t **msg)
{
    status_t status = OSA_SOK;
    
    osa_console_object_t * pobj = NULL;

    pobj = (osa_console_object_t *)ud;

    switch (msg_get_cmd((*msg)))
    {
        case TASK_CMD_INIT:
            status |= task_set_state(tsk, TASK_STATE_INIT);
            status |= __osa_console_do_initialize(pobj);

            break;

        case TASK_CMD_EXIT:
            status |= task_set_state(tsk, TASK_STATE_EXIT);
            status |= __osa_console_do_deinitialize(pobj);

            break;

        case TASK_CMD_PROC:
            status |= task_set_state(tsk, TASK_STATE_PROC);

            break;

        case OSA_CONSOLE_CMD_PROC:
            status |= __osa_console_do_process(pobj);

            break;

        case OSA_CONSOLE_CMD_FREE_SESSION:
            status |= __osa_console_free_session(pobj, (unsigned int)msg_get_payload_ptr(*msg));

            break;

        default:
            break;
    }

    return status;
}

static status_t __osa_console_session_external_main(void *ud, task_t tsk, msg_t **msg);

static status_t __osa_console_create_session(osa_console_session_t *pobj)
{
    status_t status = OSA_SOK;

    /* Initialize osa console session task */
    snprintf(pobj->m_name, sizeof (pobj->m_name) - 1, "OSA_CONSOLE_SESSION%d", pobj->m_id);

    pobj->m_tsk_obj.m_name = pobj->m_name;
    pobj->m_tsk_obj.m_main = __osa_console_session_external_main;
    pobj->m_tsk_obj.m_find = NULL;
    pobj->m_tsk_obj.m_pri = 0;
    pobj->m_tsk_obj.m_stack_size = 0;
    pobj->m_tsk_obj.m_init_state = 0;
    pobj->m_tsk_obj.m_userdata = (void *)pobj;
    pobj->m_tsk_obj.m_task = TASK_INVALID_TSK;

    /* Register osa console task */
    task_object_t *ptsk = &pobj->m_tsk_obj;
    status = task_mgr_register(ptsk);
    if (OSA_ISERROR(status)) {
        return status;
    }

    /* Start osa console session task */
    status = task_mgr_start(ptsk);
    if (OSA_ISERROR(status)) {
        status |= task_mgr_unregister(ptsk);
        return status;
    }

    return status;
}

static status_t __osa_console_delete_session(osa_console_session_t *pobj)
{
    status_t status = OSA_SOK;

	shutdown(pobj->m_sockfd, SHUT_RDWR);

    task_object_t *ptsk = &pobj->m_tsk_obj;

    status |= task_mgr_stop(ptsk);

    status |= task_mgr_unregister(ptsk);

    return status;
}

static status_t __osa_console_session_do_initialize(osa_console_session_t *pobj)
{
    status_t status = OSA_SOK;

    return status;
}

static int __osa_console_session_read(osa_console_session_t *pobj, char *str)
{
    int i = 0;
    int retval;

    while (i < OSA_ARRAYSIZE(pobj->m_linebuf) - 1) {
        retval = recv(pobj->m_sockfd, str, 1, 0);
        if (retval < 0) {
            printf("Failed to read from tcp socket. errno=%d\n", errno);
            return retval;
        }

        if (retval == 0) {
            printf("Socket is closed by peer.\n");
            return -1;
        }

        if (*str == '\r') continue;
        if (*str == '\n') break;

        i   += retval;
        str += retval;
    }

    (*str) = 0;

    return 0;
}

static int __osa_console_session_write(osa_console_session_t *pobj, const char *str)
{
    int retval;
    int size_to_write = strlen(str);

    while (size_to_write) {
        retval = send(pobj->m_sockfd, str, size_to_write, 0);
        if(retval <= 0) {
            printf("Failed to write to tcp socket. errno=%s\n", strerror(errno));
            return retval;
        }

        str           += retval;
        size_to_write -= retval;
    }

    return 0;
}

static status_t __osa_console_session_do_deinitialize(osa_console_session_t *pobj)
{
    if (pobj->m_sockfd > 0) {
        close(pobj->m_sockfd);
        pobj->m_sockfd = -1;
        pobj->m_connected = FALSE;
    }

    return OSA_SOK;
}

static status_t __osa_console_session_synchronize(void *ud, task_t tsk, msg_t *msg)
{
    status_t status = OSA_SOK;
    osa_console_session_t *pobj = NULL;

    pobj = (osa_console_session_t *)ud;

    switch (msg_get_cmd(msg))
    {
        case TASK_CMD_EXIT:
            status |= __osa_console_session_do_deinitialize(pobj);
            status |= task_set_state(tsk, TASK_CMD_EXIT);
            break;

        default:
            break;
    }

    return status;
}

static int __osa_console_session_cmdshell_process(osa_console_session_t *pobj);

static status_t __osa_console_session_do_process(osa_console_session_t *pobj, task_t tsk, msg_t **msg)
{
    status_t status = OSA_SOK;
    task_state_t tsk_state = TASK_STATE_PROC;

	char console_msg[] = "[root@localhost ~]$ ";

    /*
     *  Note: if we do not return from this routine right now, ack the msg first.
     */
    msg_set_status(*msg, OSA_SOK);
    status = task_ack_free_msg(tsk, *msg);

    (*msg) = NULL;

    while (!OSA_ISERROR(task_get_state(tsk, &tsk_state))
            && tsk_state != TASK_STATE_EXIT) {

        /* Synchronize task */
        status = task_synchronize((void *)pobj, tsk, __osa_console_session_synchronize, 0);

        if (__osa_console_session_write(pobj, console_msg)) {
            break;
        }

        if (__osa_console_session_read(pobj, pobj->m_linebuf)) {
            break;
        }

        if (pobj->m_linebuf[0] == '\0') {
            continue;
        }

        DBG(DBG_INFO, GT_NAME, "%s received: %s\n", pobj->m_name, pobj->m_linebuf);
#if 0
        /* echo back */
        strcat(pobj->m_linebuf, "\n");
        if (__osa_console_session_write(pobj, pobj->m_linebuf)) {
            break;
        }
#endif

        if ((!strncmp(pobj->m_linebuf, "quit", 4))
                || (!strncmp(pobj->m_linebuf, "exit", 4))) {
            break;
        }

        __osa_console_session_cmdshell_process(pobj);
    }

    status |= task_mgr_synchronize(&glb_osa_console_obj.m_tsk_obj,
                                   OSA_CONSOLE_CMD_FREE_SESSION, (void *)pobj->m_id, sizeof(pobj->m_id), 0);

    return status;
}

static status_t 
__osa_console_session_external_main(void *ud, task_t tsk, msg_t **msg)
{
    status_t status = OSA_SOK;
    
    osa_console_session_t * pobj = NULL;

    pobj = (osa_console_session_t *)ud;

    switch (msg_get_cmd((*msg)))
    {
        case TASK_CMD_INIT:
            status |= task_set_state(tsk, TASK_STATE_INIT);
            status |= __osa_console_session_do_initialize(pobj);

            break;

        case TASK_CMD_EXIT:
            status |= task_set_state(tsk, TASK_STATE_EXIT);
            status |= __osa_console_session_do_deinitialize(pobj);

            break;

        case TASK_CMD_PROC:
            status |= task_set_state(tsk, TASK_STATE_PROC);
            status |= __osa_console_session_do_process(pobj, tsk, msg);

            break;

        default:
            break;
    }

    return status;
}

static int split_string(char *str, char **split_strs, int *split_max, char split_char)
{
    int i     = 0;
    int valid = 0;
    char *p   = str;

    while (*p) {
        if ((*p) == split_char || isblank(*p)) {
            valid = 0;
            (*p) = 0;
        } else {
            if (!valid) {
                if(i < *split_max) {
                    split_strs[i++] = p;
                }
                else {
                    return -EINVAL;
                }
                valid = 1;
            }
        }
        p ++;
    }

    (*split_max) = i;

    return 0;
}

static int __osa_console_set_argument(osa_cmd_req_arg_t *arg, const char *argstr, osa_cmd_arg_type_t arg_type)
{
    int nVal = 0;

    switch(arg_type)
    {
        case OSA_CMD_ARG_STRING:
            arg->m_strValue = (char *) argstr;
            break;

        case OSA_CMD_ARG_SIGNED:
            if (sscanf(argstr, "%d", &nVal) != 1) {
                return -EINVAL;
            }

            arg->m_iValue = nVal;
            break;

        case OSA_CMD_ARG_UNSIGNED:
            if (sscanf(argstr, "%d", &nVal) != 1) {
                return -EINVAL;
            }

            if (nVal < 0) {
                return -EINVAL;
            }
            arg->m_uValue = nVal;

            break;

        case OSA_CMD_ARG_BOOLEAN:
            if (strcasecmp(argstr, "yes") == 0 || strcasecmp(argstr, "true") == 0 ||
                    strcmp(argstr, "1") == 0) {
                arg->m_iValue = 1;
            } else if(strcasecmp(argstr, "no") == 0 || strcasecmp(argstr, "false") == 0 ||
                    strcmp(argstr, "0") == 0) {
                arg->m_iValue = 0;
            } else {
                return -EINVAL;
            }
            break;
    }

    return 0;
}


static int __osa_console_cmdshell_write_response(char *respline, osa_cmd_req_t *req)
{
    sprintf(respline, "#0x%08x|%s|%s\r\n", req->m_status, osa_status_get_description(req->m_status), req->m_msgs);
    return 0;
}

static int __osa_console_cmdshell_write_response2(char *respline, osa_cmd_req_t *req)
{
    sprintf(respline, "#%s\r\n", req->m_msgs);
    return 0;
}

#define HELP_TAB    (40)
static void get_cmd_helpmsg(char *msg, osa_cmd_t *cmd)
{
    int i;

    sprintf(msg, "%s", cmd->m_cmd);
    for (i = 0; i < cmd->m_nargs; i ++) {
        osa_cmd_arg_t * cmd_arg = &cmd->m_args[i];

        strcat(msg, " -");
        strcat(msg, cmd_arg->m_tag);

        if (cmd_arg->m_required) {
            strcat(msg, " [");
        } else {
            strcat(msg, " <");
        }

        strcat(msg, cmd_arg->m_name);

        if (cmd_arg->m_required) {
            strcat(msg, "]");
        } else {
            strcat(msg, ">");
        }
    }

    i = strlen(msg);
    while(i < HELP_TAB) {
        msg[i++] = ' ';
    }
    msg[i] = 0;

    strcat(msg, " --- ");
    strcat(msg, cmd->m_description);
}

static status_t __osa_console_cmd_request(osa_console_session_t *pobj, int *written)
{
    (*written) = 1;

    return osa_cmd_request(&pobj->m_req);
}

static int __osa_console_session_cmdshell_process(osa_console_session_t *pobj)
{
	int     r;
	int     i, j, k;
	char *	arg_tags[OSA_CMD_ARGS_MAX];
	char *  args[OSA_CMD_ARGS_MAX];
	char    arg_set[OSA_CMD_ARGS_MAX];
	int	    nargs;
	char *  split_strs[OSA_CMD_ARGS_MAX * 2 + 1];
	int     split_max = OSA_CMD_ARGS_MAX * 2 + 1;
	char *  cmd_txt;
	int	    is_arg;
	int	    arg_found;
	char *  empty_str = "";
	int	    written;
	char *  linebuff = pobj->m_linebuf;
	osa_cmd_req_t * req = &pobj->m_req;
	int	    access = 5;//session->cur_user.access_rights;

	req->m_id = -1;

	if (split_string(linebuff, split_strs, &split_max, 0)) {
		req->m_status = OSA_EARGS;
		strcpy(req->m_msgs, "Too many arguments.");
		__osa_console_cmdshell_write_response(linebuff, req);
		__osa_console_session_write(pobj, linebuff);
		return -EINVAL;
    }

    if (split_max == 0) {
        req->m_status = OSA_EINVAL;
        req->m_msgs[0] = 0;
        __osa_console_cmdshell_write_response(linebuff, req);
        __osa_console_session_write(pobj, linebuff);
        return -EINVAL;
    }

	nargs = 0;
	cmd_txt = split_strs[0];
	is_arg = 0;

	for(i = 1; i < split_max; i ++) {
		if(is_arg) {
			is_arg = 0;
			args[nargs++] = split_strs[i];
		}
        else {
            if (split_strs[i][0] == TAG_PREFIX) {
                if (nargs >= OSA_CMD_ARGS_MAX) {
                    req->m_status = OSA_EINVAL;
                    strcpy(req->m_msgs, "Too many arguments.");
                    __osa_console_cmdshell_write_response(linebuff, req);
                    __osa_console_session_write(pobj, linebuff);
                    return -EINVAL;
                }
				arg_tags[nargs] = split_strs[i] + 1;
				args[nargs] = empty_str;
				is_arg = 1;
			}
			else {

                req->m_status = OSA_EINVAL;
                strcpy(req->m_msgs, "No space is allowed in a single argument.");
                __osa_console_cmdshell_write_response(linebuff, req);
                __osa_console_session_write(pobj, linebuff);
                return -EINVAL;
			}
		}
	}

	/* Help */
	if (strcasecmp(cmd_txt, "help") == 0) {
		req->m_status = OSA_SOK;
		sprintf(req->m_msgs, "%s", "Commands list: [mandatory arg] <optional arg>");
		__osa_console_cmdshell_write_response2(linebuff, req);
		__osa_console_session_write(pobj, linebuff);

		req->m_msgs[0] = 0; // blank remark line
		__osa_console_cmdshell_write_response2(linebuff, req);
		__osa_console_session_write(pobj, linebuff);

		req->m_status = OSA_SOK;
        //for (i = 0; i < MODULE_CMDS_NUM; i ++) {
        for (i = 0; i < VCS_CMDS_NUM; i ++) {
            osa_cmd_t *cmd = &glb_vcs_cmds[i];
            if(cmd->m_access <= access) {
                get_cmd_helpmsg(req->m_msgs, cmd);
                __osa_console_cmdshell_write_response2(linebuff, req);
                __osa_console_session_write(pobj, linebuff);
            }
        }

		req->m_status = OSA_SOK;
		strcpy(req->m_msgs, "Ends.");
		__osa_console_cmdshell_write_response2(linebuff, req);
		__osa_console_session_write(pobj, linebuff);

		return 0;
	}

	/* Looking up commands */
	//for (i = 0; i < MODULE_CMDS_NUM; i ++) {
	for (i = 0; i < VCS_CMDS_NUM; i ++) {
		osa_cmd_t * cmd = &glb_vcs_cmds[i];
		if (cmd->m_access <= access && strcasecmp(cmd->m_cmd, cmd_txt) == 0) {
			req->m_id = cmd->m_id;
			memset(req->m_args, 0, sizeof(osa_cmd_req_arg_t ) * OSA_CMD_ARGS_MAX);
			memset(arg_set, 0, OSA_CMD_ARGS_MAX);
			for (j = 0; j < nargs; j ++) {
				arg_found = 0;
				for (k = 0; k < cmd->m_nargs; k ++) {
					osa_cmd_arg_t * cmd_arg = &cmd->m_args[k];
					if(strcasecmp(cmd_arg->m_tag, arg_tags[j]) == 0) {
						arg_found = 1;

                        if (__osa_console_set_argument(&req->m_args[k], args[j], cmd_arg->m_type)) {
                            req->m_status = OSA_EINVAL;
                            sprintf(req->m_msgs, "-%s = \"%s\"", cmd_arg->m_tag, args[j]);

                            __osa_console_cmdshell_write_response(linebuff, req);
                            __osa_console_session_write(pobj, linebuff);

                            return -EINVAL;
                        }

						arg_set[k] = 1;
						break; // argument found
					}
				}

                if (!arg_found) {
                    req->m_status = OSA_EINVAL;
                    sprintf(req->m_msgs, "Argument -%s = \"%s\" does not support.", arg_tags[j], args[j]);

                    __osa_console_cmdshell_write_response(linebuff, req);
                    __osa_console_session_write(pobj, linebuff);

                    return -EINVAL;
                }
			}

			for (k = 0; k < cmd->m_nargs; k ++) {
				if (!arg_set[k]) {
					osa_cmd_arg_t * cmd_arg = &cmd->m_args[k];
                    if (cmd_arg->m_required) {
                        req->m_status = OSA_EINVAL;
                        sprintf(req->m_msgs, "Required argument \'-%s\' not set.", cmd_arg->m_tag);
                        __osa_console_cmdshell_write_response(linebuff, req);
                        __osa_console_session_write(pobj, linebuff);
                        return -EINVAL;

                    } else {
						req->m_args[k].m_pValue = cmd_arg->m_default_value;
					}
				}
			}

			break; // command found
		}
	}

	if (req->m_id >= 0) {
		/* Call the ioctl after the command shell is parsed */

        //mutex_lock(&glb_osa_console_obj.m_mutex);

		req->m_status = OSA_SOK;
		req->m_msgs[0] = 0;
		written       = 0;

		r = __osa_console_cmd_request(pobj, &written);

        //mutex_unlock(&glb_osa_console_obj.m_mutex);

        if (written) {
            __osa_console_cmdshell_write_response(linebuff, req);
            __osa_console_session_write(pobj, linebuff);
        }

	} else {
		req->m_status = OSA_EBADCMD;
		sprintf(req->m_msgs, "cmd=\'%s\' does not support.", cmd_txt);
		__osa_console_cmdshell_write_response(linebuff, req);
		__osa_console_session_write(pobj, linebuff);

		return -EINVAL;
	}

	return r;
}

#if defined(__cplusplus)
}
#endif
