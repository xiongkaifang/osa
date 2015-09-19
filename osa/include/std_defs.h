/** ============================================================================
 *
 *  Copyright (C), 1987 - 2015, xiong-kaifang Tech. Co, Ltd.
 *
 *  @File Name:	std_defs.h
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2012-09-12
 *
 *  @Description:   // 用于详细说明此程序文件完成的主要功能，与其它模块
 *                  // 或函数的接口，输出值，取值范围、含义及参数间的控
 *                  // 制、顺序、独立或依赖等关系
 *                  //
 *
 *                  The header file for standard defines.
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
 *  xiong-kaifang   2012-09-12     v1.0	        Write this module.
 *
 *  xiong-kaifang   2015-08-21     v1.1         Add datatype Arg.
 *
 *  xiong-kaifang   2015-08-22     v1.2         Add datatype 'HANDLE'.
 *
 *  xiong-kaifang   2015-09-19     v1.3         1. Changed 'HANDLE' type from
 *                                                 'void *' to 'unsigned long'.
 *                                              2. Add dateype 'bool_t'.
 *                                              3. Add four macro:
 *                                                 INVALID_HANDLE
 *                                                 HANDLE_IS_VALID
 *                                                 HANDLE_IS_INVALID
 *                                                 HANDLE_TO_POINTER
 *
 *  ============================================================================
 */

#if !defined (__OSA_STD_DEFS_H)
#define __OSA_STD_DEFS_H

/*  --------------------- Include system headers ---------------------------- */
//#include <linux/types.h>

/*  --------------------- Include user headers   ---------------------------- */


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
#define INVALID_HANDLE          ((unsigned long)0)

#define HANDLE_IS_VALID(hdl)    (INVALID_HANDLE != (hdl))

#define HANDLE_IS_INVALID(hdl)  (INVALID_HANDLE == (hdl))

#define HANDLE_TO_POINTER(h)    ((void *)(h))

/*
 *  --------------------- Data type definition ---------------------------------
 */

typedef unsigned char   u_int8_t;
typedef unsigned short  u_int16_t;
typedef unsigned int    u_int32_t;

typedef int             (*Fxn)();

typedef void *          Arg;

typedef unsigned long   HANDLE;

typedef unsigned short  bool_t;

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

#if defined(__cplusplus)
}
#endif  /* defined(__cplusplus) */

#endif  /* if !defined (__OSA_STD_DEFS_H) */
