/** ============================================================================
 *
 *  Copyright (C), 1987 - 2015, xiong-kaifang Tech. Co.,, Ltd.
 *
 *  @File name:	thread.c
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2012-09-13
 *
 *  @Description:   The thread.
 *
 *
 *  @Version:       v1.0
 *
 *  @Function List: // 主要函数及功能
 *      1.  －－－－－
 *      2.  －－－－－
 *
 *  @History:       // 历史修改记录
 *
 *  <author>        <time>       <version>      <description>
 *
 *  xiong-kaifang   2012-09-13     v1.0	        Write this module.
 *
 *  xiong-kaifang   2015-08-22     v1.1         Destroy thread attributes
 *                                              object.
 *
 *  xiong-kaifang   2015-09-18     v1.2         1. Create new thread detached.
 *                                              2. Add 'THREAD_DEBUG' macro.
 *                                              2. Delete 'thread_handle'
 *                                                 datatype, using 'thread_t'.
 *
 *  ============================================================================
 */

/*  --------------------- Include system headers ---------------------------- */
#include <stdarg.h>
#include <pthread.h>

/*  --------------------- Include user headers   ---------------------------- */
#include "osa.h"
#include "dlist.h"
#include "std_defs.h"
#include "thread.h"
#include "osa_mem.h"
#include "osa_status.h"
#include "osa_debugger.h"

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
#define THREAD_MAX_ARGS (4)

#define thread_check_arguments(arg)         osa_check_arguments(arg)

#define thread_check_arguments2(arg1, arg2) osa_check_arguments2(arg1, arg2)

/** ============================================================================
 *  @Macro:         THREAD_DEBUG
 *
 *  @Description:   Description of this macro.
 *  ============================================================================
 */
//#define THREAD_DEBUG

#if !defined(THREAD_DEBUG)

#ifdef DBG
#undef DBG
#define DBG(level, tag, arg...)
#endif

#endif  /* if defined THREAD_DEBUG */
/*
 *  --------------------- Structure definition ---------------------------------
 */

/** ----------------------------------------------------------------------------
 *  @Name:          Structure name
 *
 *  @Description:   Description of the structure.
 *
 *  @Field:         Field1 member
 *
 *  @Field          Field2 member
 *  ----------------------------------------------------------------------------
 */
struct __thread_t
{
    DLIST_ELEMENT_RESERVED;

    pthread_t       thd_id;
    pthread_attr_t  pattrs;

    Fxn             fxn;
    Arg             args[THREAD_MAX_ARGS];

    int             pri;
    char          * name;
    int             thd_status;
    int             exit_status;

    void          * env;
};

/*
 *  --------------------- Global variable definition ---------------------------
 */

/** ----------------------------------------------------------------------------
 *  @Name:          Variable name
 *
 *  @Description:   Description of the variable.
 * -----------------------------------------------------------------------------
 */
thread_attrs_t default_thd_attrs = {
    1,
    1024,
    0,
    NULL
};

static int                glb_cur_init = 0;

static pthread_key_t      glb_thd_key;

static const char * const GT_NAME = "thread";
/*
 *  --------------------- Local function forward declaration -------------------
 */

/** ============================================================================
 *
 *  @Function:      Local function forward declaration.
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
 *  @Output:        //	对输出参数的说明
 *
 *  @Return:        //	函数返回值的说明
 *
 *  @Enter          //  Precondition
 *
 *  @Leave          //  Postcondition
 *
 *  @Others:        //	其它说明
 *
 *  ============================================================================
 */
static void   __thread_cleanup(void *hdl);

static void * __thread_run_stub(struct __thread_t *pthd);

/*
 *  --------------------- Public function definition ---------------------------
 */

/** ============================================================================
 *
 *  @Function:      Public function definition.
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
int          thread_init(void)
{
    if (glb_cur_init++ == 0) {
        pthread_key_create(&glb_thd_key, NULL);
    }

    return 0;
}

void         thread_exit(void)
{
    if (--glb_cur_init == 0 ) {
        pthread_key_delete(glb_thd_key);
    }
}

status_t     thread_create(thread_t *pthd, Fxn fxn, thread_attrs_t * attrs, ...)
{
    int                 i;
    va_list             va;
    status_t            status = OSA_SOK;
    struct __thread_t * thd    = NULL;

    thread_check_arguments2(pthd, fxn);

    (*pthd) = INVALID_HANDLE;

    status = OSA_memAlloc(sizeof(struct __thread_t), &thd);
    if (OSA_ISERROR(status) || thd == NULL) {
        return status;
    }

    thd->pri         = attrs->priority;

    /*
     *  TODO: Bug here, the attrs variable may be exist at stack,
     *        the thd_hdl->name pointer may be invalid soon.
     */
    thd->name        = attrs->name;

    thd->env         = attrs->environ;
    thd->fxn         = fxn;
    thd->thd_status  = -1;
    thd->exit_status = -1;

    /* Copy up to 'THREAD_MAX_ARGS' arguments from task */
    va_start(va, attrs);
    for (i = 0; i < OSA_ARRAYSIZE(thd->args); i++) {
        thd->args[i] = va_arg(va, Arg);
    }
    va_end(va);

    /* Create a realtime detached thread */
    pthread_attr_init(&thd->pattrs);
    pthread_attr_setdetachstate(&thd->pattrs, PTHREAD_CREATE_DETACHED);

    thd->thd_status = pthread_create(&thd->thd_id, &thd->pattrs,
            (void * (*)(void *))__thread_run_stub, (void *)thd);
    if (thd->thd_status != 0) {
        thread_delete((thread_t *)&thd);
    }

    (*pthd) = (thread_t)thd;

    return status;
}

