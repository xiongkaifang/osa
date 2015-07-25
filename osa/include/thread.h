/** ============================================================================
 *
 *  Copyright (C), 1987 - 2015, xiong-kaifang Tech. Co, Ltd.
 *
 *  @File Name:	thread.h
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2012-09-13
 *
 *  @Description:   //	用于详细说明此程序文件完成的主要功能，与其它模块
 *		            //	或函数的接口，输出值，取值范围、含义及参数间的控
 *		            //	制、顺序、独立或依赖等关系
 *		            //
 *
 *	                The header file thread.
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
 *  xiong-kaifang   2012-09-13     v1.0	        Write this module.
 *
 *  ============================================================================
 */

#if !defined (THREAD_H_)
#define THREAD_H_

/*  --------------------- Include system headers ---------------------------- */
#include "std_defs.h"

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
#define MAILBOX_MAX_MSG_SIZE    (64)

/*
 *  --------------------- Data type definition ---------------------------------
 */
struct __thread_t; typedef struct __thread_t thread_t, *thread_handle;

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
struct __thread_attrs_t;
typedef struct __thread_attrs_t thread_attrs_t;

struct __thread_attrs_t {
    int     priority;
    int     stacksize;
    int     stackseg;
    char  * name;
    void  * environ;
};

struct __thread_stat_t;
typedef struct __thread_stat_t thread_stat_t;

struct __thread_stat_t {
    int     stacksize;
    int     stackused;
};

extern thread_attrs_t default_thd_attrs;

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
thread_handle   thread_create(Fxn fxn, thread_attrs_t * attrs, ...);

void            thread_delete(thread_handle hdl);

//int             thread_get_pri(thread_handle hdl);

void *          thread_get_env(thread_handle hdl);

const char *    thread_get_name(thread_handle hdl);

//int             thread_set_pri(thread_handle hdl, int newpri);

//void *          thread_set_env(thread_handle hdl, void *env);

const char *    thread_set_name(thread_handle hdl, char *name);

thread_handle   thread_self(void);

int             thread_cancel(thread_handle hdl);

int             thread_join(thread_handle hdl);

//int             thread_stat(thread_handle hdl);

void            thread_yield(void);

#if defined(__cplusplus)
}
#endif  /* defined(__cplusplus) */

#endif  /* if !defined (THREAD_H_) */
