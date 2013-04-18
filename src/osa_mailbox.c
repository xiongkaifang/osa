/** ============================================================================
 *
 *  Copyright (C), 1987 - 2013, xiong-kaifang Tech. Co.,, Ltd.
 *
 *  @File name:	osa_mailbox.c
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2013-04-05
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
 *  xiong-kaifang   2013-04-05     v1.0	        write this module.
 *
 *
 *  ============================================================================
 */

/*  --------------------- Include system headers ---------------------------- */
#include <stdio.h>

/*  --------------------- Include user headers   ---------------------------- */
#include "osa_mailbox.h"
#include "osa_msgq.h"
#include "osa_mem.h"
#include "osa_mutex.h"
#include "dlist.h"

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
#define MAILBOX_SYSTEM_MBX_NUMS             (10)

#define MBX_SYS_CRITICAL_ENTER()            \
    do {                                    \
        mutex_lock(&glb_mbx_sys_mutex);     \
    } while (0)

#define MBX_SYS_CRITICAL_LEAVE()            \
    do {                                    \
        mutex_unlock(&glb_mbx_sys_mutex);   \
    } while (0)

#define MAILBOX_GET_MBX_HANDLE(mbx)         ((mailbox_handle)mbx)

#define MAILBOX_IS_INVALID_MBX(mbx)         (MAILBOX_INVALID_MBX == (mbx))
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
struct __mailbox_t
{
	unsigned int	m_reserved[2];
	unsigned int	m_mbx_id;
	
    unsigned char   m_names[MSG_TYPE_MAX][32];
	msgq_t			m_msgqs[MSG_TYPE_MAX];
};

typedef struct __mailbox_t * mailbox_handle;

struct __mailbox_node_t; typedef struct __mailbox_node_t mailbox_node_t;
struct __mailbox_node_t
{
    unsigned int        m_reserved[2];
    unsigned char       m_name[32];
    struct __mailbox_t  m_mbx;
};

struct __mailbox_system_t; typedef struct __mailbox_system_t mailbox_system_t;
struct __mailbox_system_t
{
    unsigned int        m_reserved[2];
    Bool                m_initialized;

    mutex_t             m_mutex;
    unsigned int        m_mbx_id;
    unsigned int        m_mbx_cnt;
    unsigned int        m_mbx_used;
    mailbox_node_t    * m_mbxs;
    dlist_t             m_busy_list;
    dlist_t             m_free_list;
};

typedef struct __mailbox_system_t * mailbox_system_handle;

/*
 *  --------------------- Global variable definition ---------------------------
 */

/** ----------------------------------------------------------------------------
 *  @Name:          Variable name
 *
 *  @Description:   Description of the variable.
 * -----------------------------------------------------------------------------
 */
static mailbox_system_prm_t glb_mbx_sys_prm  = {
    .m_mbx_cnt    = MAILBOX_SYSTEM_MBX_NUMS,
};

static mailbox_system_t glb_mbx_sys_obj = {
    .m_initialized = FALSE,
};

static unsigned int glb_cur_init = 0;

OSA_DECLARE_AND_INIT_MUTEX(glb_mbx_sys_mutex);

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
static status_t
mailbox_init(mailbox_handle hdl, const char * name, unsigned int mbx_id);

static status_t mailbox_deinit(mailbox_handle hdl);

static unsigned int mailbox_system_get_mbx_id(mailbox_system_handle hdl);

static status_t 
mailbox_system_internal_find(mailbox_system_t *mbx_sys,
        const char *name, mailbox_node_t **node);

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
status_t mailbox_open(mailbox_t *mbx, mailbox_params_t *prm)
{
    return mailbox_system_register(prm->m_name, mbx);
}

status_t mailbox_recv_msg(mailbox_t mbx, msg_t **msg, msg_type_t msgt, unsigned int timeout)
{
	status_t status = OSA_SOK;
	mailbox_handle mbx_hdl = MAILBOX_GET_MBX_HANDLE(mbx);
	
	OSA_assert(mbx_hdl != NULL && msg != NULL);
	
	status = msgq_recv(mbx_hdl->m_msgqs[msgt], msg, timeout);
	
	return status;
}

