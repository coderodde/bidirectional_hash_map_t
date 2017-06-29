#include "bidirectional_hash_map_2.h"
#include <stdlib.h>

static float max_float(float a, float b)
{
    return a > b ? a : b;
}

static int max_int(int a, int b)
{
    return a > b ? a : b;
}

static size_t max_size_t(size_t a, size_t b)
{
    return a > b ? a : b;
}

static int get_primary_tree_node_height(primary_collision_tree_node_t* node)
{
    return node ? node->height : -1;
}

static int get_secondary_tree_node_height(secondary_collision_tree_node_t* node)
{
    return node ? node->height : -1;
}

/****************************************************************
* Returns an integer that is a power of two no less than 'num'. *
****************************************************************/
static size_t to_power_of_two(size_t num)
{
    size_t ret = 1;
    
    while (ret < num)
    {
        ret <<= 1;
    }
    
    return  ret;
}

static const float  MINIMUM_LOAD_FACTOR      = 0.2;
static const size_t MINIMUM_INITIAL_CAPACITY = 8;

static primary_collision_tree_node_t*
get_primary_minimum_tree_node_of(primary_collision_tree_node_t* node)
{
    while (node->left)
    {
        node = node->left;
    }
    
    return node;
}

static secondary_collision_tree_node_t*
get_secondary_minimum_tree_node_of(secondary_collision_tree_node_t* node)
{
    while (node->left)
    {
        node = node->left;
    }
    
    return node;
}

static primary_collision_tree_node_t* get_primary_collision_tree_node_successor(
                    primary_collision_tree_node_t* primary_collision_tree_node)
{
    primary_collision_tree_node_t* parent_of_primary_collision_tree_node;
    
    if (primary_collision_tree_node->right)
    {
        return get_minimum_tree_node_of(primary_collision_tree_node->right);
    }
    
    parent_of_primary_collision_tree_node = primary_collision_tree_node->parent;
    
    while (parent_of_primary_collision_tree_node
           && parent_of_primary_collision_tree_node->right
           == primary_collision_tree_node)
    {
        primary_collision_tree_node = parent_of_primary_collision_tree_node;
        parent_of_primary_collision_tree_node =
        parent_of_primary_collision_tree_node->parent;
    }
    
    return parent_of_primary_collision_tree_node;
}

// OK
/***********************************************************
* Performs left rotation of a primary collision tree node. *
***********************************************************/
static primary_collision_tree_node_t*
primary_collision_tree_left_rotate(primary_collision_tree_node_t* node_1)
{
    primary_collision_tree_node_t* node_2 = node_1->right;
    
    node_2->parent = node_1->parent;
    node_1->parent = node_2;
    node_1->right  = node_2->left;
    node_2->left   = node_1;
    
    if (node_1->right)
    {
        node_1->right->parent = node_1;
    }
    
    node_1->height = int_max(get_primary_tree_node_height(node_1->left),
                             get_primary_tree_node_height(node_1->right)) + 1;
    node_2->height = int_max(get_primary_tree_node_height(node_2->left),
                             get_primary_tree_node_height(node_2->right)) + 1;
    return node_2;
}

// OK
/************************************************************
* Performs right rotation of a primary collision tree node. *
************************************************************/
static primary_collision_tree_node_t*
primary_collision_tree_right_rotate(primary_collision_tree_node_t* node_1)
{
    primary_collision_tree_node_t* node_2 = node_1->left;
    
    node_2->parent = node_1->parent;
    node_1->parent = node_2;
    node_1->left = node_2->right;
    node_2->right = node_1;
    
    if (node_1->left)
    {
        node_1->left->parent = node_1;
    }
    
    node_1->height = int_max(get_primary_tree_node_height(node_1->left),
                             get_primary_tree_node_height(node_1->right)) + 1;
                             
    node_2->height = int_max(get_primary_tree_node_height(node_2->left),
                             get_primary_tree_node_height(node_2->right)) + 1;
    
    return node_2;
}

// OK
/**************************************************************************
* Performs a double right/left rotation of a primary collision tree node. *
**************************************************************************/
static primary_collision_tree_node_t*
primary_collision_tree_right_left_rotate(primary_collision_tree_node_t* node_1)
{
    primary_collision_tree_node_t* node_2 = node_1->right;
    node_1->right = primary_collision_tree_right_rotate(node_2);
    return primary_collision_tree_left_rotate(node_1);
}

// OK
/**************************************************************************
* Performs a double left/right rotation of a primary collision tree node. *
**************************************************************************/
static primary_collision_tree_node_t*
primary_collision_tree_left_right_rotate(primary_collision_tree_node_t* node_1)
{
    primary_collision_tree_node_t* node_2 = node_1->left;
    node_1->left = primary_collision_tree_left_rotate(node_2);
    return primary_collision_tree_right_rotate(node_1);
}

// OK
/****************************************************************************
* This function is responsible for balancing a primary collision tree after *
* inserting a node into it.                                                 *
****************************************************************************/
static void fix_primary_collision_tree_after_insertion(
                                        bidirectional_hash_map_2_t* map,
                                        primary_collision_tree_node_t* node)
{
    primary_collision_tree_node_t* grand_parent;
    primary_collision_tree_node_t* parent = node->parent;
    primary_collision_tree_node_t* sub_tree;
    size_t node_bucket_index;
    
    while (parent)
    {
        if (get_primary_tree_node_height(parent->left) ==
            get_primary_tree_node_height(parent->right) + 2)
        {
            grand_parent = parent->parent;
            
            if (get_primary_tree_node_height(parent->left->left) >=
                get_primary_tree_node_height(parent->left->right))
            {
                sub_tree = primary_right_rotate(parent);
            }
            else
            {
                sub_tree = primary_left_right_rotate(parent);
            }
            
            if (!grand_parent)
            {
                node_bucket_index = node->key_pair->primary_key_hash
                                  & map->modulo_mask;
                map->primary_key_table[node_bucket_index] = sub_tree;
            }
            else if (grand_parent->left == parent)
            {
                grand_parent->left = sub_tree;
            }
            else
            {
                grand_parent->right = sub_tree;
            }
            
            if (grand_parent)
            {
                grand_parent->height =
                int_max(get_primary_tree_node_height(grand_parent->left),
                        get_primary_tree_node_height(grand_parent->right)) + 1;
            }
            
            return;
        }
        else if (get_primary_tree_node_height(parent->right) ==
                 get_primary_tree_node_height(parent->left) + 2)
        {
            grand_parent = parent->parent;
            
            if (get_primary_tree_node_height(parent->right->right) >=
                get_primary_tree_node_height(parent->right->left))
            {
                sub_tree = primary_left_rotate(parent);
            }
            else
            {
                sub_tree = primary_right_left_rotate(parent);
            }
            
            // OPTIMIZE?
            if (!grand_parent)
            {
                node_bucket_index = node->key_pair->primary_key_hash
                                  & map->modulo_mask;
                map->primary_key_table[node_bucket_index] = sub_tree;
            }
            else if (grand_parent->left == parent)
            {
                grand_parent->left = sub_tree;
            }
            else
            {
                grand_parent->right = sub_tree;
            }
            
            if (grand_parent)
            {
                grand_parent->height =
                int_max(get_primary_tree_node_height(grand_parent->left),
                        get_primary_tree_node_height(grand_parent->right)) + 1;
            }
            
            return;
        }
        
        parent->height =
        int_max(get_primary_tree_node_height(parent->left),
                get_primary_tree_node_height(parent->right)) + 1;
        
        parent = parent->parent;
    }
}

