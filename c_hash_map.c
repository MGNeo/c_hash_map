#include "c_hash_map.h"

// Создание пустого хэш-отображения.
// В случае успеха возвращает указатель на созданное отображение.
// В случае ошибки возвращает NULL.
// Позволяет создать отображение с нулем слотов.
c_hash_map *c_hash_map_create(size_t (*const _hash_func)(const void *const _key),
                              size_t (*const _comp_func)(const void *const _a,
                                                         const void *const _b),
                              const size_t _key_size,
                              const size_t _data_size,
                              const size_t _slots_count,
                              const float _max_load_factor)
{
    if (_hash_func == NULL) return NULL;
    if (_comp_func == NULL) return NULL;
    if (_key_size == 0) return NULL;
    if (_data_size == 0) return NULL;
    if (_max_load_factor <= 0.0f) return NULL;

    void *new_slots = NULL;

    if (_slots_count > 0)
    {
        // Определим размер новых слотов.
        const size_t new_slots_size = _slots_count * sizeof(void*);
        if ( (new_slots_size == 0) ||
             (new_slots_size / _slots_count != sizeof(void*)) )
        {
            return NULL;
        }

        // Попытаемся выделить память под новые слоты.
        new_slots = malloc(new_slots_size);
        if (new_slots == NULL) return NULL;

        // Обнулим слоты.
        memset(new_slots, 0, new_slots_size);
    }

    // Попытаемся создать хэш-отображение.
    c_hash_map *const new_hash_map = (c_hash_map*)malloc(sizeof(c_hash_map));
    if (new_hash_map == NULL)
    {
        free(new_slots);
        return NULL;
    }

    new_hash_map->hash_func = _hash_func;
    new_hash_map->comp_func = _comp_func;

    new_hash_map->key_size = _key_size;
    new_hash_map->data_size = _data_size;

    new_hash_map->slots_count = _slots_count;
    new_hash_map->nodes_count = 0;

    new_hash_map->slots = new_slots;

    return new_hash_map;
}

// Удаляет хэш-отображение.
// В случае успеха возвращает > 0, наче < 0.
ptrdiff_t c_hash_map_delete(c_hash_map *const _hash_map,
                            void (*const _del_key_func)(void *const _key),
                            void (*const _del_data_func)(void *const _data))
{
    if (_hash_map == NULL) return -1;

    if (_hash_map->nodes_count > 0)
    {
        size_t count = _hash_map->nodes_count;
        for (size_t s = 0; (s < _hash_map->nodes_count)&&(count > 0); ++s)
        {
            if (((void**)_hash_map->slots)[s] != NULL)
            {
                void *select_node = ((void**)_hash_map)[s],
                     *delete_node;
                while(select_node != NULL)
                {
                    delete_node = select_node;
                    select_node = *((void**)select_node);

                    // Если нужно, вызываем для удаления ключа специальную функцию.
                    if (_del_key_func != NULL)
                    {
                        _del_key_func( (uint8_t*)delete_node + sizeof(void*) + sizeof(size_t) );
                    }

                    // Если нужно, вызываем для удаления данных специальную функцию.
                    if (_del_data_func != NULL)
                    {
                        _del_data_func( (uint8_t*)delete_node + sizeof(void*) + sizeof(size_t)
                                        + _hash_map->key_size );
                    }

                    --count;
                }
            }
        }
    }
    free(_hash_map->slots);

    free(_hash_map);

    return 1;
}

