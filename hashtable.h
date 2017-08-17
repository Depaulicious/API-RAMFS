//
// Created by depaulicious on 11/08/17.
//

#ifndef API_RAMFS_HASHTABLE_H
#define API_RAMFS_HASHTABLE_H

// start:includes
#include <stdlib.h>
#include <stdint.h>
// end:includes

// start:macros
#define BASE_HT_SIZE 64
// end:macros

// start:datatypes
typedef struct _ht_item {
    void *key;
    void *val;
} ht_item_t;

typedef struct _ht {
    size_t size;
    size_t used;
    ht_item_t *body;
} ht_t;

typedef struct _ht_item_list
{
    struct _ht_item_list *next;
    ht_item_t            *item;
} ht_item_list_t;
// end:datatypes

// start:declarations
ht_t       *ht_new();
void        ht_del(ht_t *t);
ht_item_t  *_ht_body_new(size_t size);
size_t      _ht_index(ht_t *t, void *key);
void       *ht_getitem(ht_t *t, void *key);
uint8_t     ht_setitem(ht_t *t, void *key, void *val);
void        ht_replitem(ht_t *t, void *key, void *val);
void        ht_delitem(ht_t *t, void *key);
void        ht_grow(ht_t *t, size_t newsize);

void _ht_replitem(ht_t *t, size_t pos, void *key, void *val);

#ifdef DEBUG
void dump_hashtable(ht_t *t);
#endif
// end:declarations

#endif //API_RAMFS_HASHTABLE_H