// OK
/****************************************************************************
* This function is responsible for balancing a primary collision tree after *
* deleting a node from it.                                                  *
****************************************************************************/
static void fix_primary_collision_tree_after_deletion(
                                        bidirectional_hash_map_2_t* map,
                                        primary_collision_tree_node_t* node)
{
    primary_collision_tree_node_t* grand_parent;
    primary_collision_tree_node_t* parent = node->parent;
    primary_collision_tree_node_t* sub_tree;
    size_t node_bucket_index;
    
    while (parent)
    {
        if (get_primary_tree_node_height(parent->left) ==
            get_primary_tree_node_height(parent->right) + 2)
        {
            grand_parent = parent->parent;
            
            if (get_primary_tree_node_height(parent->left->left) >=
                get_primary_tree_node_height(parent->left->right))
            {
                sub_tree = primary_right_rotate(parent);
            }
            else
            {
                sub_tree = primary_left_right_rotate(parent);
            }
            
            // OPTIMIZE?
            if (!grand_parent)
            {
                node_bucket_index = node->key_pair->primary_key_hash
                                  & map->modulo_mask;
                map->primary_key_table[node_bucket_index] = sub_tree;
            }
            else if (grand_parent->left == parent)
            {
                grand_parent->left = sub_tree;
            }
            else
            {
                grand_parent->right = sub_tree;
            }
            
            if (grand_parent)
            {
                grand_parent->height =
                int_max(get_primary_tree_node_height(grand_parent->left),
                        get_primary_tree_node_height(grand_parent->right)) + 1;
            }
        }
        else if (get_primary_tree_node_height(parent->right) ==
                 get_primary_tree_node_height(parent->left) + 2)
        {
            grand_parent = parent->parent;
            
            if (get_primary_tree_node_height(parent->right->right) >=
                get_primary_tree_node_height(parent->right->left))
            {
                sub_tree = primary_left_rotate(parent);
            }
            else
            {
                sub_tree = primary_right_left_rotate(parent);
            }
            
            if (!grand_parent)
            {
                node_bucket_index = node->key_pair->primary_key_hash
                                  & map->modulo_mask;
                map->primary_key_table[node_bucket_index] = sub_tree;
            }
            else if (grand_parent->left == parent)
            {
                grand_parent->left = sub_tree;
            }
            else
            {
                grand_parent->right = sub_tree;
            }
            
            if (grand_parent)
            {
                grand_parent->height =
                int_max(get_primary_tree_node_height(grand_parent->left),
                        get_primary_tree_node_height(grand_parent->right)) + 1;
            }
        }
        
        parent->height =
        int_max(get_primary_tree_node_height(parent->left),
                get_primary_tree_node_height(parent->right)) + 1;
        
        parent = parent->parent;
    }
}


// OK
/*************************************************************
* Performs left rotation of a secondary collision tree node. *
*************************************************************/
static secondary_collision_tree_node_t*
secondary_collision_tree_left_rotate(secondary_collision_tree_node_t* node_1)
{
    secondary_collision_tree_node_t* node_2 = node_1->right;
    
    node_2->parent = node_1->parent;
    node_1->parent = node_2;
    node_1->right  = node_2->left;
    node_2->left   = node_1;
    
    if (node_1->right)
    {
        node_1->right->parent = node_1;
    }
    
    node_1->height = int_max(get_secondary_tree_node_height(node_1->left),
                             get_secondary_tree_node_height(node_1->right)) + 1;
    node_2->height = int_max(get_secondary_tree_node_height(node_2->left),
                             get_secondary_tree_node_height(node_2->right)) + 1;
    return node_2;
}

// OK
/**************************************************************
* Performs right rotation of a secondary collision tree node. *
**************************************************************/
static secondary_collision_tree_node_t*
secondary_collision_tree_right_rotate(secondary_collision_tree_node_t* node_1)
{
    secondary_collision_tree_node_t* node_2 = node_1->left;
    
    node_2->parent = node_1->parent;
    node_1->parent = node_2;
    node_1->left = node_2->right;
    node_2->right = node_1;
    
    if (node_1->left)
    {
        node_1->left->parent = node_1;
    }
    
    node_1->height = int_max(get_secondary_tree_node_height(node_1->left),
                             get_secondary_tree_node_height(node_1->right)) + 1;
    node_2->height = int_max(get_secondary_tree_node_height(node_2->left),
                             get_secondary_tree_node_height(node_2->right)) + 1;
    
    return node_2;
}

// OK
/****************************************************************************
* Performs a double right/left rotation of a secondary collision tree node. *
****************************************************************************/
static secondary_collision_tree_node_t*
secondary_collision_tree_right_left_rotate(
                                        secondary_collision_tree_node_t* node_1)
{
    secondary_collision_tree_node_t* node_2 = node_1->right;
    node_1->right = secondary_collision_tree_right_rotate(node_2);
    return secondary_collision_tree_left_rotate(node_1);
}

// OK
/**************************************************************************
* Performs a double left/right rotation of a primary collision tree node. *
**************************************************************************/
static primary_collision_tree_node_t*
secondary_collision_tree_left_right_rotate(
                                        secondary_collision_tree_node_t* node_1)
{
    secondary_collision_tree_node_t* node_2 = node_1->left;
    node_1->left = secondary_collision_tree_left_rotate(node_2);
    return secondary_collision_tree_right_rotate(node_1);
}

