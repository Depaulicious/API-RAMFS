//
// Created by depaulicious on 11/08/17.
//

// start:includes
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "ramfs.h"
#include "utils.h"
// end:includes


// start:definitions
// Actual in-memory file system implementation

/*
 * Create a new root node and return it.
 */

inline fs_node_t *ramfs_mkfs() {
    return _ramfs_mknode(NULL, NULL, TYPE_DIR, NULL);
}


/*
 * Creates a new node of type `type` under `root` at `path`.
 * Name is duplicated before storing it, make sure it is freed.
 * Returns 0 on success, -1 on error.
 */

int ramfs_create_node(fs_node_t *root, char *path, fs_node_type_t type) {
    char *newnode = NULL;
    fs_node_t *node = _ramfs_resolve_node(root, path, &newnode);

    // Check for error
    if (node == NULL || newnode == NULL) {
#ifdef DEBUG
        fprintf(stderr, "create node %s failed: node and newnode are null\n", path);
#endif
        return -1;
    }

    _ramfs_mknode(node, newnode, type, NULL);
    return 0;
}

/*
 * Creates a new file node under `root` at `path`.
 * Name is duplicated before storing it, make sure it is freed.
 * Returns 0 on success, -1 on error.
 */

inline int ramfs_create(fs_node_t *root, char *path) {
    return ramfs_create_node(root, path, TYPE_FILE);
}

/*
 * Creates a new directory node under `root` at `path`.
 * Name is duplicated before storing it, make sure it is freed.
 * Returns 0 on success, -1 on error.
 */

inline int ramfs_create_dir(fs_node_t *root, char *path) {
    return ramfs_create_node(root, path, TYPE_DIR);
}


/*
 * Find node at `path` under `root` and return its content.
 */

char *ramfs_read(fs_node_t *root, char *path) {
    char *newnode = NULL;
    char *backpath = calloc_or_die(strlen(path) + 1, sizeof(char));
    strcpy(backpath, path);
    fs_node_t *node = _ramfs_resolve_node(root, path, &newnode);

    // Check for error
    if (node == NULL || newnode != NULL || node->type != TYPE_FILE) {
#ifdef DEBUG
        if (newnode != NULL)
            fprintf(stderr, "read %s failed: node does not exist\n", backpath);
        if (node != NULL && node->type != TYPE_FILE)
            fprintf(stderr, "read %s failed: node is not a file\n", backpath);
        if (node == NULL)
            fprintf(stderr, "read %s failed: node is null\n", backpath);
        else
            dump_node(node);
        free(backpath);
#endif
        return NULL;
    }
#ifdef DEBUG
    free(backpath);
#endif
    return node->data.content;
}

/*
 * Write `content` to file node at `path` under `root`.
 * Content is duplicated before storing, make sure it is freed.
 * Returns 0 on success, -1 on error.
 */

int ramfs_write(fs_node_t *root, char *path, char *content) {
    char *newnode = NULL;
#ifdef DEBUG
    char *backpath = calloc_or_die(strlen(path) + 1, sizeof(char));
    strcpy(backpath, path);
#endif
    fs_node_t *node = _ramfs_resolve_node(root, path, &newnode);

    // Check for error
    if (node == NULL || newnode != NULL || node->type != TYPE_FILE) {
#ifdef DEBUG
        if (newnode != NULL)
            fprintf(stderr, "write %s failed: node does not exist\n", backpath);
        if (node != NULL && node->type != TYPE_FILE)
            fprintf(stderr, "write %s failed: node is not a file\n", backpath);
        if (node == NULL)
            fprintf(stderr, "write %s failed: node is null\n", backpath);
        else
            dump_node(node);
        free(backpath);
#endif
        return -1;
    }
#ifdef DEBUG
    free(backpath);
#endif

    size_t len = strlen(content);
    free(node->data.content);
    node->data.content = calloc_or_die(len + 1, sizeof(char));
    strncpy(node->data.content, content, len);

    return (int) len;
}

/*
 * Delete node at `path` under `root`. Both its name and content
 * are freed, make sure you don't keep any references.
 * Returns 0 on success, -1 on error.
 */

int ramfs_delete(fs_node_t *root, char *path) {
    char *newnode = NULL;
    fs_node_t *node = _ramfs_resolve_node(root, path, &newnode);

    // Do not delete root
    if (node == root) {
#ifdef DEBUG
        fprintf(stderr, "delete failed: trying to remove root");
#endif
        return -1;
    }

    // Check for error
    if (node == NULL || newnode != NULL)
        return -1;

    return _ramfs_rmnode(node, false);
}

