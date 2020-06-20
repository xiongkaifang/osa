/** ============================================================================
 *
 *  Copyright (C), 1987 - 2017, xiong-kaifang Tech. Co.,, Ltd.
 *
 *  @File name:	osa_rbtree_augmented.c
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2017-02-19
 *
 *  @Description:   The description of this file.
 *
 *                  The source file for augmented reb black tree(Linux 
 *                  augmented red black tree).
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
 *  xiong-kaifang   2017-02-19     v1.0	        Write this module.
 *
 *  ============================================================================
 */

/*  --------------------- Include system headers ---------------------------- */
#include <stdbool.h>

/*  --------------------- Include user headers   ---------------------------- */
#include "osa_rbtree_augmented.h"

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
 *  @Others:        //其它说明
 *
 *  ============================================================================
 */
static inline void rb_set_black(struct rb_node * node)
{
    node->__rb_parent_color |= RB_BLACK;
}

static inline struct rb_node * rb_red_parent(struct rb_node * red)
{
    return (struct rb_node *)red->__rb_parent_color;
}

static inline void
__rb_rotate_set_parents(struct rb_node * old, struct rb_node * new,
                        struct rb_root * root, int color)
{
    struct rb_node * parent = rb_parent(old);

    new->__rb_parent_color = old->__rb_parent_color;
    rb_set_parent_color(old, new, color);
    __rb_change_child(old, new, parent, root);
}

static inline void
__rb_insert(struct rb_node * node, struct rb_root * root,
        void (*augment_rotate)(struct rb_node * old, struct rb_node * new))
{
    struct rb_node *parent = rb_red_parent(node), *gparent, *tmp;

    while (true) {
        /*
         * Loop invariant: node is red
         *
         * If there is a black parent, we are done.
         * Otherwise, take some corrective action as we don't
         * want a red root or two consecutive red nodes.
         */
        if (!parent) {
            rb_set_parent_color(node, NULL, RB_BLACK);
            break;
        } else if (rb_is_black(parent))
            break;

        gparent = rb_red_parent(parent);

        tmp = gparent->rb_right;
        if (parent != tmp) {	/* parent == gparent->rb_left */
            if (tmp && rb_is_red(tmp)) {
                /*
                 * Case 1 - color flips
                 *
                 *       G            g
                 *      / \          / \
                 *     p   u  -->   P   U
                 *    /            /
                 *   n            n
                 *
                 * However, since g's parent might be red, and
                 * 4) does not allow this, we need to recurse
                 * at g.
                 */
                rb_set_parent_color(tmp, gparent, RB_BLACK);
                rb_set_parent_color(parent, gparent, RB_BLACK);
                node = gparent;
                parent = rb_parent(node);
                rb_set_parent_color(node, parent, RB_RED);
                continue;
            }

            tmp = parent->rb_right;
            if (node == tmp) {
                /*
                 * Case 2 - left rotate at parent
                 *
                 *      G             G
                 *     / \           / \
                 *    p   U  -->    n   U
                 *     \           /
                 *      n         p
                 *
                 * This still leaves us in violation of 4), the
                 * continuation into Case 3 will fix that.
                 */
                tmp = node->rb_left;
                WRITE_ONCE(parent->rb_right, tmp);
                WRITE_ONCE(node->rb_left, parent);
                if (tmp)
                    rb_set_parent_color(tmp, parent,
                            RB_BLACK);
                rb_set_parent_color(parent, node, RB_RED);
                augment_rotate(parent, node);
                parent = node;
                tmp = node->rb_right;
            }

            /*
             * Case 3 - right rotate at gparent
             *
             *        G           P
             *       / \         / \
             *      p   U  -->  n   g
             *     /                 \
             *    n                   U
             */
            WRITE_ONCE(gparent->rb_left, tmp); /* == parent->rb_right */
            WRITE_ONCE(parent->rb_right, gparent);
            if (tmp)
                rb_set_parent_color(tmp, gparent, RB_BLACK);
            __rb_rotate_set_parents(gparent, parent, root, RB_RED);
            augment_rotate(gparent, parent);
            break;
        } else {
            tmp = gparent->rb_left;
            if (tmp && rb_is_red(tmp)) {
                /* Case 1 - color flips */
                rb_set_parent_color(tmp, gparent, RB_BLACK);
                rb_set_parent_color(parent, gparent, RB_BLACK);
                node = gparent;
                parent = rb_parent(node);
                rb_set_parent_color(node, parent, RB_RED);
                continue;
            }

            tmp = parent->rb_left;
            if (node == tmp) {
                /* Case 2 - right rotate at parent */
                tmp = node->rb_right;
                WRITE_ONCE(parent->rb_left, tmp);
                WRITE_ONCE(node->rb_right, parent);
                if (tmp)
                    rb_set_parent_color(tmp, parent,
                            RB_BLACK);
                rb_set_parent_color(parent, node, RB_RED);
                augment_rotate(parent, node);
                parent = node;
                tmp = node->rb_left;
            }

            /* Case 3 - left rotate at gparent */
            WRITE_ONCE(gparent->rb_right, tmp); /* == parent->rb_left */
            WRITE_ONCE(parent->rb_left, gparent);
            if (tmp)
                rb_set_parent_color(tmp, gparent, RB_BLACK);
            __rb_rotate_set_parents(gparent, parent, root, RB_RED);
            augment_rotate(gparent, parent);
            break;
        }
    }
}

