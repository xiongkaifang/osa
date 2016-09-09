/** ============================================================================
 *
 *  Copyright (C), 1987 - 2016, xiong-kaifang Tech. Co, Ltd.
 *
 *  @File Name:	osa_log.h
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2016-09-08
 *
 *  @Description:   // 用于详细说明此程序文件完成的主要功能，与其它模块
 *                  // 或函数的接口，输出值，取值范围、含义及参数间的控
 *                  // 制、顺序、独立或依赖等关系
 *                  //
 *
 *                  The header file for osa logger.
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
 *  xiong-kaifang   2016-09-08     v1.0	        Write this module.
 *
 *  ============================================================================
 */

#if !defined (__OSA_LOG_H)
#define __OSA_LOG_H

/*  --------------------- Include system headers ---------------------------- */
#include <stdio.h>
#include <stdarg.h>

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
#if defined(DEBUG)
#define OSA_LOG(ctx, level, arg...) osa_log(((void *)ctx), (level), ##arg)
#else
#define OSA_LOG(ctx, level, arg...)
#endif

#define OSA_LOG_VERBOSE     0
#define OSA_LOG_DEBUG       1
#define OSA_LOG_INFO        2
#define OSA_LOG_WARNING     3
#define OSA_LOG_ERROR       4
#define OSA_LOG_FATAL       5
#define OSA_LOG_PANIC       6
#define OSA_LOG_QUIET       7

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
struct __osa_log_params_t; typedef struct __osa_log_params_t osa_log_params_t;
struct __osa_log_params_t
{
    int             m_level;
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
status_t osa_log_init(const osa_log_params_t *prm);

void     osa_log(void *ctx, int level, const char *fmt, ...);

void     osa_vlog(void *ctx, int level, const char *fmt, va_list vl);

int      osa_log_get_level(void);

status_t osa_log_set_level(int level);

void     osa_log_set_callback(void (*callback)(void *, int, const char *, va_list));

void     osa_log_default_callback(void *ctx, int level, const char *fmt, va_list vl);

status_t osa_log_deinit(void);

#if defined(__cplusplus)
}
#endif  /* defined(__cplusplus) */

#endif  /* if !defined (__OSA_LOG_H) */