/*
 * Recursively delete node and all its children at `path` under `root`.
 * Everything is freed, make sure you don't keep any references.
 * If path is an empty string or '/', the root node is NOT deleted.
 * Returns 0 if all nodes were deleted, -1 otherwise.
 */

int ramfs_delete_r(fs_node_t *root, char *path) {
    char *newnode = NULL;
    fs_node_t *node = _ramfs_resolve_node(root, path, &newnode);

    // Check for error
    if (node == NULL || newnode != NULL)
        return -1;

    return _ramfs_rmnode_r(node, false);
}

/*
 * Recursively search for `keyword` under `root`. `nres` is the pointer
 * to a size_t where the number of results will be stored.
 * Returns a sorted array of strings containing the path of each
 * result. The results and the array need to be disposed of.
 */

char **ramfs_find(fs_node_t *root, char *keyword, size_t *nres) {
    size_t len = FIND_ARRAY_SIZE;
    size_t pos = 0;
    *nres = 0;
    char **results = malloc_or_die(len*sizeof(char**));

    // Find all matching nodes first
    *nres += _ramfs_find(root, "", keyword, &results, &len, &pos);

    // Sort their paths
    qsort(results, *nres, sizeof(char *), _pathcmp);

    return results;
}

// Internal functions

/*
 * (Internal) Creates a new node of type `type` named `name` under `parent`
 * and returns it. If `parent` and `name` are NULL and `type` is TYPE_DIR,
 * a root node is created. If `data` is NULL, an hash table will be created
 * for TYPE_DIR, and an empty string for TYPE_FILE.
 * If node couldn't be created, NULL is returned.
 */

fs_node_t *_ramfs_mknode(fs_node_t *parent, char *name, fs_node_type_t type, void *data) {
    char *namecopy = NULL;

    // Files don't have children
    if (parent != NULL && parent->type == TYPE_FILE) {
#ifdef DEBUG
        fprintf(stderr, "mknode %s parent %s failed: file can't have children\n", name, parent->name);
#endif
        return NULL;
    }
    // Unless the node is root and is a directory, name can't be empty or NULL
    if ((parent == NULL && type != TYPE_DIR) ||
        (parent != NULL && (name == NULL || strlen(name) == 0))) {
#ifdef DEBUG
        fprintf(stderr, "mknode %s parent %s failed: empty name\n", name, parent->name);
#endif
        return NULL;
    }
    // If node is root, name must be NULL
    if (parent == NULL && name != NULL) {
#ifdef DEBUG
        fprintf(stderr, "mknode %s failed: root node can't have a name\n", name);
#endif
        return NULL;
    }

    if (name != NULL) {
        // Check max name length
        size_t nlen = strlen(name);
        if (nlen > MAX_NAME_LENGTH) {
#ifdef DEBUG
            fprintf(stderr, "create node %s failed: name too long\n", name);
#endif
            return NULL;
        }

        namecopy = calloc_or_die(nlen + 1, sizeof(char));
        // Copy the name so it isn't freed later
        strcpy(namecopy, name);
    }


    if (data == NULL && type == TYPE_DIR)
        data = ht_new();
    else if (data == NULL && type == TYPE_FILE)
        data = calloc_or_die(1, sizeof(char));

    uint8_t depth = (uint8_t) (parent != NULL ? parent->depth + 1 : 0);

    if (parent != NULL && depth == 0) {
        // Overflow <3 maximum depth exceeded
#ifdef DEBUG
        fprintf(stderr, "mknode %s parent %s failed: maximum depth reached\n", name, parent->name);
#endif
        return NULL;
    }

    if (parent != NULL && parent->data.children->used >= MAX_CHILDREN) {
        // Parent can't accept more children
#ifdef DEBUG
        fprintf(stderr, "mknode %s parent %s failed: maximum number of children reached\n", name, parent->name);
#endif
        return NULL;

    }

    fs_node_t *node = calloc_or_die(1, sizeof(fs_node_t));
    node->parent = parent;
    node->name = namecopy;
    node->type = type;
    node->data.raw = data;
    node->depth = depth;

    // If node is not root, add it to its parent
    if (parent != NULL)
        // If node already exists, error
        if (ht_setitem(parent->data.children, namecopy, node) != 0) {
            // We malloc'd memory so we need to free it.
            // The chance this event happens is so low that checking for
            // it earlier is worse than cleaning up.
#ifdef DEBUG
            fprintf(stderr, "mknode %s parent %s failed: node exists\n", name, parent->name);
#endif
            free(node);
            return NULL;
        }
    return node;
}

