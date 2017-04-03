#include "tvm.h"

int hash_function
    (char* key)
{
    return jit_strlen(key);//TODO CRC hash
}

tvm_map_t tvm_map_create
    (int initial_size)
{
    tvm_map_t map = jit_malloc(sizeof(struct _tvm_map));
    map->hashcodes = jit_malloc(sizeof(int) * initial_size);
    map->keys = jit_malloc(sizeof(char*) * initial_size);
    map->data = jit_malloc(sizeof(void*) * initial_size);
    map->count = 0;
    map->allocd = initial_size;
}

void tvm_map_free
    (tvm_map_t map)
{
    jit_free(map->hashcodes);
    jit_free(map->keys);
    jit_free(map->data);
    jit_free(map);
}

void tvm_map_add
    (tvm_map_t map, char* key, void* data)
{
    int hash = hash_function(key);
    int last = map->count;
    ++map->count;
    if(map->count > map->allocd)
    {
        map->allocd += map->allocd;
        map->hashcodes = jit_realloc(map->hashcodes, sizeof(int) * map->allocd);
        map->keys = jit_realloc(map->keys, sizeof(char*) * map->allocd);
        map->data = jit_realloc(map->data, sizeof(void*) * map->allocd);
    }
    map->hashcodes[last] = hash;
    map->keys[last] = key;
    map->data[last] = data;
}

void* tvm_map_get
    (tvm_map_t map, char* key)
{
    void* elem = NULL;
    int hash = hash_function(key);
    int i;
    for(i = 0; i < map->count; ++i)
    {
        if(map->hashcodes[i] == hash)
        {
            if(jit_strcmp(map->keys[i], key) == 0)
            {
                elem = map->data[i];
                break;
            }
        }
    }
    return elem;
}
