#ifndef MAP_H
#define MAP_H

#include <stddef.h>

typedef struct {
    char* key;
    void* val;
} map_item;

typedef struct {
    int        cap_index;
    size_t     cap;
    size_t     len;
    map_item** items;
} map_t;

map_t* map_new();
void   map_free(map_t* map);
void   map_insert(map_t* map, char* key, void* val);
void*  map_find(map_t* map, char* key);
void   map_delete(map_t* map, char* key);

#endif // map.h