// OK
/******************************************************************************
* This function is responsible for balancing a secondary collision tree after *
* inserting a node into it.                                                   *
******************************************************************************/
static void fix_secondary_collision_tree_after_insertion(
                                        bidirectional_hash_map_2_t* map,
                                        secondary_collision_tree_node_t* node)
{
    secondary_collision_tree_node_t* grand_parent;
    secondary_collision_tree_node_t* parent = node->parent;
    secondary_collision_tree_node_t* sub_tree;
    size_t node_bucket_index;
    
    while (parent)
    {
        if (get_secondary_tree_node_height(parent->left) ==
            get_secondary_tree_node_height(parent->right) + 2)
        {
            grand_parent = parent->parent;
            
            if (get_secondary_tree_node_height(parent->left->left) >=
                get_secondary_tree_node_height(parent->left->right))
            {
                sub_tree = secondary_right_rotate(parent);
            }
            else
            {
                sub_tree = secondary_left_right_rotate(parent);
            }
            
            if (!grand_parent)
            {
                node_bucket_index = node->key_pair->primary_key_hash
                                  & map->modulo_mask;
                map->secondary_key_table[node_bucket_index] = sub_tree;
            }
            else if (grand_parent->left == parent)
            {
                grand_parent->left = sub_tree;
            }
            else
            {
                grand_parent->right = sub_tree;
            }
            
            if (grand_parent)
            {
                grand_parent->height =
                int_max(get_secondary_tree_node_height(grand_parent->left),
                        get_secondary_tree_node_height(grand_parent->right))
                    + 1;
            }
            
            return;
        }
        else if (get_secondary_tree_node_height(parent->right) ==
                 get_secondary_tree_node_height(parent->left) + 2)
        {
            grand_parent = parent->parent;
            
            if (get_secondary_tree_node_height(parent->right->right) >=
                get_secondary_tree_node_height(parent->right->left))
            {
                sub_tree = secondary_left_rotate(parent);
            }
            else
            {
                sub_tree = secondary_right_left_rotate(parent);
            }
            
            // OPTIMIZE?
            if (!grand_parent)
            {
                node_bucket_index = node->key_pair->secondary_key_hash
                                  & map->modulo_mask;
                map->secondary_key_table[node_bucket_index] = sub_tree;
            }
            else if (grand_parent->left == parent)
            {
                grand_parent->left = sub_tree;
            }
            else
            {
                grand_parent->right = sub_tree;
            }
            
            if (grand_parent)
            {
                grand_parent->height =
                int_max(get_secondary_tree_node_height(grand_parent->left),
                        get_secondary_tree_node_height(grand_parent->right))
                    + 1;
            }
            
            return;
        }
        
        parent->height =
        int_max(get_secondary_tree_node_height(parent->left),
                get_secondary_tree_node_height(parent->right)) + 1;
        
        parent = parent->parent;
    }
}

// OK
/******************************************************************************
* This function is responsible for balancing a secondary collision tree after *
* deleting a node from it.                                                    *
******************************************************************************/
static void fix_secondary_collision_tree_after_deletion(
                                        bidirectional_hash_map_2_t* map,
                                        secondary_collision_tree_node_t* node)
{
    secondary_collision_tree_node_t* grand_parent;
    secondary_collision_tree_node_t* parent = node->parent;
    secondary_collision_tree_node_t* sub_tree;
    size_t node_bucket_index;
    
    while (parent)
    {
        if (get_secondary_tree_node_height(parent->left) ==
            get_secondary_tree_node_height(parent->right) + 2)
        {
            grand_parent = parent->parent;
            
            if (get_secondary_tree_node_height(parent->left->left) >=
                get_secondary_tree_node_height(parent->left->right))
            {
                sub_tree = secondary_right_rotate(parent);
            }
            else
            {
                sub_tree = secondary_left_right_rotate(parent);
            }
            
            // OPTIMIZE?
            if (!grand_parent)
            {
                node_bucket_index = node->key_pair->secondary_key_hash
                                  & map->modulo_mask;
                map->secondary_key_table[node_bucket_index] = sub_tree;
            }
            else if (grand_parent->left == parent)
            {
                grand_parent->left = sub_tree;
            }
            else
            {
                grand_parent->right = sub_tree;
            }
            
            if (grand_parent)
            {
                grand_parent->height =
                int_max(get_secondary_tree_node_height(grand_parent->left),
                        get_secondary_tree_node_height(grand_parent->right))
                    + 1;
            }
        }
        else if (get_secondary_tree_node_height(parent->right) ==
                 get_secondary_tree_node_height(parent->left) + 2)
        {
            grand_parent = parent->parent;
            
            if (get_secondary_tree_node_height(parent->right->right) >=
                get_secondary_tree_node_height(parent->right->left))
            {
                sub_tree = secondary_left_rotate(parent);
            }
            else
            {
                sub_tree = secondary_right_left_rotate(parent);
            }
            
            if (!grand_parent)
            {
                node_bucket_index = node->key_pair->secondary_key_hash
                                  & map->modulo_mask;
                map->secondary_key_table[node_bucket_index] = sub_tree;
            }
            else if (grand_parent->left == parent)
            {
                grand_parent->left = sub_tree;
            }
            else
            {
                grand_parent->right = sub_tree;
            }
            
            if (grand_parent)
            {
                grand_parent->height =
                int_max(get_secondary_tree_node_height(grand_parent->left),
                        get_secondary_tree_node_height(grand_parent->right))
                    + 1;
            }
        }
        
        parent->height =
        int_max(get_secondary_tree_node_height(parent->left),
                get_secondary_tree_node_height(parent->right)) + 1;
        
        parent = parent->parent;
    }
}

/************************************************************************
* Unlinks 'node' from its collision tree when 'node' has both children. *
************************************************************************/
static void unlink_primary_collision_tree_node_with_both_children(
                    bidirectional_hash_map_2_t* map,
                    primary_collision_tree_node_t* node)
{
    primary_collision_tree_node_t* successor =
    get_minimum_tree_node_of(node->right);
    
    size_t node_bucket_index = node->key_pair->primary_key_hash
                             & map->modulo_mask;
    
    primary_collision_tree_node_t* child_of_successor;
    primary_collision_tree_node_t* parent_of_successor;
    
    node->key_pair = successor->key_pair;
    
    child_of_successor  = successor->right;
    parent_of_successor = successor->parent;
    
    /* THIS IS STRANGE: */
    if (parent_of_successor->left == successor)
    {
        parent_of_successor->left = child_of_successor;
    }
    else
    {
        parent_of_successor->right = child_of_successor;
    }
    
    if (child_of_successor)
    {
        child_of_successor->parent = parent_of_successor;
    }
    
    fix_primary_collision_tree_after_deletion(map, successor);
}

