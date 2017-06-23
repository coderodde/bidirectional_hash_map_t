#include "bidirectional_hash_map.h"
#include <stdlib.h>

static float max_float(float a, float b)
{
    return a > b ? a : b;
}

static size_t max_size_t(size_t a, size_t b)
{
    return a > b ? a : b;
}

static size_t to_power_of_two(size_t num)
{
    size_t ret = 1;
    
    while (ret < num)
    {
        ret >>= 1;
    }
    
    return  ret;
}

static const float  MINIMUM_LOAD_FACTOR      = 0.2;
static const size_t MINIMUM_INITIAL_CAPACITY = 8;

int bidirectional_hash_map_t_init(
                                bidirectional_hash_map_t* map,
                                size_t initial_capacity,
                                float load_factor,
                                size_t (*primary_key_hasher)  (void*),
                                size_t (*secondary_key_hasher)(void*),
                                int (*primary_key_equality)   (void*, void*),
                                int (*secondary_key_equality) (void*, void*),
                                void* error_sentinel)
{
    if (!map)
    {
        return 0;
    }
    
    if (!primary_key_hasher ||
        !secondary_key_hasher ||
        !primary_key_equality ||
        !secondary_key_equality)
    {
        return 0;
    }
    
    load_factor      = max_float(load_factor, MINIMUM_LOAD_FACTOR);
    initial_capacity = max_size_t(initial_capacity, MINIMUM_INITIAL_CAPACITY);
    initial_capacity = to_power_of_two(initial_capacity);
    
    map->capacity    = initial_capacity;
    map->load_factor = load_factor;
    map->size        = 0;
    
    map->primary_key_table = calloc(initial_capacity,
                                    sizeof(collision_chain_node_t*));
    
    if (!map->primary_key_table)
    {
        map->primary_key_table = NULL;
        return 0;
    }
    
    map->secondary_key_table = calloc(initial_capacity,
                                      sizeof(collision_chain_node_t*));
    
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
    map->error_sentinel         = error_sentinel;
    
    return 1;
}

static void remove_mapping(bidirectional_hash_map_t* map,
                           collision_chain_node_t* node)
{
    collision_chain_node_t* opposite_collision_chain_node;
    size_t secondary_key_hash = node->key_pair->secondary_key_hash;
    size_t secondary_key_bucket_index = secondary_key_hash & map->modulo_mask;
    size_t bucket_index;
    
    /* Find the opposite collision chain node pointing to the same key pair
       as 'node'. */
    for (opposite_collision_chain_node =
         map->secondary_key_table[secondary_key_bucket_index];
         ;
         opposite_collision_chain_node = opposite_collision_chain_node->next)
    {
        if (opposite_collision_chain_node->key_pair == node->key_pair)
        {
            break;
        }
    }
    
    /* Purge the key pair: */
    free(node->key_pair);
    
    /* Unlink and purge the primary collision chain node: */
    if (node->prev)
    {
        node->prev->next = node->next;
    }
    else
    {
        bucket_index = node->key_pair->primary_key_hash & map->modulo_mask;
        
        map->primary_key_table[bucket_index] =
        map->primary_key_table[bucket_index]->next;
    }
    
    if (node->next)
    {
        node->next->prev = node->prev;
    }
    
    free(node);
    
    /* Unlink and purge the secondary collision chain node: */
    if (opposite_collision_chain_node->prev)
    {
        opposite_collision_chain_node->prev->next =
        opposite_collision_chain_node->next;
    }
    else
    {
        map->secondary_key_table[secondary_key_bucket_index] =
        map->secondary_key_table[secondary_key_bucket_index]->next;
    }
    
    if (opposite_collision_chain_node->next)
    {
        opposite_collision_chain_node->next->prev =
        opposite_collision_chain_node->prev;
    }
    
    free(opposite_collision_chain_node);
}

void bidirectional_hash_map_t_destroy(bidirectional_hash_map_t* map)
{
    collision_chain_node_t* collision_chain_node;
    collision_chain_node_t* collision_chain_node_next;
    
    if (!map)
    {
        return;
    }
    
    if (!map->primary_key_table)
    {
        /* The input map is invalid (failed to be constructed due to shortage
           of memory). */
        return;
    }
    
    collision_chain_node = map->first_collision_chain_node;
    
    while (collision_chain_node)
    {
        collision_chain_node_next = collision_chain_node->down;
        remove_mapping(map, collision_chain_node);
        collision_chain_node = collision_chain_node_next;
    }
    
    free(map->primary_key_table);
    free(map->secondary_key_table);
    
    map->primary_key_table   = NULL;
    map->secondary_key_table = NULL;
    map->first_collision_chain_node = NULL;
    map->last_collision_chain_node = NULL;
    map->capacity = 0;
    map->size = 0;
}

