/** ============================================================================
 *
 *  Copyright (C), 1987 - 2015, xiong-kaifang Tech. Co, Ltd.
 *
 *  @File Name:	osa_msgq.h
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2013-04-04
 *
 *  @Description:   // 用于详细说明此程序文件完成的主要功能，与其它模块
 *                  // 或函数的接口，输出值，取值范围、含义及参数间的控
 *                  // 制、顺序、独立或依赖等关系
 *                  //
 *
 *                  The header file for osa msg queue.
 *
 *  @Others:        // 其它内容说明
 *
 *  @Function List: // 主要函数列表，每条记录就包括函数名及功能简要说明
 *      1.  ...
 *      2.  ...
 *
 *  @History:       // 修改历史记录列表，每条修改记录就包括修改日期、修改
 *                  // 时间及修改内容简述
 *
 *  <author>        <time>       <version>      <description>
 *
 *  xiong-kaifang   2013-04-04     v1.0	        Write this module.
 *
 *  xiong-kaifang   2015-09-19     v1.1         Using opaque type for msgq_t.
 *
 *  ============================================================================
 */

#if !defined (__OSA_MSGQ_H)
#define __OSA_MSGQ_H

/*  --------------------- Include system headers ---------------------------- */

/*  --------------------- Include user headers   ---------------------------- */
#include "std_defs.h"
#include "osa_msg.h"
#include "osa_msgq_mgr.h"
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
#define MSGQ_INVALID_MSGQ   (~(0u))

/*
 *  --------------------- Data type definition ---------------------------------
 */

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
typedef HANDLE      msgq_t;

#if 0
struct __msgq_msg_t; typedef struct __msgq_msg_t msgq_msg_t;
struct __msgq_msg_t
{
	unsigned int	m_reserved[2];
	unsigned int	m_to;
	unsigned int	m_from;
	void          * m_prm;
	unsigned int	m_size;
	unsigned short	m_cmd;
	unsigned short 	m_flags;
	int           	m_status;
	unsigned int	m_id;
};

struct __msgq_transport_t
{	Bool			m_remote;
	
	union {
		struct {
		} local;
		
		struct {
			status_t (*read)(void *userdata, msgq_msg_t *msg, unsigned int timeout);
			status_t (*write)(void *userdata, msgq_msg_t *msg, unsigned int timeout);
		} remote;
	} transport;
};
#endif

typedef status_t (*msgq_wait)(void *userdata, unsigned int timeout);
typedef status_t (*msgq_signal)(void *userdata);

typedef struct msgq_attrs_tag {
	msgq_wait		m_wait;
	msgq_signal		m_signal;
	void		  * m_userdata;
} msgq_attrs_t;

struct __msgq_locate_attrs_t;
typedef struct __msgq_locate_attrs_t msgq_locate_attrs_t;
struct __msgq_locate_attrs_t
{
    char            m_padding[32];
};

/*
 *  --------------------- Public function declaration --------------------------
 */

/** =============================================================================
 *
 *  @Function:      // 函数名称
 *
 *  @Description:   // 函数功能、性能等的描述
 *
 *  @Calls:	        // 被本函数调用的函数清单
 *
 *  @Called By:	    // 调用本函数的函数清单
 *
 *  @Table Accessed:// 被访问的表（此项仅对于牵扯到数据库操作的程序）
 *
 *  @Table Updated: // 被修改的表（此项仅对于牵扯到数据库操作的程序）
 *
 *  @Input:	        // 对输入参数的说明
 *
 *  @Output:        // 对输出参数的说明
 *
 *  @Return:        // 函数返回值的说明
 *
 *  @Enter          // Precondition
 *
 *  @Leave          // Postcondition
 *
 *  @Others:        // 其它说明
 *
 *  ============================================================================
 */
status_t msgq_mgr_init(msgq_mgr_prm_t *prm);
status_t msgq_mgr_deinit(void);

status_t msgq_open(const char *name, msgq_t *msgq, msgq_attrs_t *attrs);

status_t msgq_locate(const char *name, msgq_t *msgq, msgq_locate_attrs_t *attrs);
status_t msgq_release(msgq_t msgq);

status_t msgq_alloc(unsigned short size, msg_t **msg);
status_t msgq_free(unsigned short size, msg_t *msg);

status_t msgq_send(msgq_t msgq, msg_t *msg, unsigned int timeout);
status_t msgq_recv(msgq_t msgq, msg_t **msg, unsigned int timeout);

status_t msgq_get_src_queue(msg_t *msg, msgq_t *msgq);
status_t msgq_set_src_queue(msg_t *msg, msgq_t msgq);
status_t msgq_get_dst_queue(msg_t *msg, msgq_t *msgq);
status_t msgq_set_dst_queue(msg_t *msg, msgq_t msgq);

status_t msgq_count(msgq_t msgq, unsigned int *cnt);

status_t msgq_close(const char *name, msgq_t *msgq);

#if defined(__cplusplus)
}
#endif  /* defined(__cplusplus) */

#endif  /* if !defined (__OSA_MSGQ_H) */
