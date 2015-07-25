/** ============================================================================
 *
 *  Copyright (C), 1987 - 2015, xiong-kaifang Tech. Co, Ltd.
 *
 *  @File Name:	osa_timer.h
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2013-09-03
 *
 *  @Description:   //	用于详细说明此程序文件完成的主要功能，与其它模块
 *		            //	或函数的接口，输出值，取值范围、含义及参数间的控
 *		            //	制、顺序、独立或依赖等关系
 *		            //
 *
 *	                The header file for osa timer.
 *
 *  @Others:	    //	其它内容说明
 *
 *  @Function List: //	主要函数列表，每条记录就包括函数名及功能简要说明
 *	    1.  ...
 *	    2.  ...
 *
 *  @History:	    //	修改历史记录列表，每条修改记录就包括修改日期、修改
 *	        	    //	时间及修改内容简述
 *
 *	<author>	    <time>	     <version>	    <desc>
 *  xiong-kaifang   2013-09-03     v1.0	        Write this module.
 *
 *  ============================================================================
 */

#if !defined (__OSA_TIMER_H)
#define __OSA_TIMER_H

/*  --------------------- Include system headers ---------------------------- */

/*  --------------------- Include user headers   ---------------------------- */
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
typedef status_t (*OSA_EVENT_FXN_T)(void *ud);

struct __osa_event_t;
typedef struct __osa_event_t osa_event_t;
struct __osa_event_t
{
    OSA_EVENT_FXN_T m_fxn;
    void *          m_ud;
    unsigned int    m_delete;
};

/*
 *  --------------------- Public function declaration --------------------------
 */
status_t osa_timer_init(void);

status_t osa_timer_register(int *pid, unsigned int delay, osa_event_t *pevent);

status_t osa_timer_unregister(int id);

status_t osa_timer_deinit(void);

#if defined(__cplusplus)
}
#endif  /* defined(__cplusplus) */

#endif  /* if !defined (__OSA_TIMER_H) */
