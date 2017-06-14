#include "bidirectional_hash_map.h"
#include <stdlib.h>

static float maxfloat(float a, float b)
{
    return a > b ? a : b;
}

static size_t maxsize_t(size_t a, size_t b)
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

const float MINIMUM_LOAD_FACTOR = 0.2;
const size_t MINIMUM_INITIAL_CAPACITY = 8;

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
    
    load_factor = maxfloat(load_factor, MINIMUM_LOAD_FACTOR);
    initial_capacity = maxsize_t(initial_capacity, MINIMUM_INITIAL_CAPACITY);
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

void bidirectional_hash_map_t_destroy(bidirectional_hash_map_t* map)
{
    collision_chain_node_t* collision_chain_node;
    collision_chain_node_t* collision_chain_node_next;
    size_t i;
    
    if (!map)
    {
        return;
    }
    
    if (!map->primary_key_table)
    {
        /* The input map is invalid (failed to be constructed due to shortage
           of memory. */
        return;
    }
    
    /* Free the key pairs and the primary collision chains: */
    for (i = 0; i < map->capacity; ++i)
    {
        collision_chain_node = map->primary_key_table[i];
        
        while (collision_chain_node)
        {
            collision_chain_node_next = collision_chain_node->next;
            free(collision_chain_node->key_pair);
            free(collision_chain_node);
            collision_chain_node = collision_chain_node_next;
        }
    }
    
    /* Free the secondary collision chains: */
    for (i = 0; i < map->capacity; ++i)
    {
        collision_chain_node = map->secondary_key_table[i];
        
        while (collision_chain_node)
        {
            collision_chain_node_next = collision_chain_node->next;
            free(collision_chain_node);
            collision_chain_node = collision_chain_node_next;
        }
    }
    
    free(map->primary_key_table);
    free(map->secondary_key_table);
}