static inline void
____rb_erase_color(struct rb_node * parent, struct rb_root * root,
        void (*augment_rotate)(struct rb_node * old, struct rb_node * new))
{
    struct rb_node *node = NULL, *sibling, *tmp1, *tmp2;

    while (true) {
        /*
         * Loop invariants:
         * - node is black (or NULL on first iteration)
         * - node is not the root (parent is not NULL)
         * - All leaf paths going through parent and node have a
         *   black node count that is 1 lower than other leaf paths.
         */
        sibling = parent->rb_right;
        if (node != sibling) {	/* node == parent->rb_left */
            if (rb_is_red(sibling)) {
                /*
                 * Case 1 - left rotate at parent
                 *
                 *     P               S
                 *    / \             / \
                 *   N   s    -->    p   Sr
                 *      / \         / \
                 *     Sl  Sr      N   Sl
                 */
                tmp1 = sibling->rb_left;
                WRITE_ONCE(parent->rb_right, tmp1);
                WRITE_ONCE(sibling->rb_left, parent);
                rb_set_parent_color(tmp1, parent, RB_BLACK);
                __rb_rotate_set_parents(parent, sibling, root,
                        RB_RED);
                augment_rotate(parent, sibling);
                sibling = tmp1;
            }
            tmp1 = sibling->rb_right;
            if (!tmp1 || rb_is_black(tmp1)) {
                tmp2 = sibling->rb_left;
                if (!tmp2 || rb_is_black(tmp2)) {
                    /*
                     * Case 2 - sibling color flip
                     * (p could be either color here)
                     *
                     *    (p)           (p)
                     *    / \           / \
                     *   N   S    -->  N   s
                     *      / \           / \
                     *     Sl  Sr        Sl  Sr
                     *
                     * This leaves us violating 5) which
                     * can be fixed by flipping p to black
                     * if it was red, or by recursing at p.
                     * p is red when coming from Case 1.
                     */
                    rb_set_parent_color(sibling, parent,
                            RB_RED);
                    if (rb_is_red(parent))
                        rb_set_black(parent);
                    else {
                        node = parent;
                        parent = rb_parent(node);
                        if (parent)
                            continue;
                    }
                    break;
                }
                /*
                 * Case 3 - right rotate at sibling
                 * (p could be either color here)
                 *
                 *   (p)           (p)
                 *   / \           / \
                 *  N   S    -->  N   sl
                 *     / \             \
                 *    sl  Sr            S
                 *                       \
                 *                        Sr
                 *
                 * Note: p might be red, and then both
                 * p and sl are red after rotation(which
                 * breaks property 4). This is fixed in
                 * Case 4 (in __rb_rotate_set_parents()
                 *         which set sl the color of p
                 *         and set p RB_BLACK)
                 *
                 *   (p)            (sl)
                 *   / \            /  \
                 *  N   sl   -->   P    S
                 *       \        /      \
                 *        S      N        Sr
                 *         \
                 *          Sr
                 */
                tmp1 = tmp2->rb_right;
                WRITE_ONCE(sibling->rb_left, tmp1);
                WRITE_ONCE(tmp2->rb_right, sibling);
                WRITE_ONCE(parent->rb_right, tmp2);
                if (tmp1)
                    rb_set_parent_color(tmp1, sibling,
                            RB_BLACK);
                augment_rotate(sibling, tmp2);
                tmp1 = sibling;
                sibling = tmp2;
            }
            /*
             * Case 4 - left rotate at parent + color flips
             * (p and sl could be either color here.
             *  After rotation, p becomes black, s acquires
             *  p's color, and sl keeps its color)
             *
             *      (p)             (s)
             *      / \             / \
             *     N   S     -->   P   Sr
             *        / \         / \
             *      (sl) sr      N  (sl)
             */
            tmp2 = sibling->rb_left;
            WRITE_ONCE(parent->rb_right, tmp2);
            WRITE_ONCE(sibling->rb_left, parent);
            rb_set_parent_color(tmp1, sibling, RB_BLACK);
            if (tmp2)
                rb_set_parent(tmp2, parent);
            __rb_rotate_set_parents(parent, sibling, root,
                    RB_BLACK);
            augment_rotate(parent, sibling);
            break;
        } else {
            sibling = parent->rb_left;
            if (rb_is_red(sibling)) {
                /* Case 1 - right rotate at parent */
                tmp1 = sibling->rb_right;
                WRITE_ONCE(parent->rb_left, tmp1);
                WRITE_ONCE(sibling->rb_right, parent);
                rb_set_parent_color(tmp1, parent, RB_BLACK);
                __rb_rotate_set_parents(parent, sibling, root,
                        RB_RED);
                augment_rotate(parent, sibling);
                sibling = tmp1;
            }
            tmp1 = sibling->rb_left;
            if (!tmp1 || rb_is_black(tmp1)) {
                tmp2 = sibling->rb_right;
                if (!tmp2 || rb_is_black(tmp2)) {
                    /* Case 2 - sibling color flip */
                    rb_set_parent_color(sibling, parent,
                            RB_RED);
                    if (rb_is_red(parent))
                        rb_set_black(parent);
                    else {
                        node = parent;
                        parent = rb_parent(node);
                        if (parent)
                            continue;
                    }
                    break;
                }
                /* Case 3 - left rotate at sibling */
                tmp1 = tmp2->rb_left;
                WRITE_ONCE(sibling->rb_right, tmp1);
                WRITE_ONCE(tmp2->rb_left, sibling);
                WRITE_ONCE(parent->rb_left, tmp2);
                if (tmp1)
                    rb_set_parent_color(tmp1, sibling,
                            RB_BLACK);
                augment_rotate(sibling, tmp2);
                tmp1 = sibling;
                sibling = tmp2;
            }
            /* Case 4 - right rotate at parent + color flips */
            tmp2 = sibling->rb_right;
            WRITE_ONCE(parent->rb_left, tmp2);
            WRITE_ONCE(sibling->rb_right, parent);
            rb_set_parent_color(tmp1, sibling, RB_BLACK);
            if (tmp2)
                rb_set_parent(tmp2, parent);
            __rb_rotate_set_parents(parent, sibling, root,
                    RB_BLACK);
            augment_rotate(parent, sibling);
            break;
        }
    }
}

