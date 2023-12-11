#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Help recieved from TA Mitchell pseudocode and help recieved from classmate - all help given on whiteboard no code was shared

// enum policies
typedef enum { FIFO, LRU, Clock } Policy;

typedef struct Node {
    char *data; // Was void instead of char previosly
    struct Node *next;
    int ref_bit; // Only used in Clock policy
} Node;

typedef struct List {
    Node *head;
    int size;
    int max_size;
} List;

typedef struct Cache {
    Policy policy;
    List *list;
    int clock_pointer;
    List *historylist; // List of items that are being evicted

    int num_compulsory_misses;
    int num_capacity_misses;
} Cache;

int insert_fifo(Cache *cache, void *element);
int insert_lru(Cache *cache, void *element);
int insert_clock(Cache *cache, void *element);
void free_cache(Cache *cache);

int list_is_full(List *list);
void list_add_back(List *list, char *data);
void list_remove_front(List *list);
int list_contains_TF(List *list, char *element);
Node *list_search_for_element(List *list, char *element);
void list_move_to_end(List *list, char *element);
Node *list_get_element_at_index(List *list, int index);
void list_overwrite_at_index(List *list, int index, char *data);
int list_get_max_size(List *list);
void free_list(List *list);

// void print_list(List *list) {
//     if (list == NULL || list->head == NULL) {
//         printf("List is empty\n");
//         return;
//     }

//     Node *current = list->head;
//     while (current != NULL) {
//         printf("%s, ", current->data); // Assuming the data is of type char*
//         current = current->next;
//     }
//     printf("\n");
// }