status_t mailbox_send_msg(mailbox_t to, mailbox_t frm, msg_t *msg, msg_type_t msgt, unsigned int timeout)
{
	status_t status = OSA_SOK;
	mailbox_handle to_hdl = MAILBOX_GET_MBX_HANDLE(to);
	
	OSA_assert(to_hdl != NULL && msg != NULL);
	
	if (MSG_TYPE_CMD == msgt) {
		msg_set_src(msg, to_hdl->m_msgqs[MSG_TYPE_ACK]);
	}
	
	msg->u.m_mbx_msg.m_to  = to;
	msg->u.m_mbx_msg.m_frm = frm;
	status = msgq_send(to_hdl->m_msgqs[msgt], msg, timeout);

	return status;
}

status_t mailbox_broadcast(mailbox_t mbx_lists[], mailbox_t frm, msg_t *msg)
{
	int i = 0;
	int mbx_cnt = 0;
	status_t status = OSA_SOK;
    msg_t *new_msg = NULL;
	mailbox_handle mbx_hdl[MAILBOX_MBX_MAX];
	
	OSA_assert(mbx_lists != NULL && msg != NULL);
	
	for (i = 0; !MAILBOX_IS_INVALID_MBX(mbx_lists[i]); i++) {
		mbx_cnt++;
		mbx_hdl[i] = (mailbox_handle)mbx_lists[i];
	}
	
	for (i = 0; i < mbx_cnt; i++) {

		status = msgq_send(mbx_hdl[i]->m_msgqs[MSG_TYPE_CMD], msg, OSA_TIMEOUT_NONE);
	}
	
	return status;
}

status_t mailbox_alloc_msg(unsigned short size, msg_t **msg)
{
    return msgq_alloc(size, msg);
}

status_t mailbox_free_msg (unsigned short size, msg_t *msg)
{
    return msgq_free(size, msg);
}

status_t mailbox_wait_msg(mailbox_t mbx, msg_t **msg, msg_type_t msgt)
{
	status_t status = OSA_SOK;
	mailbox_handle mbx_hdl = MAILBOX_GET_MBX_HANDLE(mbx);
	
	OSA_assert(mbx_hdl != NULL && msg != NULL);
	
	status = msgq_recv(mbx_hdl->m_msgqs[msgt], msg, OSA_TIMEOUT_FOREVER);
	OSA_assert(OSA_SOK == status);
	
	return status;
}

status_t mailbox_check_msg(mailbox_t mbx, msg_t **msg, msg_type_t msgt)
{
	status_t status = OSA_SOK;
	mailbox_handle mbx_hdl = MAILBOX_GET_MBX_HANDLE(mbx);
	
	OSA_assert(mbx_hdl != NULL && msg != NULL);
	
	status = msgq_recv(mbx_hdl->m_msgqs[msgt], msg, OSA_TIMEOUT_NONE);
	
	return status;
}

status_t mailbox_get_msg_count(mailbox_t mbx, unsigned int *cnt, msg_type_t msgt)
{
	status_t status = OSA_SOK;
	mailbox_handle mbx_hdl = MAILBOX_GET_MBX_HANDLE(mbx);
	
	OSA_assert(mbx_hdl != NULL && cnt != NULL);
	
	status = msgq_count(mbx_hdl->m_msgqs[msgt], cnt);
	
	return status;
}

status_t mailbox_wait_cmd(mailbox_t mbx, msg_t **msg, unsigned short cmd)
{
	status_t status = OSA_EFAIL;
	mailbox_handle mbx_hdl = MAILBOX_GET_MBX_HANDLE(mbx);
	
	OSA_assert(mbx_hdl != NULL && msg != NULL);
	
	do {
		status |= msgq_recv(mbx_hdl->m_msgqs[MAILBOX_MSGQ_CMD],
							msg, OSA_TIMEOUT_FOREVER);
		OSA_assert(OSA_SOK == status);
		
		if (msg_get_cmd(*msg) == cmd) {
			status = OSA_SOK;
			break;
		}
		
		status |= msgq_send(mbx_hdl->m_msgqs[MAILBOX_MSGQ_CMD], *msg, OSA_TIMEOUT_NONE);

	} while (1);
	
	return status;
}

#if 0
status_t mailbox_ack_msg(mailbox_t to, mailbox_t frm, mailbox_msg_t *msg)
{
    status_t status = OSA_SOK;
    task_handle hdl = NULL;

    msg->m_to  = to;
    msg->m_frm = frm;

    hdl = (task_handle)msg->m_to;
    status |= msgq_send(hdl->m_ack_msgq, (msgq_msg_t *)msg, OSA_TIMEOUT_FOREVER);

    return status;
}
#endif
 
