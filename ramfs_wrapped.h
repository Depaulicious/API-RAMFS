//
// Created by depaulicious on 11/08/17.
//

#ifndef API_RAMFS_RAMFS_WRAPPED_H
#define API_RAMFS_RAMFS_WRAPPED_H

// start:includes
#include "ramfs.h"
// end:includes

// start:declarations
void ramfs_create_w(fs_node_t *root, char *cmd);
void ramfs_create_dir_w(fs_node_t *root, char *cmd);
void ramfs_read_w(fs_node_t *root, char *cmd);
void ramfs_write_w(fs_node_t *root, char *cmd);
void ramfs_delete_w(fs_node_t *root, char *cmd);
void ramfs_delete_r_w(fs_node_t *root, char *cmd);
void ramfs_find_w(fs_node_t *root, char *cmd);
// end:declarations

#endif //API_RAMFS_RAMFS_WRAPPED_H
