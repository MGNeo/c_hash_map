/*
    Заголовочный файл хэш-отображения c_hash_map
    Разработка, отладка и сборка производилась в:
    ОС: Windows 10/x64
    IDE: Code::Blocks 17.12
    Компилятор: default Code::Blocks 17.12 MinGW

    Разработчик: Глухманюк Максим
    Эл. почта: mgneo@yandex.ru
    Место: Российская Федерация, Самарская область, Сызрань
    Дата: 11.04.2018
    Лицензия: GPLv3
*/

#ifndef C_HASH_MAP_H
#define C_HASH_MAP_H

#include <stddef.h>

typedef struct s_c_hash_map c_hash_map;

c_hash_map *c_hash_map_create(size_t (*const _hash_key)(const void *const _key),
                              size_t (*const _comp_key)(const void *const _key_a,
                                                        const void *const _key_b),
                              const size_t _slots_count,
                              const float _max_load_factor);

ptrdiff_t c_hash_map_delete(c_hash_map *const _hash_map,
                            void (*const _del_key)(void *const _key),
                            void (*const _del_data)(void *const _data));

ptrdiff_t c_hash_map_insert(c_hash_map *const _hash_map,
                            const void *const _key,
                            const void *const _data);

ptrdiff_t c_hash_map_erase(c_hash_map *const _hash_map,
                           const void *const _key,
                           void (*const _del_key)(void *const _key),
                           void (*const _del_data)(void *const _data));

ptrdiff_t c_hash_map_resize(c_hash_map *const _hash_map,
                            const size_t _slots_count);

ptrdiff_t c_hash_map_check(const c_hash_map *const _hash_map,
                           const void *const _key);

void *c_hash_map_at(const c_hash_map *const _hash_map,
                    const void *const _key);

ptrdiff_t c_hash_map_for_each(c_hash_map *const _hash_map,
                              void (*const _action_key)(const void *const _key),
                              void (*const _action_data)(void *const _data));

ptrdiff_t c_hash_map_clear(c_hash_map *const _hash_map,
                           void (*const _del_key)(void *const _key),
                           void (*const _del_data)(void *const _data));

size_t c_hash_map_slots_count(const c_hash_map *const _hash_map);

size_t c_hash_map_pairs_count(const c_hash_map *const _hash_map);

float c_hash_map_max_load_factor(const c_hash_map *const _hash_map);

#endif
