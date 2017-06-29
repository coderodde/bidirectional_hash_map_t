#ifndef BIDIRECTIONAL_HASH_MAP_2_H
#define BIDIRECTIONAL_HASH_MAP_2_H

#include <stdlib.h>

typedef struct key_pair_t {
    
    /*******************
    * The primary key. *
    *******************/
    void* primary_key;
    
    /*********************
    * The secondary key. *
    *********************/
    void* secondary_key;
    
    /*******************************
    * The hash of the primary key. *
    *******************************/
    size_t primary_key_hash;
    
    /********************************
    * The hash of the secondary key *
    ********************************/
    size_t secondary_key_hash;
}
key_pair_t;

/****************************************************************************
* The primary collision tree node. This implements essentially an AVL-tree. *
****************************************************************************/
typedef struct primary_collision_tree_node_t {
    
    /***************************************************************************
    * Points to a parent node or is set to NULL if this tree node is the root. *
    ***************************************************************************/
    struct primary_collision_tree_node_t* parent;
    
    /*********************************************
    * Points to the left child of the tree node. *
    *********************************************/
    struct primary_collision_tree_node_t* left;
    
    /**********************************************
    * Points to the right child of the tree node. *
    **********************************************/
    struct primary_collision_tree_node_t* right;
    
    /***************************************************************************
    * The height of this tree node. The leaves have height of 0 and the height *
    * grows while going upwards in the tree.                                   *
    ***************************************************************************/
    size_t height;
    
    /**************************************************************************
    * The previously added node. This field is used for faster iteration over *
    * the entire hash map.                                                    *
    **************************************************************************/
    struct primary_collision_tree_node_t* up;
    
    /***************************************************************************
    * The collision chain node added after this collision chain node. Used for *
    * faster iteration over the hash map.                                      *
    ***************************************************************************/
    struct primary_collision_tree_node_t* down;
    
    /*******************************************
    * Points to the actual key pair structure. *
    *******************************************/
    key_pair_t* key_pair;
}
primary_collision_tree_node_t;

/**********************************************************
* The collision chain node type for secondary key chains. *
**********************************************************/
typedef struct secondary_collision_tree_node_t {
    
    /***************************************************************************
    * Points to a parent node or is set to NULL if this tree node is the root. *
    ***************************************************************************/
    struct secondary_collision_tree_node_t* parent;
    
    /*********************************************
    * Points to the left child of the tree node. *
    *********************************************/
    struct secondary_collision_tree_node_t* left;
    
    /**********************************************
    * Points to the right child of the tree node. *
    **********************************************/
    struct secondary_collision_tree_node_t* right;
    
    /***************************************************************************
    * The height of this tree node. The leaves have height of 0 and the height *
    * grows while going upwards in the tree.                                   *
    ***************************************************************************/
    size_t height;
    
    /*******************************************
    * Points to the actual key pair structure. *
    *******************************************/
    key_pair_t* key_pair;
}
secondary_collision_tree_node_t;

typedef struct bidirectional_hash_map_2_t {
    
    /**********************************
    * Caches the number of key pairs. *
    **********************************/
    size_t size;
    
    /*********************************************
    * Holds the capacity of the two hash tables. *
    *********************************************/
    size_t capacity;
    
    /*************************
    * Stores the load factor *
    *************************/
    float  load_factor;
    
    /***************************************
    * The mask used for simulating modulo. *
    ***************************************/
    size_t modulo_mask;
    
    /**************************
    * The primary hash table. *
    **************************/
    struct primary_collision_tree_node_t** primary_key_table;
    
    /****************************
    * The secondary hash table. *
    ****************************/
    struct secondary_collision_tree_node_t** secondary_key_table;
    
    /***************************************************************************
    * The function producing the bucket index in the primary key table given a *
    * primary key.                                                             *
    ***************************************************************************/
    size_t (*primary_key_hasher)(void* primary_key);
    
    /***************************************************************************
    * The function producing the bucket index in the secondary key table given *
    * a secondary key.                                                         *
    ***************************************************************************/
    size_t (*secondary_key_hasher)(void* secondary_key);
    
    /*****************************************************
    * The function for comparing two given primary keys. *
    *****************************************************/
    int    (*primary_key_equality)(void* primary_key_1, void* primary_key_2);
    
    /*******************************************************
    * The function for comparing two given secondary keys. *
    *******************************************************/
    int    (*secondary_key_equality)(void* secondary_key_1,
                                     void* secondary_key_2);
    
    /***********************************************
    * The function for comparing the primary keys. *
    ***********************************************/
    int (*primary_key_compare)(void* key1, void* key2);
    
    /*************************************************
    * The function for comparing the secondary keys. *
    *************************************************/
    int (*secondary_key_compare)(void* key1, void* key2);
    
    /***************************************************************************
    * Caches the primary collision chain node of the mapping that was added to *
    * this hash map. Used for starting the iteration over all mappings. We     *
    * need this since the hash map may be too sparse after, say, adding a lot  *
    * of elements and removing most of them.                                   *
    ***************************************************************************/
    struct primary_collision_tree_node_t* first_collision_chain_node;
    
    /***************************************************************************
    * Caches the most recently added mapping to this hash map. We need this in *
    * order to link new mappings to the mapping list.                          *
    ***************************************************************************/
    struct primary_collision_tree_node_t* last_collision_chain_node;
    
    /*****************************************
    * A value that is returned upon failure. *
    *****************************************/
    void* error_sentinel;
}
bidirectional_hash_map_2_t;

