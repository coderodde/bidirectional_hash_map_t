#ifndef BIDIRECTIONAL_HASH_MAP_H
#define BIDIRECTIONAL_HASH_MAP_H

#include <stdlib.h>

typedef struct {
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

typedef struct {
    
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
    
    /******************************
    * The primary key hash table. *
    ******************************/
    struct key_pair** primary_key_table;
    
    /********************************
    * The secondary key hash table. *
    ********************************/
    struct key_pair** secondary_key_table;
    
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
****************************************************************************/
void bidirectional_hash_map_t_init(
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
********************************************************************/
int bidirectional_hash_map_t_is_working(bidirectional_hash_map_t* map);

/*****************************************************
* Returns the number of key pairs in the input map.| *
* map - the map to query.                            *
*****************************************************/
size_t bidirectional_hash_map_t_size(bidirectional_hash_map_t* map);

/*****************************************************************************
* Returns the capacity of one of the hash tables (another one has the same | *
* capacity).                                                               | *
*--------------------------------------------------------------------------+ *
* map - the map to query.                                                    *
*****************************************************************************/
size_t bidirectional_hash_map_t_capacity(bidirectional_hash_map_t* map);

/*********************************************************************
* Associates the primary key to the secondary key in the input map.| *
*------------------------------------------------------------------+ *
* map ----------- the map into which to store the pair.              *
* primary_key --- the primary key.                                   *
* secondary_key - the secondary key.                                 *
*********************************************************************/
void* bidirectional_hash_map_t_put(bidirectional_hash_map_t* map,
                                   void* primary_key,
                                   void* secondary_key);

/******************************************
* Removes a key pair by its primary key.| *
*---------------------------------------+ *
* map --------- the map.                  *
* primary_key - the primary key.          *
******************************************/
void* bidiretional_hash_map_t_remove_by_primary_key(
                                                    bidirectional_hash_map_t* map,
                                                    void* primary_key);
/*********************************************
* Removes a key pair by its secondary key.| *
*-----------------------------------------+ *
* map --------- the map.                    *
* secondary_key - the primary key.          *
********************************************/
void* bidiretional_hash_map_t_remove_by_secondary_key(
                                                    bidirectional_hash_map_t* map,
                                                    void* secondary_key);

/**************************************************
* Queries the secondary key via its primary key.| *
*-----------------------------------------------+ *
* map --------- the map to query.                 *
* primary_key - the primary key to use.           *
**************************************************/
void* bidirectional_hash_map_t_get_by_primary_key(bidirectional_hash_map_t* map,
                                                  void* primary_key);

/**************************************************
* Queries the primary key via its secondary key.| *
*-----------------------------------------------+ *
* map --------- the map to query.                 *
* secondary_key - the secondary key to use.       *
**************************************************/
void* bidirectional_hash_map_t_get_by_secondary_key(
                                                bidirectional_hash_map_t* map,
                                                void* secondary_key);


#endif /* BIDIRECTIONAL_HASH_MAP_H */
