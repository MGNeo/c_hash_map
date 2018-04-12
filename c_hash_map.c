/*
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

#include "c_hash_map.h"

// Создание пустого хэш-отображения.
// В случае успеха возвращает указатель на созданное отображение.
// В случае ошибки возвращает NULL.
// Позволяет создать хэш-отображение с нулем слотов.
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

    new_hash_map->max_load_factor = _max_load_factor;

    new_hash_map->slots = new_slots;

    return new_hash_map;
}

// Удаляет хэш-отображение.
// В случае успеха возвращает > 0, иначе < 0.
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
                void *select_node = ((void**)_hash_map->slots)[s],
                     *delete_node;

                while (select_node != NULL)
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

                    free(delete_node);

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
// Если данные с указанным ключом уже есть в хэш-отображении, функция возвращает 0, не перезаписывая старые
// данные новыми.
// В случае ошибки возвращает < 0.
ptrdiff_t c_hash_map_insert(c_hash_map *const _hash_map,
                            const void *const _key,
                            const void *const _data)
{
    if (_hash_map == NULL) return -1;
    if (_key == NULL) return -2;
    if (_data == NULL) return -3;

    // Первым делом контролируем процесс увеличения количества слотов.

    // Если слотов нет вообще.
    if (_hash_map->slots_count == 0)
    {
        // Задаем новое количество слотов с некоторым запасом.
        const size_t new_slots_count = EXTENSION_FROM_ZERO;

        // Попытаемся расширить слоты.
        if (c_hash_map_resize(_hash_map, new_slots_count) < 0)
        {
            return -4;
        }
    } else {
        // Если слоты есть, то при достижении предела загруженности увеличиваем количество слотов.
        const float load_factor = (float)_hash_map->nodes_count / _hash_map->slots_count;
        if (load_factor >= _hash_map->max_load_factor)
        {
            // Определим новое количество слотов.
            size_t new_slots_count = _hash_map->slots_count * 1.75f;
            if (new_slots_count < _hash_map->slots_count)
            {
                return -5;
            }
            new_slots_count += 1;
            if (new_slots_count == 0)
            {
                return -6;
            }

            // Попытаемся расширить слоты.
            if (c_hash_map_resize(_hash_map, new_slots_count) < 0)
            {
                return -7;
            }
        }
    }
    // Проверим, имеются ли в хэш-отображении данные с заданным ключом.
    ptrdiff_t r_code = c_hash_map_check(_hash_map, _key);

    // Ошибка.
    if (r_code < 0) return -8;

    // Данные уже имеются.
    if (r_code > 0) return 0;

    // Вставляем.

    // Определим размер создаваемого узла.
    size_t new_node_size = _hash_map->key_size + _hash_map->data_size;
    if (new_node_size < _hash_map->key_size)
    {
        return -9;
    }
    new_node_size += sizeof(void*) + sizeof(size_t);
    if (new_node_size < sizeof(void*) + sizeof(size_t))
    {
        return -10;
    }

    // Попытаемся выделить память под узел.
    void *const new_node = malloc(new_node_size);
    if (new_node == NULL)
    {
        return -11;
    }

    // Неприведенный хэш ключа вставляемых данных.
    const size_t hash = _hash_map->hash_func(_key);

    // Приведенный хэш ключа вставляемых данных.
    const size_t presented_hash = hash % _hash_map->slots_count;

    // Заносим в узел непрвиеденный хэш ключа вставляемых данных.
    *((size_t*)((void**)new_node + 1)) = hash;

    // Копируем в узел ключ.
    memcpy((uint8_t*)new_node + sizeof(void*) + sizeof(size_t), _key, _hash_map->key_size);

    // Копируем в узел данные.
    memcpy((uint8_t*)new_node + sizeof(void*) + sizeof(size_t) + _hash_map->key_size, _data, _hash_map->data_size);

    // Добавляем узел в слот.
    *((void**)new_node) = ((void**)_hash_map->slots)[presented_hash];
    ((void**)_hash_map->slots)[presented_hash] = new_node;

    ++_hash_map->nodes_count;

    return 1;
}

// Удаление из хэш-отображения данных с заданным ключом.
// В случае успешного удаления возвращает > 0.
// В случае, если данные с заданным ключом отсутствуют, возвращает 0.
// В случае ошибки возвращает < 0.
ptrdiff_t c_hash_map_erase(c_hash_map *const _hash_map,
                           const void *const _key,
                           void (*const _del_key_func)(void *const _key),
                           void (*const _del_data_func)(void *const _data))
{
    if (_hash_map == NULL) return -1;
    if (_key == NULL) return -2;

    if (_hash_map->nodes_count == 0) return 0;

    // Вычислим неприведенный хэш ключа удаляемых данных.
    const size_t hash = _hash_map->hash_func(_key);

    // Вычислим приведенный хэш ключа удаляемых данных.
    const size_t presented_hash = hash % _hash_map->slots_count;

    // Если требуемый слот пуст, значит данных с таким ключом в хэш-отображении нет.
    if (((void**)_hash_map->slots)[presented_hash] == NULL)
    {
        return 0;
    }

    // Просмотр слота на наличие данных с заданным ключом.
    void *select_node = ((void**)_hash_map->slots)[presented_hash],
         *prev_node = &((void**)_hash_map)[presented_hash];

    while (select_node != NULL)
    {
        // Неприведенный хэш ключа узла.
        const size_t hash_n = *((size_t*)((void**)select_node + 1));

        if (hash == hash_n)
        {
            // Ключ узла.
            void *key_n = (uint8_t*)select_node + sizeof(void*) + sizeof(size_t);
            if (_hash_map->comp_func(_key, key_n) > 0)
            {
                // Удаляем данный узел.

                // Ампутация узла из слота.
                *((void**)prev_node) = *((void**)select_node);

                // Если для ключа задана функция удаления, вызываем ее.
                if (_del_key_func != NULL)
                {
                    _del_key_func( (uint8_t*)select_node + sizeof(void*) + sizeof(size_t) );
                }

                // Если для данных задана функция удаления, вызываем ее.
                if (_del_data_func != NULL)
                {
                    _del_data_func( (uint8_t*)select_node + sizeof(void*) + sizeof(size_t) + _hash_map->key_size );
                }

                // Удаляем узел.
                free(select_node);

                --_hash_map->nodes_count;

                return 1;
            }
        }

        prev_node = select_node;
        select_node = *((void**)select_node);
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
        const size_t new_slots_size = _slots_count * sizeof(void*);
        if ( (new_slots_size == 0) ||
             (new_slots_size / _slots_count != sizeof(void*)) )
        {
            return -3;
        }

        // Попытаемся выделить память под новые слоты.
        void *const new_slots = malloc(new_slots_size);
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
                if (((void**)_hash_map->slots)[s] != NULL)
                {
                    void *select_node = ((void**)_hash_map->slots)[s],
                         *relocate_node;

                    while (select_node != NULL)
                    {
                        relocate_node = select_node;
                        select_node = *((void**)select_node);

                        // Неприведенный хэш ключа переносимого узла.
                        const size_t hash = *((size_t*)((void**)relocate_node + 1));
                        // Хэш ключа переносимого узла, приведенный к новому количеству слотов.
                        const size_t presented_hash = hash % _slots_count;

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
    const size_t hash = _hash_map->hash_func(_key);

    // Приведенный хэш искомого ключа.
    const size_t presented_hash = hash % _hash_map->slots_count;

    void *select_node = ((void**)_hash_map->slots)[presented_hash];

    while (select_node != NULL)
    {
        // Неприведенный хэш ключа узла.
        const size_t hash_n = *((size_t*)((void**)select_node + 1));

        if (hash == hash_n)
        {
            // Ключ узла.
            const void *const key_n = (uint8_t*)select_node + sizeof(void*) + sizeof(size_t);

            if (_hash_map->comp_func(_key, key_n) > 0)
            {
                return 1;
            }
        }

        select_node = *((void**)select_node);
    }

    return 0;
}

// Обращение к данным с заданным ключом.
// В случае успеха возвращает указатель на данные, которые связаны с заданным ключом.
// В случае ошибки, или если таких данных нет, возвращает NULL.
void *c_hash_map_at(const c_hash_map *const _hash_map,
                          const void *const _key)
{
    if (_hash_map == NULL) return NULL;
    if (_key == NULL) return NULL;

    if (_hash_map->nodes_count == 0) return NULL;

    // Неприведенный хэш искомого ключа.
    const size_t hash = _hash_map->hash_func(_key);

    // Приведенный хэш искомого ключа.
    const size_t presented_hash = hash % _hash_map->slots_count;

    void *select_node = ((void**)_hash_map->slots)[presented_hash];

    while (select_node != NULL)
    {
        // Неприведенный хэш ключа узла.
        const size_t hash_n = *((size_t*)((void**)select_node + 1));

        if (hash == hash_n)
        {
            // Ключ узла.
            const void *const key_n = (uint8_t*)select_node + sizeof(void*) + sizeof(size_t);

            if (_hash_map->comp_func(_key, key_n) > 0)
            {
                return (uint8_t*)select_node + sizeof(void*) + sizeof(size_t) + _hash_map->key_size;
            }
        }

        select_node = *((void**)select_node);
    }

    return 0;
}

// Проходит по всем элементам хэш-отображения и выполняет над ключами и данными заданные действия.
// В случае успешного выполнения возвращает > 0.
// В случае, если в хэш-отображении нет элементов, возвращает 0.
// В случае ошибки возвращает < 0.
ptrdiff_t c_hash_map_for_each(const c_hash_map *const _hash_map,
                              void (*const _key_func)(const void *const _key),
                              void (*const _data_func)(const void *const _data))
{
    if (_hash_map == NULL) return -1;
    if (_key_func == NULL) return -2;
    if (_data_func == NULL) return -3;

    if (_hash_map->nodes_count == 0) return 0;

    size_t count = _hash_map->nodes_count;

    for (size_t s = 0; (s < _hash_map->slots_count)&&(count > 0); ++s)
    {
        if (((void**)_hash_map->slots)[s] != NULL)
        {
            void *select_node = ((void**)_hash_map->slots)[s];

            while (select_node != NULL)
            {
                _key_func( (uint8_t*)select_node + sizeof(void*) + sizeof(size_t) );
                _data_func( (uint8_t*)select_node + sizeof(void*) + sizeof(size_t) + _hash_map->key_size );

                select_node = *((void**)select_node);

                --count;
            }
        }
    }

    return 1;
}

// Очищает хэш-отображение ото всех элементов, сохраняя количество слотов.
// В случае успеха возвращает > 0.
// Если в хэш-отображении не было элементов, возвращает 0.
// В случае ошибки возвращает < 0.
ptrdiff_t c_hash_map_clear(c_hash_map *const _hash_map,
                           void (*const _del_key_func)(void *const _key),
                           void (*const _del_data_func)(void *const _data))
{
    if (_hash_map == NULL) return -1;

    if (_hash_map->nodes_count == 0) return 0;

    size_t count = _hash_map->nodes_count;

    for (size_t s = 0; (s < _hash_map->slots_count)&&(count > 0); ++s)
    {
        if (((void**)_hash_map->slots)[s] != NULL)
        {
            void *select_node = ((void**)_hash_map->slots)[s],
                 *delete_node;
            while (select_node != NULL)
            {
                delete_node = select_node;
                select_node = *((void**)select_node);

                if (_del_key_func != NULL)
                {
                    _del_key_func( (uint8_t*)delete_node + sizeof(void*) + sizeof(size_t) );
                }

                if (_del_data_func != NULL)
                {
                    _del_data_func( (uint8_t*)delete_node + sizeof(void*) + sizeof(size_t) + _hash_map->key_size);
                }

                free(delete_node);
                --count;
            }

            ((void**)_hash_map->slots)[s] = NULL;
        }
    }

    _hash_map->nodes_count = 0;

    return 1;
}