void __rb_erase_color(struct rb_node * parent, struct rb_root * root,
        void (*augment_rotate)(struct rb_node * old, struct rb_node * new))
{
    ____rb_erase_color(parent, root, augment_rotate);
}

static inline void dummy_propagate(struct rb_node * node, struct rb_node * stop)
{
}
static inline void dummy_copy(struct rb_node * old, struct rb_node * new)
{
}
static inline void dummy_rotate(struct rb_node * old, struct rb_node * new)
{
}

static const struct rb_augment_callbacks dummy_callbacks = {
    dummy_propagate, dummy_copy, dummy_rotate
};

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
void rb_insert_color(struct rb_node * node, struct rb_root * root)
{
    __rb_insert(node, root, dummy_rotate);
}

void rb_erase(struct rb_node * node, struct rb_root * root)
{
    struct rb_node * rebalance;

    rebalance = __rb_erase_augmented(node, root, &dummy_callbacks);

    if (rebalance) {
        ____rb_erase_color(rebalance, root, dummy_rotate);
    }
}

void __rb_insert_augmented(struct rb_node * node, struct rb_root * root,
        void (*augment_rotate)(struct rb_node * old, struct rb_node * new))
{
    __rb_insert(node, root, augment_rotate);
}

struct rb_node * rb_first(const struct rb_root * root)
{
    struct rb_node * n;

    n = root->rb_node;
    if (!n) {
        return NULL;
    }

    while (n->rb_left) {
        n = n->rb_left;
    }

    return n;
}

struct rb_node * rb_last(const struct rb_root * root)
{
    struct rb_node * n;

    n = root->rb_node;
    if (!n) {
        return NULL;
    }