/*************************************************************************
* Unlinks 'node' from its collision tree when 'node' has only one child. *
*************************************************************************/
static void unlink_primary_collision_tree_node_with_one_child(
                    bidirectional_hash_map_2_t* map,
                    primary_collision_tree_node_t* node)
{
    primary_collision_tree_node_t* child;
    primary_collision_tree_node_t* parent;
    
    size_t node_bucket_index = node->key_pair->primary_key_hash
                             & map->modulo_mask;
    
    if (node->left)
    {
        child = node->left;
    }
    else
    {
        child = node->right;
    }
    
    parent = node->parent;
    child->parent = parent;
    
    if (!parent)
    {
        map->primary_key_table[node_bucket_index] = child;
        return;
    }
    
    if (node == parent->left)
    {
        parent->left = child;
    }
    else
    {
        parent->right = child;
    }
    
    fix_primary_collision_tree_after_deletion(map, node);
}

/**********************************************************************
* Unlinks 'node' from its collision tree when 'node' has no children. *
**********************************************************************/
static void unlink_primary_collision_tree_node_with_no_children(
                                            bidirectional_hash_map_2_t* map,
                                            primary_collision_tree_node_t* node)
{
    primary_collision_tree_node_t* parent = node->parent;
    
    size_t node_bucket_index = node->key_pair->primary_key_hash
                             & map->modulo_mask;
    
    if (!parent)
    {
        map->primary_key_table[node_bucket_index] = NULL;
        return;
    }
    
    if (node == parent->left)
    {
        parent->left = NULL;
    }
    else
    {
        parent->right = NULL;
    }
    
    fix_primary_collision_tree_after_deletion(map, node->parent);
}

/******************************************************************************
* This function unlinks 'primary_collision_tree_node' from it collision tree. *
******************************************************************************/
static void unlink_primary_collision_tree_node(
                    bidirectional_hash_map_2_t* map,
                    primary_collision_tree_node_t* primary_collision_tree_node)
{
    if (primary_collision_tree_node->left &&
        primary_collision_tree_node->right)
    {
        /***************************************************
        * 'primary_collision_tree_node' has both children. *
        ***************************************************/
        unlink_primary_collision_tree_node_with_both_children(
                                                map,
                                                primary_collision_tree_node);
    }
    else if (primary_collision_tree_node->left ||
             primary_collision_tree_node->right)
    {
        /****************************************************
        * 'primary_collision_tree_node' has only one child. *
        ****************************************************/
        unlink_primary_collision_tree_node_with_one_child(
                                                map,
                                                primary_collision_tree_node);
    }
    else
    {
        /*************************************************
        * 'primary_collision_tree_node' has no children. *
        *************************************************/
        unlink_primary_collision_tree_node_with_no_children(
                                                map,
                                                primary_collision_tree_node);
    }
}


/************************************************************************
* Unlinks 'node' from its collision tree when 'node' has both children. *
************************************************************************/
static void unlink_secondary_collision_tree_node_with_both_children(
                                        bidirectional_hash_map_2_t* map,
                                        secondary_collision_tree_node_t* node)
{
    primary_collision_tree_node_t* successor =
    get_minimum_tree_node_of(node->right);
    
    size_t node_bucket_index = node->key_pair->primary_key_hash
    & map->modulo_mask;
    
    primary_collision_tree_node_t* child_of_successor;
    primary_collision_tree_node_t* parent_of_successor;
    
    node->key_pair = successor->key_pair;
    
    child_of_successor  = successor->right;
    parent_of_successor = successor->parent;
    
    /* THIS IS STRANGE: */
    if (parent_of_successor->left == successor)
    {
        parent_of_successor->left = child_of_successor;
    }
    else
    {
        parent_of_successor->right = child_of_successor;
    }
    
    if (child_of_successor)
    {
        child_of_successor->parent = parent_of_successor;
    }
    
    fix_primary_collision_tree_after_deletion(map, successor);
}

/*************************************************************************
* Unlinks 'node' from its collision tree when 'node' has only one child. *
*************************************************************************/
static void unlink_secondary_collision_tree_node_with_one_child(
                                        bidirectional_hash_map_2_t* map,
                                        secondary_collision_tree_node_t* node)
{
    primary_collision_tree_node_t* child;
    primary_collision_tree_node_t* parent;
    
    size_t node_bucket_index = node->key_pair->primary_key_hash
    & map->modulo_mask;
    
    if (node->left)
    {
        child = node->left;
    }
    else
    {
        child = node->right;
    }
    
    parent = node->parent;
    child->parent = parent;
    
    if (!parent)
    {
        map->primary_key_table[node_bucket_index] = child;
        return;
    }
    
    if (node == parent->left)
    {
        parent->left = child;
    }
    else
    {
        parent->right = child;
    }
    
    fix_primary_collision_tree_after_deletion(map, node);
}

/**********************************************************************
* Unlinks 'node' from its collision tree when 'node' has no children. *
**********************************************************************/
static void unlink_secondary_collision_tree_node_with_no_children(
                                        bidirectional_hash_map_2_t* map,
                                        secondary_collision_tree_node_t* node)
{
    primary_collision_tree_node_t* parent = node->parent;
    
    size_t node_bucket_index = node->key_pair->primary_key_hash
    & map->modulo_mask;
    
    if (!parent)
    {
        map->primary_key_table[node_bucket_index] = NULL;
        return;
    }
    
    if (node == parent->left)
    {
        parent->left = NULL;
    }
    else
    {
        parent->right = NULL;
    }
    
    fix_primary_collision_tree_after_deletion(map, node->parent);
}

/***************************************************************************
* This function unlinks 'secondary_collision_tree_node' from its collision *
* tree.                                                                    *
***************************************************************************/
static void unlink_secondary_collision_tree_node(
                bidirectional_hash_map_2_t* map,
                secondary_collision_tree_node_t* secondary_collision_tree_node)
{
    if (secondary_collision_tree_node->left &&
        secondary_collision_tree_node->right)
    {
        /*****************************************************
        * 'secondary_collision_tree_node' has both children. *
        *****************************************************/
        unlink_secondary_collision_tree_node_with_both_children(
                                                map,
                                                secondary_collision_tree_node);
    }
    else if (secondary_collision_tree_node->left || secondary_collision_tree_node->right)
    {
        /******************************************************
        * 'secondary_collision_tree_node' has only one child. *
        ******************************************************/
        unlink_secondary_collision_tree_node_with_one_child(
                                                map,
                                                secondary_collision_tree_node);
    }
    else
    {
        /***************************************************
        * 'secondary_collision_tree_node' has no children. *
        ***************************************************/
        unlink_secondary_collision_tree_node_with_no_children(
                                                map,
                                                secondary_collision_tree_node);
    }
}

