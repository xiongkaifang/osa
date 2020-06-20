/** ============================================================================
 *
 *  Copyright (C), 1987 - 2017, xiong-kaifang Tech. Co, Ltd.
 *
 *  @File Name:	osa_rbtree_augmented.h
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

#if !defined (__OSA_RBTREE_AUGMENTED_H)
#define __OSA_RBTREE_AUGMENTED_H

/*  --------------------- Include system headers ---------------------------- */
#include <stddef.h>

/*  --------------------- Include user headers   ---------------------------- */
#include "osa_rbtree.h"

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
#define RB_RED              0
#define RB_BLACK            1

#define __rb_parent(pc)     ((struct rb_node *)(pc & ~3))

#define __rb_color(pc)      ((pc) & 1)
#define __rb_is_black(pc)   __rb_color(pc)
#define __rb_is_red(pc)     (!__rb_color(pc))
#define rb_color(rb)        __rb_color((rb)->__rb_parent_color)
#define rb_is_red(rb)       __rb_is_red((rb)->__rb_parent_color)
#define rb_is_black(rb)     __rb_is_black((rb)->__rb_parent_color)

#define RB_DECLARE_CALLBACKS(rbstatic, rbname, rbstruct, rbfield,	    \
        rbtype, rbaugmented, rbcompute)		                            \
static inline void							                            \
rbname ## _propagate(struct rb_node *rb, struct rb_node *stop)		    \
{								    	                                \
    while (rb != stop) {						                        \
        rbstruct *node = rb_entry(rb, rbstruct, rbfield);	            \
        rbtype augmented = rbcompute(node);			                    \
        if (node->rbaugmented == augmented)			                    \
        break;						                                    \
        node->rbaugmented = augmented;				                    \
        rb = rb_parent(&node->rbfield);				                    \
    }								                                    \
}									                                    \
static inline void							                            \
rbname ## _copy(struct rb_node *rb_old, struct rb_node *rb_new)		    \
{									                                    \
    rbstruct *old = rb_entry(rb_old, rbstruct, rbfield);		        \
    rbstruct *new = rb_entry(rb_new, rbstruct, rbfield);		        \
    new->rbaugmented = old->rbaugmented;				                \
}									                                    \
static void							                                	\
rbname ## _rotate(struct rb_node *rb_old, struct rb_node *rb_new)	    \
{									                                    \
    rbstruct *old = rb_entry(rb_old, rbstruct, rbfield);		        \
    rbstruct *new = rb_entry(rb_new, rbstruct, rbfield);		        \
    new->rbaugmented = old->rbaugmented;				                \
    old->rbaugmented = rbcompute(old);				                    \
}									                                    \
rbstatic const struct rb_augment_callbacks rbname = {			        \
    .propagate = rbname ## _propagate,				                    \
    .copy = rbname ## _copy,					                        \
    .rotate = rbname ## _rotate					                        \
};

#if !defined(WRITE_ONCE)
#define WRITE_ONCE(var, v)  (var) = (v)
#endif

#if !defined(__always_inline)
#define __always_inline     
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
struct rb_augment_callbacks
{
    void (*propagate)(struct rb_node * nod, struct rb_node * stp);
    void (*copy)     (struct rb_node * old, struct rb_node * new);
    void (*rotate)   (struct rb_node * old, struct rb_node * new);
};

extern void __rb_insert_augmented(struct rb_node * node, struct rb_root * root,
        void (*augment_rotate)(struct rb_node * old, struct rb_node * new));

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
static inline void
rb_insert_augmented(struct rb_node * node, struct rb_root * root,
        const struct rb_augment_callbacks * augment)
{
    __rb_insert_augmented(node, root, augment->rotate);
}

static inline void rb_set_parent(struct rb_node *rb, struct rb_node *p)
{
	rb->__rb_parent_color = rb_color(rb) | (unsigned long)p;
}

static inline void rb_set_parent_color(struct rb_node *rb,
				       struct rb_node *p, int color)
{
	rb->__rb_parent_color = (unsigned long)p | color;
}

static inline void
__rb_change_child(struct rb_node *old, struct rb_node *new,
		  struct rb_node *parent, struct rb_root *root)
{
	if (parent) {
		if (parent->rb_left == old)
			WRITE_ONCE(parent->rb_left, new);
		else
			WRITE_ONCE(parent->rb_right, new);
	} else
		WRITE_ONCE(root->rb_node, new);
}