status_t     thread_delete(thread_t *pthd)
{
    status_t            status = OSA_SOK;
    struct __thread_t * thd    = (struct __thread_t *)(*pthd);

    DBG(DBG_DETAILED, GT_NAME, "thread_delete: Enter (pthd=0x%x).\n", thd);

    thread_check_arguments2(pthd, thd);

    unsigned short cancel = (thd->thd_status == 0);

    if (((thread_t)thd) != thread_self()) {
        if (cancel) {
            int r;

            r = pthread_cancel(thd->thd_id);
            r = pthread_join(thd->thd_id, NULL);
        }
    }

    /*
     *  Modified by: xiong-kaifang.
     *
     *  Date       : Aug 22, 2015.
     *
     *  Description:
     *
     *      Destroy thread attributes object.
     *
     */
    pthread_attr_destroy(&thd->pattrs);

    status |= OSA_memFree(sizeof(struct __thread_t), thd);

    (*pthd) = INVALID_HANDLE;

    DBG(DBG_DETAILED, GT_NAME, "thread_delete: Exit.\n");

    return status;
}

//int        thread_get_pri(thread_t thd);

void *       thread_get_env(thread_t thd)
{
    struct __thread_t * pthd = (struct __thread_t *)thd;

    return (pthd != NULL ? pthd->env : NULL);
}

const char * thread_get_name(thread_t thd)
{
    struct __thread_t * pthd = (struct __thread_t *)thd;

    return (pthd != NULL ? pthd->name : NULL);
}

//int        thread_set_pri(thread_t thd, int newpri);

//void *     thread_set_env(thread_t thd, void *env);

const char * thread_set_name(thread_t thd, char *name)
{
    char              * orign_name = NULL;
    struct __thread_t * pthd = (struct __thread_t *)thd;

    if (pthd != NULL) {
        orign_name = pthd->name;
        pthd->name = name;
    }

    return orign_name;
}

thread_t     thread_self(void)
{
    struct __thread_t * pthd = NULL;

    DBG(DBG_DETAILED, GT_NAME, "thread_self: Enter.\n");

    pthd = (struct __thread_t *)pthread_getspecific(glb_thd_key);

    DBG(DBG_DETAILED, GT_NAME, "thread_self: Exit (pthd=0x%x).\n", pthd);

    return ((thread_t)pthd);
}

int          thread_cancel(thread_t thd)
{
    int                status = OSA_SOK;
    struct __thread_t * pthd = (struct __thread_t *)thd;

    DBG(DBG_DETAILED, GT_NAME, "thread_cancel: Enter (thd=0x%x).\n", thd);

    thread_check_arguments(pthd);

    status |= pthread_cancel(pthd->thd_id);

    DBG(DBG_DETAILED, GT_NAME, "thread_cancel: Exit (status=0x%x).\n", status);

    return status;
}

int          thread_join(thread_t thd)
{
    int                 ret    = -1;
    int                 status = OSA_SOK;
    struct __thread_t * pthd   = (struct __thread_t *)thd;

    DBG(DBG_DETAILED, GT_NAME, "thread_join: Enter.\n");

    thread_check_arguments(pthd);

    status = pthread_join(pthd->thd_id, (void **)&ret);

    DBG(DBG_DETAILED, GT_NAME, "thread_join: Exit (status=0x%x, ret=%d).\n", status, ret);

    return status = OSA_SOK;
}

//int          thread_stat(thread_t thd);

void         thread_yield(void)
{

    DBG(DBG_DETAILED, GT_NAME, "thread_yield: Enter.\n");

    sched_yield();

    DBG(DBG_DETAILED, GT_NAME, "thread_yield: Exit.\n");
}

/*
 *  --------------------- Local function definition ----------------------------
 */

/** ============================================================================
 *
 *  @Function:      Local function definition.
 *
 *  @Description:   // 函数功能、性能等的描述
 *
 *  ============================================================================
 */
static void * __thread_run_stub(struct __thread_t *pthd)
{
    int status = 0;

    pthread_setspecific(glb_thd_key, pthd);

    //pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

    status = (int)pthd->fxn(
            pthd->args[0], pthd->args[1], pthd->args[2], pthd->args[3]
            );

    /* TODO: Exit this thread */
    //pthread_exit(hdl);

    return ((void *)status);
}

static void __thread_cleanup(void *hdl)
{
    if (--glb_cur_init == 0) {
        pthread_key_delete(glb_thd_key);
    }
}

#if defined(__cplusplus)
}
#endif