/************************************************************
* Finds a secondary collision tree node that corresponds to *
* 'primary_collision_tree_node'.                            *
************************************************************/
static secondary_collision_tree_node_t*
find_secondary_collision_tree_node_via_primary_collision_tree_node(
                    bidirectional_hash_map_2_t* map,
                    primary_collision_tree_node_t* primary_collision_tree_node)
{
    /* TODO! */
    return NULL;
}

/***********************************************************
 * Finds a primary collision chain node that corresponds to *
 * 'secondary_collision_chain_node'.                        *
 ***********************************************************/
static primary_collision_tree_node_t*
find_primary_collision_tree_node_via_secondary_collision_tree_node(
                bidirectional_hash_map_2_t* map,
                secondary_collision_tree_node_t* secondary_collision_chain_node)
{
    /* TODO! */
    return NULL;
}

/**************************************************************************
 * This function removes 'primary_collision_chain_node' from the iteration *
 * list.                                                                   *
 **************************************************************************/
static void unlink_primary_collision_tree_node_from_iteraton_list(
                    bidirectional_hash_map_2_t* map,
                    primary_collision_tree_node_t* primary_collision_tree_node)
{
    if (primary_collision_tree_node->up == NULL)
    {
        map->first_collision_chain_node = primary_collision_tree_node->down;
    }
    else
    {
        primary_collision_tree_node->up->down =
        primary_collision_tree_node->down;
    }
    
    if (primary_collision_tree_node->down == NULL)
    {
        map->last_collision_chain_node = primary_collision_tree_node->up;
    }
    else
    {
        primary_collision_tree_node->down->up =
        primary_collision_tree_node->up;
    }
}

/****************************************************************************
* This function is responsible for removing a primary/secondary key mapping *
* from the bidirectional hash map.                                          *
****************************************************************************/
static void remove_mapping(
                    bidirectional_hash_map_2_t* map,
                    primary_collision_tree_node_t* primary_collision_tree_node)
{
    secondary_collision_tree_node_t* secondary_collision_tree_node =
    find_secondary_collision_tree_node_via_primary_collision_tree_node(
                                                map,
                                                primary_collision_tree_node);
    
    free(primary_collision_tree_node->key_pair);
    unlink_primary_collision_tree_node_from_iteraton_list(
                                                map,
                                                primary_collision_tree_node);
    
    /*****************************************************
    * Unlink and purge the primary collision chain node: *
    *****************************************************/
    unlink_primary_collision_tree_node(map, primary_collision_tree_node);
    free(primary_collision_tree_node);
    
    /*******************************************************
    * Unlink and purge the secondary collision chain node: *
    *******************************************************/
    unlink_secondary_collision_tree_node(map, secondary_collision_tree_node);
    free(secondary_collision_tree_node);
}

/*************************************************************************
* This functions returns a primary collision chain node corresponding to *
* 'primary_key'.                                                         *
*************************************************************************/
static primary_collision_tree_node_t* find_primary_collision_tree_node(
                                                bidirectional_hash_map_2_t* map,
                                                void* primary_key)
{
    size_t primary_key_hash = map->primary_key_hasher(primary_key);
    
    size_t primary_key_collision_chain_bucket_index =
    primary_key_hash & map->modulo_mask;
    
    primary_collision_tree_node_t* primary_collision_chain_node =
    map->primary_key_table[primary_key_collision_chain_bucket_index];
    
    /* TODO! */
    return primary_collision_chain_node;
}

/***************************************************************************
 * This functions returns a secondary collision chain node corresponding to *
 * 'secondary_key'.                                                         *
 ***************************************************************************/
static secondary_collision_tree_node_t* find_secondary_collision_tree_node(
                                                bidirectional_hash_map_2_t* map,
                                                void* secondary_key)
{
    size_t secondary_key_hash = map->secondary_key_hasher(secondary_key);
    
    size_t secondary_key_collision_chain_bucket_index =
    secondary_key_hash & map->modulo_mask;
    
    /* TODO! */
    
    return NULL;
}

int bidirectional_hash_map_2_t_init(
                                  bidirectional_hash_map_2_t* map,
                                  size_t initial_capacity,
                                  float load_factor,
                                  size_t (*primary_key_hasher)  (void*),
                                  size_t (*secondary_key_hasher)(void*),
                                  int (*primary_key_equality)   (void*, void*),
                                  int (*secondary_key_equality) (void*, void*),
                                  int (*primary_key_compare)    (void*, void*),
                                  int (*secondary_key_compare)  (void*, void*),
                                  void* error_sentinel)
{
    if (!map)
    {
        return 0;
    }
    
    if (!primary_key_hasher
        || !primary_key_equality
        || !primary_key_compare
        || !secondary_key_hasher
        || !secondary_key_equality
        || !secondary_key_compare)
    {
        return 0;
    }
    
    load_factor      = max_float(load_factor, MINIMUM_LOAD_FACTOR);
    initial_capacity = max_size_t(initial_capacity, MINIMUM_INITIAL_CAPACITY);
    initial_capacity = to_power_of_two(initial_capacity);
    
    map->primary_key_table   = NULL;
    map->secondary_key_table = NULL;
    map->capacity            = initial_capacity;
    map->load_factor         = load_factor;
    map->size                = 0;
    
    map->primary_key_table = calloc(initial_capacity,
                                    sizeof(primary_collision_tree_node_t*));
    
    if (!map->primary_key_table)
    {
        return 0;
    }
    
    map->secondary_key_table =
    calloc(initial_capacity, sizeof(secondary_collision_tree_node_t*));
    
    if (!map->secondary_key_table)
    {
        free(map->primary_key_table);
        map->primary_key_table = NULL;
        return 0;
    }
    
    map->modulo_mask            = map->capacity - 1;
    map->primary_key_hasher     = primary_key_hasher;
    map->secondary_key_hasher   = secondary_key_hasher;
    map->primary_key_equality   = primary_key_equality;
    map->secondary_key_equality = secondary_key_equality;
    map->primary_key_compare    = primary_key_compare;
    map->secondary_key_compare  = secondary_key_compare;
    map->error_sentinel         = error_sentinel;
    
    return 1;
}

