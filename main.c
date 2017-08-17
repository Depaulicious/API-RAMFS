// start:includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "ramfs_wrapped.h"
// end:includes

// start:definitions
int main() {
    char *cmdline = NULL;
    char *cmdline_saveptr;
    size_t cmdline_s = 0;
    ssize_t gl_ret;

    fs_node_t *root = ramfs_mkfs();

    do {
        gl_ret = getline_depau(&cmdline, &cmdline_s, stdin);
        cmdline = readcmd(cmdline, &cmdline_saveptr);

        // Empty line
        if (cmdline == NULL) {
            continue;
        }

#ifdef DEBUG
        printf("%lu %s ", get_linecount(), cmdline);
        if (get_linecount() == 0) {

            get_linecount();
        }
#endif

        // cmdline_saveptr is already at the beginning of next token
        if (strcmp(cmdline, "create") == 0) {
            ramfs_create_w(root, cmdline_saveptr);
        } else if (strcmp(cmdline, "create_dir") == 0) {
            ramfs_create_dir_w(root, cmdline_saveptr);
        } else if (strcmp(cmdline, "read") == 0) {
            ramfs_read_w(root, cmdline_saveptr);
        } else if (strcmp(cmdline, "write") == 0) {
            ramfs_write_w(root, cmdline_saveptr);
        } else if (strcmp(cmdline, "delete") == 0) {
            ramfs_delete_w(root, cmdline_saveptr);
        } else if (strcmp(cmdline, "delete_r") == 0) {
            ramfs_delete_r_w(root, cmdline_saveptr);
        } else if (strcmp(cmdline, "find") == 0) {
            ramfs_find_w(root, cmdline_saveptr);
        } else if (strcmp(cmdline, "exit") == 0) {
            break;
        } else {
            printf("no\n");
        }
#ifdef DEBUG
        increment_linecount();
#endif
    } while (gl_ret >= 0);

    // The same buffer is always used, eventually realloc'd.
    // We only need to free it once at the end.
    free(cmdline);
    // Remove root children
    _ramfs_rmnode_r(root, 0);
    // Remove root node
    _ramfs_rmnode(root, 0);

    return 0;
}
// end:definitions