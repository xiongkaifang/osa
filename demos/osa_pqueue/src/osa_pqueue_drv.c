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
#include "osa_bstree.h"

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
#define             OSA_BSTREE_TEST

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
    osa_tree_node_t m_tre;

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
#if     defined(OSA_PQUEUE_TEST)
static int          __pqueue_cmppri_fxn(pqueue_pri_t next, pqueue_pri_t curr);
static pqueue_pri_t __pqueue_getpri_fxn(void * value);
static void         __pqueue_setpri_fxn(void * value, pqueue_pri_t pri);
static size_t       __pqueue_getpos_fxn(void * value);
static void         __pqueue_setpos_fxn(void * value, size_t pos);

static void         __pqueue_print_fxn (FILE * out, void * pvalue);

#elif   defined(OSA_BSTREE_TEST)
static int          __bstree_cmp_fxn   (const void *, const void *);
static void         __bstree_apply_fxn (const void *, const void *, void * cl);
#endif

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
#if     defined(OSA_PQUEUE_TEST)
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

#elif   defined(OSA_BSTREE_TEST)
    status_t  status;
    pqueue_t  pbst;
    node_t  * pnode = NULL;
    node_t  * pelem = NULL;

    pnode = (node_t *)osa_mallocz(10 * sizeof(node_t));
    if (!pnode) {
        return 0;
    }

    status = bstree_create(&pbst, __bstree_cmp_fxn);
    if (OSA_ISERROR(status)) {
        osa_free(pnode);
    }

	pnode[0].m_pri = 5; pnode[0].m_val = -5; bstree_insert(pbst, &pnode[0], &pnode[0]);
	pnode[1].m_pri = 4; pnode[1].m_val = -4; bstree_insert(pbst, &pnode[1], &pnode[1]);
	pnode[2].m_pri = 2; pnode[2].m_val = -2; bstree_insert(pbst, &pnode[2], &pnode[2]);
	pnode[3].m_pri = 7; pnode[3].m_val = -6; bstree_insert(pbst, &pnode[3], &pnode[3]);
	pnode[4].m_pri = 2; pnode[4].m_val = -1; bstree_insert(pbst, &pnode[4], &pnode[4]);

    bstree_traverse(pbst, __bstree_apply_fxn, stdout);

    fprintf(stdout, "----------------\n");
    pelem = NULL;
    status = bstree_find(pbst, &pnode[4], &pelem);
    if (!OSA_ISERROR(status)) {
        fprintf(stdout, "pri: %d, val: %d.\n", pelem->m_pri, pelem->m_val);
    }

    fprintf(stdout, "----------------\n");
    status = bstree_find_max(pbst, &pelem);
    while (!OSA_ISERROR(status)) {
        fprintf(stdout, "pri: %d, val: %d.\n", pelem->m_pri, pelem->m_val);

        bstree_remove(pbst, pelem);
        
        status = bstree_find_max(pbst, &pelem);
    }

    bstree_delete(&pbst);
    osa_free(pnode);
#endif

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
#if     defined(OSA_PQUEUE_TEST)
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

#elif   defined(OSA_BSTREE_TEST)
static int          __bstree_cmp_fxn   (const void * next, const void * curr)
{
    node_t * pnext = (node_t *)next;
    node_t * pcurr = (node_t *)curr;

#if 0
    /*
     *  Case 1: bstree_insert.
     */
    if (pcurr->m_pri > pnext->m_pri) {
        /* The keys are higher, insert on the right child */
        return  1;
    } else if (pcurr->m_pri < pnext->m_pri) {
        /* The keys are lower, insert on the left child */
        return -1;
    } else {
        /* The keys are equal, insert on the left child */
        return 0;
    }
#endif

    /*
     *  Case 2: bstree_find.
     */
    if (pcurr->m_pri > pnext->m_pri) {
        /* The keys are higher, find on the right child */
        return  1;
    } else if (pcurr->m_pri < pnext->m_pri) {
        /* The keys are lower, find on the left child */
        return -1;
    } else {
        /*
         *  If the keys value are equal, there are two cases.
         *
         *  Case 1: The curr and next are the same node.
         *
         *  Case 2: The curr and next are different node, but with the same key
         *          value.
         */
        if (pcurr == pnext) {
            /* Case 1: We found it. */
            return 0;
        } else {
            /*
             *  Case 2: Since we suppose that the new node with the same key
             *          vaule is 'lower' than the existing node, we insert or
             *          find it on the left child.
             *
             *  Note  : If we think that the new node with the same key value is
             *          'higher' than the existing node, just return 1 instead.
             */
            return -1;
        }
    }
}

static void         __bstree_apply_fxn (const void * pkey, const void * pval, void * cl)
{
    node_t * pnode = (node_t *)pkey;
    
    fprintf((FILE *)cl, "pri: %d, val: %d.\n", pnode->m_pri, pnode->m_val);
}

#endif

#if defined(__cplusplus)
}
#endif