void bidirectional_hash_map_2_t_destroy(bidirectional_hash_map_2_t* map)
{
    primary_collision_tree_node_t* primary_collision_chain_node;
    primary_collision_tree_node_t* primary_collision_chain_node_next;
    
    if (!map)
    {
        return;
    }
    
    if (!map->primary_key_table)
    {
        /*********************************************************************
        * The input map is invalid (failed to be constructed due to shortage *
        * of memory).                                                        *
        *********************************************************************/
        return;
    }
    
    primary_collision_chain_node = map->first_collision_chain_node;
    
    /*************************
    * Free the mapping data. *
    *************************/
    while (primary_collision_chain_node)
    {
        primary_collision_chain_node_next = primary_collision_chain_node->down;
        remove_mapping(map, primary_collision_chain_node);
        primary_collision_chain_node = primary_collision_chain_node_next;
    }
    
    /*******************************
    * Free the actual hash tables. *
    *******************************/
    free(map->primary_key_table);
    free(map->secondary_key_table);
    
    map->primary_key_table   = NULL;
    map->secondary_key_table = NULL;
    map->first_collision_chain_node = NULL;
    map->last_collision_chain_node = NULL;
    map->capacity = 0;
    map->size = 0;
}

int bidirectional_hash_map_2_t_is_working(bidirectional_hash_map_2_t* map)
{
    return map->primary_key_table ? 1 : 0;
}

size_t bidirectional_hash_map_2_t_size(bidirectional_hash_map_2_t* map)
{
    return map->size;
}

size_t bidirectional_hash_map_2_t_capacity(bidirectional_hash_map_2_t* map)
{
    return map->capacity;
}

/*****************************************************************************
 * This function relinks all the mappings (key pairs and collision chains) to *
 * new hash tables.                                                           *
 *****************************************************************************/
static void relink_to_new_tables(
                    bidirectional_hash_map_2_t* map,
                    primary_collision_tree_node_t* primary_collision_chain_node,
                    primary_collision_tree_node_t** next_primary_hash_table,
                    secondary_collision_tree_node_t** next_secondary_hash_table)
{
    size_t primary_collision_chain_bucket_index;
    size_t secondary_collision_chain_bucket_index;
    size_t next_capacity;
    size_t next_modulo_mask;
    
    secondary_collision_tree_node_t* secondary_collision_chain_node =
    find_secondary_collision_chain_node_via_primary_collision_chain_node(
                                                map,
                                                primary_collision_chain_node);
    
    /******************************************************************
    * Unlink the 'primary_collision_tree_node' from its current tree. *
    ******************************************************************/
    unlink_primary_collision_tree_node(map, primary_collision_chain_node);
    
    /*********************************************************
    * Unlink the opposite collision tree node of             *
    * 'primary_collision_tree_node' from the collision tree. *
    *********************************************************/
    unlink_secondary_collision_tree_node(map, secondary_collision_chain_node);
    
    /******************************************************
    * Relink both 'primary_collision_tree_node' and       *
    * 'secondary_collision_tree_node' to new hash tables. *
    ******************************************************/
    next_capacity = map->capacity << 1;
    next_modulo_mask = next_capacity - 1;
    
    /************************************************************
    * Link 'primary_collision_tree_node' to its new hash table. *
    ************************************************************/
    primary_collision_chain_bucket_index =
    primary_collision_chain_node->key_pair->primary_key_hash & next_modulo_mask;
    
    /* TODO! */
    
    /**************************************************************
    * Link 'secondary_collision_tree_node' to its new hash table. *
    **************************************************************/
    
    /* TODO! */
}

/*******************************************************************************
 * This function is responsible for allocating larger hash tables and relinking *
 * all current collision chain nodes and key pairs to them.                     *
 *******************************************************************************/
static int expand_hash_map(bidirectional_hash_map_2_t* map)
{
    size_t next_capacity;
    size_t next_modulo_mask;
    primary_collision_tree_node_t** next_primary_hash_table;
    secondary_collision_tree_node_t** next_secondary_hash_table;
    primary_collision_tree_node_t* primary_collision_chain_node;
    primary_collision_tree_node_t* primary_collision_chain_node_next;
    
    next_capacity = map->capacity << 1;
    
    next_primary_hash_table = calloc(next_capacity,
                                     sizeof(primary_collision_tree_node_t*));
    
    if (!next_primary_hash_table)
    {
        return 0;
    }
    
    next_secondary_hash_table =
    calloc(next_capacity, sizeof(secondary_collision_tree_node_t*));
    
    if (!next_secondary_hash_table)
    {
        free(next_primary_hash_table);
        return 0;
    }
    
    next_modulo_mask = next_capacity - 1;
    primary_collision_chain_node = map->first_collision_chain_node;
    
    while (primary_collision_chain_node)
    {
        primary_collision_chain_node_next = primary_collision_chain_node->down;
        relink_to_new_tables(map,
                             primary_collision_chain_node,
                             next_primary_hash_table,
                             next_secondary_hash_table);
        
        primary_collision_chain_node = primary_collision_chain_node_next;
    }
    
    free(map->primary_key_table);
    free(map->secondary_key_table);
    
    map->primary_key_table = next_primary_hash_table;
    map->secondary_key_table = next_secondary_hash_table;
    map->capacity = next_capacity;
    map->modulo_mask = next_modulo_mask;
    
    return 1;
}

/************************************************************************
 * This function is responsible for updating a primary key of a mapping. *
 ************************************************************************/
static void* update_primary_key(
                bidirectional_hash_map_2_t* map,
                secondary_collision_tree_node_t* secondary_collision_chain_node,
                void* new_primary_key)
{
    void* old_primary_key;
    size_t new_primary_key_hash;
    size_t new_primary_key_collision_chain_bucket_index;
    
    /*******************************************************
    * Find the corresponding primary collision chain node: *
    *******************************************************/
    primary_collision_tree_node_t* primary_collision_chain_node =
    find_primary_collision_tree_node_via_secondary_collision_tree_node(
                                                map,
                                                secondary_collision_chain_node);
    
    old_primary_key = primary_collision_chain_node->key_pair->primary_key;
    
    /**************************************************************************
    * Unlink 'primary_collision_chain_node' from its current collision chain: *
    **************************************************************************/
    unlink_primary_collision_chain_node(map, primary_collision_chain_node);
    
    /************************************************************************
    * Link the unlinked 'primary_collision_chain_node' to its new collision *
    * chain. Updates the actual key and its hash as well.                   *
    ************************************************************************/
    new_primary_key_hash = map->primary_key_hasher(new_primary_key);
    new_primary_key_collision_chain_bucket_index =
    new_primary_key_hash & map->modulo_mask;
    
    primary_collision_chain_node->key_pair->primary_key = new_primary_key;
    primary_collision_chain_node->key_pair->primary_key_hash =
    new_primary_key_hash;
    
    primary_collision_chain_node->prev = NULL;
    primary_collision_chain_node->next =
    map->primary_key_table[new_primary_key_collision_chain_bucket_index];
    
    if (map->primary_key_table[new_primary_key_collision_chain_bucket_index])
    {
        map->primary_key_table[new_primary_key_collision_chain_bucket_index]
        ->prev = primary_collision_chain_node;
    }
    
    map->primary_key_table[new_primary_key_collision_chain_bucket_index] =
    primary_collision_chain_node;
    
    return old_primary_key;
}