static inline void
__rb_change_child_rcu(struct rb_node *old, struct rb_node *new,
		      struct rb_node *parent, struct rb_root *root)
{
	if (parent) {
		if (parent->rb_left == old)
			rcu_assign_pointer(parent->rb_left, new);
		else
			rcu_assign_pointer(parent->rb_right, new);
	} else
		rcu_assign_pointer(root->rb_node, new);
}

extern void __rb_erase_color(struct rb_node *parent, struct rb_root *root,
	void (*augment_rotate)(struct rb_node *old, struct rb_node *new));

static __always_inline struct rb_node *
__rb_erase_augmented(struct rb_node *node, struct rb_root *root,
		     const struct rb_augment_callbacks *augment)
{
	struct rb_node *child = node->rb_right;
	struct rb_node *tmp = node->rb_left;
	struct rb_node *parent, *rebalance;
	unsigned long pc;

	if (!tmp) {
		/*
		 * Case 1: node to erase has no more than 1 child (easy!)
		 *
		 * Note that if there is one child it must be red due to 5)
		 * and node must be black due to 4). We adjust colors locally
		 * so as to bypass __rb_erase_color() later on.
		 */
		pc = node->__rb_parent_color;
		parent = __rb_parent(pc);
		__rb_change_child(node, child, parent, root);
		if (child) {
			child->__rb_parent_color = pc;
			rebalance = NULL;
		} else
			rebalance = __rb_is_black(pc) ? parent : NULL;
		tmp = parent;
	} else if (!child) {
		/* Still case 1, but this time the child is node->rb_left */
		tmp->__rb_parent_color = pc = node->__rb_parent_color;
		parent = __rb_parent(pc);
		__rb_change_child(node, tmp, parent, root);
		rebalance = NULL;
		tmp = parent;
	} else {
		struct rb_node *successor = child, *child2;

		tmp = child->rb_left;
		if (!tmp) {
			/*
			 * Case 2: node's successor is its right child
			 *
			 *    (n)          (s)
			 *    / \          / \
			 *  (x) (s)  ->  (x) (c)
			 *        \
			 *        (c)
			 */
			parent = successor;
			child2 = successor->rb_right;

			augment->copy(node, successor);
		} else {
			/*
			 * Case 3: node's successor is leftmost under
			 * node's right child subtree
			 *
			 *    (n)          (s)
			 *    / \          / \
			 *  (x) (y)  ->  (x) (y)
			 *      /            /
			 *    (p)          (p)
			 *    /            /
			 *  (s)          (c)
			 *    \
			 *    (c)
			 */
			do {
				parent = successor;
				successor = tmp;
				tmp = tmp->rb_left;
			} while (tmp);
			child2 = successor->rb_right;
			WRITE_ONCE(parent->rb_left, child2);
			WRITE_ONCE(successor->rb_right, child);
			rb_set_parent(child, successor);

			augment->copy(node, successor);
			augment->propagate(parent, successor);
		}

		tmp = node->rb_left;
		WRITE_ONCE(successor->rb_left, tmp);
		rb_set_parent(tmp, successor);

		pc = node->__rb_parent_color;
		tmp = __rb_parent(pc);
		__rb_change_child(node, successor, tmp, root);

		if (child2) {
			successor->__rb_parent_color = pc;
			rb_set_parent_color(child2, parent, RB_BLACK);
			rebalance = NULL;
		} else {
			unsigned long pc2 = successor->__rb_parent_color;
			successor->__rb_parent_color = pc;
			rebalance = __rb_is_black(pc2) ? parent : NULL;
		}
		tmp = successor;
	}

	augment->propagate(tmp, NULL);
	return rebalance;
}

static __always_inline void
rb_erase_augmented(struct rb_node *node, struct rb_root *root,
		   const struct rb_augment_callbacks *augment)
{
	struct rb_node *rebalance = __rb_erase_augmented(node, root, augment);
	if (rebalance)
		__rb_erase_color(rebalance, root, augment->rotate);
}

#if defined(__cplusplus)
}
#endif  /* defined(__cplusplus) */

#endif  /* if !defined (__OSA_RBTREE_AUGMENTED_H) */
