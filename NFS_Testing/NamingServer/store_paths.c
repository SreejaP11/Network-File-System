#include "naming_server.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Function to create a new Trie node
TrieNode* create_trie_node(const char* name) {
    TrieNode* node = (TrieNode*)malloc(sizeof(TrieNode));
    strncpy(node->name, name, PATH_MAX_LEN - 1);
    node->name[PATH_MAX_LEN - 1] = '\0';
    node->is_directory = 0;
    node->storage_server = NULL;
    node->max_children = 10;
    node->child_count = 0;
    node->children = (TrieNode**)malloc(sizeof(TrieNode*) * node->max_children);
    return node;
}

// Function to split a path into components
char** split_path(const char* path, int* count) {
    char* temp = strdup(path);
    char** components = (char**)malloc(PATH_MAX_LEN * sizeof(char*));
    *count = 0;

    char* token = strtok(temp, "/");
    while (token != NULL) {
        components[(*count)++] = strdup(token);
        token = strtok(NULL, "/");
    }

    free(temp);
    return components;
}

// Function to insert a path into the Trie
void insert_path(TrieNode* root, const char* path, StorageServer* server) {
    int component_count;
    char** components = split_path(path, &component_count);

    TrieNode* current = root;
    for (int i = 0; i < component_count; i++) {
        int found = 0;
        for (int j = 0; j < current->child_count; j++) {
            if (strcmp(current->children[j]->name, components[i]) == 0) {
                current = current->children[j];
                found = 1;
                break;
            }
        }
        if (!found) {
            if (current->child_count == current->max_children) {
                current->max_children *= 2;
                current->children = realloc(current->children, sizeof(TrieNode*) * current->max_children);
            }
            TrieNode* new_node = create_trie_node(components[i]);
            current->children[current->child_count++] = new_node;
            current = new_node;
        }
    }
    current->storage_server = server;

    for (int i = 0; i < component_count; i++) {
        free(components[i]);
    }
    free(components);
}

// Function to search for a path in the Trie
StorageServer* search_path(TrieNode* root, const char* path) {
    int component_count;
    char** components = split_path(path, &component_count);

    TrieNode* current = root;
    for (int i = 0; i < component_count; i++) {
        int found = 0;
        for (int j = 0; j < current->child_count; j++) {
            if (strcmp(current->children[j]->name, components[i]) == 0) {
                current = current->children[j];
                found = 1;
                break;
            }
        }
        if (!found) {
            for (int i = 0; i < component_count; i++) {
                free(components[i]);
            }
            free(components);
            return NULL;  // Path not found
        }
    }

    for (int i = 0; i < component_count; i++) {
        free(components[i]);
    }
    free(components);

    return current->storage_server;  // Return the associated storage server
}

int remove_path_recursive(TrieNode* current, char** components, int depth, int component_count) {
    if (depth == component_count) {
        // Base case: We're at the target node to remove
        for (int i = 0; i < current->child_count; i++) {
            free_trie(current->children[i]); // Free all child nodes recursively
        }
        free(current->children); // Free the children array
        free(current);           // Free the current node itself
        return 1; // Indicate success
    }

    // Find the child corresponding to the next component
    for (int i = 0; i < current->child_count; i++) {
        if (strcmp(current->children[i]->name, components[depth]) == 0) {
            // Recur to remove the child
            if (remove_path_recursive(current->children[i], components, depth + 1, component_count)) {
                // Shift remaining children after deletion
                for (int j = i; j < current->child_count - 1; j++) {
                    current->children[j] = current->children[j + 1];
                }
                current->child_count--;
                return (current->child_count == 0 && current->storage_server == NULL); // Indicate if current can be removed
            }
            return 0; // Failed to remove child
        }
    }
    return 0; // Path not found
}

int remove_path(TrieNode* root, const char* path) {
    int component_count;
    char** components = split_path(path, &component_count);

    int result = 0;
    for (int i = 0; i < root->child_count; i++) {
        if (strcmp(root->children[i]->name, components[0]) == 0) {
            // Call the recursive removal function on the matched child
            if (remove_path_recursive(root->children[i], components, 1, component_count)) {
                // Shift remaining children after deletion
                for (int j = i; j < root->child_count - 1; j++) {
                    root->children[j] = root->children[j + 1];
                }
                root->child_count--;
                result = 1; // Indicate success
            }
            break;
        }
    }

    // Free memory allocated for path components
    for (int i = 0; i < component_count; i++) {
        free(components[i]);
    }
    free(components);

    return result; // Return whether the path was successfully removed
}


// Function to clean up the Trie recursively
void free_trie(TrieNode* root) {
    for (int i = 0; i < root->child_count; i++) {
        free_trie(root->children[i]);
    }
    free(root->children);
    free(root);
}

// Helper function to recursively print all paths
void print_paths_recursive(TrieNode* node, char* path, int depth) {
    // Append the current node's name to the path
    if (depth > 0) { // Skip root node as it has no name
        strcat(path, "/");
        strcat(path, node->name);
    }

    // If the node is associated with a storage server, print the path
    if (node->storage_server != NULL) {
        printf("%s\n", path);
    }

    // Recur for all children
    for (int i = 0; i < node->child_count; i++) {
        char new_path[PATH_MAX_LEN];
        strncpy(new_path, path, PATH_MAX_LEN - 1);
        new_path[PATH_MAX_LEN - 1] = '\0';
        print_paths_recursive(node->children[i], new_path, depth + 1);
    }
}

// Function to print all paths in the Trie
void print_all_paths(TrieNode* root) {
    char path[PATH_MAX_LEN] = "";
    print_paths_recursive(root, path, 0);
}
