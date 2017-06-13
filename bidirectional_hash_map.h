#ifndef BIDIRECTIONAL_HASH_MAP_H
#define BIDIRECTIONAL_HASH_MAP_H

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
    
    struct key_pair_t* prev;
    struct key_pair_t* next;
}
key_pair_t;

typedef struct collision_chain_node_t {
    /*************************************************************************
    * Points to the previous collision chain node or is set to NULL if there *
    * is no previous collision chain node.                                   *
    *************************************************************************/
    struct collision_chain_node_t* prev;
    
    /***************************************************************************
    * Points to the next collision chain node or is set to NULL if there is no *
    * next collision chain node.                                               *
    ***************************************************************************/
    struct collision_chain_node_t* next;
    
    /*******************************************
    * Points to the actual key pair structure. *
    *******************************************/
    key_pair_t* key_pair;
}
collision_chain_node_t;

typedef struct bidirectional_hash_map_t {
    
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
    
    /**************************
    * The primary hash table. *
    **************************/
    struct collision_chain_node_t** primary_key_table;
    
    /****************************
    * The secondary hash table. *
    ****************************/
    struct collision_chain_node_t** secondary_key_table;
    
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
}
bidirectional_hash_map_t;

/****************************************************************************
* Builds a new, empty bidirectional hash map.|                              *
*--------------------------------------------+                              *
* map -------------------- the map to initialize.                           *
* initial_capacity ------- the initial capacity of both the hash tables.    *
* load_factor ------------ the load factor.                                 *
* primary_key_hasher ----- the function for producing primary key hashes.   *
* secondary_key_hasher --- the function for producing secondary key hashes. *
* primary_key_equality --- the function for comparing primary keys.         *
* secondary_key_equality - the function for comparing secondary keys.       *
*-----------------------------------------------------------+               *
* RETURNS: 1 if initialization was successfull, 0 otherwise.|               *
****************************************************************************/
int bidirectional_hash_map_t_init(
        bidirectional_hash_map_t* map,
        size_t initial_capacity,
        float load_factor,
        size_t (*primary_key_hasher)  (void*),
        size_t (*secondary_key_hasher)(void*),
        int (*primary_key_equality)   (void*, void*),
        int (*secondary_key_equality  (void*, void*)));

/************************************************
* Releases all the resources of the input map.| *
*---------------------------------------------+ *
* map - the map to destroy.                     *
************************************************/
void bidirectional_hash_map_t_destroy(bidirectional_hash_map_t* map);

/********************************************************************
* Checks that the map is well formed and is ready to receive data.| *
*-----------------------------------------------------------------+ *
* map - the map to check.                                           *
*------------------------------------------------+                  *
* RETURNS: 1 if the map is in order, 0 otherwise.|                  *
********************************************************************/
int bidirectional_hash_map_t_is_working(bidirectional_hash_map_t* map);

/*****************************************************
* Returns the number of key pairs in the input map.| *
*--------------------------------------------------+ *
* map - the map to query.                            *
*----------------------------------------------+     *
* RETURNS: the number of key pairs in this map.|     *
*****************************************************/
size_t bidirectional_hash_map_t_size(bidirectional_hash_map_t* map);

/*****************************************************************************
* Returns the capacity of one of the hash tables (another one has the same | *
* capacity).                                                               | *
*--------------------------------------------------------------------------+ *
* map - the map to query.                                                    *
*------------------------------------------+                                 *
* RETURNS: the capacity of each hash table.|                                 *
*****************************************************************************/
size_t bidirectional_hash_map_t_capacity(bidirectional_hash_map_t* map);

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
void* bidirectional_hash_map_t_put_by_primary(bidirectional_hash_map_t* map,
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
void* bidirectional_hash_map_t_put_by_secondary(bidirectional_hash_map_t* map,
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
void* bidiretional_hash_map_t_remove_by_primary_key(
        bidirectional_hash_map_t* map,
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
void* bidiretional_hash_map_t_remove_by_secondary_key(
        bidirectional_hash_map_t* map,
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
void* bidirectional_hash_map_t_get_by_primary_key(bidirectional_hash_map_t* map,
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
void* bidirectional_hash_map_t_get_by_secondary_key(
        bidirectional_hash_map_t* map,
        void* secondary_key);


#endif /* BIDIRECTIONAL_HASH_MAP_H */
