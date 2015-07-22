/** ============================================================================
 *
 *  Copyright (C), 1987 - 2013, xiong-kaifang Tech. Co.,, Ltd.
 *
 *  @File name:	osa_msgq.c
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2013-04-04
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
#include <stdio.h>
#include <string.h>
#include <pthread.h>

/*  --------------------- Include user headers   ---------------------------- */
#include "osa.h"
#include "osa_msgq.h"
#include "osa_mutex.h"
#include "osa_mem.h"
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
#define MSGQ_MGR_MSGQ_MAX   (32)

#define MSGQ_MGR_CRITICAL_ENTER()       \
    do {                                \
        mutex_lock(&glb_mgr_mutex);     \
    } while (0)

#define MSGQ_MGR_CRITICAL_LEAVE()       \
    do {                                \
        mutex_unlock(&glb_mgr_mutex);   \
    } while (0)

#define MSGQ_MSG_GET_SRC_QUEUE(msg)         (msg_get_src(msg))
#define MSGQ_MSG_SET_SRC_QUEUE(msg, src)    (msg_set_src(msg, src))
#define MSGQ_MSG_GET_DST_QUEUE(msg)         (msg_get_dst(msg))
#define MSGQ_MSG_SET_DST_QUEUE(msg, dst)    (msg_set_dst(msg, dst))

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
struct __msgq_t
{
	unsigned int	m_rd_idx;
	unsigned int	m_wr_idx;
	unsigned int	m_len;
	unsigned int	m_count;

	msg_t	      * m_msgs[MSGQ_MGR_MSGQ_MAX];
	
	//mutex_t		m_mutex;
    pthread_mutex_t m_mutex;
	pthread_cond_t	m_rd_cond;
	pthread_cond_t	m_wr_cond;
};

typedef struct __msgq_t * msgq_handle;

struct __msgq_node_t; typedef struct __msgq_node_t msgq_node_t;
struct __msgq_node_t
{
    unsigned int    m_reserved[2];
    unsigned char   m_name[32];
    struct __msgq_t m_msgq;
};

struct __msgq_mgr_t; typedef struct __msgq_mgr_t msgq_mgr_t;
struct __msgq_mgr_t
{
    unsigned int    m_reserver[2];
    Bool            m_initialized;

    mutex_t         m_mutex;
    unsigned int    m_msgq_cnt;
    unsigned int    m_msgq_used;
    msgq_node_t   * m_msgqs;
    dlist_t         m_busy_list;
    dlist_t         m_free_list;
};

typedef struct __msgq_mgr_t * msgq_mgr_handle;

/*
 *  --------------------- Global variable definition ---------------------------
 */

/** ----------------------------------------------------------------------------
 *  @Name:          Variable name
 *
 *  @Description:   Description of the variable.
 * -----------------------------------------------------------------------------
 */
static msgq_mgr_prm_t   glb_mgr_prm  = {
    .m_msgq_cnt    = MSGQ_MGR_MSGQ_MAX
};

static msgq_mgr_t       glb_msgq_mgr = {
    .m_initialized = FALSE,
};

static unsigned int     glb_cur_init = 0;

OSA_DECLARE_AND_INIT_MUTEX(glb_mgr_mutex);

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
msgq_mgr_internal_find(msgq_mgr_t *msgq_mgr, const char *name, msgq_node_t **node);

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
status_t msgq_init(struct __msgq_t *msgq)
{
	int i;
	status_t status = OSA_SOK;
	pthread_condattr_t	cond_attr;
	pthread_mutexattr_t	mutex_attr;
    msgq_handle hdl = (msgq_handle)msgq;
	
	OSA_assert(hdl != NULL);
	
	hdl->m_rd_idx = 0;
	hdl->m_wr_idx = 0;
	hdl->m_len    = MSGQ_MGR_MSGQ_MAX;
	hdl->m_count  = 0;
	
	for (i = 0; i < hdl->m_len; i++) {
		hdl->m_msgs[i] = NULL;
	}
	
	status |= pthread_mutexattr_init(&mutex_attr);
	status |= pthread_condattr_init(&cond_attr);
	
	status |= pthread_mutex_init(&hdl->m_mutex, &mutex_attr);
	status |= pthread_cond_init(&hdl->m_rd_cond, &cond_attr);
	status |= pthread_cond_init(&hdl->m_wr_cond, &cond_attr);
	
	pthread_mutexattr_destroy(&mutex_attr);
	pthread_condattr_destroy(&cond_attr);
	
	return status;
}

