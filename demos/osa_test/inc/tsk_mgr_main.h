/** ============================================================================
 *
 *  Copyright (C), 1987 - 2013, xiong-kaifang Tech. Co, Ltd.
 *
 *  @File Name:	tsk_mgr_main.h
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2012-02-24
 *
 *  @Description:   //	用于详细说明此程序文件完成的主要功能，与其它模块
 *		            //	或函数的接口，输出值，取值范围、含义及参数间的控
 *		            //	制、顺序、独立或依赖等关系
 *		            //
 *
 *	                The format for header file.
 *
 *  @Others:	    //	其它内容说明
 *
 *  @Function List: //	主要函数列表，每条记录就包括函数名及功能简要说明
 *	    1.  ...
 *	    2.  ...
 *
 *  @History:	    //	修改历史记录列表，每条修改记录就包括修改日期、修改
 *	        	    //	时间及修改内容简述
 *	    1.  Date:
 *	        Author:
 *	        Modification:
 *	    2.  ...
 *
 *  ============================================================================
 */

#if !defined (__OSA_TSK_MGR_MAIN_H)
#define __OSA_TSK_MGR_MAIN_H

/*  --------------------- Include system headers ---------------------------- */

/*  --------------------- Include user headers   ---------------------------- */
#include "osa_task.h"
#include "osa_timer.h"
#include "osa_task_mgr.h"
#include "osa_mutex.h"
#include "tsk_common.h"

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
#define TASK_MGR_TSK_MAX    (10)

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
enum {
    TASK_MGR_TSK0 = 0,
    TASK_MGR_TSK1 = 1,
    TASK_MGR_TSK2 = 2,
    TASK_MGR_TSK3 = 3,
    TASK_MGR_TSK4 = 4,
    TASK_MGR_TSK5 = 5,
};

struct __task_mgr_params_t;
typedef struct __task_mgr_params_t task_mgr_params_t;
struct __task_mgr_params_t
{
    task_mgr_prm_t          m_tsk_mgr_prm;
};

struct __task_mgr_object_t
{
    task_mgr_params_t       m_params;
    unsigned int            m_tsk_cnt;
    unsigned int            m_cur_cnt;

    int                     m_event1_id;
    osa_event_t             m_event1;
    int                     m_event2_id;
    osa_event_t             m_event2;

    task_t                  m_cur_tsk;

    task_object_t         * m_tsklists[TASK_MGR_TSK_MAX];
};

typedef struct __task_mgr_object_t task_mgr_object_t;
typedef struct __task_mgr_object_t * task_mgr_handle;

/*
 *  --------------------- Public function declaration --------------------------
 */

/** =============================================================================
 *
 *  @Function:	    //	函数名称
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
extern task_mgr_object_t glb_tsk_mgr_obj;

#if defined(__cplusplus)
}
#endif  /* defined(__cplusplus) */

#endif  /* if !defined (__OSA_TSK_MGR_MAIN_H) */
