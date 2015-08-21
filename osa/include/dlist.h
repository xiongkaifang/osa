/** ============================================================================
 *
 *  Copyright (C), 1987 - 2015, xiong-kaifang Tech. Co, Ltd.
 *
 *  @File Name:	dlist.h
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2012-09-14
 *
 *  @Description:   //	用于详细说明此程序文件完成的主要功能，与其它模块
 *		            //	或函数的接口，输出值，取值范围、含义及参数间的控
 *		            //	制、顺序、独立或依赖等关系
 *		            //
 *
 *	                The header file for double link list.
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
 *  xiong-kaifang   2012-09-14     v1.0	        Write this module.
 *
 *  xiong-kaifang   2015-08-21     V1.1         Add macro
 *                                              'DLIST_ELEMENT_RESERVED'.
 *
 *  ============================================================================
 */

#if !defined (DLIST_H_)
#define DLIST_H_

/*  --------------------- Include system headers ---------------------------- */
#include <stdbool.h>

/*  --------------------- Include user headers   ---------------------------- */
#include "std_defs.h"

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
#define DLIST_ELEMENT_RESERVED Arg \
    m_reserved[2]

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
