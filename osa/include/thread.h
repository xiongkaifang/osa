/*
 *  thread.h
 *
 *  Created on: Sep 13, 2012
 *  Author    : xkf
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
