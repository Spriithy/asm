#include "map.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

static uint64_t map_PRIME_1 = 151;
static uint64_t map_PRIME_2 = 163;

static map_item MAP_DELETED_ITEM = { NULL, NULL };

static int is_prime(const int x)
{
    if (x < 2) {
        return -1;
    }

    if (x < 4) {
        return 1;
    }

    if ((x % 2) == 0) {
        return 0;
    }

    for (int i = 3; i <= floor(sqrt((double)x)); i += 2) {
        if ((x % i) == 0) {
            return 0;
        }
    }

    return 1;
}

static int next_prime(int x)
{
    while (is_prime(x) != 1) {
        x++;
    }

    return x;
}

static uint64_t strhash(char* str, int a, int m)
{
    uint64_t hash = 0;
    size_t   len = strlen(str);
    for (size_t i = 0; i < len; i++) {
        hash += (uint64_t)pow(a, len - (i + 1)) * str[i];
        hash = hash % m;
    }
    return hash;
}

static uint64_t map_hash(char* str, int nbuckets, int attempts)
{
    uint64_t hash_a = strhash(str, map_PRIME_1, nbuckets);
    uint64_t hash_b = strhash(str, map_PRIME_2, nbuckets);
    return (hash_a + (attempts * (hash_b + 1))) % nbuckets;
}

static map_item* new_item(char* key, void* val)
{
    map_item* item = malloc(sizeof(*item));
    item->key = strdup(key);
    item->val = val;
    return item;
}

static void del_item(map_item* item)
{
    free(item->key);
    free(item);
}

static map_t* map_new_cap(int cap_index)
{
    map_t* map = malloc(sizeof(*map));
    map->cap_index = cap_index;

    int base_size = 50 << map->cap_index;
    map->cap = next_prime(base_size);

    map->len = 0;
    map->items = calloc((size_t)map->cap, sizeof(map_item*));
    return map;
}

static void map_resize(map_t* map, int direction)
{
    int ncap = map->cap_index + direction;
    if (ncap < 0) {
        // Don't resize down the smallest hash table
        return;
    }

    // Create a temporary new hash table to insert items into
    map_t* nmap = map_new_cap(ncap);
    // Iterate through existing hash table, add all items to new
    for (size_t i = 0; i < map->cap; i++) {
        map_item* item = map->items[i];
        if (item != NULL && item != &MAP_DELETED_ITEM) {
            map_insert(nmap, item->key, item->val);
        }
    }

    map->cap_index = nmap->cap_index;
    map->len = nmap->len;

    int tmp_size = map->cap;
    map->cap = nmap->cap;
    nmap->cap = tmp_size;

    map_item** tmp_items = map->items;
    map->items = nmap->items;
    nmap->items = tmp_items;

    map_free(nmap);
}

map_t* map_new()
{
    map_t* map = malloc(sizeof(*map));
    map->cap = 53;
    map->len = 0;
    map->items = calloc((size_t)map->cap, sizeof(map_item*));
    return map;
}

void map_free(map_t* map)
{
    for (size_t i = 0; i < map->len; i++) {
        map_item* item = map->items[i];
        if (item != NULL) {
            del_item(item);
        }
    }

    free(map->items);
    free(map);
}

void map_insert(map_t* map, char* key, void* val)
{
    // Resize if load > 0.7
    int load = map->len * 100 / map->cap;
    if (load > 70) {
        map_resize(map, 1);
    }

    map_item* item = new_item(key, val);

    // cycle though filled buckets until we hit an empty or deleted one
    int       index = map_hash(item->key, map->cap, 0);
    map_item* cur_item = map->items[index];

    int i = 1;
    while (cur_item != NULL && cur_item != &MAP_DELETED_ITEM) {
        if (strcmp(cur_item->key, key) == 0) {
            del_item(cur_item);
            map->items[index] = item;
            return;
        }

        index = map_hash(item->key, map->cap, i);
        cur_item = map->items[index];
        i++;
    }

    // index points to a free bucket
    map->items[index] = item;
    map->len++;
}

void* map_find(map_t* map, char* key)
{
    int       index = map_hash(key, map->cap, 0);
    map_item* item = map->items[index];

    int i = 1;
    while (item != NULL && item != &MAP_DELETED_ITEM) {
        if (strcmp(item->key, key) == 0) {
            return item->val;
        }

        index = map_hash(key, map->cap, i);
        item = map->items[index];
        i++;
    }

    return NULL;
}

void map_delete(map_t* map, char* key)
{
    // Resize if load < 10%
    int load = map->len * 100 / map->cap;
    if (load < 10) {
        map_resize(map, -1);
    }

    int       index = map_hash(key, map->cap, 0);
    map_item* item = map->items[index];
    int       i = 1;
    while (item != NULL && item != &MAP_DELETED_ITEM) {
        if (strcmp(item->key, key) == 0) {
            del_item(item);
            map->items[index] = &MAP_DELETED_ITEM;
        }

        index = map_hash(key, map->cap, i);
        item = map->items[index];
        i++;
    }

    map->len--;
}