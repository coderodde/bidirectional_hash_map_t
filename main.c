#include "bidirectional_hash_map.h"
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
    return ((int) a) == ((int) b);
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
    bidirectional_hash_map_t map;
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
        bidirectional_hash_map_t_put_by_primary(&map, i, 32 + i);
        ASSERT(bidirectional_hash_map_t_size(&map) == i + 1);
    }
    
    
    for (i = 0; i < 32; ++i)
    {
        bidirectional_hash_map_t_put_by_secondary(&map, i, 32 + i);
    }
    
    puts("yo");
    for (i = 0; i < 32; ++i)
    {
        printf("%d\n", i);
        ret = bidirectional_hash_map_t_get_by_primary_key(&map, i);
        ASSERT((int) ret == i + 32);
    }
        
    return 0;
}
