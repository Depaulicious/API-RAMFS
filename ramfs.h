//
// Created by depaulicious on 11/08/17.
//

#ifndef API_RAMFS_RAMFS_H
#define API_RAMFS_RAMFS_H

// start:includes
#include "hashtable.h"
// end:includes

// start:macros
#define MAX_NAME_LENGTH 255
#define FIND_ARRAY_SIZE 64
// end:macros

// start:datatypes
typedef enum _node_type {
    TYPE_DIR,
    TYPE_FILE
} fs_node_type_t;

typedef union _fs_node_data {
    void *raw;
    char *content;
    ht_t *children;
} fs_node_data_u;

typedef struct _fs_node {
    char *name;
    struct _fs_node *parent;
    fs_node_type_t type;
    fs_node_data_u data;
    uint8_t depth;
} fs_node_t;
// end:datatypes

// start:declarations
int ramfs_create(fs_node_t *root, char *path);
int ramfs_create_dir(fs_node_t *root, char *path);
char * ramfs_read(fs_node_t *root, char *path);
int ramfs_write(fs_node_t *root, char *path, char *content);
int ramfs_delete(fs_node_t *root, char *path);
int ramfs_delete_r(fs_node_t *root, char *path);
char **ramfs_find(fs_node_t *root, char *keyword, size_t *nres);
fs_node_t  *ramfs_mkfs();

fs_node_t *_ramfs_resolve_node(fs_node_t *root, char *path, char **newname);
char       *_ramfs_getpath(fs_node_t *node);
fs_node_t  *_ramfs_mknode(fs_node_t *parent, char *name, fs_node_type_t type, void *data);
int _ramfs_rmnode(fs_node_t *node, uint8_t no_rm_from_parent);
int _ramfs_rmnode_r(fs_node_t *node, uint8_t no_rm_from_parent);
size_t _ramfs_find(fs_node_t *node, char *curpath, char *keyword, char ***results, size_t *len, size_t *pos);
int _pathcmp(const void *p1, const void *p2);


#ifdef DEBUG
void dump_node(fs_node_t *node);
#endif
// end:declarations

#endif //API_RAMFS_RAMFS_H
