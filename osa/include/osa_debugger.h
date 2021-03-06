/** ============================================================================
 *
 *  Copyright (C), 1987 - 2015, xiong-kaifang Tech. Co, Ltd.
 *
 *  @File Name:	osa_debugger.h
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2013-11-20
 *
 *  @Description:   // 用于详细说明此程序文件完成的主要功能，与其它模块
 *                  // 或函数的接口，输出值，取值范围、含义及参数间的控
 *                  // 制、顺序、独立或依赖等关系
 *                  //
 *
 *                  The header file for osa debugger.
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
 *  xiong-kaifang   2013-11-20     v1.0	        Write this module.
 *
 *  xiong-kaifang   2016-09-09     v1.1         Add backward compatibility macro.
 *
 *  ============================================================================
 */

#if !defined (__OSA_DEBUGGER_H)
#define __OSA_DEBUGGER_H

/*  --------------------- Include system headers ---------------------------- */
#include <stdio.h>

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
#if defined(OSA_LOG)

#define DBG_DETAILED    OSA_LOG_VERBOSE
#define DBG_INFO        OSA_LOG_INFO
#define DBG_WARNING     OSA_LOG_WARNING
#define DBG_ERROR       OSA_LOG_ERROR
#define DBG_FATAL       OSA_LOG_FATAL
#define DBG_SILENT      OSA_LOG_QUIET

#define DBG(level, tag, arg...) osa_log(((void *)tag), (level), ##arg)

#else

#define DBG_DETAILED    0
#define DBG_INFO        1
#define DBG_WARNING     2
#define DBG_ERROR       3
#define DBG_FATAL       4
#define DBG_SILENT      1000

#define DBG(level, tag, arg...) osa_debugger((level), (tag), ##arg);

#endif

// For back compatience
#define SUCCEEDED(r)    (!OSA_ISERROR(r))
#define FAILED(r)       (OSA_ISERROR(r))

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
struct __osa_debugger_prm_t;
typedef struct __osa_debugger_prm_t osa_debugger_prm_t;
struct __osa_debugger_prm_t
{
    int             m_debug_level;
    FILE *          m_out;
    unsigned char * m_name;
    unsigned char * m_folder;
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
status_t osa_debugger_init(const osa_debugger_prm_t *prm);

void     osa_debugger(int level, const char *tags, const char *fmt, ...);

status_t osa_debugger_setlevel(int level);

status_t osa_debugger_deinit(void);

#if defined(__cplusplus)
}
#endif  /* defined(__cplusplus) */

#endif  /* if !defined (__OSA_DEBUGGER_H) */
