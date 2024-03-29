﻿/*
    Файл реализации хэш-отображения c_hash_map
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

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <memory.h>

#include "c_hash_map.h"

// Количество слотов, задаваемое хэш-отображению с нулем слотов при автоматическом расширении.
#define C_HASH_MAP_0 ( (size_t) 1024 )

// Минимально допустимое значение max_load_factor.
#define C_HASH_MAP_MLF_MIN ( (float) 0.01f )

// Минимально допустимое значение max_load_factor.
#define C_HASH_MAP_MLF_MAX ( (float) 1.f )

typedef struct s_c_hash_map_node c_hash_map_node;

struct s_c_hash_map_node
{
    struct s_c_hash_map_node *next_node;
    size_t hash;
    void *key,
         *data;
};

struct s_c_hash_map
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
};

// Если расположение задано, в него помещается код.
static void error_set(size_t *const _error,
                      const size_t _code)
{
    if (_error != NULL)
    {
        *_error = _code;
    }
}

// Создание пустого хэш-отображения.
// В случае ошибки возвращает NULL, и если _error != NULL, в заданное расположение помещается
// код причины ошибки (> 0).
// Позволяет создать хэш-отображение с нулем слотов.
c_hash_map *c_hash_map_create(size_t (*const _hash_key)(const void *const _key),
                              size_t (*const _comp_key)(const void *const _a_key,
                                                         const void *const _b_key),
                              const size_t _slots_count,
                              const float _max_load_factor,
                              size_t *const _error)
{
    if (_hash_key == NULL)
    {
        error_set(_error, 1);
        return NULL;
    }
    if (_comp_key == NULL)
    {
        error_set(_error, 2);
        return NULL;
    }
    if  ( (_max_load_factor < C_HASH_MAP_MLF_MIN) ||
          (_max_load_factor > C_HASH_MAP_MLF_MAX) )
    {
        error_set(_error, 3);
        return NULL;
    }

    c_hash_map_node **new_slots = NULL;

    if (_slots_count > 0)
    {
        // Определим размер новых слотов.
        const size_t new_slots_size = _slots_count * sizeof(c_hash_map_node*);
        if ( (new_slots_size == 0) ||
             (new_slots_size / _slots_count != sizeof(c_hash_map_node*)) )
        {
            error_set(_error, 4);
            return NULL;
        }

        // Попытаемся выделить память под новые слоты.
        new_slots = malloc(new_slots_size);
        if (new_slots == NULL)
        {
            error_set(_error, 5);
            return NULL;
        }
        // Обнулим слоты.
        memset(new_slots, 0, new_slots_size);
    }

    // Попытаемся создать хэш-отображение.
    c_hash_map *const new_hash_map = malloc(sizeof(c_hash_map));
    if (new_hash_map == NULL)
    {
        free(new_slots);
        error_set(_error, 6);
        return NULL;
    }

    new_hash_map->hash_key = _hash_key;
    new_hash_map->comp_key = _comp_key;

    new_hash_map->slots_count = _slots_count;
    new_hash_map->nodes_count = 0;

    new_hash_map->max_load_factor = _max_load_factor;

    new_hash_map->slots = new_slots;

    return new_hash_map;
}

// Удаляет хэш-отображение.
// В случае успеха возвращает > 0.
// В случае ошибки возвращает < 0.
ptrdiff_t c_hash_map_delete(c_hash_map *const _hash_map,
                            void (*const _del_key)(void *const _key),
                            void (*const _del_data)(void *const _data))
{
    if (c_hash_map_clear(_hash_map, _del_key, _del_data) < 0)
    {
        return -1;
    }
    free(_hash_map->slots);

    free(_hash_map);

    return 1;
}

// Вставка данных в хэш-отображение.
// В случае успешной вставки возвращает > 0, ключ и данные захватываются хэш-отображением.
// Если данные с указанным ключом уже есть в хэш-отображении, функция возвращает 0,
// ключ и данные не захватываются хэш-отображением.
// В случае ошибки возвращает < 0, ключ и данные не захватываются хэш-отображением.
ptrdiff_t c_hash_map_insert(c_hash_map *const _hash_map,
                            const void *const _key,
                            const void *const _data)
{
    if (_hash_map == NULL) return -1;
    if (_key == NULL) return -2;
    if (_data == NULL) return -3;

    // Проверим, имеются ли в хэш-отображении данные с заданным ключом.
    ptrdiff_t r_code = c_hash_map_check(_hash_map, _key);

    // Ошибка.
    if (r_code < 0) return -4;

    // Данные уже имеются.
    if (r_code > 0) return 0;

    // Начинаем вставлять.

    // Если слотов нет вообще.
    if (_hash_map->slots_count == 0)
    {
        // Попытаемся расширить слоты.
        if (c_hash_map_resize(_hash_map, C_HASH_MAP_0) <= 0)
        {
            return -5;
        }
    } else {
        // Если слоты есть, то при достижении предела загруженности увеличиваем количество слотов.
        const float load_factor = (float)_hash_map->nodes_count / _hash_map->slots_count;
        if (load_factor >= _hash_map->max_load_factor)
        {
            // Определим новое количество слотов.
            size_t new_slots_count = (size_t)(_hash_map->slots_count * 1.75f);
            if (new_slots_count < _hash_map->slots_count)
            {
                return -6;
            }
            new_slots_count += 1;
            if (new_slots_count == 0)
            {
                return -7;
            }

            // Попытаемся расширить слоты.
            if (c_hash_map_resize(_hash_map, new_slots_count) < 0)
            {
                return -8;
            }
        }
    }

    // Попытаемся выделить память под узел.
    c_hash_map_node *const new_node = malloc(sizeof(c_hash_map_node));
    if (new_node == NULL)
    {
        return -9;
    }

    // Неприведенный хэш ключа вставляемых данных.
    const size_t hash = _hash_map->hash_key(_key);

    // Приведенный хэш ключа вставляемых данных.
    const size_t presented_hash = hash % _hash_map->slots_count;

    // Заносим в узел непрвиеденный хэш ключа вставляемых данных.
    new_node->hash = hash;

    // Связываем узел с ключем.
    new_node->key = (void*)_key;

    // Связываем узел с данными.
    new_node->data = (void*)_data;

    // Добавляем узел в слот.
    new_node->next_node = _hash_map->slots[presented_hash];
    _hash_map->slots[presented_hash] = new_node;

    ++_hash_map->nodes_count;

    return 1;
}

// Удаление из хэш-отображения данных с заданным ключом.
// В случае успешного удаления возвращает > 0.
// В случае, если данные с заданным ключом отсутствуют, возвращает 0.
// В случае ошибки возвращает < 0.
ptrdiff_t c_hash_map_erase(c_hash_map *const _hash_map,
                           const void *const _key,
                           void (*const _del_key)(void *const _key),
                           void (*const _del_data)(void *const _data))
{
    if (_hash_map == NULL) return -1;
    if (_key == NULL) return -2;

    if (_hash_map->nodes_count == 0) return 0;

    // Вычислим неприведенный хэш ключа удаляемых данных.
    const size_t hash = _hash_map->hash_key(_key);

    // Вычислим приведенный хэш ключа удаляемых данных.
    const size_t presented_hash = hash % _hash_map->slots_count;

    // Если требуемый слот пуст, значит данных с таким ключом в хэш-отображении нет.
    if (_hash_map->slots[presented_hash] == NULL)
    {
        return 0;
    }

    // Просмотр слота на наличие данных с заданным ключом.
    c_hash_map_node *select_node = _hash_map->slots[presented_hash],
                    *prev_node = NULL;

    while (select_node != NULL)
    {
        if (hash == select_node->hash)
        {
            if (_hash_map->comp_key(_key, select_node->key) > 0)
            {
                // Удаляем данный узел.

                // Ампутация узла из слота.
                if (prev_node != NULL)
                {
                    prev_node->next_node = select_node->next_node;
                } else {
                    _hash_map->slots[presented_hash] = select_node->next_node;
                }

                // Если для ключа задана функция удаления, вызываем ее.
                if (_del_key != NULL)
                {
                    _del_key( select_node->key );
                }

                // Если для данных задана функция удаления, вызываем ее.
                if (_del_data != NULL)
                {
                    _del_data( select_node->data );
                }

                // Удаляем узел.
                free(select_node);

                --_hash_map->nodes_count;

                return 1;
            }
        }

        prev_node = select_node;
        select_node = select_node->next_node;
    }

    return 0;
}

// Задает хэш-отображению новое количество слотов.
// Позволяет расширить хэш-отображение с нулем слотов.
// Если в хэш-отображении есть хотя бы один элемент, то попытка задать нулевое количество слотов считается ошибкой.
// Если хэш-отображение перестраивается, функция возвращает > 0.
// Если хэш-отображение не перестраивается, функция возвращает 0.
// В случае ошибки возвращает < 0.
ptrdiff_t c_hash_map_resize(c_hash_map *const _hash_map,
                            const size_t _slots_count)
{
    if (_hash_map == NULL) return -1;

    if (_hash_map->slots_count == _slots_count) return 0;

    if (_slots_count == 0)
    {
        if (_hash_map->slots_count != 0)
        {
            return -2;
        }

        free(_hash_map->slots);
        _hash_map->slots = NULL;

        _hash_map->slots_count = 0;

        return 1;
    } else {
        // Определяем новый размер, необходимый под slots.
        const size_t new_slots_size = _slots_count * sizeof(c_hash_map_node*);
        if ( (new_slots_size == 0) ||
             (new_slots_size / _slots_count != sizeof(c_hash_map_node*)) )
        {
            return -3;
        }

        // Попытаемся выделить память под новые слоты.
        c_hash_map_node **const new_slots = malloc(new_slots_size);
        if (new_slots == NULL)
        {
            return -4;
        }

        // Обнулим новые слоты.
        memset(new_slots, 0, new_slots_size);

        // Если есть узлы, которые необходимо перенести из старых слотов в новые.
        if (_hash_map->nodes_count > 0)
        {
            size_t count = _hash_map->nodes_count;
            for (size_t s = 0; (s < _hash_map->slots_count)&&(count > 0); ++s)
            {
                if (_hash_map->slots[s] != NULL)
                {
                    c_hash_map_node *select_node = _hash_map->slots[s],
                                    *relocate_node;

                    while (select_node != NULL)
                    {
                        relocate_node = select_node;
                        select_node = select_node->next_node;

                        // Хэш ключа переносимого узла, приведенный к новому количеству слотов.
                        const size_t presented_hash = relocate_node->hash % _slots_count;

                        relocate_node->next_node = new_slots[presented_hash];
                        new_slots[presented_hash] = relocate_node;

                        --count;
                    }
                }
            }

        }

        free(_hash_map->slots);

        // Используем новые слоты.
        _hash_map->slots = new_slots;
        _hash_map->slots_count = _slots_count;

        return 2;
    }
}

// Проверка на наличие в хэш-отображении данных с заданным ключом.
// В случае наличия данных с заданным ключом возвращает > 0.
// В случае отсутствия данных с заданным ключом возвращает 0.
// В случае ошибки возвращает < 0.
ptrdiff_t c_hash_map_check(const c_hash_map *const _hash_map,
                           const void *const _key)
{
    if (_hash_map == NULL) return -1;
    if (_key == NULL) return -2;

    if (_hash_map->nodes_count == 0) return 0;

    // Неприведенный хэш искомого ключа.
    const size_t hash = _hash_map->hash_key(_key);

    // Приведенный хэш искомого ключа.
    const size_t presented_hash = hash % _hash_map->slots_count;

    c_hash_map_node *select_node = _hash_map->slots[presented_hash];

    while (select_node != NULL)
    {
        if (hash == select_node->hash)
        {
            if (_hash_map->comp_key(_key, select_node->key) > 0)
            {
                return 1;
            }
        }

        select_node = select_node->next_node;
    }

    return 0;
}

// Обращение к данным с заданным ключом.
// В случае успеха возвращает указатель на данные, которые связаны с заданным ключом.
// Если данных нет, функция возвращает NULL, это не считается ошибкой.
// В случае ошибки функция возвращает NULL, и если _error != NULL, то в заданное расположение помещается
// код причины ошибки (> 0).
// Так как функция может возвращать NULL и в случае успеха, и в случае ошибки, для детектирования ошибки
// перед вызовом функции необходимо поместить 0 в заданное расположение ошибки.
void *c_hash_map_at(const c_hash_map *const _hash_map,
                    const void *const _key,
                    size_t *const _error)
{
    if (_hash_map == NULL)
    {
        error_set(_error, 1);
        return NULL;
    }
    if (_key == NULL)
    {
        error_set(_error, 2);
        return NULL;
    }

    if (_hash_map->nodes_count == 0) return NULL;

    // Неприведенный хэш искомого ключа.
    const size_t hash = _hash_map->hash_key(_key);

    // Приведенный хэш искомого ключа.
    const size_t presented_hash = hash % _hash_map->slots_count;

    c_hash_map_node *select_node = _hash_map->slots[presented_hash];

    while (select_node != NULL)
    {
        if (hash == select_node->hash)
        {
            if (_hash_map->comp_key(_key, select_node->key) > 0)
            {
                return select_node->data;
            }
        }

        select_node = select_node->next_node;
    }

    return NULL;
}

// Проходит по всем элементам хэш-отображения и выполняет над ключами и данными заданные действия.
// Ключи нельзя удалять или менять.
// Данные нельзя удалять, но можно менять.
// Должно быть задано действие хотя бы для ключа или хотя бы для данных.
// В случае успешного выполнения возвращает > 0.
// В случае, если в хэш-отображении нет элементов, возвращает 0.
// В случае ошибки возвращает < 0.
ptrdiff_t c_hash_map_for_each(c_hash_map *const _hash_map,
                              void (*const _action_key)(const void *const _key),
                              void (*const _action_data)(void *const _data))
{
    if (_hash_map == NULL) return -1;
    if ( (_action_key == NULL) &&
         (_action_data == NULL) )
    {
        return -2;
    }

    if (_hash_map->nodes_count == 0) return 0;

    size_t count = _hash_map->nodes_count;

    // Макросы дублирования кода для избавления от проверок внутри циклов.

    // Открытие циклов.
    #define C_HASH_MAP_FOR_EACH_BEGIN\
    for (size_t s = 0; (s < _hash_map->slots_count)&&(count > 0); ++s) \
    {\
        if (_hash_map->slots[s] != NULL)\
        {\
            c_hash_map_node *select_node = _hash_map->slots[s];\
            while (select_node != NULL)\
            {

    // Закрытие циклов.
    #define C_HASH_MAP_FOR_EACH_END\
                select_node = select_node->next_node;\
                --count;\
            }\
        }\
    }

    // Заданы действия и для ключей, и для данных.
    if ( (_action_key != NULL) && (_action_data != NULL) )
    {
        C_HASH_MAP_FOR_EACH_BEGIN

        _action_key( select_node->key );
        _action_data( select_node->data );

        C_HASH_MAP_FOR_EACH_END
    } else {
        // Задано действие только для ключей.
        if (_action_key != NULL)
        {
            C_HASH_MAP_FOR_EACH_BEGIN

            _action_key( select_node->key );

            C_HASH_MAP_FOR_EACH_END
        } else {
            // Задано действие только для данных.

            C_HASH_MAP_FOR_EACH_BEGIN

            _action_data( select_node->data );

            C_HASH_MAP_FOR_EACH_END
        }
    }

    #undef C_HASH_MAP_FOR_EACH_BEGIN
    #undef C_HASH_MAP_FOR_EACH_END

    return 1;
}

// Очищает хэш-отображение ото всех элементов, сохраняя количество слотов.
// В случае успешной очистки возвращает > 0.
// Если в хэш-отображении не было элементов, возвращает 0.
// В случае ошибки возвращает < 0.
ptrdiff_t c_hash_map_clear(c_hash_map *const _hash_map,
                           void (*const _del_key_func)(void *const _key),
                           void (*const _del_data_func)(void *const _data))
{
    if (_hash_map == NULL) return -1;

    if (_hash_map->nodes_count == 0) return 0;

    size_t count = _hash_map->nodes_count;

    // Макросы дублирования кода для избавленияот проверок внутри циклов.

    // Открытие циклов.
    #define C_HASH_MAP_CLEAR_BEGIN\
    for (size_t s = 0; (s < _hash_map->slots_count)&&(count > 0); ++s)\
    {\
        if (_hash_map->slots[s] != NULL)\
        {\
            c_hash_map_node *select_node = _hash_map->slots[s],\
                            *delete_node;\
            while (select_node != NULL)\
            {\
                delete_node = select_node;\
                select_node = select_node->next_node;

    // Закрытие циклов.
    #define C_HASH_MAP_CLEAR_END \
                free(delete_node);\
                --count;\
            }\
            _hash_map->slots[s] = NULL;\
        }\
    }

    // Заданы функции удаления и для ключей, и для данных.
    if ( (_del_key_func != NULL) && (_del_data_func != NULL) )
    {
        C_HASH_MAP_CLEAR_BEGIN

        _del_key_func( delete_node->key );
        _del_data_func( delete_node->data );

        C_HASH_MAP_CLEAR_END
    } else {
        // Задана функция удаления только для ключей.
        if (_del_key_func != NULL)
        {
            C_HASH_MAP_CLEAR_BEGIN

            _del_key_func( delete_node->key );

            C_HASH_MAP_CLEAR_END
        } else {
            // Задана функция удаления только для данных.
            if (_del_data_func != NULL)
            {
                C_HASH_MAP_CLEAR_BEGIN

                _del_data_func( delete_node->data );

                C_HASH_MAP_CLEAR_END
            }
        }
    }

    #undef C_HASH_MAP_CLEAR_BEGIN
    #undef C_HASH_MAP_CLEAR_END

    _hash_map->nodes_count = 0;

    return 1;
}

// Возвращает количество слотов в хэш-отображении.
// В случае ошибки возвращает 0, и если _error != NULL, в заданное расположение
// помещается код причины ошибки (> 0).
// Так как функция может возвращать 0 и в случае успеха, и в случае ошибки, для детектирования
// ошибки перед вызовом функции необходимо поместить 0 в заданное расположение ошибки.
size_t c_hash_map_slots_count(const c_hash_map *const _hash_map,
                              size_t *const _error)
{
    if (_hash_map == NULL)
    {
        error_set(_error, 1);
        return 0;
    }

    return _hash_map->slots_count;
}

// Возвращает количество узло в хэш-отображение.
// В случае ошибки возвращает 0, и если _error != NULL, в заданное расположение помещается
// код причины ошибки (> 0).
// Так как функция может возвращать 0 и в случае успеха, и в случае ошибки, для детектирования ошибки
// перед вызовом функции необходимо поместить 0 в заданное расположение ошибки.
size_t c_hash_map_pairs_count(const c_hash_map *const _hash_map,
                              size_t *const _error)
{
    if (_hash_map == NULL)
    {
        error_set(_error, 1);
        return 0;
    }

    return _hash_map->nodes_count;
}

// Возвращает коэф. максимальной загрузки хэш-отображения.
// В случае ошибки возвращает 0.0f.
float c_hash_map_max_load_factor(const c_hash_map *const _hash_map)
{
    if (_hash_map == NULL)
    {
        return 0.0f;
    }

    return _hash_map->max_load_factor;
}
