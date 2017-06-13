#include "bidirectional_hash_map.h"

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
    size_t initial_table_capacity;
    
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
    
    map->modulo_mask            = to_power_of_two(map->capacity) - 1;
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
            purge_tables(next_primary_hash_table, next_secondary_hash_table);
            return 0;
        }
        
        node = next_node;
    }
    
    return 1;
}

void* bidirectional_hash_map_t_put_by_primary(bidirectional_hash_map_t* map,
                                              void* primary_key,
                                              void* secondary_key)
{
    collision_chain_node_t* collision_chain_node;
    
    if (map->size > map->load_factor * map->capacity)
    {
        if (!expand_hash_map(map))
        {
            return NULL;
        }
    }
    
    size_t primary_key_hash = map->primary_key_hasher(primary_key);
    size_t primary_key_chain_index = primary_key_hash & map->modulo_mask;
    
    collision_chain_node = map->primary_key_table[primary_key_chain_index];
    
    while (collision_chain_node)
    {
        
    }
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
                                                  void* primary_key);

void* bidirectional_hash_map_t_get_by_secondary_key(
                                                    bidirectional_hash_map_t* map,
                                                    void* secondary_key);


