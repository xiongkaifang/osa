/** ============================================================================
 *
 *  Copyright (C), 1987 - 2017, xiong-kaifang Tech. Co, Ltd.
 *
 *  @File Name:	osa_rbtree.h
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2017-03-06
 *
 *  @Description:   // 用于详细说明此程序文件完成的主要功能，与其它模块
 *                  // 或函数的接口，输出值，取值范围、含义及参数间的控
 *                  // 制、顺序、独立或依赖等关系
 *                  //
 *
 *                  The header file for reb black tree(Linux red black tree).
 *
 *  @Others:        // 其它内容说明
 *
 *  @Function List: // 主要函数列表，每条记录就包括函数名及功能简要说明
 *      1.  ...
 *      2.  ...
 *
 *  @History:       // 修改历史记录列表，每条修改记录就包括修改日期、修改
 *                  // 时间及修改内容简述
 *
 *  <author>        <time>       <version>      <description>
 *
 *  xiong-kaifang   2017-03-06     v1.0	        Write this module.
 *
 *  ============================================================================
 */

#if !defined (__OSA_RBTREE_H)
#define __OSA_RBTREE_H

/*  --------------------- Include system headers ---------------------------- */
#include <stddef.h>

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
#if !defined(offset_of)
#define offset_of(type, member) ((size_t)&((type *)0)->member)
#endif

#if !defined(container_of)
#define container_of(ptr, type, member) ({                              \
    const typeof ( ((type *)0)->member ) * __mptr = (ptr);              \
    (type *)( (char *)__mptr - offset_of(type, member) );})
#endif

#define rb_parent(r)    ((struct rb_node *)((r)->__rb_parent_color & ~3))

#define RB_ROOT         (struct rb_root) { NULL, }

#define rb_entry(ptr, type, member) container_of(ptr, type, member)

#define RB_EMPTY_ROOT(root) ((root)->rb_node == NULL)

#define RB_EMPTY_NODE(node)                                             \
        ((node)->__rb_parent_color == (unsigned long)(node))
#define RB_CLEAR_NODE(node)                                             \
        ((node)->__rb_parent_color =  (unsigned long)(node))

#define rb_entry_safe(ptr, type, member)                                \
    ({  typeof(ptr) ____ptr = (ptr);                                    \
        ____ptr ? rb_entry(____ptr, type, member) : NULL;               \
    })

#define rbtree_postorder_for_each_entry_safe(pos, n, root, field)       \
    for (pos =                                                          \
         rb_entry_safe(rb_first_postorder(root), typeof(*pos), field);  \
         pos && ({ n = rb_entry_safe(rb_next_postorder(&pos->field),    \
         typeof(*pos), field); 1; });                                   \
         pos = n)

#if !defined(rcu_assign_pointer)
#define rcu_assign_pointer(ptr, value)  (ptr) = (value)
#endif

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
struct rb_node
{
    struct rb_node * rb_right;
    struct rb_node * rb_left;
    unsigned long    __rb_parent_color;
};

struct rb_root
{
    struct rb_node * rb_node;
};

/*
 *  --------------------- Public function declaration --------------------------
 */

/** =============================================================================
 *
 *  @Function:      // 函数名称
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
extern void rb_insert_color(struct rb_node *, struct rb_root *);
extern void rb_erase       (struct rb_node *, struct rb_root *);

extern struct rb_node * rb_next (const struct rb_node *);
extern struct rb_node * rb_prev (const struct rb_node *);
extern struct rb_node * rb_first(const struct rb_root *);
extern struct rb_node * rb_last (const struct rb_root *);

extern struct rb_node * rb_first_postorder(const struct rb_root *);
extern struct rb_node * rb_next_postorder (const struct rb_node *);

extern void rb_replace_node(struct rb_node * victim, struct rb_node * new,
                            struct rb_root * root);
extern void rb_replace_node_rcu(struct rb_node * victim, struct rb_node * new,
                            struct rb_root * root);

static inline void rb_link_node(struct rb_node * node, struct rb_node * parent,
                            struct rb_node ** rb_link)
{
    node->__rb_parent_color = (unsigned long)parent;
    node->rb_left = node->rb_right = NULL;

    *rb_link = node;
}

static inline void rb_link_node_rcu(struct rb_node * node,
        struct rb_node * parent, struct rb_node ** rb_link)
{
    node->__rb_parent_color = (unsigned long)parent;
    node->rb_left = node->rb_right = NULL;

    rcu_assign_pointer(*rb_link, node);
}

#if defined(__cplusplus)
}
#endif  /* defined(__cplusplus) */

#endif  /* if !defined (__OSA_RBTREE_H) */
