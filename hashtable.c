//
// Created by depaulicious on 11/08/17.
//

// start:includes
#include <stdlib.h>
#include <string.h>
#include "hashtable.h"
#include "utils.h"
// end:includes


// start:definitions
// Hash table manipulation library

/*
 * Create a new hash table in memory and return a pointer to it.
 * Hash table needs to be freed with `ht_del`.
 */

ht_t *ht_new() {
    ht_t *ht;
    ht = malloc_or_die(sizeof(ht_t));
    ht->size = BASE_HT_SIZE;
    ht->used = 0;
    ht->body = _ht_body_new(BASE_HT_SIZE);

    return ht;
}

/*
 * (Internal) Create a new hash table body with size `size`.
 */

ht_item_t *_ht_body_new(size_t size) {
    return calloc_or_die(size, sizeof(ht_item_t));
}

/*
 * Looks up `key` in the hash table `t` and returns a pointer to its value.
 * If `key` does not exist, returns NULL.
 */

void *ht_getitem(ht_t *t, void *key) {
    size_t i = _ht_index(t, key);
    if (t->body[i].key == NULL)
        return NULL;
    return t->body[i].val;
}

/*
 * (Internal) Helper function to replace an item in a hash table.
 */

void _ht_replitem(ht_t *t, size_t pos, void *key, void *val) {
    // Check load factor and resize table
    if ((float) (t->used + 1) / (float) t->size > 0.8) {
        ht_grow(t, t->size * 2);
        pos = _ht_index(t, key);
    }
    // Add item to table
    t->used++;
    t->body[pos].key = key;
    t->body[pos].val = val;
}

/*
 * Add `key` to hash table `t` and associate value `val` to it.
 * If `key` already exists, does nothing. Note that `key` and `val`
 * will *not* be freed when removing item or destroying hash table.
 * Returns 0 if item was added, 1 otherwise.
 */

uint8_t ht_setitem(ht_t *t, void *key, void *val) {
    size_t i = _ht_index(t, key);
    // Key exists
    if (t->body[i].key != NULL)
        return 1;
    // Key does not exist, set item
    _ht_replitem(t, i, key, val);
    return 0;
}

/*
 * Unconditionally sets or replaces `key`'s value in hash table `t`.
 * See notes for `ht_setitem`.
 */

void ht_replitem(ht_t *t, void *key, void *val) {
    size_t i = _ht_index(t, key);
    _ht_replitem(t, i, key, val);
}

/*
 * Removes `key` from hash table.
 * Implements this pseudo code from old Wikipedia:
 * https://en.wikipedia.org/w/index.php?title=Hash_table&oldid=95275577#Example_pseudocode
 */

void ht_delitem(ht_t *t, void *key) {
    size_t i = _ht_index(t, key);
    size_t j, k;

    // Key does not exist
    if (t->body[i].key == NULL)
        return;

    j = i;
    // Rearrange following items
    for (;;) {
        j = (j+1) % t->size;

        // Slot is empty
        if (t->body[j].key == NULL)
            break;

        k = hash(t->body[j].key, strlen(t->body[j].key)) % t->size;

        if ((j > i && (k <= i || k > j)) ||
            (j < i && (k <= i && k > j))) {
            t->body[i].key = t->body[j].key;
            t->body[i].val = t->body[j].val;
            i = j;
        }
    }

    t->used--;
    t->body[i].key = NULL;
    t->body[i].val = NULL;
}

/*
 * (Internal) Finds index for `key` in hash table `t`. If `key`
 * is not in the table, returns the index of the first empty slot
 * in which `key` can be stored.
 */

size_t _ht_index(ht_t *t, void *key) {
    size_t i = hash(key, strlen(key)) % t->size;
    // Find key slot or first empty slot
    while (t->body[i].key != NULL && strcmp(t->body[i].key, key) != 0) {
        i = (i + 1) % t->size;
    }
    return i;
}

/*
 * Grows hash table `t` so it can host `newsize` keys.
 */

void ht_grow(ht_t *t, size_t newsize) {
    ht_item_t *oldbody = t->body;
    size_t oldsize = t->size;
    size_t i = 0;

    // Create new hash table body
    t->body = _ht_body_new(newsize);
    t->used = 0;
    t->size = newsize;

    // Move keys to new body
    for (; i < oldsize; i++) {
        if (oldbody[i].key != NULL)
            ht_setitem(t, oldbody[i].key, oldbody[i].val);
    }
    free(oldbody);
}

/*
 * Frees hash table `t`'s data structures from memory.
 * No keys or values are freed.
 */

void ht_del(ht_t *t) {
    // Free data structures
    free(t->body);
    free(t);
}


#ifdef DEBUG

/*
 * Dumps to stderr for debugging.
 */

void dump_hashtable(ht_t *t) {
    fprintf(stderr, "--- DUMP HASHTABLE ---\n");
    fprintf(stderr, "Size: %u, Used: %u\n", (unsigned int) t->size, (unsigned int) t->used);
    for (size_t i = 0; i < t->size; i++) {
        if (t->body[i].key != NULL) {
            fprintf(stderr, "[%u] %s\n", (
                    unsigned int) i, (char *) t->body[i].key);
        }
    }
    fprintf(stderr, "--- END DUMP HASHTABLE ---\n\n");
}

#endif

// end:definitions