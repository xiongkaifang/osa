/** ============================================================================
 *
 *  Copyright (C), 1987 - 2015, xiong-kaifang Tech. Co, Ltd.
 *
 *  @File Name:	osa.h
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2013-04-06
 *
 *  @Description:   // 用于详细说明此程序文件完成的主要功能，与其它模块
 *                  // 或函数的接口，输出值，取值范围、含义及参数间的控
 *                  // 制、顺序、独立或依赖等关系
 *                  //
 *
 *                  The common macro and datatypes for osa module.
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
 *  xiong-kaifang   2013-04-06     v1.0	        Write this module.
 *
 *  xiong-kaifang   2013-12-11     v1.1         Add two macro:
 *                                              OSA_ARRAYINDEX and
 *                                              OSA_ARRAYISINVALIDENTRY
 *
 *  xiong-kaifang   2015-09-19     v.2          Add macro used to check arguments.
 *
 *  ============================================================================
 */

#if !defined (__OSA_H)
#define __OSA_H

/*  --------------------- Include system headers ---------------------------- */
#include <stdio.h>

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
#define OSA_memAlloc(size, ptr) (mem_alloc((size), NULL, ((void **)(ptr))))

#define OSA_memFree(size, ptr)  (mem_free((size), NULL, ((void *)(ptr))))

#ifdef  TRUE
#undef  TRUE
#endif
#define TRUE    (1)

#ifdef  FALSE
#undef  FALSE
#endif
#define FALSE   (0)

#define OSA_ARRAYSIZE(array)        ((sizeof(array) / sizeof((array)[0])))

#define OSA_ARRAYINDEX(elem,array)  ((elem) - &((array)[0]))

#define OSA_ARRAYISVALIDENTRY(elem,array)   ((OSA_ARRAYINDEX(elem,array) <  \
                                             OSA_ARRAYSIZE(array))          \
                                             ? TRUE                         \
                                             : FALSE)

#define OSA_TIMEOUT_NONE        (0)
#define OSA_TIMEOUT_FOREVER     (~(0u))

#define OSA_assert(x)  \
{ \
  if( (x) == 0) { \
    fprintf(stderr, " ASSERT (%s|%s|%d)\r\n", __FILE__, __func__, __LINE__); \
    while (getchar()!='q')  \
      ; \
  } \
} 

#define osa_check_arguments(arg)                \
    do {                                        \
        if ((arg) == NULL) { return OSA_EARGS; }\
    } while (0)

#define osa_check_arguments2(arg1, arg2)        \
    do {                                        \
        if ((arg1) == NULL || (arg2) == NULL)   \
            { return OSA_EARGS; }               \
    } while (0)

#define osa_check_arguments3(arg1, arg2, arg3)  \
    do {                                        \
        if ((arg1) == NULL || (arg2) == NULL || (arg3) == NULL) \
            { return OSA_EARGS; }               \
    } while (0)

#define osa_check_arguments4(arg1, arg2, arg3, arg4)    \
    do {                                        \
        if ((arg1) == NULL || (arg2) == NULL || (arg3) == NULL || (arg4) == NULL) \
            { return OSA_EARGS; }               \
    } while (0)

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
typedef unsigned short  Bool;

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

#endif  /* if !defined (__OSA_H) */
