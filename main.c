#include "bidirectional_hash_map.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define ASSERT(CONDITION) assert(CONDITION, #CONDITION, __FILE__, __LINE__);

void assert(int condition,
            const char* const condition_string,
            const char* const file_name,
            int line_number)
{
    if (!condition)
    {
        fprintf(stderr,
                "Condition \"%s\" failed in file \"%s\", line %d.\n",
                condition_string,
                file_name,
                line_number);
    }
}

size_t primary_key_hasher(void* key)
{
    return (size_t) key;
}

size_t secondary_key_hasher(void* key)
{
    return primary_key_hasher(key);
}

int primary_key_equality(void* a, void* b)
{
    return (uintptr_t) a == (uintptr_t) b;
}

int secondary_key_equality(void* a, void* b)
{
    return primary_key_equality(a, b);
}

int main()
{
    int i ;
    void* ret;
    void* error_sentinel = malloc(1);
    void* primary_key;
    void* secondary_key;
    bidirectional_hash_map_t map;
    bidirectional_hash_map_iterator_t iterator;
    
    bidirectional_hash_map_t_init(&map,
                                  0,
                                  1.0f,
                                  primary_key_hasher,
                                  secondary_key_hasher,
                                  primary_key_equality,
                                  secondary_key_equality,
                                  error_sentinel);
    
    for (i = 0; i < 32; ++i)
    {
        ASSERT(bidirectional_hash_map_t_size(&map) == i);
        bidirectional_hash_map_t_put_by_primary(&map, (void*) i, (void*)(32 + i));
        ASSERT(bidirectional_hash_map_t_size(&map) == i + 1);
    }
    
    for (i = 0; i < 32; ++i)
    {
        ASSERT(bidirectional_hash_map_t_size(&map) == 32 + i);
        bidirectional_hash_map_t_put_by_secondary(&map, (void*)(32 + i), (void*) i);
        ASSERT(bidirectional_hash_map_t_size(&map) == 33 + i);
    }
    
    for (i = 0; i < 32; ++i)
    {
        ret = bidirectional_hash_map_t_get_by_primary_key(&map, (void*) i);
        ASSERT((int) ret == i + 32);
    }
    
    for (i = 0; i < 32; ++i)
    {
        ret = bidirectional_hash_map_t_get_by_secondary_key(&map, (void*) i);
        ASSERT((int) ret == i + 32);
        
    }
    
    for (i = 0; i < 32; ++i)
    {
        ASSERT(bidirectional_hash_map_t_contains_primary_key(&map, (void*) i));
        ASSERT(bidirectional_hash_map_t_contains_primary_key(&map, (void*)(i + 32)));
        ASSERT(bidirectional_hash_map_t_contains_secondary_key(&map, (void*) i));
        ASSERT(bidirectional_hash_map_t_contains_secondary_key(&map, (void*)(i + 32)));
    }
    
    bidirectional_hash_map_t_destroy(&map);
    
    bidirectional_hash_map_t_init(&map,
                                  0,
                                  1.0f,
                                  primary_key_hasher,
                                  secondary_key_hasher,
                                  primary_key_equality,
                                  secondary_key_equality,
                                  error_sentinel);
    
    for (i = 0; i < 10; ++i)
    {
        ASSERT(bidirectional_hash_map_t_put_by_primary(&map, (void*) i, (void*)(i + 1000))
               == NULL);
    }
    
    ASSERT(bidirectional_hash_map_t_remove_by_primary_key(&map, (void*) 1) == (void*) 1001);
    ASSERT(bidirectional_hash_map_t_remove_by_secondary_key(&map, (void*) 1002) == (void*) 2);
    
    bidirectional_hash_map_iterator_t_init(&map, &iterator);
    
    for (i = 0; i < 8; ++i)
    {
        ASSERT(bidirectional_hash_map_iterator_t_has_next(&iterator));
        bidirectional_hash_map_iterator_t_next(&iterator,
                                               &primary_key,
                                               &secondary_key);
        
        ASSERT((int) primary_key + 1000 == (int) secondary_key);
    }
    
    ASSERT(bidirectional_hash_map_iterator_t_has_next(&iterator));
    
    puts("Tests done.");
    return 0;
}
