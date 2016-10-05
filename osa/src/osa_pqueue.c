/** ============================================================================
 *
 *  Copyright (C), 1987 - 2016, xiong-kaifang Tech. Co.,, Ltd.
 *
 *  @File name:	osa_queue.c
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2016-10-05
 *
 *  @Description:   The osa priority queue(using dynamic array).
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
 *  xiong-kaifang   2016-10-05     v1.0	        Write this module.
 *
 *  ============================================================================
 */

/*  --------------------- Include system headers ---------------------------- */
#include <string.h>

/*  --------------------- Include user headers   ---------------------------- */
#include "osa.h"
#include "osa_mem.h"
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
#define pqueue_left(pos)                    ((pos) << 1)
#define pqueue_right(pos)                   (pqueue_left(pos) + 1)
#define pqueue_parent(pos)                  ((pos) >> 1)

#define pqueue_check_arguments(arg)         osa_check_arguments(arg)
#define pqueue_check_arguments2(arg1, arg2) osa_check_arguments2(arg1, arg2)
#define pqueue_check_arguments3(arg1, arg2, arg3) osa_check_arguments3(arg1, arg2, arg3)

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
struct __pqueue_t
{
    size_t              m_msize;
    size_t              m_avail;
    size_t              m_mstep;

    PQUEUE_CMP_PRI_FXN  m_cmppri_fxn;
    PQUEUE_GET_PRI_FXN  m_getpri_fxn;
    PQUEUE_SET_PRI_FXN  m_setpri_fxn;
    PQUEUE_GET_POS_FXN  m_getpos_fxn;
    PQUEUE_SET_POS_FXN  m_setpos_fxn;