/*
 * (Internal) Delete node `node` and free its content. If
 * `no_rm_from_parent` is true, it is not removed from its parent.
 * Returns 0 on success, -1 on error.
 */

int _ramfs_rmnode(fs_node_t *node, uint8_t no_rm_from_parent) {
    // Make sure directory is empty
    if (node->type == TYPE_DIR && node->data.children->used > 0) {
#ifdef DEBUG
        fprintf(stderr, "rmnode %s failed: directory not empty\n", node->name);
        dump_node(node);
#endif
        return -1;
    }

    // Destroy node data
    if (node->type == TYPE_DIR) {
        ht_del(node->data.children);
    } else {
        free(node->data.content);
    }

    // Remove from parent (unless no_rm_from_parent is true)
    if (!no_rm_from_parent && node->parent != NULL)
        ht_delitem(node->parent->data.children, node->name);

    // Destroy node
    free(node->name);
    node->name = NULL;
    free(node);

    return 0;
}


/*
 * (Internal) Recursively delete node `node` and all its children. If
 * `no_rm_from_parent` is used internally to speed up deletion.
 * It should always be 0/false.
 * Returns 0 if all nodes were removed, -1 otherwise.
 */

int _ramfs_rmnode_r(fs_node_t *node, uint8_t no_rm_from_parent) {
    size_t i;
    ht_item_t *item;
    int error = 0;

    // Node has children
    if (node->type == TYPE_DIR && node->data.children->used > 0) {
        for (i = 0; i < node->data.children->size; i++) {
            if (node->data.children->body[i].key == NULL)
                continue;
            item = &node->data.children->body[i];
            // Always use no_rm_from_parent when recursively calling self
            // Hashtable is going to be deleted anyway, no need to remove
            // children from parent.
            error |= _ramfs_rmnode_r(item->val, true);
        }
        node->data.children->used = 0;
    }

    // Node is (now) a leaf
    if (node->parent != NULL  // node is not root and
        && (node->type == TYPE_FILE // (node is file or
            || (node->type == TYPE_DIR // node is dir and
                && node->data.children->used == 0))) { // dir is empty)
        error |= _ramfs_rmnode(node, no_rm_from_parent);
    }

    return error != 0 ? -1 : 0;
}

/*
 * (Internal) Returns human-readable path of `node` up to the root node.
 */

char *_ramfs_getpath(fs_node_t *node) {
    char *dirnames[255] = {0};
    fs_node_t *n = node;

    unsigned int len = 256;
    unsigned int pos = 0;
    char *string = malloc_or_die(len * sizeof(char));
    char *tmp;

    for (; n != NULL; n = n->parent)
        dirnames[n->depth] = n->name;

    string[0] = '/';
    pos++;

    for (int i = 1; i <= 255 && dirnames[i] != NULL; i++) {
        tmp = dirnames[i];
        for (int j = 0; tmp != NULL && tmp[j] != '\0'; j++) {
            // len-3 to make sure there are some spare chars
            if (pos + 1 > len - 3) {
                len += 256;
                string = realloc_or_die(string, len);
            }
            string[pos] = tmp[j];
            pos++;
        }
        string[pos] = '/';
        pos++;
    }

    // Remove trailing / if node is not directory
    if (node->type != TYPE_DIR)
        pos--;

    // Terminate key
    string[pos] = '\0';

    return string;
}

/*
 * (Internal) Walk tree starting from `root` to find the node at `path`.
 * If the node is found, it is returned. If the node was not found, but
 * its direct parent was, the parent is returned and the new node name
 * is stored into `newname`, so that the new node can be created.
 * Otherwise, NULL is returned.
 */

