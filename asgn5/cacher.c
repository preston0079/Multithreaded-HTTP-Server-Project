#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define the cache policies
typedef enum { FIFO, LRU, Clock } Policy;

// Define a node in the cache
typedef struct Node {
    void *data;
    struct Node *next;
    int ref_bit; // Only used in Clock policy
} Node;

// Define a linked list
typedef struct List {
    Node *head;
    int size;
    int max_size;
} List;

// Define the cache structure
typedef struct Cache {
    Policy policy;
    List *list;
    int clock_pointer;
    List *historylist; // list of items that are being evicted

    int num_compulsory_misses;
    int num_capacity_misses;
} Cache;

// Function prototypes
Cache *init_cache(int size, Policy policy);
int insert_fifo(Cache *cache, void *element);
int insert_lru(Cache *cache, void *element);
int insert_clock(Cache *cache, void *element);
void free_cache(Cache *cache);

int list_is_full(List *list);
void list_add_back(List *list, void *data);
void list_remove_front(List *list);
int list_contains(List *list, void *element);
void list_move_to_end(List *list, void *element);
Node *list_search_for_element(List *list, void *element);
Node *list_get_element_at_index(List *list, int index);
void list_overwrite_at_index(List *list, int index, void *data);
int list_get_max_size(List *list);
void list_free(List *list);

// Function to initialize a cache
Cache *init_cache(int size, Policy policy) {
    Cache *cache = (Cache *) malloc(sizeof(Cache));
    if (cache == NULL) {
        perror("Failed to allocate memory for cache");
        exit(EXIT_FAILURE);
    }

    cache->policy = policy;
    cache->list = (List *) malloc(sizeof(List));
    cache->list->head = NULL;
    cache->list->size = 0;
    cache->list->max_size = size;

    cache->historylist = (List *) malloc(sizeof(List));
    cache->historylist->head = NULL;
    cache->historylist->size = 0;
    cache->historylist->max_size = 0; // History list has no size limit

    cache->num_compulsory_misses = 0;
    cache->num_capacity_misses = 0;
    cache->clock_pointer = 0;

    return cache;
}

// Function to insert into FIFO cache
int insert_fifo(Cache *cache, void *element) {
    if (cache == NULL || element == NULL) {
        return -1; // Error
    }

    // Check if the element is in the cache
    if (list_contains(cache->list, element)) {
        // HIT
        printf("HIT\n");
        return 1;
    }

    // MISS
    printf("MISS\n");

    // check if the list is full
    if (list_is_full(cache->list)) {
        list_remove_front(cache->list);

        // check if the history list contains the element
        if (!list_contains(cache->historylist, element)) {
            cache->num_compulsory_misses++;

            list_add_back(cache->historylist, element);
        } else {
            cache->num_capacity_misses++;
        }
    } else {
        // check if the history list contains the element
        if (!list_contains(cache->historylist, element)) {
            cache->num_compulsory_misses++;

            list_add_back(cache->historylist, element);
        }
    }

    // add the element to the back of the list
    list_add_back(cache->list, element);

    return 0; // return false if the element was NOT in the cache
}

// Function to insert into LRU cache
int insert_lru(Cache *cache, void *element) {
    if (cache == NULL || element == NULL) {
        return -1; // Error
    }

    // Check if the element is in the cache
    if (list_contains(cache->list, element)) {
        // HIT
        list_move_to_end(cache->list, element);
        printf("HIT\n");
        return 1;
    }

    // MISS
    printf("MISS\n");

    // check if the list is full
    if (list_is_full(cache->list)) {
        list_remove_front(cache->list);

        // check if the history list contains the element
        if (!list_contains(cache->historylist, element)) {
            cache->num_compulsory_misses++;

            list_add_back(cache->historylist, element);
        } else {
            cache->num_capacity_misses++;
        }
    } else {
        // check if the history list contains the element
        if (!list_contains(cache->historylist, element)) {
            cache->num_compulsory_misses++;

            list_add_back(cache->historylist, element);
        }
    }

    // add the element to the back of the list
    list_add_back(cache->list, element);

    return 0; // return false if the element was NOT in the cache
}

// Function to insert into Clock-style cache
int insert_clock(Cache *cache, void *element) {
    if (cache == NULL || element == NULL) {
        return -1; // Error
    }

    // Check if the element is in the cache
    Node *node = cache->list->head;
    while (node != NULL) {
        if (node->data == element) {
            // HIT
            node->ref_bit = 1; // Set referenced bit for Clock policy
            printf("HIT\n");
            return 1;
        }
        node = node->next;
    }

    // MISS
    printf("MISS\n");

    // check if the list is full
    if (list_is_full(cache->list)) {
        Node *item = list_get_element_at_index(cache->list, cache->clock_pointer);

        while (item->ref_bit == 1) {
            item->ref_bit = 0;
            cache->clock_pointer = (cache->clock_pointer + 1) % list_get_max_size(cache->list);
            item = list_get_element_at_index(cache->list, cache->clock_pointer);
        }

        list_overwrite_at_index(cache->list, cache->clock_pointer, element);
        cache->clock_pointer = (cache->clock_pointer + 1) % list_get_max_size(cache->list);

        // check if the history list contains the element
        if (!list_contains(cache->historylist, element)) {
            cache->num_compulsory_misses++;

            list_add_back(cache->historylist, element);
        } else {
            cache->num_capacity_misses++;
        }

        return 0; // return false if the element was NOT in the cache
    } else {
        // check if the history list contains the element
        if (!list_contains(cache->historylist, element)) {
            cache->num_compulsory_misses++;

            // if the list is not full - add the element to the back of the list (if it's a miss)
            list_add_back(cache->historylist, element);
        }
    }

    // if the list is not full - add the element to the back of the list (if it's a miss)
    list_add_back(cache->list, element);

    return 0; // return false if the element was NOT in the cache
}

