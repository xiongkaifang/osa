/*
 *  dlist.h
 *
 *  Created on: Sep 14, 2012
 *  Author    : xkf
 */

#if !defined (DLIST_H_)
#define DLIST_H_

/*  --------------------- Include system headers ---------------------------- */
#include <stdbool.h>

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
struct __dlist_element_t; typedef struct __dlist_element_t dlist_element_t;
struct __dlist_element_t {
    struct __dlist_element_t * next;
    struct __dlist_element_t * prev;
};

struct __dlist_t; typedef struct __dlist_t dlist_t;
struct __dlist_t {
    dlist_element_t head;
};

typedef bool (*DLIST_MATCH_FXN)(dlist_element_t * elem, void * data);

typedef int  (*DLIST_APPLY_FXN)(dlist_element_t * elem, void * data);

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
int  dlist_init(dlist_t * list);

int  dlist_create(dlist_t ** list);

int  dlist_delete(dlist_t * list);

int  dlist_is_empty(dlist_t * list);

int  dlist_initialize_element(dlist_element_t * element);

int  dlist_insert_before(dlist_t *          list, 
                         dlist_element_t *  insert_element,
                         dlist_element_t *  existing_element);

int  dlist_put_tail(dlist_t * list, dlist_element_t * element);

int  dlist_remove_element(dlist_t * list, dlist_element_t * element);

int  dlist_first(dlist_t * list, dlist_element_t ** element);

int  dlist_get_head(dlist_t * list, dlist_element_t ** head_element);

int  dlist_next(dlist_t *          list,
                dlist_element_t *  current_element,
                dlist_element_t ** next_element);

int  dlist_search_element(dlist_t *          list,
                          void    *          data,
                          dlist_element_t ** elem,
                          DLIST_MATCH_FXN    match_fxn);

int dlist_map(dlist_t * list, DLIST_APPLY_FXN apply_fxn, void * data);

int dlist_count(dlist_t * list, unsigned int * count);

#if defined(__cplusplus)
}
#endif  /* defined(__cplusplus) */

#endif  /* if !defined (DLIST_H_) */