typedef struct bidirectional_hash_map_2_iterator_t {
    
    /************************************
    * The mapping next to iterate over. *
    ************************************/
    struct primary_collision_tree_node_t* current_node;
    
    /**************************************
    * Number of mappings iterated so far. *
    **************************************/
    size_t iterated;
    
    /**************************************
    * The size of the map being iterated. *
    **************************************/
    size_t map_size;
}
bidirectional_hash_map_2_iterator_t;

/****************************************************************************
* Builds a new, empty bidirectional hash map.|                              *
*--------------------------------------------+                              *
* map -------------------- the map to initialize.                           *
* initial_capacity ------- the initial capacity of both the hash tables.    *
* load_factor ------------ the load factor.                                 *
* primary_key_hasher ----- the function for producing primary key hashes.   *
* secondary_key_hasher --- the function for producing secondary key hashes. *
* primary_key_equality --- the function for querying primary key equality.  *
* secondary_key_equality - the function for querying secondary key equality.*
* primary_key_compare ---- the function for comparing primary keys.         *
* secondary_key_compare -- the function for comparing secondary keys.       *
* error_sentinel --------- the sentinel return on failed addition.          *
*-----------------------------------------------------------+               *
* RETURNS: 1 if initialization was successfull, 0 otherwise.|               *
****************************************************************************/
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
                                  void* error_sentinel);

/************************************************
* Releases all the resources of the input map.| *
*---------------------------------------------+ *
* map - the map to destroy.                     *
************************************************/
void bidirectional_hash_map_2_t_destroy(bidirectional_hash_map_2_t* map);

/********************************************************************
* Checks that the map is well formed and is ready to receive data.| *
*-----------------------------------------------------------------+ *
* map - the map to check.                                           *
*------------------------------------------------+                  *
* RETURNS: 1 if the map is in order, 0 otherwise.|                  *
********************************************************************/
int bidirectional_hash_map_2_t_is_working(bidirectional_hash_map_2_t* map);

/*****************************************************
* Returns the number of key pairs in the input map.| *
*--------------------------------------------------+ *
* map - the map to query.                            *
*----------------------------------------------+     *
* RETURNS: the number of key pairs in this map.|     *
*****************************************************/
size_t bidirectional_hash_map_2_t_size(bidirectional_hash_map_2_t* map);

/*****************************************************************************
* Returns the capacity of one of the hash tables (another one has the same | *
* capacity).                                                               | *
*--------------------------------------------------------------------------+ *
* map - the map to query.                                                    *
*------------------------------------------+                                 *
* RETURNS: the capacity of each hash table.|                                 *
*****************************************************************************/
size_t bidirectional_hash_map_2_t_capacity(bidirectional_hash_map_2_t* map);

/******************************************************************************
* Associates the primary key to the secondary key in the input map.|          *
*------------------------------------------------------------------+          *
* map ----------- the map into which to store the pair.                       *
* primary_key --- the primary key.                                            *
* secondary_key - the secondary key.                                          *
*-------------------------------------------------------------------------- + *
* RETURNS: old secondary key in case the primary key is in the map, NULL if | *
* the primary key has no mappings yet.                                      | *
******************************************************************************/
void* bidirectional_hash_map_2_t_put_by_primary(bidirectional_hash_map_2_t* map,
                                                void* primary_key,
                                                void* secondary_key);

/******************************************************************************
* Associates the secondary key to the primary key in the input map.|          *
*------------------------------------------------------------------+          *
* map ----------- the map into which to store the pair.                       *
* primary_key --- the primary key.                                            *
* secondary_key - the secondary key.                                          *
*---------------------------------------------------------------------------+ *
* RETURNS: old primary key in case the secondary key is in the map, NULL if | *
* the secondary key has no mappings yet.                                    | *
******************************************************************************/
void* bidirectional_hash_map_2_t_put_by_secondary(
                                                bidirectional_hash_map_2_t* map,
                                                void* primary_key,
                                                void* secondary_key);