status_t msgq_deinit(struct __msgq_t *msgq)
{
	status_t status = OSA_SOK;
    msgq_handle hdl = (msgq_handle)msgq;
	
	OSA_assert(hdl != NULL);
	
	pthread_cond_destroy(&hdl->m_wr_cond);
	pthread_cond_destroy(&hdl->m_rd_cond);
	pthread_mutex_destroy(&hdl->m_mutex);
	
	return status;
}

status_t msgq_open(const char *name, msgq_t *msgq, msgq_attrs_t *attrs)
{
    return msgq_mgr_register(name, msgq, attrs);
}

status_t msgq_locate(const char *name, msgq_t *msgq, msgq_locate_attrs_t *attrs)
{
	return msgq_mgr_find(name, msgq);
}

status_t msgq_release(msgq_t msgq)
{
	return OSA_SOK;
}

status_t msgq_alloc(unsigned short size, msg_t **msg)
{
    return OSA_memAlloc(size, (void **)msg);
}

status_t msgq_free(unsigned short size, msg_t *msg)
{
    return OSA_memFree(size, (void *)msg);
}

status_t msgq_send(msgq_t msgq, msg_t *msg, unsigned int timeout)
{
	status_t status = OSA_ENOENT;
	msgq_handle hdl = (msgq_handle)msgq;
	
	OSA_assert(hdl != NULL && msg != NULL);
	
	pthread_mutex_lock(&hdl->m_mutex);
	
	while (1) {
		if (hdl->m_count < hdl->m_len) {
			hdl->m_msgs[hdl->m_wr_idx] = msg;
			hdl->m_wr_idx = (hdl->m_wr_idx + 1) % hdl->m_len;
			hdl->m_count++;
			status = OSA_SOK;
			pthread_cond_signal(&hdl->m_rd_cond);
			break;
		} else  {
			if (OSA_TIMEOUT_NONE == timeout) {
				break;
			}
		
			status = pthread_cond_wait(&hdl->m_wr_cond, &hdl->m_mutex);
		}
	}
	
	pthread_mutex_unlock(&hdl->m_mutex);
	
	return status;
}

status_t msgq_recv(msgq_t msgq, msg_t **msg, unsigned int timeout)
{
	status_t status = OSA_ENOENT;
	msgq_handle hdl = (msgq_handle)msgq;
	
	OSA_assert(hdl != NULL && msg != NULL);
	
	pthread_mutex_lock(&hdl->m_mutex);
	
	while (1) {
		if (hdl->m_count > 0) {
            
            if (msg != NULL) {
			    (*msg) = hdl->m_msgs[hdl->m_rd_idx];
            }

			hdl->m_rd_idx = (hdl->m_rd_idx + 1) % hdl->m_len;
			hdl->m_count--;
			status = OSA_SOK;
			pthread_cond_signal(&hdl->m_wr_cond);
			break;
		} else  {
			if (OSA_TIMEOUT_NONE == timeout) {
				break;
			}
		
			status = pthread_cond_wait(&hdl->m_rd_cond, &hdl->m_mutex);
		}
	}
	
	pthread_mutex_unlock(&hdl->m_mutex);
	
	return status;
}

status_t msgq_get_src_queue(msg_t *msg, msgq_t *msgq)
{
	status_t status = OSA_SOK;
	
	OSA_assert(msg != NULL && msgq != NULL);
	
	(*msgq) = (msgq_t)MSGQ_MSG_GET_SRC_QUEUE(msg);
	
	return status;
}