int bidirectional_hash_map_t_is_working(bidirectional_hash_map_t* map)
{
    if (map->primary_key_table)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

size_t bidirectional_hash_map_t_size(bidirectional_hash_map_t* map)
{
    return map->size;
}

size_t bidirectional_hash_map_t_capacity(bidirectional_hash_map_t* map)
{
    return map->capacity;
}

static void unlink_collision_chain_node(bidirectional_hash_map_t* map,
                                        collision_chain_node_t* node)
{
    map->first_collision_chain_node = node->down;
    
    if (node->prev)
    {
        node->prev->next = node->next;
    }
    else
    {
        
    }
    
    if (node->next)
    {
        node->next->prev = node->prev;
    }
    else
    {
        
    }
}

static int link_to_new_hash_tables(
                                bidirectional_hash_map_t* map,
                                collision_chain_node_t* node,
                                collision_chain_node_t** primary_hash_table,
                                collision_chain_node_t** secondary_hash_table)
{
    
    return 1;
}

static void purge_tables(collision_chain_node_t** primary_hash_table,
                         collision_chain_node_t** secondary_hash_table,
                         size_t hash_table_length)
{
    collision_chain_node_t* node;
    collision_chain_node_t* next_node;
    
    size_t i;
    
    for (i = 0; i < hash_table_length; ++i)
    {
        if ((node = primary_hash_table[i]))
        {
            while (node)
            {
                next_node = node->next;
                //free(node->key_pair)
                node = next_node;
            }
        }
    }
}

static int expand_hash_map(bidirectional_hash_map_t* map)
{
    size_t next_capacity;
    size_t next_modulo_mask;
    collision_chain_node_t** next_primary_hash_table;
    collision_chain_node_t** next_secondary_hash_table;
    collision_chain_node_t* node;
    collision_chain_node_t* next_node;
    collision_chain_node_t* n;
    
    next_capacity = map->capacity >> 1;
    next_modulo_mask = next_capacity - 1;
    
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
    
    node = map->first_collision_chain_node;
    
    while (node)
    {
        next_node = node->down;
        unlink_collision_chain_node(map, node);
        
        if (!link_to_new_hash_tables(map,
                                     node,
                                     next_primary_hash_table,
                                     next_secondary_hash_table))
        {
            //purge_tables(next_primary_hash_table, next_secondary_hash_table);
            return 0;
        }
        
        node = next_node;
    }
    
    return 1;
}

static void update_primary_key(bidirectional_hash_map_t* map,
                               void* new_primary_key,
                               size_t primary_key_hash,
                               collision_chain_node_t* collision_chain_node)
{
    size_t current_primary_key_chain_index =
        primary_key_hash & map->modulo_mask;
    size_t new_primary_key_chain_index;
    
    /* Unlink collision_chain_node from its current collision chain: */
    if (collision_chain_node->prev)
    {
        collision_chain_node->prev->next = collision_chain_node->next;
    }
    else
    {
        map->primary_key_table[current_primary_key_chain_index] =
        map->primary_key_table[current_primary_key_chain_index]->next;
    }
    
    if (collision_chain_node->next)
    {
        collision_chain_node->next->prev = collision_chain_node->prev;
    }
    
    /* Link collision_chain_node to a new chain: */
    new_primary_key_chain_index =
    map->primary_key_hasher(new_primary_key) & map->modulo_mask;
    
    collision_chain_node->prev = NULL;
    collision_chain_node->next = map->primary_key_table
                                [new_primary_key_chain_index];
    map->primary_key_table[new_primary_key_chain_index] = collision_chain_node;
}

static void update_secondary_key(bidirectional_hash_map_t* map,
                                 void* new_secondary_key,
                                 size_t secondary_key_hash,
                                 collision_chain_node_t* collision_chain_node)
{
    size_t current_secondary_key_chain_index =
        secondary_key_hash & map->modulo_mask;
    size_t new_secondary_key_chain_index;
    
    /* Unlink collision_chain_node from its current collision chain: */
    if (collision_chain_node->prev)
    {
        collision_chain_node->prev->next = collision_chain_node->next;
    }
    else
    {
        map->secondary_key_table[current_secondary_key_chain_index] =
        map->secondary_key_table[current_secondary_key_chain_index]->next;
    }
    
    if (collision_chain_node->next)
    {
        collision_chain_node->next->prev = collision_chain_node->prev;
    }
    
    /* Link collision_chain_node to a new chain: */
    new_secondary_key_chain_index =
    map->secondary_key_hasher(new_secondary_key) & map->modulo_mask;
    
    collision_chain_node->prev = NULL;
    collision_chain_node->next = map->secondary_key_table
                                [new_secondary_key_chain_index];
    map->secondary_key_table[new_secondary_key_chain_index] =
        collision_chain_node;
}

void* bidirectional_hash_map_t_put_by_primary(bidirectional_hash_map_t* map,
                                              void* primary_key,
                                              void* secondary_key)
{
    size_t primary_key_hash;
    size_t primary_key_chain_index;
    size_t new_secondary_key_hash;
    size_t new_primary_table_index;
    size_t new_secondary_table_index;
    collision_chain_node_t* collision_chain_node;
    collision_chain_node_t* primary_collision_chain_node;
    collision_chain_node_t* secondary_collision_chain_node;
    key_pair_t* key_pair;
    
    if (map->size > map->load_factor * map->capacity)
    {
        if (!expand_hash_map(map))
        {
            return map->error_sentinel;
        }
    }
    
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
                
                /* A mapping found; update the secondary key. */
                update_secondary_key(map,
                                     secondary_key,
                                     new_secondary_key_hash,
                                     collision_chain_node);
                
                return collision_chain_node->key_pair->secondary_key;
            }
        }
    }
    
    /* If we got here, primary_key is not mapped to any value; add the mapping
     */
    key_pair = malloc(sizeof(*key_pair));
    
    if (!key_pair)
    {
        return map->error_sentinel;
    }
    
    primary_collision_chain_node =
        malloc(sizeof(*primary_collision_chain_node));
    
    if (!primary_collision_chain_node)
    {
        free(key_pair);
        return map->error_sentinel;
    }
    
    secondary_collision_chain_node =
        malloc(sizeof(*secondary_collision_chain_node));
    
    if (!secondary_collision_chain_node)
    {
        free(key_pair);
        free(primary_collision_chain_node);
        return map->error_sentinel;
    }
    
    key_pair->primary_key = primary_key;
    key_pair->secondary_key = secondary_key;
    key_pair->primary_key_hash = map->primary_key_hasher(primary_key);
    key_pair->secondary_key_hash = map->secondary_key_hasher(secondary_key);
    
    primary_collision_chain_node->key_pair = key_pair;
    primary_collision_chain_node->prev = NULL;
    
    new_primary_table_index = key_pair->primary_key_hash & map->modulo_mask;
    
    primary_collision_chain_node->next =
        map->primary_key_table[new_primary_table_index];
    
    if (map->primary_key_table[new_primary_table_index])
    {
        map->primary_key_table[new_primary_table_index]->prev =
        primary_collision_chain_node;
    }
        
    map->primary_key_table[new_primary_table_index] =
        primary_collision_chain_node;
    
    secondary_collision_chain_node->key_pair = key_pair;
    secondary_collision_chain_node->prev = NULL;
    
    new_secondary_table_index = key_pair->secondary_key_hash & map->modulo_mask;
    
    secondary_collision_chain_node->next =
        map->secondary_key_table[new_secondary_table_index];
    
    if (map->secondary_key_table[new_secondary_table_index])
    {
        map->secondary_key_table[new_secondary_table_index]->prev =
        secondary_collision_chain_node;
    }
    
    map->secondary_key_table[new_secondary_table_index] =
        secondary_collision_chain_node;
    
    if (map->size == 0)
    {
        map->first_collision_chain_node = primary_collision_chain_node;
    }
    
    map->size++;
    map->last_collision_chain_node = primary_collision_chain_node;
    
    return NULL;
}

void* bidirectional_hash_map_t_put_by_secondary(bidirectional_hash_map_t* map,
                                                void* primary_key,
                                                void* secondary_key);

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
    
    if (!node)
    {
        return NULL;
    }
    
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
    
    if (!node)
    {
        return NULL;
    }
    
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