int insert_fifo(Cache *cache, void *element) {
    if (cache == NULL || element == NULL) {
        return -1; // Error
    }

    // Check if the element is in the cache
    if (list_contains_TF(cache->list, element) == 1) {
        return 1;
    }

    // check if the list is full
    if (list_is_full(cache->list)) {
        list_remove_front(cache->list);

        // check if the history list contains the element
        if (!list_contains_TF(cache->historylist, element)) {
            cache->num_compulsory_misses++;

            list_add_back(cache->historylist, element);
        } else {
            cache->num_capacity_misses++;
        }
    } else {
        // check if the history list contains the element
        if (!list_contains_TF(cache->historylist, element)) {
            cache->num_compulsory_misses++;

            list_add_back(cache->historylist, element);
        }
        // no posibility for it to be a capacity miss
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
    if (list_contains_TF(cache->list, element) == 1) {
        list_move_to_end(cache->list, element);

        return 1;
    }

    // check if the list is full
    if (list_is_full(cache->list)) {
        list_remove_front(cache->list);

        // check if the history list contains the element
        if (list_contains_TF(cache->historylist, element) == 0) {
            cache->num_compulsory_misses++;

            list_add_back(cache->historylist, element);
        } else {
            cache->num_capacity_misses++;
        }
    } else {
        // check if the history list contains the element
        if (list_contains_TF(cache->historylist, element) == 0) {
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

    // check if the cache list contains the element (if it's a HIT)
    if (list_contains_TF(cache->list, element) == 1) {
        Node *item = list_search_for_element(cache->list, element);

        if (item == NULL) {
            return -1; // exit with an error
        }

        item->ref_bit = 1;
        //print_list(cache->list);
        return 1; // return true if the elemtn is in the cache
    }

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
        if (list_contains_TF(cache->historylist, element) == 0) {
            cache->num_compulsory_misses++;

            list_add_back(cache->historylist, element);
        } else {
            cache->num_capacity_misses++;
        }
        //print_list(cache->list);
        return 0; // return false if the element was NOT in the cache

    } else {
        // check if the history list contains the element
        if (list_contains_TF(cache->historylist, element) == 0) {
            cache->num_compulsory_misses++;

            // if the list is not full - add the element to the back of the list (if it's a miss)
            list_add_back(cache->historylist, element);
        }
    }

    // if the list is not full - add the element to the back of the list (if it's a miss)
    list_add_back(cache->list, element);

    //print_list(cache->list);
    return 0; // return false if the element was NOT in the cache
}

void free_cache(Cache *cache) {
    if (cache == NULL) {
        return;
    }

    free_list(cache->list);
    free_list(cache->historylist);

    // Free the memory for the cache itself
    free(cache);
}

int list_is_full(List *list) {
    return list->size == list->max_size;
}

void list_add_back(List *list, char *data) {
    if (list == NULL || data == NULL) {
        return; // NULL error
    }

    // Duplicate the data
    char *new_data = strdup(data);
    if (new_data == NULL) {
        perror("Failed to allocate memory for a new node");
        exit(EXIT_FAILURE);
    }

    // Allocate memory for the new node
    Node *new_node = (Node *) malloc(sizeof(Node));
    if (new_node == NULL) {
        perror("Failed to allocate memory for a new node");
        free(new_data); // Allocated new_data before so if there is failure then free this as well
        exit(EXIT_FAILURE);
    }

    new_node->data = new_data;
    new_node->next = NULL;

    // Check if the list is empty
    if (list->head == NULL) {
        // If the list is empty, the new node becomes the head of the list
        list->head = new_node;
    } else {
        // If the list is not empty, go to the end to add the new node
        Node *current = list->head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
    }

    list->size++;
}

void list_remove_front(List *list) {
    if (list == NULL || list->head == NULL) {
        return;
    }

    Node *front = list->head;
    list->head = front->next;

    // Free the data and the node
    free(front->data);
    free(front);

    list->size--;
}

// Function to check if the list contains a specific element (returns 1 or 0)
int list_contains_TF(List *list, char *element) {
    if (list == NULL || element == NULL) {
        return -1;
    }

    Node *current = list->head;
    while (current != NULL) {
        // Using strcmp for string comparison
        if (strcmp(current->data, element) == 0) {
            return 1; // Element found in the list
        }
        current = current->next;
    }

    return 0; // Element not found in the list
}

// Function to search for an element in the list and returns the node
Node *list_search_for_element(List *list, char *element) {
    if (list == NULL || element == NULL) {
        return NULL;
    }

    Node *current = list->head;
    while (current != NULL) {
        if (strcmp(current->data, element) == 0) {
            return current;
        }
        current = current->next;
    }

    return NULL;
}

void list_move_to_end(List *list, char *element) {
    if (list == NULL || element == NULL || list->head == NULL) {
        return;
    }

    Node *current = list->head;
    Node *prev = NULL;

    // Finds the node
    while (current != NULL && strcmp(current->data, element) != 0) {
        prev = current;
        current = current->next;
    }

    // If the element is found and it's not already at the end, move it to the end of the list
    if (current != NULL && current->next != NULL) {
        if (prev != NULL) {
            prev->next = current->next;
        } else {
            list->head = current->next;
        }

        // Find the last node in the list
        Node *last = list->head;
        while (last->next != NULL) {
            last = last->next;
        }

        // Add the found node to the back of the list
        if (last != NULL) {
            last->next = current;
            current->next = NULL;
        }
    }
}

Node *list_get_element_at_index(List *list, int index) {
    if (list == NULL || index < 0 || index > list->size) {
        return NULL;
    }

    Node *current = list->head;
    int count = 0;

    // Traverse the list to the given index
    while (current != NULL && count < index) {
        current = current->next;
        count++;
    }

    return current;
}

void list_overwrite_at_index(List *list, int index, char *data) {
    if (list == NULL || index < 0 || index >= list->size || data == NULL) {
        return;
    }

    Node *current = list->head;
    int count = 0;

    // Traverse the list to the given index
    while (current != NULL && count < index) {
        current = current->next;
        count++;
    }

    if (current != NULL) {
        // Free the original data
        free(current->data);

        // Duplicate the new data and assign it to the node
        current->data = strdup(data);

        if (current->data == NULL) {
            perror("Failed to assign duplicate data");
            return;
        }
    }
}

int list_get_max_size(List *list) {
    if (list == NULL) {
        return -1;
    }

    return list->max_size;
}

void free_list(List *list) {
    if (list == NULL) {
        return;
    }

    Node *current = list->head;
    Node *next;

    // Free each node in the list
    while (current != NULL) {
        next = current->next;
        free(current->data);
        free(current);
        current = next;
    }

    list->head = NULL;

    // Free the memory for the list itself
    free(list);
}

int main(int argc, char *argv[]) {
    Policy policy = FIFO; // Default is FIFO

    // Parse command line arguments
    if (argc < 3 || argc > 4) {
        fprintf(stderr, "Usage: %s [-N size] <policy>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[1], "-N") != 0) {
        fprintf(stderr, "Cache size error\n");
        return -1;
    }

    int size = atoi(argv[2]);

    if (size < 1) {
        fprintf(stderr, "Cache size error\n");
        return -1;
    }

    // Assigns policy
    const char *policy_str = argv[3];
    if (strcmp(policy_str, "-F") == 0) {
        policy = FIFO;
    } else if (strcmp(policy_str, "-L") == 0) {
        policy = LRU;
    } else if (strcmp(policy_str, "-C") == 0) {
        policy = Clock;
    } else {
        fprintf(stderr, "Invalid policy\n");
        return -1;
    }

    // Initialize cache
    Cache *cache = (Cache *) malloc(sizeof(Cache));
    if (cache == NULL) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    // Set initial values
    cache->policy = policy;
    cache->list = (List *) malloc(sizeof(List));
    cache->list->head = NULL;
    cache->list->size = 0;
    cache->list->max_size = size;

    cache->historylist = (List *) malloc(sizeof(List));
    cache->historylist->head = NULL;
    cache->historylist->size = 0;
    cache->historylist->max_size = 0;

    cache->num_compulsory_misses = 0;
    cache->num_capacity_misses = 0;
    cache->clock_pointer = 0;

    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        // Remove newline character from the end
        buffer[strcspn(buffer, "\n")] = '\0';

        int result = -1;
        switch (policy) {
        case FIFO: result = insert_fifo(cache, buffer); break;

        case LRU: result = insert_lru(cache, buffer); break;

        case Clock: result = insert_clock(cache, buffer); break;

        default:
            perror("Policy Error");
            exit(EXIT_FAILURE);
            break;
        }

        // Print HIT or MISS based on the result
        if (result == 1) {
            printf("HIT\n");
        } else if (result == 0) {
            printf("MISS\n");
        } else {
            printf("NULL error");
        }
    }

    // Print summary
    printf("%d %d\n", cache->num_compulsory_misses, cache->num_capacity_misses);

    // Free memory used by the cache
    free_cache(cache);

    return 0;
}