    void **             m_queue;
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
 *  @Function:      Local function forward declaration.
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
static void   __pqueue_percolate_up  (struct __pqueue_t * pque, size_t pos);

static void   __pqueue_percolate_down(struct __pqueue_t * pque, size_t pos);

static size_t __pqueue_maxchild      (struct __pqueue_t * pque, size_t pos);

static bool_t __pqueue_subtree_valid (struct __pqueue_t * pque, size_t pos);

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
status_t pqueue_create  (pqueue_t         * pque,
                         unsigned int       maxsize,
                         PQUEUE_CMP_PRI_FXN cmppri,
                         PQUEUE_GET_PRI_FXN getpri,
                         PQUEUE_SET_PRI_FXN setpri,
                         PQUEUE_GET_POS_FXN getpos,
                         PQUEUE_SET_POS_FXN setpos
                        )
{
    status_t            status = OSA_SOK;
    struct __pqueue_t * que    = NULL;

    if (!pque || !cmppri || !getpri || !setpri || !getpos || !setpos) {
        return OSA_EARGS;
    }

    (*pque) = INVALID_HANDLE;

    que  = (struct __pqueue_t *)osa_mallocz(sizeof(struct __pqueue_t));
    if (!que) {
        return OSA_EMEM;
    }

    /* Need to allocate maxsize + 1 elements since element 0 isn't used */
    que->m_queue = osa_mallocz(sizeof(void *) * (maxsize + 1));
    if (!que->m_queue) {
        osa_free(que);
        return OSA_EMEM;
    }

    que->m_msize      = 1;
    que->m_avail      = maxsize + 1;
    que->m_mstep      = maxsize + 1;

    que->m_cmppri_fxn = cmppri;
    que->m_getpri_fxn = getpri;
    que->m_setpri_fxn = setpri;
    que->m_getpos_fxn = getpos;
    que->m_setpos_fxn = setpos;

    (*pque) = (pqueue_t)que;

    return status;
}

status_t pqueue_put     (pqueue_t   que , void * value)
{
    size_t              pos;
    size_t              newsize;
    void              * tmp    = NULL;
    status_t            status = OSA_SOK;
    struct __pqueue_t * pque   = (struct __pqueue_t *)que;

    pqueue_check_arguments2(pque, value);

    /* Allocate more memory if necessary */
    if (pque->m_msize >= pque->m_avail) {
        newsize = pque->m_msize + pque->m_mstep;

        tmp = (struct __pqueue_t *)osa_realloc(
                pque->m_queue, sizeof(void *) * newsize);
        if (!tmp) {
            return OSA_EMEM;
        }

        pque->m_queue = tmp;
        pque->m_avail = newsize;
    }

    /* Insert item */
    pos                = pque->m_msize++;
    pque->m_queue[pos] = value;

    __pqueue_percolate_up(pque, pos);

    return status;
}

status_t pqueue_get     (pqueue_t   que , void ** pvalue)
{
    status_t            status = OSA_SOK;
    struct __pqueue_t * pque   = (struct __pqueue_t *)que;

    pqueue_check_arguments2(pque, pvalue);

    (*pvalue) = NULL;

    if (pque->m_msize == 1) {
        return OSA_ENOENT;
    }

    (*pvalue)        = pque->m_queue[1];
    pque->m_queue[1] = pque->m_queue[--pque->m_msize];

    __pqueue_percolate_down(pque, 1);

    return status;
}

status_t pqueue_peek    (pqueue_t   que , void ** pvalue)
{
    status_t            status = OSA_SOK;
    struct __pqueue_t * pque   = (struct __pqueue_t *)que;

    pqueue_check_arguments2(pque, pvalue);

    (*pvalue) = NULL;

    if (pque->m_msize == 1) {
        return OSA_ENOENT;
    }

    (*pvalue) = pque->m_queue[1];

    return status;
}

status_t pqueue_remove  (pqueue_t   que , void * value)
{
    size_t              posn;
    status_t            status = OSA_SOK;
    struct __pqueue_t * pque   = (struct __pqueue_t *)que;

    pqueue_check_arguments2(pque, value);

    if (pque->m_msize == 1) {
        return OSA_ENOENT;
    }

    posn = pque->m_getpos_fxn(value);

    /* Move the last node to the position which occupied by the deleted node */
    pque->m_queue[posn] = pque->m_queue[--pque->m_msize];

    if (pque->m_cmppri_fxn(pque->m_getpri_fxn(value),
                           pque->m_getpri_fxn(pque->m_queue[posn]))) {
        /*
         *  Author     : xiong-kaifang.
         *
         *  Date       : Oct 05, 2016.
         *
         *  Description:
         *
         *      If the priority of the last node is higher than current
         *      node's that is being deleted, percolate the last node up.
         */
        __pqueue_percolate_up  (pque, posn);

    } else {
        /*
         *  Author     : xiong-kaifang.
         *
         *  Date       : Oct 05, 2016.
         *
         *  Description:
         *
         *      If the priority of the last node is lower than current
         *      node's that is being deleted, percolate the last node down.
         */
        __pqueue_percolate_down(pque, posn);
    }

    return status;
}

status_t pqueue_change_priority(pqueue_t que , void * value, pqueue_pri_t new_pri)
{
    size_t              posn;
    pqueue_pri_t        old_pri;
    status_t            status = OSA_SOK;
    struct __pqueue_t * pque   = (struct __pqueue_t *)que;

    pqueue_check_arguments2(pque, value);

    if (pque->m_msize == 1) {
        return OSA_ENOENT;
    }

    old_pri = pque->m_getpri_fxn(value);
    pque->m_setpri_fxn(value, new_pri);
    posn    = pque->m_getpos_fxn(value);

    if (pque->m_cmppri_fxn(old_pri, new_pri)) {
        /*
         *  Author     : xiong-kaifang.
         *
         *  Date       : Oct 05, 2016.
         *
         *  Description:
         *
         *      If the new priority of node is higher than old priority,
         *      percolate current node up.
         */
        __pqueue_percolate_up  (pque, posn);

    } else {
        /*
         *  Author     : xiong-kaifang.
         *
         *  Date       : Oct 05, 2016.
         *
         *  Description:
         *
         *      If the new priority of node is lower than old priority,
         *      percolate current node down.
         */
        __pqueue_percolate_down(pque, posn);
    }

    return status;
}

status_t pqueue_size    (pqueue_t   que , unsigned int * psize)
{
    status_t            status = OSA_SOK;
    struct __pqueue_t * pque   = (struct __pqueue_t *)que;

    pqueue_check_arguments2(pque, psize);

    /* Queue element 0 exists but doesn't count since it isn't used */
    (*psize) = pque->m_msize - 1;

    return status;
}

bool_t   pqueue_is_empty(pqueue_t   que)
{
    bool_t              is_empty = FALSE;
    struct __pqueue_t * pque     = (struct __pqueue_t *)que;

    if (pque == NULL) {
        return is_empty;
    }

    if (pque->m_msize == 1) {
        is_empty = TRUE;
    } else {
        is_empty = FALSE;
    }

    return is_empty;
}

bool_t   pqueue_is_full (pqueue_t   que)
{
    bool_t              is_full = TRUE;
    struct __pqueue_t * pque    = (struct __pqueue_t *)que;

    if (pque == NULL) {
        return is_full;
    }

    if (pque->m_msize >= pque->m_avail) {
        is_full = TRUE;
    } else {
        is_full = FALSE;
    }

    return is_full;
}

status_t pqueue_cleanup (pqueue_t   que , PQUE_CLEANUP cleanup, void * userdata)
{
    void              * pvalue = NULL;
    status_t            status = OSA_SOK;
    struct __pqueue_t * pque   = (struct __pqueue_t *)que;

    pqueue_check_arguments(pque);

    if (!cleanup) {
        while (pque->m_msize > 1) {
            pqueue_get(que, &pvalue);
            (*cleanup)(pvalue, userdata);
        }
    }

    return status;
}

status_t pqueue_delete  (pqueue_t * pque)
{
    status_t            status = OSA_SOK;
    struct __pqueue_t * que    = (struct __pqueue_t *)(*pque);

    pqueue_check_arguments2(pque, que);

    osa_free(que->m_queue);
    osa_free(que);

    (*pque) = INVALID_HANDLE;

    return status;
}

status_t pqueue_print   (pqueue_t   que, FILE * out, PQUEUE_PRINT_ENTRY_FXN print)
{
    void              * pvalue = NULL;
    pqueue_t            dummy;
    status_t            status = OSA_SOK;
    struct __pqueue_t * pdum   = NULL;
    struct __pqueue_t * pque   = (struct __pqueue_t *)que;

    pqueue_check_arguments3(pque, out, print);

    status = pqueue_create(&dummy, pque->m_msize,
                           pque->m_cmppri_fxn,
                           pque->m_getpri_fxn,
                           pque->m_setpri_fxn,
                           pque->m_getpos_fxn,
                           pque->m_setpos_fxn);
    if (OSA_ISERROR(status)) {
        return status;
    }

    pdum = (struct __pqueue_t *)dummy;

    pdum->m_msize = pque->m_msize;
    pdum->m_avail = pque->m_avail;
    pdum->m_mstep = pque->m_mstep;

    memcpy(pdum->m_queue, pque->m_queue, (pque->m_msize * sizeof(void *)));

    status = pqueue_get(dummy, &pvalue);
    while (!OSA_ISERROR(status) && pvalue != NULL) {
        (*print)(out, pvalue);

        status = pqueue_get(dummy, &pvalue);
    }

    status = pqueue_delete(&dummy);

    return status;
}

status_t pqueue_dump    (pqueue_t   que, FILE * out, PQUEUE_PRINT_ENTRY_FXN print)
{
    size_t              i;
    status_t            status = OSA_SOK;
    struct __pqueue_t * pque   = (struct __pqueue_t *)que;

    pqueue_check_arguments3(pque, out, print);

    fprintf(stdout, "posn\tleft\tright\tparent\tmaxchild\t...\n") ;

    for (i = 1; i < pque->m_msize; i++) {
        fprintf(stdout, "%d\t%d\t%d\t%d\t%u\t",
                i, pqueue_left(i), pqueue_right(i), pqueue_parent(i),
                (unsigned int)__pqueue_maxchild(pque, i));
        (*print)(out, pque->m_queue[i]);
    }

    return status;
}

bool_t   pqueue_is_valid(pqueue_t   que)
{
    bool_t              valid  = FALSE;
    struct __pqueue_t * pque   = (struct __pqueue_t *)que;

    if (pque) {
        valid = __pqueue_subtree_valid(pque, 1);
    }

    return valid;
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
static void   __pqueue_percolate_up  (struct __pqueue_t * pque, size_t pos)
{
    size_t       parent_node;
    void       * moving_node = pque->m_queue[pos];
    pqueue_pri_t moving_pri  = pque->m_getpri_fxn(moving_node);

    for (parent_node = pqueue_parent(pos);
            ((pos > 1) &&
             pque->m_cmppri_fxn(
                 pque->m_getpri_fxn(pque->m_queue[parent_node]), moving_pri));
            pos = parent_node, parent_node = pqueue_parent(pos)
            ) {

        /*
         *  Author     : xiong-kaifang.
         *
         *  Date       : Oct 05, 2016.
         *
         *  Description:
         *
         *      If the current node's priority is higher than parent child's,
         *      percolate current node up, percolate parent node down.
         */
        pque->m_queue[pos] = pque->m_queue[parent_node];
        pque->m_setpos_fxn(pque->m_queue[pos], pos);
    }

    pque->m_queue[pos] = moving_node;
    pque->m_setpos_fxn(moving_node, pos);
}

static void   __pqueue_percolate_down(struct __pqueue_t * pque, size_t pos)
{
    size_t       child_node;
    void       * moving_node = pque->m_queue[pos];
    pqueue_pri_t moving_pri  = pque->m_getpri_fxn(moving_node);

    while ((child_node = __pqueue_maxchild(pque, pos)) &&
             pque->m_cmppri_fxn(
                 moving_pri, pque->m_getpri_fxn(pque->m_queue[child_node]))
          ) {

        /*
         *  Author     : xiong-kaifang.
         *
         *  Date       : Oct 05, 2016.
         *
         *  Description:
         *
         *      If the priority of current node's maxchild is higher than
         *      current node's, percolate maxchild node up, percolate current
         *      node down.
         */
        pque->m_queue[pos] = pque->m_queue[child_node];
        pque->m_setpos_fxn(pque->m_queue[pos], pos);

        pos = child_node;
    }

    pque->m_queue[pos] = moving_node;
    pque->m_setpos_fxn(moving_node, pos);
}

static size_t __pqueue_maxchild      (struct __pqueue_t * pque, size_t pos)
{
    size_t child_node = pqueue_left(pos);

    if (child_node >= pque->m_msize) {
        return 0;
    }

    if (((child_node + 1) < pque->m_msize) &&
            pque->m_cmppri_fxn(
                pque->m_getpri_fxn(pque->m_queue[child_node]),
                pque->m_getpri_fxn(pque->m_queue[child_node + 1]))) {
        /*
         *  Author     : xiong-kaifang.
         *
         *  Date       : Oct 05, 2016.
         *
         *  Description:
         *
         *      If the left child node's priority is lower than right child's,
         *      use the right childe instead left.
         */
        child_node++;
    }

    return child_node;
}

static bool_t __pqueue_subtree_valid (struct __pqueue_t * pque, size_t pos)
{
    if (pqueue_left(pos) < pque->m_msize) {
        /* Has a left child */
        if (pque->m_cmppri_fxn(
                    pque->m_getpri_fxn(pque->m_queue[pos]),
                    pque->m_getpri_fxn(pque->m_queue[pqueue_left(pos)]))) {
            return FALSE;
        }

        if (!__pqueue_subtree_valid(pque, pqueue_left(pos))) {
            return FALSE;
        }
    }
    if (pqueue_right(pos) < pque->m_msize) {
        /* Has a right child */
        if (pque->m_cmppri_fxn(
                    pque->m_getpri_fxn(pque->m_queue[pos]),
                    pque->m_getpri_fxn(pque->m_queue[pqueue_right(pos)]))) {
            return FALSE;
        }

        if (!__pqueue_subtree_valid(pque, pqueue_right(pos))) {
            return FALSE;
        }
    }

    return TRUE;
}

#if defined(__cplusplus)
}
#endif