int bidirectional_hash_map_t_is_working(bidirectional_hash_map_t* map)
{
    return map->primary_key_table ? 1 : 0;
}

size_t bidirectional_hash_map_t_size(bidirectional_hash_map_t* map)
{
    return map->size;
}

size_t bidirectional_hash_map_t_capacity(bidirectional_hash_map_t* map)
{
    return map->capacity;
}

static void relink_to_new_tables(
                            bidirectional_hash_map_t* map,
                            collision_chain_node_t* node,
                            collision_chain_node_t** next_primary_hash_table,
                            collision_chain_node_t** next_secondary_hash_table)
{
    size_t bucket_index;
    size_t next_capacity;
    size_t next_modulo_mask;
    collision_chain_node_t* opposite_node;
    
    /* 'node' is a node of a primary table collision chain. */
    /* Unlink the node from its collision chain. */
    if (node->prev)
    {
        node->prev->next = node->next;
    }
    else
    {
        bucket_index = node->key_pair->primary_key_hash & map->modulo_mask;
        
        map->primary_key_table[bucket_index] =
        map->primary_key_table[bucket_index]->next;
    }
    
    if (node->next)
    {
        node->next->prev = node->prev;
    }
    
    /* Unlink the opposite node of 'node'. */
    bucket_index = node->key_pair->secondary_key_hash & map->modulo_mask;
    opposite_node = map->secondary_key_table[bucket_index];
    
    for (;; opposite_node = opposite_node->next)
    {
        if (opposite_node->key_pair == node->key_pair)
        {
            break;
        }
    }
    
    if (opposite_node->prev)
    {
        opposite_node->prev->next = opposite_node->next;
    }
    else
    {
        bucket_index =
        opposite_node->key_pair->secondary_key_hash & map->modulo_mask;
        
        map->secondary_key_table[bucket_index] =
        map->secondary_key_table[bucket_index]->next;
    }
    
    if (opposite_node->next)
    {
        opposite_node->next->prev = opposite_node->prev;
    }
    
    /* Once here, both 'node' and its opposite node are unlinked from their 
       collision chains. Next, link them into the new provided tables: */
    next_capacity = map->capacity >> 1;
    next_modulo_mask = next_capacity - 1;
    
    /* Link node to the next_primary_hash_table: */
    bucket_index = node->key_pair->primary_key_hash & next_modulo_mask;
    node->prev = NULL;
    node->next = next_primary_hash_table[bucket_index];
    next_primary_hash_table[bucket_index] = node;
    
    /* Link opposite node to the next_secondary_hash_table: */
    bucket_index =
    opposite_node->key_pair->secondary_key_hash & next_modulo_mask;
    opposite_node->prev = NULL;
    opposite_node->next = next_secondary_hash_table[bucket_index];
    next_secondary_hash_table[bucket_index] = opposite_node;
}

static int expand_hash_map(bidirectional_hash_map_t* map)
{
    size_t next_capacity;
    size_t next_modulo_mask;
    collision_chain_node_t** next_primary_hash_table;
    collision_chain_node_t** next_secondary_hash_table;
    collision_chain_node_t* node;
    collision_chain_node_t* next_node;
    
    next_capacity = map->capacity >> 1;
    
    next_primary_hash_table = calloc(next_capacity,
                                     sizeof(collision_chain_node_t*));
    
    if (!next_primary_hash_table)
    {
        return 0;
    }
    
    next_secondary_hash_table = calloc(next_capacity,
                                       sizeof(collision_chain_node_t*));
    
    if (!next_secondary_hash_table)
    {
        free(next_primary_hash_table);
        return 0;
    }
    
    next_modulo_mask = next_capacity - 1;
    node = map->first_collision_chain_node;
    
    while (node)
    {
        next_node = node->down;
        relink_to_new_tables(map,
                             node,
                             next_primary_hash_table,
                             next_secondary_hash_table);
        node = next_node;
    }
    
    free(map->primary_key_table);
    free(map->secondary_key_table);
    
    map->primary_key_table = next_primary_hash_table;
    map->secondary_key_table = next_secondary_hash_table;
    map->capacity = next_capacity;
    map->modulo_mask = next_modulo_mask;

    return 1;
}