static void* update_secondary_key(
                    bidirectional_hash_map_2_t* map,
                    primary_collision_tree_node_t* primary_collision_chain_node,
                    void* new_secondary_key)
{
    void* old_secondary_key;
    size_t new_secondary_key_hash;
    size_t new_secondary_key_collision_chain_bucket_index;
    
    /*********************************************************
    * Find the corresponding secondary collision chain node: *
    *********************************************************/
    secondary_collision_tree_node_t* secondary_collision_tree_node =
    find_secondary_collision_chain_node_via_primary_collision_chain_node(
                                                                         map,
                                                                         primary_collision_chain_node);
    
    old_secondary_key = secondary_collision_chain_node->key_pair->secondary_key;
    
    /*********************************************************************
    * Unlink 'secondary_collision_chain_node' from its current collision *
    * chain:                                                             *
    *********************************************************************/
    unlink_secondary_collision_chain_node(map, secondary_collision_chain_node);
    
    /***************************************************************************
    * Links the unlinked 'secondary_collision_chain_node' to its new collision *
    * chain. Updates the actual key and its has as well.                       *
    ***************************************************************************/
    new_secondary_key_hash = map->secondary_key_hasher(new_secondary_key);
    new_secondary_key_collision_chain_bucket_index =
    new_secondary_key_hash & map->modulo_mask;
    
    secondary_collision_chain_node->key_pair->secondary_key = new_secondary_key;
    secondary_collision_chain_node->key_pair->secondary_key_hash =
    new_secondary_key_hash;
    
    secondary_collision_chain_node->prev = NULL;
    secondary_collision_chain_node->next =
    map->secondary_key_table[new_secondary_key_collision_chain_bucket_index];
    
    if (map->
        secondary_key_table[new_secondary_key_collision_chain_bucket_index])
    {
        map->secondary_key_table[new_secondary_key_collision_chain_bucket_index]
        ->prev = secondary_collision_chain_node;
    }
    
    map->secondary_key_table[new_secondary_key_collision_chain_bucket_index] =
    secondary_collision_chain_node;
    
    return old_secondary_key;
}

/*******************************************************************************
* Adds a new mapping to the map. A mapping (primary_key, secondary_key) is     *
* "new" if primary_key is not mapped to anything and secondary is not mapped   
* to anything as well. This function also increments the 'size' of the map.    *
*******************************************************************************/
static int add_new_mapping(bidirectional_hash_map_2_t* map,
                           void* primary_key,
                           void* secondary_key)
{
    key_pair_t* key_pair;
    primary_collision_tree_node_t* primary_collision_chain_node;
    secondary_collision_tree_node_t* secondary_collision_chain_node;
    size_t primary_key_collision_chain_bucket_index;
    size_t secondary_key_collision_chain_bucket_index;
    
    if (map->size > map->capacity * map->load_factor)
    {
        if (!expand_hash_map(map))
        {
            return 0;
        }
    }
    
    key_pair = malloc(sizeof(*key_pair));
    
    if (!key_pair)
    {
        return 0;
    }
    
    primary_collision_chain_node =
    malloc(sizeof(*primary_collision_chain_node));
    
    if (!primary_collision_chain_node)
    {
        free(key_pair);
        return 0;
    }
    
    secondary_collision_chain_node =
    malloc(sizeof(*secondary_collision_chain_node));
    
    if (!secondary_collision_chain_node)
    {
        free(key_pair);
        free(primary_collision_chain_node);
        return 0;
    }
    
    key_pair->primary_key = primary_key;
    key_pair->primary_key_hash = map->primary_key_hasher(primary_key);
    key_pair->secondary_key = secondary_key;
    key_pair->secondary_key_hash = map->secondary_key_hasher(secondary_key);
    
    /****************************************************
    * Link 'primary_collision_chain_node' to its table: *
    ****************************************************/
    primary_collision_chain_node->key_pair = key_pair;
    primary_collision_chain_node->prev = NULL;
    primary_key_collision_chain_bucket_index =
    key_pair->primary_key_hash & map->modulo_mask;
    
    primary_collision_chain_node->next =
    map->primary_key_table[primary_key_collision_chain_bucket_index];
    
    if (map->primary_key_table[primary_key_collision_chain_bucket_index])
    {
        map->primary_key_table[primary_key_collision_chain_bucket_index]->prev =
        primary_collision_chain_node;
    }
    
    map->primary_key_table[primary_key_collision_chain_bucket_index] =
    primary_collision_chain_node;
    
    /******************************************************
    * Link 'secondary_collision_chain_node' to its table: *
    ******************************************************/
    secondary_collision_chain_node->key_pair = key_pair;
    secondary_collision_chain_node->prev = NULL;
    secondary_key_collision_chain_bucket_index =
    key_pair->secondary_key_hash & map->modulo_mask;
    
    secondary_collision_chain_node->next =
    map->secondary_key_table[secondary_key_collision_chain_bucket_index];
    
    if (map->secondary_key_table[secondary_key_collision_chain_bucket_index])
    {
        map->secondary_key_table[secondary_key_collision_chain_bucket_index]
        ->prev = secondary_collision_chain_node;
    }
    
    map->secondary_key_table[secondary_key_collision_chain_bucket_index] =
    secondary_collision_chain_node;
    
    /********************************
    * Deal with the iteration list. *
    ********************************/
    if (map->size == 0)
    {
        map->first_collision_chain_node = primary_collision_chain_node;
        map->last_collision_chain_node = primary_collision_chain_node;
        primary_collision_chain_node->up = NULL;
        primary_collision_chain_node->down = NULL;
    }
    else
    {
        primary_collision_chain_node->up = map->last_collision_chain_node;
        primary_collision_chain_node->down = NULL;
        map->last_collision_chain_node->down = primary_collision_chain_node;
        map->last_collision_chain_node = primary_collision_chain_node;
    }
    
    map->size++;
    return 1;
}

void* bidirectional_hash_map_t_put_by_primary(bidirectional_hash_map_t* map,
                                              void* primary_key,
                                              void* secondary_key)
{
    size_t primary_key_hash;
    size_t primary_key_collision_chain_bucket_index;
    primary_collision_chain_node_t* primary_collision_chain_node;
    void* return_value;
    
    primary_key_hash = map->primary_key_hasher(primary_key);
    primary_key_collision_chain_bucket_index =
    primary_key_hash & map->modulo_mask;
    
    primary_collision_chain_node =
    find_primary_collision_chain_node(map, primary_key);
    
    if (primary_collision_chain_node)
    {
        return update_secondary_key(map,
                                    primary_collision_chain_node,
                                    secondary_key);
    }
    else
    {
        add_new_mapping(map, primary_key, secondary_key);
        return NULL;
    }
}

