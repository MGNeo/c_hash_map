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

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <memory.h>
#include <stddef.h>

// Количество слотов, задаваемое хэш-отображению с нулем слотов при автоматическом расширении.
#define C_HASH_MAP_0 ( (size_t) 1024 )

typedef struct s_c_hash_map_node
{
    struct s_c_hash_map_node *next_node;
    size_t hash;
    void *key,
         *data;
} c_hash_map_node;

typedef struct s_c_hash_map
{
    // Функция, генерирующая хэш на основе ключа.
    size_t (*hash_key)(const void *const _key);
    // Функция детального сравнения ключей.
    // В случае идентичности ключей должна возвращать > 0, иначе 0.
    size_t (*comp_key)(const void *const _key_a,
                       const void *const _key_b);

    size_t slots_count,
           nodes_count;

    float max_load_factor;

    c_hash_map_node **slots;
} c_hash_map;

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

ptrdiff_t c_hash_map_for_each(const c_hash_map *const _hash_map,
                              void (*const _action_key)(const void *const _key),
                              void (*const _action_data)(void *const _data));

ptrdiff_t c_hash_map_clear(c_hash_map *const _hash_map,
                           void (*const _del_key)(void *const _key),
                           void (*const _del_data)(void *const _data));

#endif