status_t msgq_set_src_queue(msg_t *msg, msgq_t msgq)
{
	status_t status = OSA_SOK;
	
	OSA_assert(msg != NULL);
	
	MSGQ_MSG_SET_SRC_QUEUE(msg, msgq);
	
	return status;
}

status_t msgq_get_dst_queue(msg_t *msg, msgq_t *msgq)
{
	status_t status = OSA_SOK;
	
	OSA_assert(msg != NULL && msgq != NULL);
	
	(*msgq) = (msgq_t)MSGQ_MSG_GET_DST_QUEUE(msg);
	
	return status;
}

status_t msgq_set_dst_queue(msg_t *msg, msgq_t msgq)
{
	status_t status = OSA_SOK;
	
	OSA_assert(msg != NULL);
	
	MSGQ_MSG_SET_DST_QUEUE(msg, msgq);
	
	return status;
}

status_t msgq_count (msgq_t msgq, unsigned int *cnt)
{
	status_t status = OSA_SOK;
	msgq_handle hdl = (msgq_handle)msgq;
	
	OSA_assert(hdl != NULL && cnt != NULL);
	
	pthread_mutex_lock(&hdl->m_mutex);
	
	(*cnt) = hdl->m_count;
	
	pthread_mutex_unlock(&hdl->m_mutex);
	
	return status;
}

status_t msgq_close(const char *name, msgq_t msgq)
{
    return msgq_mgr_unregister(name, msgq);
}

status_t msgq_mgr_init(msgq_mgr_prm_t *prm)
{
    int i;
    Bool need_init = FALSE;
    status_t status = OSA_SOK;
    msgq_mgr_handle hdl = &glb_msgq_mgr;

    MSGQ_MGR_CRITICAL_ENTER();
    if (glb_cur_init++ == 0) {
        need_init = TRUE;
    }
    MSGQ_MGR_CRITICAL_LEAVE();

    if (!need_init) {
        return status;
    }

    OSA_assert(TRUE == need_init);

    if (prm == NULL) {
        prm = &glb_mgr_prm;
    }

    hdl->m_msgq_cnt = prm->m_msgq_cnt;
    status = OSA_memAlloc(sizeof(msgq_node_t) * hdl->m_msgq_cnt, (void **)&hdl->m_msgqs);
    if (OSA_ISERROR(status) || hdl->m_msgqs == NULL) {
        return OSA_EMEM;
    }
    hdl->m_msgq_used = 0;

    status |= mutex_create(&hdl->m_mutex);
    status |= dlist_init(&hdl->m_busy_list);
    status |= dlist_init(&hdl->m_free_list);
    OSA_assert(OSA_SOK == status);

    for (i = 0; i < hdl->m_msgq_cnt; i++) {
		status |= msgq_init(&(hdl->m_msgqs + i)->m_msgq);
        status |= dlist_initialize_element((dlist_element_t *)&hdl->m_msgqs[i]);
        status |= dlist_put_tail(&hdl->m_free_list, (dlist_element_t *)&hdl->m_msgqs[i]);
        OSA_assert(OSA_SOK == status);
    }

    hdl->m_initialized = TRUE;

    return status;
}

status_t msgq_mgr_register(const char *name, msgq_t *msgq, msgq_attrs_t *attrs)
{
    status_t status = OSA_SOK;
    msgq_node_t *msgq_node;
    msgq_mgr_handle hdl = &glb_msgq_mgr;

    OSA_assert(hdl->m_initialized == TRUE);

    mutex_lock(&hdl->m_mutex);
    
    if (hdl->m_msgq_used < hdl->m_msgq_cnt) {
        status |= dlist_get_head(&hdl->m_free_list, (dlist_element_t **)&msgq_node);

        snprintf(msgq_node->m_name, sizeof(msgq_node->m_name) - 1, "%s", name);
        (*msgq) = (msgq_t)&msgq_node->m_msgq;

        status |= dlist_put_tail(&hdl->m_busy_list, (dlist_element_t *)msgq_node);
        hdl->m_msgq_used++;
    } else {
        status = OSA_ENOENT;
    }

    mutex_unlock(&hdl->m_mutex);

    return status;

}

