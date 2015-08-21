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
 *  @Version:	    v1.0
 *
 *  @Function List:  //	主要函数及功能
 *	    1.  －－－－－
 *	    2.  －－－－－
 *
 *  @History:	     //	历史修改记录
 *
 *	<author>	    <time>	     <version>	    <desc>
 *  xiong-kaifang   2012-09-13     v1.0	        Write this module.
 *
 *
 *  ============================================================================
 */

/*  --------------------- Include system headers ---------------------------- */
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

/*  --------------------- Include user headers   ---------------------------- */
#include "osa.h"
#include "dlist.h"
#include "std_defs.h"
#include "thread.h"
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
struct __thread_t {
    DLIST_ELEMENT_RESERVED;
    pthread_t           thd_id;
    pthread_attr_t      pattrs;

    Fxn                 fxn;
    Arg                 args[THREAD_MAX_ARGS];

    int                 pri;
    char              * name;
    int                 thd_status;
    int                 exit_status;

    void              * arg;
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

static int              cur_init = 0;
static pthread_key_t    thd_key;

static const char * const GT_NAME = "thread";
/*
 *  --------------------- Local function forward declaration -------------------
 */

/** ============================================================================
 *
 *  @Function:	    Local function forward declaration.
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
static void   __thread_cleanup(void *hdl);

static void * __thread_run_stub(thread_handle hdl);

/*
 *  --------------------- Public function definition ---------------------------
 */

/** ============================================================================
 *
 *  @Function:	    Public function definition.
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
int             thread_init(void)
{
    if (cur_init++ == 0) {
        pthread_key_create(&thd_key, NULL);
    }

    return 0;
}

void            thread_exit(void)
{
    if (--cur_init == 0 ) {
        pthread_key_delete(thd_key);
    }
}

thread_handle   thread_create(Fxn fxn, thread_attrs_t * attrs, ...)
{
    int           i;
    va_list       va;
    thread_handle thd_hdl = NULL;

    DBG(DBG_DETAILED, GT_NAME, "thread_create: Enter (fxn=0x%x, attrs=0x%x)\n", 
            fxn, attrs);

    if (attrs == NULL) {
        attrs = &default_thd_attrs;
    }

    thd_hdl = calloc(1, sizeof(*thd_hdl));
    if (thd_hdl != NULL) {
        thd_hdl->pri   = attrs->priority;

        /*
         *  TODO: Bug here, the attrs variable may be exist at stack,
         *        the thd_hdl->name pointer may be invalid soon.
         */
        thd_hdl->name  = attrs->name;

        thd_hdl->arg   = attrs->environ;

        thd_hdl->fxn   = fxn;

        thd_hdl->thd_status  = -1;
        thd_hdl->exit_status = -1;

        /* Copy up to 8 arguments from task */
        va_start(va, attrs);
        for (i = 0; i < OSA_ARRAYSIZE(thd_hdl->args); i++) {
            thd_hdl->args[i] = va_arg(va, Arg);
        }
        va_end(va);

        /* Create a realtime thread */
        pthread_attr_init(&thd_hdl->pattrs);

        thd_hdl->thd_status = pthread_create(&thd_hdl->thd_id, &thd_hdl->pattrs,
             (void * (*)(void *))__thread_run_stub, (void *)thd_hdl);
        if (thd_hdl->thd_status != 0) {
            thread_delete(thd_hdl);
            thd_hdl = NULL;
        }
    }

    DBG(DBG_DETAILED, GT_NAME, "thread_create: Exit (hdl=0x%x)\n", thd_hdl);

    return thd_hdl;
}

void            thread_delete(thread_handle hdl)
{
    DBG(DBG_DETAILED, GT_NAME, "thread_delete: Enter (hdl=0x%x)\n", hdl);

    if (hdl != NULL) {
        unsigned short cancel = (hdl->thd_status == 0);

        if (hdl != thread_self()) {
            if (cancel) {
                int r;

                r = pthread_cancel(hdl->thd_id);
                r = pthread_join(hdl->thd_id, NULL);
            }
        }

        free(hdl);
    }

    DBG(DBG_DETAILED, GT_NAME, "thread_delete: Exit\n");
}

//int             thread_get_pri(thread_handle hdl);

void *          thread_get_env(thread_handle hdl)
{
    return (hdl != NULL ? hdl->arg : NULL);
}

const char *    thread_get_name(thread_handle hdl)
{
    return (hdl != NULL ? hdl->name : NULL);
}

//int             thread_set_pri(thread_handle hdl, int newpri);

//void *          thread_set_env(thread_handle hdl, void *env);

const char *    thread_set_name(thread_handle hdl, char *name)
{
    char * orign_name = NULL;

    if (hdl != NULL) {
        orign_name = hdl->name;
        hdl->name  = name;
    }

    return orign_name;
}

thread_handle   thread_self(void)
{
    thread_handle   hdl = NULL;

    DBG(DBG_DETAILED, GT_NAME, "thread_self: Enter\n");

    hdl = (thread_handle)pthread_getspecific(thd_key);

    DBG(DBG_DETAILED, GT_NAME, "thread_self: Exit (hdl=0x%x)\n", hdl);

    return hdl;
}

int             thread_cancel(thread_handle hdl)
{
    int     status = 0;

    DBG(DBG_DETAILED, GT_NAME, "thread_cancel: Enter (hdl=0x%x)\n", hdl);

    if (hdl == NULL) {
        DBG(DBG_ERROR, GT_NAME, "thread_cancel: Invalid arguments\n");
        return -EINVAL;
    }

    status = pthread_cancel(hdl->thd_id);

    DBG(DBG_DETAILED, GT_NAME, "thread_cancel: Exit (status=0x%x)\n", status);

    return status;
}


int             thread_join(thread_handle hdl)
{
    int     status = 0;

    DBG(DBG_DETAILED, GT_NAME, "thread_join: Enter\n");

    if (hdl == NULL) {
        DBG(DBG_ERROR, GT_NAME, "thread_join: Invalid arguments\n");
        return -EINVAL;
    }

    status = pthread_join(hdl->thd_id, NULL);

    DBG(DBG_DETAILED, GT_NAME, "thread_join: Exit (status=0x%x)\n", status);

    return status;

}

//int             thread_stat(thread_handle hdl);

void            thread_yield(void)
{

    DBG(DBG_DETAILED, GT_NAME, "thread_cancel: Enter\n");

    sched_yield();

    DBG(DBG_DETAILED, GT_NAME, "thread_cancel: Exit\n");
}


/*
 *  --------------------- Local function definition ----------------------------
 */

/** ============================================================================
 *
 *  @Function:	    Local function definition.
 *
 *  @Description:   //	函数功能、性能等的描述
 *
 *  ============================================================================
 */
static void * __thread_run_stub(thread_handle hdl)
{
    pthread_setspecific(thd_key, hdl);

    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    hdl->exit_status = (int)hdl->fxn(
            hdl->args[0], hdl->args[1],hdl->args[2], hdl->args[3]
            );

    /* TODO: Exit this thread */
    //pthread_exit(hdl);

    return NULL;
}

static void __thread_cleanup(void *hdl)
{
    if (--cur_init == 0) {
        pthread_key_delete(thd_key);
    }
}

#if defined(__cplusplus)
}
#endif