void* bidirectional_hash_map_t_put_by_secondary(bidirectional_hash_map_t* map,
                                                void* primary_key,
                                                void* secondary_key)
{
    size_t secondary_key_hash;
    size_t secondary_key_collision_chain_bucket_index;
    secondary_collision_chain_node_t* secondary_collision_chain_node;
    void* return_value;
    
    secondary_key_hash = map->secondary_key_hasher(secondary_key);
    secondary_key_collision_chain_bucket_index =
    secondary_key_hash & map->modulo_mask;
    
    secondary_collision_chain_node =
    find_secondary_collision_chain_node(map, secondary_key);
    
    if (secondary_collision_chain_node)
    {
        return update_primary_key(map,
                                  secondary_collision_chain_node,
                                  primary_key);
    }
    else
    {
        add_new_mapping(map, primary_key, secondary_key);
        return NULL;
    }
}

void* bidirectional_hash_map_t_remove_by_primary_key(
                                                     bidirectional_hash_map_t* map,
                                                     void* primary_key)
{
    primary_collision_chain_node_t* primary_collision_chain_node =
    find_primary_collision_chain_node(map, primary_key);
    
    secondary_collision_chain_node_t* secondary_collision_chain_node =
    find_secondary_collision_chain_node_via_primary_collision_chain_node(
                                                                         map,
                                                                         primary_collision_chain_node);
    void* secondary_key;
    
    if (primary_collision_chain_node == NULL)
    {
        return NULL;
    }
    
    unlink_primary_collision_chain_node(map, primary_collision_chain_node);
    unlink_secondary_collision_chain_node(map, secondary_collision_chain_node);
    secondary_key = primary_collision_chain_node->key_pair->secondary_key;
    
    unlink_primary_collision_chain_node_from_iteraton_list(
                                                           map,
                                                           primary_collision_chain_node);
    
    free(primary_collision_chain_node->key_pair);
    free(primary_collision_chain_node);
    free(secondary_collision_chain_node);
    
    return secondary_key;
}

void* bidirectional_hash_map_t_remove_by_secondary_key(
                                                       bidirectional_hash_map_t* map,
                                                       void* secondary_key)
{
    secondary_collision_chain_node_t* secondary_collision_chain_node =
    find_secondary_collision_chain_node(map, secondary_key);
    
    primary_collision_chain_node_t* primary_collision_chain_node =
    find_primary_collision_chain_node_via_secondary_collision_chain_node(
                                                                         map,
                                                                         secondary_collision_chain_node);
    void* primary_key;
    
    if (secondary_collision_chain_node == NULL)
    {
        return NULL;
    }
    
    unlink_primary_collision_chain_node(map, primary_collision_chain_node);
    unlink_secondary_collision_chain_node(map, secondary_collision_chain_node);
    primary_key = primary_collision_chain_node->key_pair->primary_key;
    
    unlink_primary_collision_chain_node_from_iteraton_list(
                                                           map,
                                                           primary_collision_chain_node);
    
    unlink_primary_collision_chain_node_from_iteraton_list(map, primary_collision_chain_node);
    
    free(primary_collision_chain_node->key_pair);
    free(primary_collision_chain_node);
    free(secondary_collision_chain_node);
    
    return primary_key;
}

void* bidirectional_hash_map_t_get_by_primary_key(bidirectional_hash_map_t* map,
                                                  void* primary_key)
{
    primary_collision_chain_node_t* primary_collision_chain_node =
    find_primary_collision_chain_node(map, primary_key);
    
    secondary_collision_chain_node_t* secondary_collision_chain_node;
    
    if (primary_collision_chain_node == NULL)
    {
        return NULL;
    }
    
    secondary_collision_chain_node =
    find_secondary_collision_chain_node_via_primary_collision_chain_node(
                                                                         map,
                                                                         primary_collision_chain_node);
    
    return secondary_collision_chain_node->key_pair->secondary_key;
}

void* bidirectional_hash_map_t_get_by_secondary_key(
                                                    bidirectional_hash_map_t* map,
                                                    void* secondary_key)
{
    secondary_collision_chain_node_t* secondary_collision_chain_node =
    find_secondary_collision_chain_node(map, secondary_key);
    
    primary_collision_chain_node_t* primary_collision_chain_node;
    
    if (secondary_collision_chain_node == NULL)
    {
        return NULL;
    }
    
    primary_collision_chain_node =
    find_primary_collision_chain_node_via_secondary_collision_chain_node(
                                                                         map,
                                                                         secondary_collision_chain_node);
    
    return secondary_collision_chain_node->key_pair->primary_key;
}

int bidirectional_hash_map_t_contains_primary_key(bidirectional_hash_map_t* map,
                                                  void* primary_key)
{
    primary_collision_chain_node_t* primary_collision_chain_node =
    find_primary_collision_chain_node(map, primary_key);
    
    return primary_collision_chain_node != NULL ? 1 : 0;
}

int bidirectional_hash_map_t_contains_secondary_key(
                                                    bidirectional_hash_map_t* map,
                                                    void* secondary_key)
{
    secondary_collision_chain_node_t* secondary_collision_chain_node =
    find_secondary_collision_chain_node(map, secondary_key);
    
    return secondary_collision_chain_node != NULL ? 1 : 0;
}

int bidirectional_hash_map_iterator_t_init(
                                           bidirectional_hash_map_t* map,
                                           bidirectional_hash_map_iterator_t* iterator)
{
    if (!map)
    {
        return 0;
    }
    
    iterator->current_node = map->first_collision_chain_node;
    iterator->iterated = 0;
    iterator->map_size = map->size;
    
    return 1;
}

int bidirectional_hash_map_iterator_t_has_next(
                                               bidirectional_hash_map_iterator_t* iterator)
{
    return iterator->iterated < iterator->map_size;
}

int bidirectional_hash_map_iterator_t_next(
                                           bidirectional_hash_map_iterator_t* iterator,
                                           void** primary_key_ptr,
                                           void** secondary_key_ptr)
{
    if (iterator->iterated >= iterator->map_size)
    {
        return 0;
    }
    
    *primary_key_ptr = iterator->current_node->key_pair->primary_key;
    *secondary_key_ptr = iterator->current_node->key_pair->secondary_key;
    iterator->current_node = iterator->current_node->down;
    return 1;
}
