//
// Created by depaulicious on 11/08/17.
//

// start:includes
#include "ramfs_wrapped.h"
#include "utils.h"
#include "ramfs.h"
// end:includes

// start:definitions
// Wrapper for ramfs.h

void ramfs_create_w(fs_node_t *root, char *cmd) {
    call_with_1(ramfs_create, root, cmd);
}

void ramfs_create_dir_w(fs_node_t *root, char *cmd) {
    call_with_1(ramfs_create_dir, root, cmd);
}

void ramfs_read_w(fs_node_t *root, char *cmd) {
    char *save_ptr;
    char *arg1 = readcmd(cmd, &save_ptr);
    char* ret = ramfs_read(root, arg1);
    if (ret == NULL)
        printf("no\n");
    else
        printf("contenuto %s\n", ret);
}

void ramfs_write_w(fs_node_t *root, char *cmd) {
    call_with_2(ramfs_write, root, cmd);
}

void ramfs_delete_w(fs_node_t *root, char *cmd) {
    call_with_1(ramfs_delete, root, cmd);
}

void ramfs_delete_r_w(fs_node_t *root, char *cmd) {
    call_with_1(ramfs_delete_r, root, cmd);
}

void ramfs_find_w(fs_node_t *root, char *cmd) {
    char *save_ptr;
    char *arg1 = readcmd(cmd, &save_ptr);
    size_t nres;
    char **results = ramfs_find(root, arg1, &nres);

    if (nres == 0)
        printf("no\n");
    else {
        for (size_t i = 0; i < nres; i++) {
            printf("ok %s\n", results[i]);
            free(results[i]);
        }
    }
    free(results);
}
// end:definitions