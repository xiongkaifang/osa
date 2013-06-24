/*
 *  dlist.c
 *
 *  Created on: Sep 14, 2012
 *  Author    : xkf
 *
 */

/*  --------------------- Include system headers ---------------------------- */
#include <string.h>
#include <stdlib.h>
#include <errno.h>

/*  --------------------- Include user headers   ---------------------------- */
#include "dlist.h"
#include "debug.h"
//#include "message.h"


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
static int
__dlist_count_apply_fxn(dlist_element_t * elem, void * data);

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
int  dlist_init(dlist_t * list)
{
    int       status = 0;
    dlist_t * mylist = NULL;

    DBG(DBG_DETAILED, "dlist_init: Enter (list=0x%x)\n", list);

    if (list == NULL) {
        return -EINVAL;
    } else {
        list->head.next = &(list->head);
        list->head.prev = &(list->head);
    }

    DBG(DBG_DETAILED, "dlist_init: Exit (list=0x%x)\n", list);

    return status;
}

int  dlist_create(dlist_t ** list)
{
    int       status = 0;
    dlist_t * mylist = NULL;

    DBG(DBG_DETAILED, "dlist_create: Enter (list=0x%x)\n", list);

    if (list == NULL) {
        return -EINVAL;
    } else {
        mylist = calloc(1, sizeof(*mylist));
        if (mylist == NULL) {
            status = -ENOMEM;
        } else {
            mylist->head.next = &(mylist->head);
            mylist->head.prev = &(mylist->head);

            (*list) = mylist;
        }
    }

    DBG(DBG_DETAILED, "dlist_create: Exit (list=0x%x)\n", list);

    return status;
}

int  dlist_delete(dlist_t * list)
{
    int     status = 0;

    DBG(DBG_DETAILED, "dlist_delete: Enter (list=0x%x)\n", list);

    if (list == NULL) {
        status = -EINVAL;
    } else {
        free(list);
    }

    DBG(DBG_DETAILED, "dlist_delete: Exit (list=0x%x)\n", list);

    return status;
}

int  dlist_is_empty(dlist_t * list)
{
    return ((list->head.next) == (&list->head));
}

int  dlist_initialize_element(dlist_element_t * element)
{
    int     status = 0;

    DBG(DBG_DETAILED, "dlist_initialize_element: Enter (element=0x%x)\n",
            element);

    if (element == NULL) {
        status = -EINVAL; 
    } else {
        element->next = element->prev = NULL;
    }

    DBG(DBG_DETAILED, "dlist_initialize_element: Exit (element=0x%x)\n",
            element);

    return status;
}


int  dlist_insert_before(dlist_t *          list, 
                         dlist_element_t *  insert_element,
                         dlist_element_t *  existing_element)
{
    int     status = 0;

    DBG(DBG_DETAILED, "dlist_insert_before: Enter (list=0x%x, "
            "insert_element=0x%x, existing_element=0x%x)\n", 
            list, insert_element, existing_element);

    if (list == NULL || insert_element == NULL || existing_element == NULL) {
        status = -EINVAL;
    } else {
        existing_element->prev->next = insert_element;
        insert_element->prev         = existing_element->prev;
        insert_element->next         = existing_element;
        existing_element->prev       = insert_element;
    }

    DBG(DBG_DETAILED, "dlist_insert_before: Exit (list=0x%x, "
            "insert_element=0x%x, existing_element=0x%x)\n", 
            list, insert_element, existing_element);

    return status;
}


int  dlist_put_tail(dlist_t * list, dlist_element_t * element)
{
    int     status = 0;

    DBG(DBG_DETAILED, "dlist_put_tail: Enter (list=0x%x, element=0x%x)\n",
            list, element);

    if (list == NULL || element == NULL) {
        status = -EINVAL;
    } else {
        element->prev       = list->head.prev;
        element->next       = &list->head;
        list->head.prev     = element;
        element->prev->next = element;
    }

    DBG(DBG_DETAILED, "dlist_put_tail: Exit (list=0x%x, element=0x%x)\n",
            list, element);

    return status;
}


int  dlist_remove_element(dlist_t * list, dlist_element_t * element)
{
    int     status = 0;

    DBG(DBG_DETAILED, "dlist_remove_element: Enter (list=0x%x, element=0x%x)\n",
            list, element);

    if (list == NULL || element == NULL) {
        status = -EINVAL;
    } else {
        if (!dlist_is_empty(list)) {
            element->prev->next = element->next;
            element->next->prev = element->prev;

            element->next = element->prev = NULL;
        } else {
            status = -ENOENT;
        }
    }

    DBG(DBG_DETAILED, "dlist_remove_element: Exit (list=0x%x, element=0x%x)\n",
            list, element);

    return status;
}

