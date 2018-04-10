#ifndef C_HASH_MAP_H
#define C_HASH_MAP_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <memory.h>
#include <stddef.h>

/* Слот - это односвязный список.
 *
 * Структура узла слота:
 *
 * Что за данные:                   |__next___|___hash___|_________key_________|_________data_________|
 * Представление:                   |__void*__|__size_t__|__uint8_t[key_size]__|__uint8_t[data_size]__|
 *
 * Указатель на узел указывает сюда ^
 */

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

ptrdiff_t c_hash_map_delete(c_hash_map *const _hash_map,
                            void (*const _del_key_func)(void *const _key),
                            void (*const _del_data_func)(void *const _data));

ptrdiff_t c_hash_map_insert(c_hash_map *const _hash_map,
                            const void *const _key,
                            const void *const _data);

ptrdiff_t c_hash_map_erase(c_hash_map *const _hash_map,
                           const void *const _key,
                           void (*const _del_key_func)(void *const _key),
                           void (*const _del_data_func)(void *const _data));

ptrdiff_t c_hash_map_resize(c_hash_map *const _hash_map,
                            const size_t _slots_count);

ptrdiff_t c_hash_map_check(const c_hash_map *const _hash_map,
                           const void *const _key);

const void *c_hash_map_at(const c_hash_map *const _hash_map,
                          const void *const _key);

ptrdiff_t c_hash_map_for_each(const c_hash_map *const _hash_map,
                              void (*const _key_func)(const void *const _key),
                              void (*const _data_func)(const void *const _data));

ptrdiff_t c_hash_map_clear(c_hash_map *const _hash_map,
                           void (*const _del_key_func)(void *const _key),
                           void (*const _del_data_func)(void *const _data));

#endif
