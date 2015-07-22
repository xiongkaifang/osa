/*
 *  std_defs.h
 *
 *  Create on: Sep 12, 2012
 *  Author   : xkf
 */

#if !defined (STD_DEFS_H_)
#define STD_DEFS_H_

/*  --------------------- Include system headers ---------------------------- */
#include <linux/types.h>

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

/*
 *  --------------------- Data type definition ---------------------------------
 */

typedef unsigned char   u_int8_t;
typedef unsigned short  u_int16_t;
typedef unsigned int    u_int32_t;

typedef int (*Fxn)();

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

#if defined(__cplusplus)
}
#endif  /* defined(__cplusplus) */

#endif  /* if !defined (STD_DEFS_H_) */