    while (n->rb_right) {
        n = n->rb_right;
    }

    return n;
}

struct rb_node * rb_next(const struct rb_node * node)
{
    struct rb_node * parent;

    if (RB_EMPTY_NODE(node)) {
        return NULL;
    }

    /*
     *  If we have right-hand child, go down and then left as far as we can.
     */
    if (node->rb_right) {
        node = node->rb_right;
        while (node->rb_left) {
            node = node->rb_left;
        }

        return (struct rb_node *)node;
    }

    /*
     *  No right-hand children. Everything down and left is smaller than us,
     *  so any 'next' node muse be in the general direction of our parent.
     *
     *  Go up the tree; any time the ancestor is a right-hand child is its
     *  parent, keep going up. First time it's a left-hand child of its
     *  parent, said parent is our 'next' node.
     */
    while ((parent = rb_parent(node)) && node == parent ->rb_right) {
        node = parent;
    }

    return parent;
}

struct rb_node * rb_prev(const struct rb_node * node)
{
    struct rb_node * parent;

    if (RB_EMPTY_NODE(node)) {
        return NULL;
    }

    /*
     *  If we have left-hand child, go down and then right as far as we can.
     */
    if (node->rb_left) {
        node = node->rb_left;
        while (node->rb_right) {
            node = node->rb_right;
        }

        return (struct rb_node *)node;
    }

    /*
     *  No left-hand children. Everything down and left is smaller than us,
     *  so any 'next' node muse be in the general direction of our parent.
     *
     *  Go up the tree; any time the ancestor is a left-hand child is its
     *  parent, keep going up. First time it's a right-hand child of its
     *  parent, said parent is our 'next' node.
     */
    while ((parent = rb_parent(node)) && node == parent ->rb_left) {
        node = parent;
    }

    return parent;
}

void rb_replace_node(struct rb_node * victim, struct rb_node * new,
        struct rb_root * root)
{
    struct rb_node * parent = rb_parent(victim);

    /* Copy the pointers/color from the victim to the replacement */
    *new = *victim;

    /* Set the surrounding nodes to point to the replacement */
    if (victim->rb_left) {
        rb_set_parent(victim->rb_left, new);
    }
    if (victim->rb_right) {
        rb_set_parent(victim->rb_right, new);
    }

    __rb_change_child(victim, new, parent, root);
}

void rb_replace_node_rcu(struct rb_node * victim, struct rb_node * new,
        struct rb_root * root)
{
    struct rb_node * parent = rb_parent(victim);

    /* Copy the pointers/color from the victim to the replacement */
    *new = *victim;

    /* Set the surrounding nodes to point to the replacement */
    if (victim->rb_left) {
        rb_set_parent(victim->rb_left, new);
    }
    if (victim->rb_right) {
        rb_set_parent(victim->rb_right, new);
    }

	/* 
     * Set the parent's pointer to the new node last after an RCU barrier
	 * so that the pointers onwards are seen to be set correctly when doing
	 * an RCU walk over the tree.
     *
	 */
    __rb_change_child_rcu(victim, new, parent, root);
}

static struct rb_node * rb_left_deepest_node(const struct rb_node * node)
{
    for ( ; ; ) {
        if (node->rb_left) {
            node = node->rb_left;
        } else if (node->rb_right) {
            node = node->rb_right;
        } else {
            return (struct rb_node *)node;
        }
    }
}

struct rb_node * rb_next_postorder(const struct rb_node * node)
{
    const struct rb_node * parent;

    if (!node) {
        return NULL;
    }

	parent = rb_parent(node);

    /* If we're sitting on node, we've already seen our children */
    if (parent && node == parent->rb_left && parent->rb_right) {
        /*
         *  If we are the parent's left node, go the parent's right
         *  node then all the way down to the left.
         */
        return rb_left_deepest_node(parent->rb_right);
    } else {
        /*
         *  Otherwise we are the parent's right node, and the parent
         *  shoud be next.
         */
        return (struct rb_node *)parent;
    }
}

struct rb_node * rb_first_postorder(const struct rb_root * root)
{
    if (!root->rb_node) {
        return NULL;
    }

    return rb_left_deepest_node(root->rb_node);
}

/*
 *  --------------------- Local function definition ----------------------------
 */

/** ============================================================================
 *
 *  @Function:      Local function definition.
 *
 *  @Description:   //  函数功能、性能等的描述
 *
 *  ============================================================================
 */

#if defined(__cplusplus)
}
#endif