// Вставка данных в хэш-отображение.
// В случае успешной вставки возвращает > 0.
// В случае наличия данных с таким же ключем в хэш-отображении возвращает 0.
// В случае ошибки возвращает < 0.
ptrdiff_t c_hash_map_insert(c_hash_map *const _hash_map,
                            const void *const _key,
                            const void *const _data)
{
    if (_hash_map == NULL) return -1;
    if (_key == NULL) return -2;
    if (_data == NULL) return -3;

    // Сперва определим, есть ли необходимость перестроить (расширить) таблицу хэш-отображения.
    size_t rebuild = 0;

    if (_hash_map->slots_count == 0)
    {
        // Вообще нет слотов.
        rebuild = 1;
    } else {
        // Превышена максимальная загруженность.
        const float load_factor = (float)_hash_map->nodes_count / _hash_map->slots_count;
        if (load_factor > _hash_map->max_load_factor)
        {
            rebuild = 1;
        }
    }

    // Если нужно, то перестраиваем.
    if (rebuild == 1)
    {
        // Определим новое количество слотов.
        size_t new_slots_count = _hash_map->slots_count * 1.75f;
        if (new_slots_count < _hash_map->slots_count)
        {
            return -4;
        }
        new_slots_count += 1;
        if (new_slots_count == 0)
        {
            return -5;
        }
        // Определим количество памяти, необходимое под новые слоты.
        const size_t new_slots_size = new_slots_count * sizeof(void*);
        if ( (new_slots_size == 0) ||
             (new_slots_size / new_slots_count != sizeof(void*)) )
        {
            return -6;
        }

        // Попытаемся выделить память под новые слоты.
        void *const new_slots = malloc(new_slots_size);
        if (new_slots == NULL)
        {
            return -7;
        }

        // Обнулим новые слоты.
        memset(new_slots, 0, new_slots_size);

        // Если в старых слотах есть узлы, то переносим их в новые слоты.
        if (_hash_map->nodes_count > 0)
        {
            size_t count = _hash_map->nodes_count;
            for (size_t s = 0; (s < _hash_map->slots_count)&&(count > 0); ++s)
            {
                if (((void**)_hash_map->slots)[s] != NULL)
                {
                    void *select_node = ((void**)_hash_map->slots)[s],
                         *relocate_node;
                    while(select_node != NULL)
                    {
                        relocate_node = select_node;
                        select_node = *((void**)select_node);

                        // Неприведенный хэш ключа переносимого узла.
                        const size_t hash = *((size_t*)((void**)relocate_node + 1));
                        // Хэш ключа переносимого узла, приведенный к количеству новых слотов.
                        const size_t presented_hash = hash % new_slots_count;

                        // Переносим узел в новый слот.
                        *((void**)relocate_node) = ((void**)new_slots)[presented_hash];
                        ((void**)new_slots)[presented_hash] = relocate_node;

                        --count;
                    }
                }
            }
        }

        free(_hash_map->slots);

        // Используем новые слоты.
        _hash_map->slots = new_slots;
        _hash_map->slots_count = new_slots_count;
    }

    // Неприведенный хэш ключа вставляемых данных.
    const size_t hash = _hash_map->hash_func(_key);

    // Приведенный хэш ключа вставляемых данных.
    const size_t presented_hash = hash % _hash_map->slots_count;

    // Проверим, имеются ли данные с таким же ключом в слоте.
    void *select_node = ((void**)_hash_map->slots)[presented_hash];
    while (select_node != NULL)
    {
        // Реальный хэш ключа просматриваемого узла.
        const size_t hash_n = *((size_t*)((void**)select_node + 1));
        if (hash == hash_n)
        {
            // Ключ просматриваемого узла.
            const void *const key_n = (uint8_t*)select_node + sizeof(void*) + sizeof(size_t);
            // Сравним детально.
            if (_hash_map->comp_func(_key, key_n) > 0)
            {
                return 0;
            }
        }

        select_node = *((void**)select_node);
    }

    // Если в слоте нет элемента с таким же ключем, то вставляем его туда.

    // Определим размер формируемого узла.
    const size_t new_node_size = sizeof(void*) + sizeof(size_t) + _hash_map->key_size + _hash_map->data_size;
    if (new_node_size < )// Тут должна быть более детальная проверка, чем в c_hash_set.
}