int  dlist_first(dlist_t * list, dlist_element_t ** element)
{
    int     status = 0;

    DBG(DBG_DETAILED, "dlist_first: Enter (list=0x%x, element=0x%x)\n",
            list, element);

    if (list == NULL || element == NULL) {
        status = -EINVAL;
    } else {
        if (!dlist_is_empty(list)) {
            (*element) = list->head.next;
        } else {
            (*element) = NULL;
        }
    }

    DBG(DBG_DETAILED, "dlist_first: Exit (list=0x%x, element=0x%x)\n",
            list, element);

    return status;
}

int  dlist_get_head(dlist_t * list, dlist_element_t ** head_element)
{
    int     status = 0;

    DBG(DBG_DETAILED, "dlist_get_head: Enter (list=0x%x, element=0x%x)\n",
            list, head_element);

    if (list == NULL || head_element == NULL) {
        status = -EINVAL;
    } else {
        if (!dlist_is_empty(list)) {
            (*head_element) = list->head.next;
            list->head.next = (*head_element)->next;
            (*head_element)->next->prev = &list->head;
        } else {
            (*head_element) = NULL;
        }
    }

    DBG(DBG_DETAILED, "dlist_get_head: Exit (list=0x%x, element=0x%x)\n",
            list, head_element);

    return status;
}


int  dlist_next(dlist_t *          list,
                dlist_element_t *  current_element,
                dlist_element_t ** next_element)
{
    int     status = 0;

    DBG(DBG_DETAILED, "dlist_next: Enter (list=0x%x, current_element=0x%x "
            "next_element=0x%x)\n", list, current_element, next_element);

    if (list == NULL || current_element == NULL || next_element == NULL) {
        status = -EINVAL;
    } else {
        (*next_element) = NULL;

        if (!dlist_is_empty(list)) {
            if (current_element->next != (&list->head)) {
                (*next_element) = current_element->next;
            }
        }
    }

    DBG(DBG_DETAILED, "dlist_next: Exit (list=0x%x, current_element=0x%x "
            "next_element=0x%x)\n", list, current_element, next_element);

    return status;
}


int  dlist_search_element(dlist_t *          list,
                          void    *          data,
                          dlist_element_t ** elem,
                          DLIST_MATCH_FXN    match_fxn)
{
    int               status = 0;
    bool              found  = false;
    dlist_element_t * temp   = NULL;
    dlist_element_t * temp1  = NULL;

    DBG(DBG_DETAILED, "dlist_search_element: Enter (list=0x%x, data=0x%x "
            "elem=0x%x, match_fxn=0x%x)\n", list, data, elem, match_fxn);

    if (list == NULL || data == NULL || elem == NULL || match_fxn == NULL) {
        status = -EINVAL;
    } else {
        if (dlist_is_empty(list)) {
            status = -ENOENT;
        }

        if (SUCCEEDED(status)) {
            status = dlist_first(list, &temp);
            if (SUCCEEDED(status)) {
                while ((found == false) && (temp != NULL)) {
                    if ((*match_fxn)(temp, data) == true) {
                        found = true;
                    } else {
                        temp1 = temp;
                        dlist_next(list, temp1, &temp);
                    }
                }
                
                if (found == true) {
                    (*elem) = temp;
                } else {
                    (*elem) = NULL;
                    status  = -ENOENT;
                }
            } else {
                (*elem) = NULL;
                status = -ENOENT;
            }
        }
    }

    DBG(DBG_DETAILED, "dlist_search_element: Enter (list=0x%x, data=0x%x "
            "elem=0x%x, match_fxn=0x%x)\n", list, data, elem, match_fxn);

    return status;
}

int  dlist_map(dlist_t * list, DLIST_APPLY_FXN apply_fxn, void * data)
{
    int               status = 0;
    int               retval = 0;
    dlist_element_t * temp   = NULL;
    dlist_element_t * temp1  = NULL;

    DBG(DBG_DETAILED, "dlist_map: Enter (list=0x%x, apply_fxn=0x%x, "
            "data=0x%x)\n", list, apply_fxn, data);

    if (list == NULL || apply_fxn == NULL) {
        status = -EINVAL;
    } else {
        status = dlist_first(list, &temp);
        while (SUCCEEDED(status) && temp != NULL) {

            retval = (*apply_fxn)(temp, data);
            temp1  = temp;
            status = dlist_next(list, temp1, &temp);
        }
    }

    DBG(DBG_DETAILED, "dlist_map: Exit (list=0x%x, status=0x%x\n", list, status);

    return status;
}

int dlist_count(dlist_t * list, unsigned int * count)
{
    int               status = 0;

    if (list == NULL || count == NULL) {
        status = -EINVAL;
    } else {
        (*count) = 0;

        status = dlist_map(list, __dlist_count_apply_fxn, (void *)count);
    }

    return status;
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
static int
__dlist_count_apply_fxn(dlist_element_t * elem, void * data)
{
    (*((unsigned int *)data))++;

    return 0;
}

#if defined(__cplusplus)
}
#endif