/******************************************************************************
* Removes a key pair by its primary key.|                                     *
*---------------------------------------+                                     *
* map --------- the map.                                                      *
* primary_key - the primary key.                                              *
*---------------------------------------------------------------------------+ *
* RETURNS: NULL if the primary key is not mapped. The current associated    | *
* secondary key otherwise.                                                  | *
******************************************************************************/
void* bidirectional_hash_map_2_t_remove_by_primary_key(
                                                bidirectional_hash_map_2_t* map,
                                                void* primary_key);

/****************************************************************************
* Removes a key pair by its secondary key.|                                 *
*-----------------------------------------+                                 *
* map --------- the map.                                                    *
* secondary_key - the primary key.                                          *
*-------------------------------------------------------------------------+ *
* RETURNS: NULL if the seconary key is not mapped. The current associated | *
* primary key otherwise.                                                  | *
****************************************************************************/
void* bidirectional_hash_map_2_t_remove_by_secondary_key(
                                                bidirectional_hash_map_2_t* map,
                                                void* secondary_key);

/******************************************************************************
* Queries the secondary key via its primary key.|                             *
*-----------------------------------------------+                             *
* map --------- the map to query.                                             *
* primary_key - the primary key to use.                                       *
*---------------------------------------------------------------------------+ *
* RETURNS: If the primary key is associated with a secondary key, that very | *
* secondary key is returned. Otherwise, NULL is returned.                   | *
******************************************************************************/
void* bidirectional_hash_map_2_t_get_by_primary_key(
                                                bidirectional_hash_map_2_t* map,
                                                void* primary_key);

/******************************************************************************
* Queries the primary key via its secondary key.|                             *
*-----------------------------------------------+                             *
* map --------- the map to query.                                             *
* secondary_key - the secondary key to use.                                   *
*---------------------------------------------------------------------------+ *
* RETURNS: If the secondary key is associated with a primary key, that very | *
* primary key is returned. Otherwise, NULL is returned.                     | *
******************************************************************************/
void* bidirectional_hash_map_2_t_get_by_secondary_key(
                                                bidirectional_hash_map_2_t* map,
                                                void* secondary_key);

/**************************************************************************
* Queries whether the map contains 'primary_key' as a primary key.|       *
*-----------------------------------------------------------------+       *
* map --- the map to query.                                               *
* primary_key - the primary key to query.                                 *
*-----------------------------------------------------------------------+ *
* RETURNS: If the primary key is in the map, returns 1. Otherwise, 0 is | *
* returned.                                                             | *
**************************************************************************/
int bidirectional_hash_map_2_t_contains_primary_key(
                                                bidirectional_hash_map_2_t* map,
                                                void* primary_key);

/****************************************************************************
* Queries whether the map contains 'secondary_key' as a secondary key.|     *
*---------------------------------------------------------------------+     *
* map ----------- the map to query.                                         *
* secondary_key - the primary key to query.                                 *
*-------------------------------------------------------------------------+ *
* RETURNS: If the secondary key is in the map, returns 1. Otherwise, 0 is | *
* returned.                                                               | *
****************************************************************************/
int bidirectional_hash_map_2_t_contains_secondary_key(
                                                bidirectional_hash_map_2_t* map,
                                                void* secondary_key);

/**************************************************************
* Initializes an iterator.|                                   *
*-------------------------+                                   *
* map ------ the map to iterate.                              *
* iterator - the iterator being initialized.                  *
*-----------------------------------------------------------+ *
* RETURNS: 1 if initialization is successfull. 0 otherwise. | *
**************************************************************/
int bidirectional_hash_map_2_iterator_t_init(
                                bidirectional_hash_map_2_t* map,
                                bidirectional_hash_map_2_iterator_t* iterator);

/********************************************************
* Queries whether there is more mappings to iterate.|   *
*---------------------------------------------------+   *
* iterator - the iterator to query.                     *
*-----------------------------------------------------+ *
* RETURNS: 1 if there is more to iterate. 0 otherwise | *
********************************************************/
int bidirectional_hash_map_2_iterator_t_has_next(
                                bidirectional_hash_map_2_iterator_t* iterator);

/*******************************************************************************
* Iterates over a mapping in the map.|                                         *
*------------------------------------+                                         *
* iterator ---------- the iterator.                                            *
* primary_key_ptr --- the pointer to the location where to store the primary   *
*                     key.                                                     *
* secondary_key_ptr - the pointer to the location where to store the secondary *
*                     key.                                                     *
*-----------------------------------------------------+                        *
* RETURNS: 1 if iteration was successful, 0 otherwise.|                        *
*******************************************************************************/
int bidirectional_hash_map_2_iterator_t_next(
                                bidirectional_hash_map_2_iterator_t* iterator,
                                void** primary_key_ptr,
                                void** secondary_key_ptr);


#endif /* BIDIRECTIONAL_HASH_MAP_2_H */