static void update_primary_key(
                    bidirectional_hash_map_t* map,
                    collision_chain_node_t* secondary_collision_chain_node,
                    void* new_primary_key)
{
    size_t primary_key_hash =
    secondary_collision_chain_node->key_pair->primary_key_hash;
    
    size_t primary_key_bucket_index = primary_key_hash & map->modulo_mask;
    size_t new_primary_key_bucket_index;
    size_t new_primary_key_hash;
    
    collision_chain_node_t* primary_collision_chain_node =
    map->primary_key_table[primary_key_bucket_index];
    
    /* Find the primary collision chain node: */
    for (;
         ;
         primary_collision_chain_node = primary_collision_chain_node->next)
    {
        if (primary_collision_chain_node->key_pair ==
            secondary_collision_chain_node->key_pair)
        {
            break;
        }
    }
    
    /* Unlink the primary collision chain node: */
    if (primary_collision_chain_node->prev)
    {
        primary_collision_chain_node->prev->next =
        primary_collision_chain_node->next;
    }
    else
    {
        map->primary_key_table[primary_key_bucket_index] =
        map->primary_key_table[primary_key_bucket_index]->next;
    }
    
    if (primary_collision_chain_node->next)
    {
        primary_collision_chain_node->next->prev =
        primary_collision_chain_node->prev;
    }
    
    /* Link the unlinked collision chain node into its new collision chain and
       update its primary key and its hash value: */
    new_primary_key_hash = map->primary_key_hasher(new_primary_key);
    new_primary_key_bucket_index = new_primary_key_hash & map->modulo_mask;
    
    primary_collision_chain_node->key_pair->primary_key = new_primary_key;
    primary_collision_chain_node->key_pair->primary_key_hash =
    new_primary_key_hash;
    
    primary_collision_chain_node->prev = NULL;
    primary_collision_chain_node->next =
    map->primary_key_table[new_primary_key_bucket_index];
    
    map->primary_key_table[new_primary_key_bucket_index] =
    primary_collision_chain_node;
}

static void update_secondary_key(
                        bidirectional_hash_map_t* map,
                        collision_chain_node_t* primary_collision_chain_node,
                        void* new_secondary_key)
{
    size_t secondary_key_hash =
    primary_collision_chain_node->key_pair->secondary_key_hash;
    
    size_t secondary_key_bucket_index = secondary_key_hash & map->modulo_mask;
    size_t new_secondary_key_bucket_index;
    size_t new_secondary_key_hash;
    
    collision_chain_node_t* secondary_collision_chain_node =
    map->secondary_key_table[secondary_key_bucket_index];
    
    /* Find the secondary collision chain node: */
    for (;
         ;
         secondary_collision_chain_node = secondary_collision_chain_node->next)
    {
        if (secondary_collision_chain_node->key_pair ==
            primary_collision_chain_node->key_pair)
        {
            break;
        }
    }
    
    /* Unlink the secondary collision chain node: */
    if (secondary_collision_chain_node->prev)
    {
        secondary_collision_chain_node->prev->next =
        secondary_collision_chain_node->next;
    }
    else
    {
        map->secondary_key_table[secondary_key_bucket_index] =
        map->secondary_key_table[secondary_key_bucket_index]->next;
    }
    
    if (secondary_collision_chain_node->next)
    {
        secondary_collision_chain_node->next->prev =
        secondary_collision_chain_node->prev;
    }
    
    /* Link the unlinked secondary collision chain node into its new collision 
       chain and update its secondary key and its hash value: */
    new_secondary_key_hash = map->secondary_key_hasher(new_secondary_key);
    new_secondary_key_bucket_index = new_secondary_key_hash & map->modulo_mask;
    
    secondary_collision_chain_node->key_pair->secondary_key = new_secondary_key;
    secondary_collision_chain_node->key_pair->secondary_key_hash =
    new_secondary_key_hash;
    
    secondary_collision_chain_node->prev = NULL;
    secondary_collision_chain_node->next =
    map->secondary_key_table[new_secondary_key_bucket_index];
    
    map->secondary_key_table[new_secondary_key_bucket_index] =
    secondary_collision_chain_node;
}