status_t mailbox_flush(mailbox_t mbx)
{
    msg_t *msg = NULL;
    status_t status = OSA_SOK;

    do {
        status = mailbox_check_msg(mbx, &msg, MSG_TYPE_CMD);
        if (!OSA_ISERROR(status)) {
            mailbox_free_msg(msg_get_msg_size(msg), msg);
        }
    } while (!OSA_ISERROR(status));

    do {
        status = mailbox_check_msg(mbx, &msg, MSG_TYPE_ACK);
        if (!OSA_ISERROR(status)) {
            mailbox_free_msg(msg_get_msg_size(msg), msg);
        }
    } while (!OSA_ISERROR(status));

    return status = OSA_SOK;
}

status_t mailbox_close(mailbox_t mbx, mailbox_params_t *prm)
{
    return mailbox_system_unregister(prm->m_name, mbx);
}

////////////////////////////////////////////////////////////////////////////////

/* mailbox system */
status_t mailbox_system_init(mailbox_system_prm_t *prm)
{
    int i;
    Bool need_init = FALSE;
    status_t status = OSA_SOK;
    mailbox_system_handle hdl = &glb_mbx_sys_obj;

    MBX_SYS_CRITICAL_ENTER();
    if (glb_cur_init++ == 0) {
        need_init = TRUE;
    }
    MBX_SYS_CRITICAL_LEAVE();

    if (!need_init) {
        return status;
    }

    OSA_assert(TRUE == need_init);

    if (prm == NULL) {
        prm = &glb_mbx_sys_prm;
    }

    hdl->m_mbx_id  = 1;
    hdl->m_mbx_cnt = prm->m_mbx_cnt;
    status = OSA_memAlloc(sizeof(mailbox_node_t) * hdl->m_mbx_cnt, (void **)&hdl->m_mbxs);
    if (OSA_ISERROR(status) || hdl->m_mbxs == NULL) {
        return OSA_EMEM;
    }
    hdl->m_mbx_used = 0;

    status |= mutex_create(&hdl->m_mutex);
    status |= dlist_init(&hdl->m_busy_list);
    status |= dlist_init(&hdl->m_free_list);
    OSA_assert(OSA_SOK == status);

    for (i = 0; i < hdl->m_mbx_cnt; i++) {
		status |= mailbox_init(&(hdl->m_mbxs + i)->m_mbx, "MBX",
                    mailbox_system_get_mbx_id(hdl));
        status |= dlist_initialize_element((dlist_element_t *)&hdl->m_mbxs[i]);
        status |= dlist_put_tail(&hdl->m_free_list, (dlist_element_t *)&hdl->m_mbxs[i]);
        OSA_assert(OSA_SOK == status);
    }

    hdl->m_initialized = TRUE;

    return status;
}

status_t mailbox_system_register(const char *name, mailbox_t *mbx)
{
    status_t status = OSA_SOK;
    mailbox_node_t *mbx_node;
    mailbox_system_handle hdl = &glb_mbx_sys_obj;

    OSA_assert(hdl->m_initialized == TRUE);

    mutex_lock(&hdl->m_mutex);
    
    if (hdl->m_mbx_used < hdl->m_mbx_cnt) {

        status |= dlist_get_head(&hdl->m_free_list, (dlist_element_t **)&mbx_node);

        snprintf(mbx_node->m_name, sizeof(mbx_node->m_name) - 1, "%s", name);

        (*mbx) = (mailbox_t)(&mbx_node->m_mbx);
        
        status |= dlist_put_tail(&hdl->m_busy_list, (dlist_element_t *)mbx_node);
        hdl->m_mbx_used++;
    } else {
        status = OSA_ENOENT;
    }

    mutex_unlock(&hdl->m_mutex);

    return status;

}

status_t mailbox_system_find(const char *name, mailbox_t *mbx)
{
    status_t status = OSA_ENOENT;
    mailbox_node_t *mbx_node  = NULL;
    mailbox_system_handle hdl = &glb_mbx_sys_obj;

    OSA_assert(hdl->m_initialized == TRUE);

    (*mbx) =(mailbox_t)MAILBOX_INVALID_MBX;

    mutex_lock(&hdl->m_mutex);
    
    if (hdl->m_mbx_used > 0) {

        status = mailbox_system_internal_find(hdl, name, &mbx_node);
        if (!OSA_ISERROR(status)) {
            (*mbx) = (mailbox_t)&mbx_node->m_mbx;
        }
    } 

    mutex_unlock(&hdl->m_mutex);

    return status;
}

