/** ============================================================================
 *
 *  Copyright (C), 1987 - 2016, xiong-kaifang Tech. Co, Ltd.
 *
 *  @File Name:	osa_pqueue.h
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2016-10-05
 *
 *  @Description:   // 用于详细说明此程序文件完成的主要功能，与其它模块
 *                  // 或函数的接口，输出值，取值范围、含义及参数间的控
 *                  // 制、顺序、独立或依赖等关系
 *                  //
 *
 *                  The header file for osa priority queue(using dynamic array).
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
 *  xiong-kaifang   2016-10-05     v1.0	        Write this module.
 *
 *  ============================================================================
 */

#if !defined (__OSA_PQUEUE_H)
#define __OSA_PQUEUE_H

/*  --------------------- Include system headers ---------------------------- */

/*  --------------------- Include user headers   ---------------------------- */
#include "std_defs.h"
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

/** ----------------------------------------------------------------------------
 *  @Name:          pqueue_t
 *
 *  @Description:   Priority queue date type.
 *
 *  ----------------------------------------------------------------------------
 */
typedef HANDLE          pqueue_t;

/** ----------------------------------------------------------------------------
 *  @Name:          PQUEUE_CLEANUP
 *
 *  @Description:   Callback function to cleanup priority queue's elements.
 *
 *  ----------------------------------------------------------------------------
 */
typedef void            (* PQUE_CLEANUP)(void * value, void * userdata);

/** ----------------------------------------------------------------------------
 *  @Name:          pqueue_pri_t
 *
 *  @Description:   Priority date type.
 *
 *  ----------------------------------------------------------------------------
 */
typedef unsigned int    pqueue_pri_t;

/** ----------------------------------------------------------------------------
 *  @Name:          PQUEUE_CMP_PRI_FXN
 *
 *  @Description:   Callback function to compare the priority of an element.
 *
 *  ----------------------------------------------------------------------------
 */
typedef pqueue_pri_t    (* PQUEUE_GET_PRI_FXN)(void * vaule);

/** ----------------------------------------------------------------------------
 *  @Name:          PQUEUE_CMP_PRI_FXN
 *
 *  @Description:   Callback function to set the priority of an element.
 *
 *  ----------------------------------------------------------------------------
 */
typedef void            (* PQUEUE_SET_PRI_FXN)(void * vaule, pqueue_pri_t pri);

/** ----------------------------------------------------------------------------
 *  @Name:          PQUEUE_CMP_PRI_FXN
 *
 *  @Description:   Callback function to get the priority of an element. This
 *                  callback function to run to compare two elemetns. This
 *                  callback should return 0 for 'lower' and non-zero for
 *                  'higher', or vice versa if reverse priority is desired.
 *
 *                  If curr's priority is higher than next's, return non-zero.
 *                  If curr's priority is lower  than next's, return zero.
 *
 *  ----------------------------------------------------------------------------
 */
typedef int             (* PQUEUE_CMP_PRI_FXN)(pqueue_pri_t next, pqueue_pri_t curr);

/** ----------------------------------------------------------------------------
 *  @Name:          PQUEUE_GET_POS_FXN
 *
 *  @Description:   Callback function to get the position of an element.
 *
 *  ----------------------------------------------------------------------------
 */
typedef size_t          (* PQUEUE_GET_POS_FXN)(void * vaule);

/** ----------------------------------------------------------------------------
 *  @Name:          PQUEUE_SET_POS_FXN
 *
 *  @Description:   Callback function to set the position of an element.
 *
 *  ----------------------------------------------------------------------------
 */
typedef void            (* PQUEUE_SET_POS_FXN)(void * vaule, size_t pos);

/** ----------------------------------------------------------------------------
 *  @Name:          PQUEUE_PRINT_ENTRY_FXN
 *
 *  @Description:   Debug callback function to print an element entry.
 *
 *  ----------------------------------------------------------------------------
 */
typedef void            (* PQUEUE_PRINT_ENTRY_FXN)(FILE * out, void * vaule);

/*
 *  --------------------- Public function declaration --------------------------
 */

/** =============================================================================
 *
 *  @Function:      pqueue_create
 *
 *  @Description:   Create and initialize a priority queue.
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
status_t pqueue_create  (pqueue_t         * pque,
                         unsigned int       maxsize,
                         PQUEUE_CMP_PRI_FXN cmppri,
                         PQUEUE_GET_PRI_FXN getpri,
                         PQUEUE_SET_PRI_FXN setpri,
                         PQUEUE_GET_POS_FXN getpos,
                         PQUEUE_SET_POS_FXN setpos
                        );

status_t pqueue_put     (pqueue_t   que , void * value);
status_t pqueue_get     (pqueue_t   que , void ** pvalue);
status_t pqueue_peek    (pqueue_t   que , void ** pvalue);
status_t pqueue_remove  (pqueue_t   que , void * value);
status_t pqueue_change_priority(pqueue_t que , void * value, pqueue_pri_t new_pri);
status_t pqueue_size    (pqueue_t   que , unsigned int * psize);
bool_t   pqueue_is_empty(pqueue_t   que);
bool_t   pqueue_is_full (pqueue_t   que);
status_t pqueue_cleanup (pqueue_t   que , PQUE_CLEANUP cleanup, void * userdata);
status_t pqueue_delete  (pqueue_t * pque);

status_t pqueue_print   (pqueue_t   que, FILE * out, PQUEUE_PRINT_ENTRY_FXN print);
status_t pqueue_dump    (pqueue_t   que, FILE * out, PQUEUE_PRINT_ENTRY_FXN print);
bool_t   pqueue_is_valid(pqueue_t   que);

#if defined(__cplusplus)
}
#endif  /* defined(__cplusplus) */

#endif  /* if !defined (__OSA_PQUEUE_H) */