fs_node_t *_ramfs_resolve_node(fs_node_t *root, char *path, char **newname) {
    char *saveptr = NULL;
    char *tok;
    fs_node_t *node;
    fs_node_t *nodes[2] = {0};
    *nodes = root;
    uint8_t pos = 1;
    uint16_t count = 1;

    if ((tok = strtok_depau(path, "/", &saveptr)) != NULL && count <= 255) {
        do {
            if (strcmp(tok, "") == 0)
                continue;
            if (*newname != NULL) {
                // If we reach here it means that two nodes weren't found.
                // Path is too long, signal it by setting last node to root
#ifdef DEBUG
                fprintf(stderr, "resolve path %s (new name) failed: path too long\n",
                        path);
#endif
                nodes[pos] = root;
                nodes[!pos] = root;
                break;
            }
            // Parent node is a file, this can't be right
            if (nodes[!pos]->type == TYPE_FILE) {
#ifdef DEBUG
                fprintf(stderr, "resolve path parent %s failed: trying to find a file's child\n",
                        nodes[!pos]->name);
#endif
                nodes[pos] = root;
                nodes[!pos] = root;
                *newname = NULL;
                break;
            }
            nodes[pos] = ht_getitem(nodes[!pos]->data.children, tok);
            if (nodes[pos] == NULL && *newname == NULL) {
                // Node not found, it's probably a new node
                *newname = tok;
                count++;
                continue;
                // Go on to check if there is an extra token to read
            }
            pos = (uint8_t) !pos;
            count++;
        } while ((tok = strtok_depau(NULL, "/", &saveptr)) != NULL && count <= 255);
    }


    // Make sure no new node is created after depth 255
    if (count >= 256) {
#ifdef DEBUG
        fprintf(stderr, "resolve path may have failed: maximum depth exceeded\n");
#endif
        *newname = NULL;
    }

    if (nodes[!pos] != NULL)
        pos = (uint8_t) !pos;

    // Handle error
    if (nodes[pos] == root || nodes[pos] == NULL) {
        // count == 1 means path was "/"
        // count == 2 && *newname != NULL means path was "/dir" and "dir" did not exist
        if (count == 1 || (count == 2 && *newname != NULL))
            return root;
#ifdef DEBUG
        fprintf(stderr, "resolve node %s failed: could not resolve path\n", path);
#endif
        return NULL;
    }

    node = nodes[pos];
    return node;
}

/*
 * (Internal) Recursively search nodes matching `keyword` within `root`
 * and its children (if any). `results` is a pointer to an array of strings that
 * will be updated with any matches. `len` a pointer to the length of the array
 * and `pos` to the current position in it.
 * Returns the number of results found.
 */

size_t _ramfs_find(fs_node_t *node, char *curpath, char *keyword, char ***results, size_t *len, size_t *pos) {
    size_t i;
    size_t nres = 0;

    // node is not root
    if (node->name != NULL) {
        // Try to match current node
        if (strcmp(node->name, keyword) == 0) {
            if ((*pos)+1 >= *len) {
                *len += FIND_ARRAY_SIZE;
                *results = realloc_or_die(*results, *len*sizeof(char**));
            }
            (*results)[*pos] = strcat_auto(2, curpath, node->name);
            (*pos)++;
            nres++;
        }
    }
    if (node->type == TYPE_DIR && node->data.children->used > 0) {
        for (i = 0; i < node->data.children->size; i++) {
            if (node->data.children->body[i].key != NULL) {
                char *newpath = strcat_auto(3, curpath, node->name, "/");
                nres += _ramfs_find(node->data.children->body[i].val,
                                    newpath, keyword, results, len, pos);
                free(newpath);
            }
        }
    }


    return nres;
}

/*
 * (Internal) Helper function used to compare paths when sorting them
 */

int _pathcmp(const void *p1, const void *p2) {
    return strcmp(* (char * const *) p1, * (char * const *) p2);
}

#ifdef DEBUG
/*
 * (Internal) Dump node `node` to stderr for debugging.
 */

void dump_node(fs_node_t *node) {
    fprintf(stderr, "--- DUMP NODE (at %lu) ---\n", get_linecount());
    fprintf(stderr, "Address: %p\n", node);
    fprintf(stderr, "Name: %s\n", node->name);
    fprintf(stderr, "Type: %s\n", node->type == TYPE_FILE ? "file" : "directory");
    fprintf(stderr, "Depth: %i\n", node->depth);
    char *path = _ramfs_getpath(node);
    fprintf(stderr, "Path: %s\n", path);
    if (node->type == TYPE_DIR) {
        fprintf(stderr, "Children (%u/%u): ", (unsigned int) node->data.children->used,
                (unsigned int) node->data.children->size);
        for (size_t i = 0; i < node->data.children->size; i++) {
            if (node->data.children->body[i].key != NULL) {
                fs_node_t *child = node->data.children->body[i].val;
                fprintf(stderr, "[%c]%s, ", child->type == TYPE_FILE ? 'f' : 'd', child->name);
            }
        }
        fprintf(stderr, "\n");
    } else {
        fprintf(stderr, "Content: \"%s\"\n", node->data.content);
    }
    fprintf(stderr, "Is root: %s\n", node->parent == NULL ? "true" : "false");
    fprintf(stderr, "--- END DUMP NODE ---\n\n");

    free(path);
}
#endif

// end:definitions