status_t msgq_mgr_find(const char *name, msgq_t *msgq)
{
    status_t status = OSA_ENOENT;
    msgq_node_t *msgq_node  = NULL;
    msgq_mgr_handle hdl = &glb_msgq_mgr;

    OSA_assert(hdl->m_initialized == TRUE);

    (*msgq) =(msgq_t)MSGQ_INVALID_MSGQ;

    mutex_lock(&hdl->m_mutex);
    
    if (hdl->m_msgq_used > 0) {

        status = msgq_mgr_internal_find(hdl, name, &msgq_node);
        if (!OSA_ISERROR(status)) {
            (*msgq) = (msgq_t)&msgq_node->m_msgq;
        }
    } 

    mutex_unlock(&hdl->m_mutex);

    return status;
}

status_t msgq_mgr_unregister(const char *name, msgq_t msgq)
{
    status_t status = OSA_ENOENT;
    msgq_node_t *msgq_node  = NULL;
    msgq_mgr_handle hdl = &glb_msgq_mgr;

    OSA_assert(hdl->m_initialized == TRUE);

    mutex_lock(&hdl->m_mutex);
    
    if (hdl->m_msgq_used > 0) {

        status = msgq_mgr_internal_find(hdl, name, &msgq_node);
        if (!OSA_ISERROR(status)) {
            status |= dlist_remove_element(&hdl->m_busy_list, (dlist_element_t *)msgq_node);
            OSA_assert((&msgq_node->m_msgq) == (msgq_handle)msgq);
            status |= dlist_put_tail(&hdl->m_free_list, (dlist_element_t *)msgq_node);
            hdl->m_msgq_used--;
        }
    } 

    mutex_unlock(&hdl->m_mutex);

    return status;
}

status_t msgq_mgr_deinit(void)
{
	int i;
    Bool need_deinit = FALSE;
    status_t status = OSA_SOK;
    msgq_mgr_handle hdl = &glb_msgq_mgr;

    OSA_assert(hdl->m_initialized == TRUE);

    MSGQ_MGR_CRITICAL_ENTER();
    if (--glb_cur_init == 0) {
        need_deinit = TRUE;
    }
    MSGQ_MGR_CRITICAL_LEAVE();

    if (!need_deinit) {
        return status;
    }

    OSA_assert(TRUE == need_deinit);

	for (i = 0; i < hdl->m_msgq_cnt; i++) {
		status |= msgq_deinit(&(hdl->m_msgqs + i)->m_msgq);
	}
	
    status |= mutex_delete(&hdl->m_mutex);

    if (hdl->m_msgqs != NULL) {
        OSA_memFree(sizeof(msgq_node_t) * hdl->m_msgq_cnt, hdl->m_msgqs);
        hdl->m_msgqs = NULL;
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
static bool
msgq_mgr_find_match_fxn(dlist_element_t *elem, void *data)
{
    return (strncmp(((msgq_node_t *)elem)->m_name,
                    (const char *)data,
                    sizeof(((msgq_node_t *)elem)->m_name)) == 0);
}

static status_t
msgq_mgr_internal_find(msgq_mgr_t *msgq_mgr, const char *name, msgq_node_t **node)
{
    status_t status = OSA_ENOENT;

    (*node) = NULL;

    status = dlist_search_element(&msgq_mgr->m_busy_list, (void *)name,
                                  (dlist_element_t **)node, msgq_mgr_find_match_fxn);

    if (!OSA_ISERROR(status)) {
        status = OSA_SOK;
    }

    return status;
}

#if defined(__cplusplus)
}
#endif