static int add_new_mapping(bidirectional_hash_map_t* map,
                           void* primary_key,
                           void* secondary_key)
{
    key_pair_t* key_pair;
    collision_chain_node_t* primary_collision_chain_node;
    collision_chain_node_t* secondary_collision_chain_node;
    size_t primary_table_bucket_index;
    size_t secondary_table_bucket_index;
    
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
    
    /* Link 'primary_collision_chain_node' to its table: */
    primary_collision_chain_node->key_pair = key_pair;
    primary_collision_chain_node->prev = NULL;
    primary_table_bucket_index = key_pair->primary_key_hash & map->modulo_mask;
    primary_collision_chain_node->next =
    map->primary_key_table[primary_table_bucket_index];
    
    if (map->primary_key_table[primary_table_bucket_index])
    {
        map->primary_key_table[primary_table_bucket_index]->prev =
        primary_collision_chain_node;
    }
    
    map->primary_key_table[primary_table_bucket_index] =
    primary_collision_chain_node;
    
    /* Link 'secondary_collision_chain_node' to its table: */
    secondary_collision_chain_node->key_pair = key_pair;
    secondary_collision_chain_node->prev = NULL;
    secondary_table_bucket_index =
    key_pair->secondary_key_hash & map->modulo_mask;
    
    secondary_collision_chain_node->next =
    map->secondary_key_table[secondary_table_bucket_index];
    
    if (map->secondary_key_table[secondary_table_bucket_index])
    {
        map->secondary_key_table[secondary_table_bucket_index]->prev =
        secondary_collision_chain_node;
    }
    
    map->secondary_key_table[secondary_table_bucket_index] =
    secondary_collision_chain_node;
    
    if (map->size == 0)
    {
        primary_collision_chain_node->up = NULL;
        primary_collision_chain_node->down = NULL;
        map->first_collision_chain_node = primary_collision_chain_node;
        map->last_collision_chain_node = primary_collision_chain_node;
    }
    else
    {
        primary_collision_chain_node->up = map->last_collision_chain_node;
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
    size_t primary_key_chain_index;
    collision_chain_node_t* collision_chain_node;
    void* return_value;
    
    primary_key_hash = map->primary_key_hasher(primary_key);
    primary_key_chain_index = primary_key_hash & map->modulo_mask;
    collision_chain_node = map->primary_key_table[primary_key_chain_index];
    
    for (;
         collision_chain_node;
         collision_chain_node = collision_chain_node->next)
    {
        if (collision_chain_node->key_pair->primary_key_hash
            == primary_key_hash)
        {
            if (map->primary_key_equality(
                                primary_key,
                                collision_chain_node->key_pair->primary_key))
            {
                return_value = collision_chain_node->key_pair->secondary_key;
                update_secondary_key(map, collision_chain_node, secondary_key);
                return return_value;
            }
        }
    }
    
    if (add_new_mapping(map, primary_key, secondary_key))
    {
        return NULL;
    }
    else
    {
        return map->error_sentinel;
    }
}

void* bidirectional_hash_map_t_put_by_secondary(bidirectional_hash_map_t* map,
                                                void* primary_key,
                                                void* secondary_key)
{
    size_t secondary_key_hash;
    size_t secondary_key_chain_index;
    collision_chain_node_t* collision_chain_node;
    void* return_value;
    
    secondary_key_hash = map->secondary_key_hasher(secondary_key);
    secondary_key_chain_index = secondary_key_hash & map->modulo_mask;
    collision_chain_node = map->secondary_key_table[secondary_key_chain_index];
    
    for (;
         collision_chain_node;
         collision_chain_node = collision_chain_node->next)
    {
        if (collision_chain_node->key_pair->secondary_key_hash
            == secondary_key_hash)
        {
            if (map->secondary_key_equality(
                                secondary_key,
                                collision_chain_node->key_pair->secondary_key))
            {
                return_value = collision_chain_node->key_pair->primary_key;
                update_primary_key(map, collision_chain_node, primary_key);
                return return_value;
            }
        }
    }
    
    if (add_new_mapping(map, primary_key, secondary_key))
    {
        return NULL;
    }
    else
    {
        return map->error_sentinel;
    }
}

void* bidiretional_hash_map_t_remove_by_primary_key(
                                                bidirectional_hash_map_t* map,
                                                void* primary_key);

void* bidiretional_hash_map_t_remove_by_secondary_key(
                                                bidirectional_hash_map_t* map,
                                                void* secondary_key);

void* bidirectional_hash_map_t_get_by_primary_key(bidirectional_hash_map_t* map,
                                                  void* primary_key)
{
    size_t primary_key_hash = map->primary_key_hasher(primary_key);
    size_t collision_chain_bucket_index = primary_key_hash & map->modulo_mask;
    collision_chain_node_t* node =
        map->primary_key_table[collision_chain_bucket_index];
    
    for (; node; node = node->next)
    {
        if (node->key_pair->primary_key_hash == primary_key_hash)
        {
            if (map->primary_key_equality(node->key_pair->primary_key,
                                          primary_key))
            {
                return node->key_pair->secondary_key;
            }
        }
    }
    
    return NULL;
}

void* bidirectional_hash_map_t_get_by_secondary_key(
                                                bidirectional_hash_map_t* map,
                                                void* secondary_key)
{
    size_t secondary_key_hash = map->secondary_key_hasher(secondary_key);
    size_t collision_chain_bucket_index = secondary_key_hash & map->modulo_mask;
    collision_chain_node_t* node =
        map->secondary_key_table[collision_chain_bucket_index];
    
    for (; node; node = node->next)
    {
        if (node->key_pair->secondary_key_hash == secondary_key_hash)
        {
            if (map->secondary_key_equality(node->key_pair->secondary_key,
                                            secondary_key))
            {
                return node->key_pair->primary_key;
            }
        }
    }
    
    return NULL;
}