status_t mailbox_system_unregister(const char *name, mailbox_t mbx)
{
    status_t status = OSA_ENOENT;
    mailbox_node_t *mbx_node  = NULL;
    mailbox_system_handle hdl = &glb_mbx_sys_obj;

    OSA_assert(hdl->m_initialized == TRUE);

    mutex_lock(&hdl->m_mutex);
    
    if (hdl->m_mbx_used > 0) {

        status = mailbox_system_internal_find(hdl, name, &mbx_node);
        if (!OSA_ISERROR(status)) {
            status |= dlist_remove_element(&hdl->m_busy_list, (dlist_element_t *)mbx_node);
            OSA_assert((&mbx_node->m_mbx) == (mailbox_handle)mbx);
            status |= dlist_put_tail(&hdl->m_free_list, (dlist_element_t *)mbx_node);
            hdl->m_mbx_used--;
        }
    } 

    mutex_unlock(&hdl->m_mutex);

    return status;
}

status_t mailbox_system_deinit(void)
{
	int i;
    Bool need_deinit = FALSE;
    status_t status = OSA_SOK;
    mailbox_system_handle hdl = &glb_mbx_sys_obj;

    OSA_assert(hdl->m_initialized == TRUE);

    MBX_SYS_CRITICAL_ENTER();
    if (--glb_cur_init == 0) {
        need_deinit = TRUE;
    }
    MBX_SYS_CRITICAL_LEAVE();

    if (!need_deinit) {
        return status;
    }

    OSA_assert(TRUE == need_deinit);

	for (i = 0; i < hdl->m_mbx_cnt; i++) {
		status |= mailbox_deinit(&(hdl->m_mbxs + i)->m_mbx);
	}
	
    status |= mutex_delete(&hdl->m_mutex);

    if (hdl->m_mbxs != NULL) {
        OSA_memFree(sizeof(mailbox_node_t) * hdl->m_mbx_cnt, hdl->m_mbxs);
        hdl->m_mbxs = NULL;
    }
    hdl->m_initialized = FALSE;

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
static status_t
mailbox_init(mailbox_handle hdl, const char * name, unsigned int mbx_id)
{
    int i;
    status_t status = OSA_SOK;

    hdl->m_mbx_id = mbx_id;

    for (i = 0; i < OSA_ARRAYSIZE(hdl->m_msgqs); i++) {
        snprintf(hdl->m_names[i], sizeof(hdl->m_names[i]) - 1, 
                "%s:%02d%s", name,
                mbx_id, i == MAILBOX_MSGQ_CMD ? "CMD" : "ACK");

        status = msgq_mgr_register(hdl->m_names[i], &hdl->m_msgqs[i], NULL);
    }

    return status;
}

static status_t mailbox_deinit(mailbox_handle hdl)
{
    int i;
    status_t status = OSA_SOK;

    for (i = 0; i < OSA_ARRAYSIZE(hdl->m_msgqs); i++) {

        status = msgq_mgr_unregister(hdl->m_names[i], hdl->m_msgqs[i]);
    }

    return status;
}

static unsigned int
mailbox_system_get_mbx_id(mailbox_system_handle hdl)
{
    unsigned int id;

    mutex_lock(&hdl->m_mutex);

    id = hdl->m_mbx_id++;

    mutex_unlock(&hdl->m_mutex);

    return id;
}

status_t mailbox_system_internal_find(mailbox_system_t *mbx_sys, const char *name, mailbox_node_t **node)
{
    status_t retval = OSA_ENOENT;
    status_t status = OSA_SOK;
    Bool found = FALSE;
    mailbox_system_handle hdl = mbx_sys;
    mailbox_node_t  * cur_nod_hdl = NULL;
    mailbox_node_t  * nex_nod_hdl = NULL;

    (*node) = NULL;

    status = dlist_first(&hdl->m_busy_list, (dlist_element_t **)&cur_nod_hdl);
    while ((cur_nod_hdl != NULL) && !OSA_ISERROR(status)) {

        if (strncmp(cur_nod_hdl->m_name, name,
                    sizeof(cur_nod_hdl->m_name) - 1) == 0) {
            found = TRUE;
            (*node) = cur_nod_hdl;
            retval = OSA_SOK;
            break;
        }

        status = dlist_next(&hdl->m_busy_list,
                            (dlist_element_t *) cur_nod_hdl,
                            (dlist_element_t **)&nex_nod_hdl
                            );
        if (!OSA_ISERROR(status)) {
            cur_nod_hdl = nex_nod_hdl;
        } else {
            break;
        }
    }

    return retval;
}

#if defined(__cplusplus)
}
#endif
