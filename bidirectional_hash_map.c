#include "bidirectional_hash_map.h"

static float maxfloat(float a, float b)
{
    return a > b ? a : b;
}

static size_t maxsize_t(size_t a, size_t b)
{
    return a > b ? a : b;
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
                                int (*secondary_key_equality)  (void*, void*))
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
    
    map->primary_key_hasher     = primary_key_hasher;
    map->secondary_key_hasher   = secondary_key_hasher;
    map->primary_key_equality   = primary_key_equality;
    map->secondary_key_equality = secondary_key_equality;
    
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

int bidirectional_hash_map_t_is_working(bidirectional_hash_map_t* map);

size_t bidirectional_hash_map_t_size(bidirectional_hash_map_t* map);

size_t bidirectional_hash_map_t_capacity(bidirectional_hash_map_t* map);

void* bidirectional_hash_map_t_put_by_primary(bidirectional_hash_map_t* map,
                                              void* primary_key,
                                              void* secondary_key);
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


