#ifndef C_HASH_MAP_H
#define C_HASH_MAP_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <memory.h>
#include <stddef.h>

typedef struct s_c_hash_map
{
    // Функция, генерирующая хэш на основе ключа.
    size_t (*hash_func)(const void *const _key);
    // Функция детального сравнения ключей.
    // В случае идентичности ключей должна возвращать > 0, иначе 0.
    size_t (*comp_func)(const void *const _a,
                        const void *const _b);

    size_t key_size;
    size_t data_size;
    size_t slots_count;
    size_t nodes_count;

    float max_load_factor;

    void *slots;
} c_hash_map;

c_hash_map *c_hash_map_create(size_t (*const _hash_func)(const void *const _key),
                              size_t (*const _comp_func)(const void *const _a,
                                                         const void *const _b),
                              const size_t _key_size,
                              const size_t _data_size,
                              const size_t _slots_count,
                              const float _max_load_factor);

#endif
