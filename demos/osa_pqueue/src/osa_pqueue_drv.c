/** ============================================================================
 *
 *  Copyright (C), 1987 - 2016, xiong-kaifang Tech. Co.,, Ltd.
 *
 *  @File name:	osa_pqueue_drv.c
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2016-10-05
 *
 *  @Description:   The description of this file.
 *	
 *	                The demo to test osa priority queue routines.
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
 *  xiong-kaifang   2016-10-05     v1.0	        write this module.
 *
 *
 *  ============================================================================
 */

/*  --------------------- Include system headers ---------------------------- */

/*  --------------------- Include user headers   ---------------------------- */
#include "osa.h"
#include "osa_pqueue.h"

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
struct __node_t; typedef struct __node_t node_t;
struct __node_t
{
    pqueue_pri_t    m_pri;
    int             m_val;
    size_t          m_pos;
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
static int          __pqueue_cmppri_fxn(pqueue_pri_t next, pqueue_pri_t curr);
static pqueue_pri_t __pqueue_getpri_fxn(void * value);
static void         __pqueue_setpri_fxn(void * value, pqueue_pri_t pri);
static size_t       __pqueue_getpos_fxn(void * value);
static void         __pqueue_setpos_fxn(void * value, size_t pos);

static void         __pqueue_print_fxn (FILE * out, void * pvalue);
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
int main(int argc, char *argv[])
{
    status_t  status;
    pqueue_t  pque;
    node_t  * pnode = NULL;
    node_t  * pelem = NULL;

    pnode = (node_t *)osa_mallocz(10 * sizeof(node_t));
    if (!pnode) {
        return 0;
    }

    status = pqueue_create(&pque, 10,
                           __pqueue_cmppri_fxn,
                           __pqueue_getpri_fxn,
                           __pqueue_setpri_fxn,
                           __pqueue_getpos_fxn,
                           __pqueue_setpos_fxn
                           );
    if (OSA_ISERROR(status)) {
        osa_free(pnode);
    }

	pnode[0].m_pri = 5; pnode[0].m_val = -5; pqueue_put(pque, &pnode[0]);
	pnode[1].m_pri = 4; pnode[1].m_val = -4; pqueue_put(pque, &pnode[1]);
	pnode[2].m_pri = 2; pnode[2].m_val = -2; pqueue_put(pque, &pnode[2]);
	pnode[3].m_pri = 6; pnode[3].m_val = -6; pqueue_put(pque, &pnode[3]);
	pnode[4].m_pri = 1; pnode[4].m_val = -1; pqueue_put(pque, &pnode[4]);

    pqueue_dump(pque, stdout, __pqueue_print_fxn);

    pqueue_peek(pque, (void **)&pelem);
	fprintf(stdout, "peek: %d [%d]\n", pelem->m_pri, pelem->m_val);

	pqueue_change_priority(pque, &pnode[4], 8);
	pqueue_change_priority(pque, &pnode[2], 7);

    pqueue_peek(pque, (void **)&pelem);
	fprintf(stdout, "peek: %d [%d]\n", pelem->m_pri, pelem->m_val);

    pqueue_dump(pque, stdout, __pqueue_print_fxn);

    status = pqueue_get(pque, (void **)&pelem);
    while (!OSA_ISERROR(status)) {
        fprintf(stdout, "pop : %d [%d]\n", pelem->m_pri, pelem->m_val);
        status = pqueue_get(pque, (void **)&pelem);
    }

	pqueue_delete(&pque);
	osa_free(pnode);

    return 0;
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
static int __pqueue_cmppri_fxn(pqueue_pri_t next, pqueue_pri_t curr)
{
    return (next < curr);
}

static pqueue_pri_t __pqueue_getpri_fxn(void * value)
{
    return ((node_t *)value)->m_pri;
}

static void __pqueue_setpri_fxn(void * value, pqueue_pri_t pri)
{
    ((node_t *)value)->m_pri = pri;
}

static size_t __pqueue_getpos_fxn(void * value)
{
    return ((node_t *)value)->m_pos;
}

static void __pqueue_setpos_fxn(void * value, size_t pos)
{
    ((node_t *)value)->m_pos = pos;
}

static void __pqueue_print_fxn (FILE * out, void * pvalue)
{
    fprintf(out, "\t%u %d\n", ((node_t *)pvalue)->m_pri, ((node_t *)pvalue)->m_val);
}

#if defined(__cplusplus)
}
#endif