// Function to free the memory used by the cache
void free_cache(Cache *cache) {
    if (cache == NULL) {
        return;
    }

    // Free the memory for the cache list
    list_free(cache->list);

    // Free the memory for the history list
    list_free(cache->historylist);

    // Free the memory for the cache itself
    free(cache);
}

// Function to check if the list is full
int list_is_full(List *list) {
    return list->size == list->max_size;
}

// Function to add a node to the back of the list
void list_add_back(List *list, void *data) {
    if (list == NULL || data == NULL) {
        return;
    }

    Node *new_node = (Node *) malloc(sizeof(Node));
    if (new_node == NULL) {
        perror("Failed to allocate memory for a new node");
        exit(EXIT_FAILURE);
    }

    new_node->data = data;
    new_node->next = NULL;

    if (list->head == NULL) {
        list->head = new_node;
    } else {
        Node *current = list->head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
    }

    list->size++;
}

// Function to remove the front node of the list
void list_remove_front(List *list) {
    if (list == NULL || list->head == NULL) {
        return;
    }

    Node *front = list->head;
    list->head = front->next;
    free(front);

    list->size--;
}

// Function to check if the list contains a specific element
int list_contains(List *list, void *element) {
    if (list == NULL || element == NULL) {
        return 0;
    }

    Node *current = list->head;
    while (current != NULL) {
        if (current->data == element) {
            return 1; // Element found in the list
        }
        current = current->next;
    }

    return 0; // Element not found in the list
}

// Function to move a specific element to the end of the list
void list_move_to_end(List *list, void *element) {
    if (list == NULL || element == NULL || list->head == NULL) {
        return;
    }

    Node *current = list->head;
    Node *prev = NULL;

    // Find the node with the specified element
    while (current != NULL && current->data != element) {
        prev = current;
        current = current->next;
    }

    // If the element is found, move it to the end of the list
    if (current != NULL) {
        if (prev != NULL) {
            prev->next = current->next;
        } else {
            list->head = current->next;
        }

        current->next = NULL;

        // Add the node to the back of the list
        list_add_back(list, element);
    }
}

// Function to search for an element in the list and return its corresponding node
Node *list_search_for_element(List *list, void *element) {
    if (list == NULL || element == NULL) {
        return NULL;
    }

    Node *current = list->head;
    while (current != NULL) {
        if (current->data == element) {
            return current; // Return the node with the specified element
        }
        current = current->next;
    }

    return NULL; // Element not found in the list
}

// Function to get the element at a specific index in the list
Node *list_get_element_at_index(List *list, int index) {
    if (list == NULL || index < 0 || index >= list->size) {
        return NULL;
    }

    Node *current = list->head;
    int count = 0;

    // Traverse the list to the specified index
    while (current != NULL && count < index) {
        current = current->next;
        count++;
    }

    return current; // Return the node at the specified index
}

// Function to overwrite the data at a specific index in the list
void list_overwrite_at_index(List *list, int index, void *data) {
    if (list == NULL || index < 0 || index >= list->size || data == NULL) {
        return;
    }

    Node *current = list->head;
    int count = 0;

    // Traverse the list to the specified index
    while (current != NULL && count < index) {
        current = current->next;
        count++;
    }

    if (current != NULL) {
        current->data = data;
    }
}

// Function to get the maximum size of the list
int list_get_max_size(List *list) {
    if (list == NULL) {
        return -1;
    }

    return list->max_size;
}

// Function to free the memory used by the list
void list_free(List *list) {
    if (list == NULL) {
        return;
    }

    Node *current = list->head;
    Node *next;

    // Free each node in the list
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }

    // Free the memory for the list itself
    free(list);
}

// Main function
int main(int argc, char *argv[]) {
    // Parse command line arguments
    int size = 0;
    Policy policy = FIFO;

    if (argc == 3 && argv[1][0] == '-' && argv[1][1] == 'N') {
        size = atoi(argv[2]);
    } else if (argc == 2) {
        size = 0; // Default size
    } else {
        fprintf(stderr, "Usage: %s [-N size] <policy>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (argc == 3 || argc == 4) {
        if (argv[argc - 1][0] == '-') {
            fprintf(stderr, "Invalid policy: %s\n", argv[argc - 1]);
            exit(EXIT_FAILURE);
        }

        if (argv[argc - 1][0] == 'F') {
            policy = FIFO;
        } else if (argv[argc - 1][0] == 'L') {
            policy = LRU;
        } else if (argv[argc - 1][0] == 'C') {
            policy = Clock;
        } else {
            fprintf(stderr, "Invalid policy: %s\n", argv[argc - 1]);
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "Usage: %s [-N size] <policy>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Initialize cache
    Cache *cache = init_cache(size, policy);

    // Read items from stdin
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        // Remove newline character from the end
        buffer[strcspn(buffer, "\n")] = '\0';

        // Insert the item into the cache
        switch (policy) {
        case FIFO: insert_fifo(cache, buffer); break;
        case LRU: insert_lru(cache, buffer); break;
        case Clock: insert_clock(cache, buffer); break;
        default:
            // Handle invalid policy
            break;
        }
    }

    // Print summary
    printf("Compulsory Misses: %d\n", cache->num_compulsory_misses);
    printf("Capacity Misses: %d\n", cache->num_capacity_misses);

    // Free memory used by the cache
    free_cache(cache);

    return 0;
}
