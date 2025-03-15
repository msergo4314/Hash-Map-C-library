
#ifndef HASHMAP_H
#define HASHMAP_H

/* A library for flexible hashmaps that can support any combination of keys and values as long as they are valid options of the  DATA_TYPE enum 
*  ALL KEYS MUST HAVE THE SAME DATATYPE FOR ANY SINGLE HASHMAP. However, values can be any valid datatype in the same hashmap
*  Hash maps will resize dynamically depending on the load factor when insertions/deletions occur, so the user does not need to worry about resizing
*/

#include <stdio.h> // printing
#include <stdlib.h> // malloc and free
#include <stdbool.h> // function definitions
#include <string.h> // strdup mostly

#define MIN_LOAD_FACTOR (0.125)
#define MAX_LOAD_FACTOR (0.75)

// potential data types we can have
typedef enum {
    INTEGER_TYPE,
    STRING_TYPE,
    FLOAT_TYPE,
    DOUBLE_TYPE,
    CUSTOM_TYPE, // not used here
    // this is only for when we return structs that have DATA_TYPES and there was an error (see to_key() and to_value())
    INVALID_TYPE = -1 
} DATA_TYPE;


// a datapoint can be any type but only ever one of them
typedef union {
    int integer;
    char *string;
    float float_value;
    double double_value;
} Data;

/* KEY and VALUE structs */

// Keys are used in lookups to find associated values
typedef struct key {
    DATA_TYPE type;
    Data data;
} Key;

// values are any data associated with a key
typedef struct {
    DATA_TYPE type;
    Data data;
} Value;

/* An entry in a hash map has a key and a value and a pointer to next entry (if collisions occur) */
typedef struct Entry {
    Key key;
    Value value;
    struct Entry *next; // For handling collisions via chaining (linked list)
} Entry;

// a struct for each hash map that stores the functions we will use (depends on key type)
typedef struct {
    size_t (*hash_func)(const Key *key); // hash functions will take any key
    int (*cmp_func)(const Key *a, const Key *b); // comparison operations will take any two keys OF THE SAME TYPE
} Key_ops;

// HashMap structure definition
typedef struct {
    Entry **buckets; // pointer to list of buckets
    size_t bucket_count; // how many buckets can be filled at most
    Key_ops key_ops; // the two functions we will be using for hasing/comparison
    DATA_TYPE key_type; // the type of key the hash map has (see enum)
    size_t key_count; // number of keys currently in the table
} HashMap;

// returns current load factor (how much space is being used)
float get_hash_table_load_factor(const HashMap *map) {
    if (map == NULL) {
        perror("Passed in NULL hash map to get_hash_map_load_factor() function!\n");
    }
    // divide number of entries by number of buckets
    return ((float)(map->key_count) / (float)(map->bucket_count)); 
}

/* HASHING FUNCTIONS */

/* NOTE: THESE FUNCTIONS ASSUME THE POINTERS ARE VALID */

size_t hash_int(const Key *key) {
    return key->data.integer;
}

size_t hash_string(const Key *key) {
    size_t hash = 5381;
    char *str = key->data.string;
    while (*str) {
        hash = ((hash << 5) + hash) + *str;
        str++;
    }
    return hash;
}

size_t hash_float(const Key *key) {
    size_t hash;
    memcpy(&hash, &key->data.float_value, sizeof(float));
    return hash;
}

size_t hash_double(const Key *key) {
    size_t hash;
    memcpy(&hash, &key->data.double_value, sizeof(double));
    return hash;
}

/* COMPARISON FUNCTIONS */
/* NOTE: THESE FUNCTIONS ASSUME THE POINTERS ARE VALID */

int cmp_int(const Key *a, const Key *b) {
    return a->data.integer - b->data.integer;
}

int cmp_string(const Key *a, const Key *b) {
    return strcmp(a->data.string, b->data.string);
}

#define FLOAT_EPSILON 1e-6
int cmp_float(const Key *a, const Key *b) {
    float diff = a->data.float_value - b->data.float_value;
    if (diff > FLOAT_EPSILON) return 1;
    if (diff < -FLOAT_EPSILON) return -1;
    return 0;
}

#define DOUBLE_EPSILON 1e-9
int cmp_double(const Key *a, const Key *b) {
    double diff = a->data.double_value - b->data.double_value;
    if (diff > DOUBLE_EPSILON) return 1;
    if (diff < -DOUBLE_EPSILON) return -1;
    return 0;
}

// this function takes a pointer to a map that has already been created (non-null). Useful for a stack-allocated map.
// The fields of the hash map itself are still allocated on the heap.

// void hash_table_init(HashMap *map, size_t desired_size, DATA_TYPE key_type) {
//     if (map == NULL) {
//         perror("Passed in NULL pointer to hash_table_init function. Pass a valid pointer!\n");
//         return;
//     }
//     map->key_count = 0;
//     map->bucket_count = desired_size;
//     map->buckets = (Entry **)calloc(desired_size, sizeof(Entry *)); // calloc sets pointers to NULL
//     if (map->buckets == NULL) {
//         perror("Could not calloc space for map buckets list in hash_table_init(). Aborting\n");
//         return;
//     }
//     map->key_type = key_type;
//     switch (key_type) {
//         case INTEGER_TYPE:
//             map->key_ops.hash_func = hash_int;
//             map->key_ops.cmp_func = cmp_int;
//         break;
//         case STRING_TYPE:
//             map->key_ops.hash_func = hash_string;
//             map->key_ops.cmp_func = cmp_string;
//         break;
//         case FLOAT_TYPE:
//             map->key_ops.hash_func = hash_float;
//             map->key_ops.cmp_func = cmp_float;
//         break;
//         case DOUBLE_TYPE:
//             map->key_ops.hash_func = hash_double;
//             map->key_ops.cmp_func = cmp_double;
//         break;
//         default:
//             printf("Must have one of the following datatypes: int, string (char *), float, double\n");
//             free(map->buckets);
//             map->buckets = NULL;
//             return;
//     }
//     return;
// }

// returns the hash map structure itself on success, else NULL. The map itself is stored on the heap
HashMap *hash_table_create(size_t desired_size, DATA_TYPE key_type) {
    if (desired_size == 0) {
        perror("Error in hash_table_create() function: cannot create a hash map with a size of 0\n");
        return NULL;
    }
    HashMap *new_map = malloc(sizeof(HashMap));
    if (new_map == NULL) {
        perror("Could not malloc the hash map itself in hash_table_init() function!\n");
        return NULL;
    }

    new_map->bucket_count = desired_size;
    new_map->buckets = (Entry **)calloc(desired_size, sizeof(Entry *));
    if (new_map->buckets == NULL) {
        free(new_map);
        perror("new hash map buckets array could not be calloc'd in hash_table_create() function!\n");
        return NULL;
    }
    new_map->key_type = key_type;
    switch (key_type) {
        case INTEGER_TYPE:
            new_map->key_ops.hash_func = hash_int;
            new_map->key_ops.cmp_func = cmp_int;
        break;
        case STRING_TYPE:
            new_map->key_ops.hash_func = hash_string;
            new_map->key_ops.cmp_func = cmp_string;
        break;
        case FLOAT_TYPE:
            new_map->key_ops.hash_func = hash_float;
            new_map->key_ops.cmp_func = cmp_float;
        break;
        case DOUBLE_TYPE:
            new_map->key_ops.hash_func = hash_double;
            new_map->key_ops.cmp_func = cmp_double;
        break;
        default:
            printf("Must have one of the following datatypes: int, string (char *), float, double\n");
            free(new_map->buckets);
            new_map->buckets = NULL;
            free(new_map);
            return NULL;
    }
    new_map->key_count = 0;
    return new_map;
}

// a function that increases/decreases the size of the hashMap
bool hash_table_resize(HashMap *map, size_t new_buckets_count) {
    // printf("INVOKED RESIZE HERE\n");
    if (map == NULL ) {
        perror("Null map passed in to hash_table_resize() function!\n");
        return false;
    }
    if (new_buckets_count == 0) {
        perror("cannot resize a hash map to have 0 buckets!\n");
        return false;
    }

    // Allocate a new bucket array with the new size
    Entry **new_buckets = calloc(new_buckets_count, sizeof(Entry *));
    if (new_buckets == NULL) {
        perror("Memory allocation failed while resizing hash table in hash_table_resize()!\n");
        return false;
    }

    // Rehash each entry into the new bucket array
    for (size_t i = 0; i < map->bucket_count; i++) {
        Entry *current = map->buckets[i];
        while (current) {
            Entry *next_entry = current->next; // Save next pointer before moving

            // Compute new bucket index
            size_t new_index = map->key_ops.hash_func(&(current->key)) % new_buckets_count;

            // Insert entry into new bucket array
            current->next = new_buckets[new_index];
            new_buckets[new_index] = current;

            // Move to the next entry in the old bucket
            current = next_entry;
        }
    }

    // Free the old bucket array (but not the entries, as they were moved to the new block)
    free(map->buckets);

    // Update the hashmap with new bucket array and size
    map->buckets = new_buckets;
    map->bucket_count = new_buckets_count;
    return true;
}

// this function doubles as an update function since it replaces key data if the key is already present
bool hash_table_insert(HashMap *map, const Key *key, const Value *value) {
    if (map == NULL) {
        perror("Map is NULL in hash table insert function!\n");
        return false;
    }
    if (key == NULL || value == NULL) {
        if (!key) {
            perror("Passed in NULL key to hash_table_insert() function!\n");
        } else {
            perror("Passed in NULL value to hash_table_insert() function!\n");
        }
        return false;
    }
    if (key->type != map->key_type) {
        fprintf(stderr, "You cannot insert a key of type %d into a hash map that uses keys of type %d!\n", key->type, map->key_type);
        return false;
    }
    if (map->bucket_count == 0) {
        perror("cannot insert into a map with 0 buckets!\n");
        return false;
    }
    // make sure that the hash fits in the table with modulus
    size_t hash = map->key_ops.hash_func(key);
    hash = hash % (map->bucket_count);

    if (map->buckets == NULL) {
        perror("Hash table is uninitialized!\n");
        return false;   
    }
    Entry *current = (map->buckets)[hash];
    while (current != NULL) {
        // case where we already have this exact key in the hashmap -- replace the data!
        if (map->key_ops.cmp_func(&(current->key), key) == 0) {
            // we already have an exact match of keys in this case, so no need to replace the key data
            if (current->value.type == STRING_TYPE) {
                // free the old string that was malloc'd if the value was a string
                free(current->value.data.string);
                current->value.data.string = NULL;
            }
            if(value->type == STRING_TYPE) {
                current->value.data.string = strdup(value->data.string);
                if (current->value.data.string == NULL) {
                    perror("strdup() failed to malloc a new string in hash_table_insert() function!\n");
                    return false;
                }
            } else {
                // safe to copy structs if there are no pointers
                current->value = *value;
            }
            // make sure the new type of the value is correct if it changed
            current->value.type = value->type;
            return true;
        }
        current = current->next;
    }

    // Key not found, add a new entry
    Entry *new_node = (Entry *)malloc(sizeof(Entry));
    if (new_node == NULL) {
        perror("Could not malloc a new node for hash table insertion!\n");
        return false;
    }
    // copy key type
    new_node->key.type = key->type;

    // strings should be copied with strdup, other datatypes can just be copied directly
    if (key->type == STRING_TYPE) {
        new_node->key.data.string = (char *)strdup(key->data.string);
        if (new_node->key.data.string == NULL) {
            perror("strdup failed for key string data in hash_table_insert()!\n");
            free(new_node);
            return false;
        }
    } else {
        new_node->key.data = key->data;
    }

    // Copy value type and data
    new_node->value.type = value->type;
    if (value->type == STRING_TYPE) {
        new_node->value.data.string = (char *)strdup(value->data.string);
        if (new_node->value.data.string == NULL) {
            perror("strdup failed for value string data!\n");
            free(new_node->key.data.string); // Free key string
            free(new_node);
            return false;
        }
    } else {
        new_node->value.data = value->data;
    }

    new_node->next = map->buckets[hash];
    map->buckets[hash] = new_node;
    (map->key_count)++;
    float load_factor = get_hash_table_load_factor(map);
    if (load_factor > MAX_LOAD_FACTOR) {
        hash_table_resize(map, (map->bucket_count) * 2);
    }
    return true;
}

// return pointer to the entry if success, else NULL
Entry *hash_table_entry_lookup(const HashMap *map, const Key *key_to_search_for) {
    if (map == NULL) {
        perror("HashMap is NULL\n");
        return NULL;
    }
    if (map->bucket_count == 0) {  // Edge case: No buckets in the map
        return NULL;
    }
    if (key_to_search_for == NULL) {
        perror("Key passed into hash_table_entry_lookup() is NULL!\n");
        return NULL;
    }
    if (key_to_search_for->type != map->key_type) {
        fprintf(stderr, "Key passed into hash_table_entry_lookup() has the wrong key type! Expected %d, got %d\n", map->key_type, key_to_search_for->type);
        return NULL;
    }

    size_t hash = map->key_ops.hash_func(key_to_search_for);
    hash = hash % (map->bucket_count);

    Entry *current = (map->buckets)[hash];
    while (current != NULL) {
        if (map->key_ops.cmp_func(&(current->key), key_to_search_for) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// returns true if key exists, else false
bool hash_table_contains(const HashMap *map, const Key *key) {
    if (!map) {
        perror("passed in NULL hashmap into hash_table_contains() function!\n");
        return false;
    }
    if (!key) {
        perror("passed in NULL key into hash_table_contains() function!\n");
        return false;
    }
    return (hash_table_entry_lookup(map, key) != NULL);
}

// returns true on deletion else false.
bool hash_table_entry_delete(HashMap *map, const Key *key_to_delete) {
    if (map == NULL) {
        perror("HashMap is NULL\n");
        return false;
    }
    if (map->bucket_count == 0) {  // Edge case: No buckets in the map
        return false;
    }
    if (key_to_delete == NULL) {
        perror("Key to delete in hash_table_entry_delete() is NULL!\n");
        return false;
    }
    if (key_to_delete->type != map->key_type) {
        fprintf(stderr, "Key passed into hash_table_entry_delete() has the wrong key type! Expected %d, got %d\n", map->key_type, key_to_delete->type);
        return false;
    }

    size_t hash = map->key_ops.hash_func(key_to_delete);
    hash = hash % map->bucket_count;  // Ensure valid index

    Entry *current = map->buckets[hash];
    Entry *prev = NULL;

    while (current != NULL) {
        // Compare the current entry's key with the key to delete
        if (map->key_ops.cmp_func(&(current->key), key_to_delete) == 0) {
            // Key matched, proceed to delete
            if (current->key.type == STRING_TYPE) {
                free(current->key.data.string);
            }
            if (current->value.type == STRING_TYPE) {
                free(current->value.data.string);
            }

            // If the key to delete is at the head of the list
            if (prev == NULL) {
                map->buckets[hash] = current->next;
            } else {
                prev->next = current->next;
            }

            free(current);  // Free the entry itself
            (map->key_count)--; // decrement key count
            
            float load_factor = get_hash_table_load_factor(map);
            // do not want to reduce the hash map size below 10
            if ((load_factor < MIN_LOAD_FACTOR && ((map->bucket_count) >= 20))) {
                hash_table_resize(map, (map->bucket_count * 3) / 4);
            }
            return true;    // Successfully deleted
        }
        prev = current;
        current = current->next;
    }

    return false;  // Key not found in the hash table
}

// Frees the entire hashMap and sets the original pointer to NULL
bool hash_table_destroy(HashMap **map) {
    if (map == NULL || *map == NULL) {
        perror("HashMap to destroy is NULL!\n");
        return false;
    }

    // Iterate through each bucket
    for (size_t i = 0; i < (*map)->bucket_count; i++) {
        Entry *current_node = (*map)->buckets[i];
        while (current_node != NULL) {
            // Free the key and value data if they are strings
            if (current_node->key.type == STRING_TYPE) {
                free(current_node->key.data.string);
            }
            if (current_node->value.type == STRING_TYPE) {
                free(current_node->value.data.string);
            }

            // Free the current entry and move to the next one
            Entry *next_node = current_node->next;
            free(current_node);
            current_node = next_node;
        }
        // After freeing all nodes in this bucket, set the bucket to NULL
        (*map)->buckets[i] = NULL;
    }

    // Finally, free the bucket array and the hash map itself
    free((*map)->buckets);
    (*map)->buckets = NULL; // not necessary since map itself is free'd

    free(*map);  // Free the hash map structure itself
    *map = NULL; // Set the original pointer to NULL

    return true;
}

// removes all data from the hashmap, but does not destroy it. True on success, else false
bool hash_table_clear(HashMap *map) {
    if (map == NULL) {
        perror("Passed in NULL HashMap to hash_table_clear() function!\n");
        return false;
    }
    Entry *current_bucket, *next_bucket = NULL;
    for (size_t i = 0; i < map->bucket_count; i++) {
        current_bucket = map->buckets[i];
        while (current_bucket) {
            // Free the key if it is a string
            // alternate option: if map->key_type == STRING_TYPE
            if ((current_bucket->key).type == STRING_TYPE) {
                free((current_bucket->key).data.string);
                current_bucket->key.data.string = NULL;
            }
            // Free the value if it is a string
            if ((current_bucket->value).type == STRING_TYPE) {
                free((current_bucket->value).data.string);
                (current_bucket->value).data.string = NULL;
            }
            next_bucket = current_bucket->next;  // Store the next entry
            free(current_bucket);                // Free the current entry
            current_bucket = next_bucket;        // Move to the next entry
        }
        (map->buckets)[i] = NULL; // set the pointer to be NULL to indicate no mappings
    }
    map->key_count = 0; // number of buckets in unchanged (map->buckets was not altered), but no more keys are held
    return true;
}

// prints the hash table contents to stdout
void hash_table_print(const HashMap *map) {
    if (map == NULL) {
        perror("attempted to print a NULL hash map in hash_table_print() function!\n");
        return;
    }
    if (map->bucket_count == 0) {
        printf("Hash Map has no buckets\n");
        return;
    }
    if (map->buckets == NULL) {
        perror("map has NULL buckets list in hash_table_print() function!\n");
        return;
    }

    Entry *current_entry = NULL;
    printf("--start of hash table--\n");
    for (size_t i = 0; i < map->bucket_count; i++) {
        current_entry = (map->buckets)[i];
        if (current_entry != NULL) printf("Bucket #%u:\n", (unsigned int)i);
        while(current_entry) {
            // print key
            switch(current_entry->key.type) {
                // ensure padding is consistent here
                case INTEGER_TYPE:
                    printf("%-40d\t | \t", current_entry->key.data.integer);
                    break;
                case STRING_TYPE:
                    printf("%-40s\t | \t", current_entry->key.data.string);
                    break;
                case FLOAT_TYPE:
                    printf("%-40.6f\t | \t", current_entry->key.data.float_value);
                    break;
                case DOUBLE_TYPE:
                    printf("%-40.6f\t | \t", current_entry->key.data.double_value);
                    break;
                default:
                    printf("Unknonwn data type detected for Key in bucket #%u\n", (unsigned int)i);
            }

            // print corresponding value
            switch(current_entry->value.type) {
                // ensure padding is consistent here
                case INTEGER_TYPE:
                    printf("%-40d (type: int)\n", current_entry->value.data.integer);
                    break;
                case STRING_TYPE:
                    printf("%-40s (type: string)\n", current_entry->value.data.string);
                    break;
                case FLOAT_TYPE:
                    printf("%-40.6f (type: float)\n", current_entry->value.data.float_value);
                    break;
                case DOUBLE_TYPE:
                    printf("%-40.6f (type: double)\n", current_entry->value.data.double_value);
                    break;
                default:
                    printf("Unknown data type detected for Value in bucket #%u\n", (unsigned int)i);
            }
            current_entry = current_entry->next;
        }
    }
    printf("--end of hash table--\n");
    return;
}

// returns list of all keys in the hasmap or NULL on failure
Key *get_hash_table_keys(const HashMap *map) {
    if (map == NULL) {
        perror("passed NULL HashMap into get_hash_table_keys() function!\n");
        return NULL;
    }
    size_t number_of_keys = map->key_count;
    if (number_of_keys == 0) {
        return NULL; // No keys in the hash table
    }
    Key *array_of_keys = (Key *)malloc(sizeof(Key) * number_of_keys);
    if (array_of_keys == NULL) {
        perror("array of keys could not be malloc'd in get_hash_table_keys() function!\n");
        return NULL;
    }

    if (map->buckets == NULL) {
        free(array_of_keys);
        perror("The map passed into get_hash_table_keys() has a NULL list of buckets!\n");
        return NULL;
    }
    Entry *current_bucket = NULL;
    size_t keys_added_to_array = 0;

    for(size_t i = 0; i < map->bucket_count; i++) {
        current_bucket = (map->buckets)[i];
        while (current_bucket) {
            array_of_keys[keys_added_to_array] = current_bucket->key;

            /* in the case of the key being a string (char *), we need to duplicate the
            string with strdup instead of copying the pointer to the hash map string (which may be free'd later causing a dangling pointer) */
            if (current_bucket->key.type == STRING_TYPE) {
                array_of_keys[keys_added_to_array].data.string = strdup(current_bucket->key.data.string);
                if (array_of_keys[keys_added_to_array].data.string == NULL) {
                    perror("strdup for key string failed inside get_hash_table_keys() function!\n");
                    while (keys_added_to_array) { // do not use postincrement here -- could get underflow
                        keys_added_to_array--;
                        if (array_of_keys[keys_added_to_array].type == STRING_TYPE) {
                            free(array_of_keys[keys_added_to_array].data.string);
                        }
                    }
                    free(array_of_keys);
                    return NULL;
                }
            }
            keys_added_to_array++;
            current_bucket = current_bucket->next;
        }
    }
    return array_of_keys;
}

// returns list of all values in the hasmap or NULL on failure. Exact same as get_hash_table_keys but with value structs instead
Value *get_hash_table_values(const HashMap *map) {
    if (map == NULL) {
        perror("passed NULL HashMap into get_hash_table_values() function!\n");
        return NULL;
    }
    size_t number_of_values = map->key_count;
    if (number_of_values == 0) {
        return NULL; // No values in the hash table
    }
    if (map->buckets == NULL) {
        perror("The map passed into get_hash_table_values() has a NULL list of buckets!\n");
        return NULL;
    }
    Value *array_of_values = (Value *)malloc(sizeof(Value) * number_of_values);
    if (array_of_values == NULL) {
        perror("array of keys could not be malloc'd in get_hash_table_values() function!\n");
        return NULL;
    }
    Entry *current_bucket = NULL;
    size_t values_added_to_array = 0;

    for(size_t i = 0; i < map->bucket_count; i++) {
        current_bucket = (map->buckets)[i];
        while (current_bucket) {
            array_of_values[values_added_to_array] = current_bucket->value;

            /* in the case of the key being a string (char *), we need to duplicate the
            string with strdup instead of copying the pointer to the hash map string (which may be free'd later causing a dangling pointer) */
            if (current_bucket->key.type == STRING_TYPE) {
                array_of_values[values_added_to_array].data.string = strdup(current_bucket->value.data.string);
                if (array_of_values[values_added_to_array].data.string == NULL) {
                    perror("strdup for key string failed inside get_hash_table_values() function!\n");
                    while (values_added_to_array) { // do not use postincrement here -- could get underflow
                        values_added_to_array--;
                        if (array_of_values[values_added_to_array].type == STRING_TYPE) {
                            free(array_of_values[values_added_to_array].data.string);
                        }
                    }
                    free(array_of_values);
                    return NULL;
                }
            }
            values_added_to_array++;
            current_bucket = current_bucket->next;
        }
    }
    return array_of_values;
}

// converts any 1 dimensional array of a valid DATA_TYPE to an array of keys
Key *convert_array_to_keys(void *array, size_t number_of_elements, const DATA_TYPE type) {
    if (!array || number_of_elements == 0) return NULL;

    Key *keys = malloc(number_of_elements * sizeof(Key));
    if (keys == NULL) {
        perror("Malloc failed for keys in convert_to_keys function!\n");
        return NULL;
    }

    switch (type) {
        case INTEGER_TYPE: {
            int *int_array = (int *)array;
            for (size_t i = 0; i < number_of_elements; i++) {
                keys[i].type = type;
                keys[i].data.integer = int_array[i];
            }
            break;
        }
        case FLOAT_TYPE: {
            float *float_array = (float *)array;
            for (size_t i = 0; i < number_of_elements; i++) {
                keys[i].type = type;
                keys[i].data.float_value = float_array[i];
            }
            break;
        }
        case DOUBLE_TYPE: {
            double *double_array = (double *)array;
            for (size_t i = 0; i < number_of_elements; i++) {
                keys[i].type = type;
                keys[i].data.double_value = double_array[i];
            }
            break;
        }
        case STRING_TYPE: {
            char **string_array = (char **)array;
            for (size_t i = 0; i < number_of_elements; i++) {
                keys[i].type = type;
                keys[i].data.string = (char *)strdup(string_array[i]);
                if (!keys[i].data.string) {
                    // Cleanup in case of strdup failure
                    perror("Failed to duplicate string with strdup in convert_array_to_keys()!");
                    while (i > 0) {
                        i--;
                        free(keys[i].data.string);
                    }
                    free(keys);
                    return NULL;
                }
            }
            break;
        }
        default:
            fprintf(stderr, "Invalid data type %d detected in convert_array_to_keys()!\n", type);
            free(keys);
            return NULL;
    }

    return keys;
}

// converts any 1 dimensional array of a valid DATA_TYPE to an array of values
Value *convert_array_to_values(void *array, size_t number_of_elements, const DATA_TYPE type) {
    if (!array || number_of_elements == 0) return NULL;

    Value *values = malloc(number_of_elements * sizeof(Value));
    if (values == NULL) {
        perror("Malloc failed for values in convert_to_values function!\n");
        return NULL;
    }

    for (size_t i = 0; i < number_of_elements; i++) {
        values[i].type = type;
    }

    switch (type) {
        case INTEGER_TYPE: {
            int *int_array = (int *)array;
            for (size_t i = 0; i < number_of_elements; i++) {
                values[i].data.integer = int_array[i];
            }
            break;
        }
        case FLOAT_TYPE: {
            float *float_array = (float *)array;
            for (size_t i = 0; i < number_of_elements; i++) {
                values[i].data.float_value = float_array[i];
            }
            break;
        }
        case DOUBLE_TYPE: {
            double *double_array = (double *)array;
            for (size_t i = 0; i < number_of_elements; i++) {
                values[i].data.double_value = double_array[i];
            }
            break;
        }
        case STRING_TYPE: {
            char **string_array = (char **)array;
            for (size_t i = 0; i < number_of_elements; i++) {
                values[i].data.string = (char *)strdup(string_array[i]);
                if (!(values[i].data.string)) {
                    // Cleanup in case of strdup failure
                    perror("Failed to duplicate string with strdup in convert_array_to_values()!");
                    while (i > 0) {
                        --i;
                        free(values[i].data.string);
                    }
                    free(values);
                    return NULL;
                }
            }
            break;
        }
        default:
            fprintf(stderr, "Invalid data type %d detected in convert_array_to_values()!\n", type);
            free(values);
            return NULL;
    }

    return values;
}

// converts a primitive/string to a Key
Key to_key(const void *generic_pointer, const DATA_TYPE type) {
    Key new_key;
    new_key.type = type;
    if (generic_pointer == NULL) {
        perror("generic_pointer is NULL in to_key() function!\n");
        new_key.type = INVALID_TYPE;
        return new_key;
    }
    
    switch (type) {
        case INTEGER_TYPE:
            new_key.data.integer = *((const int *)generic_pointer);
            break;
        case STRING_TYPE:
            new_key.data.string = strdup(((const char *)generic_pointer));
            if (new_key.data.string == NULL) {
                perror("could not malloc with strdup in to_key function!\n");
            }
            break;
        case FLOAT_TYPE:
            new_key.data.float_value = *((const float *)generic_pointer);
            break;
        case DOUBLE_TYPE:
            new_key.data.double_value = *((const double *)generic_pointer);
            break;
        default:
            perror("Invalid data type for key/value conversion!\n");
            new_key.type = -1; // Indicate invalid type
            break;
    }
    return new_key;
}

// converts a primitive/string to a Value
Value to_value(const void *generic_pointer, const DATA_TYPE type) {
    Value new_value;
    new_value.type = type;
    if (generic_pointer == NULL) {
        perror("generic_pointer is NULL in to_key function!\n");
        new_value.type = INVALID_TYPE;
        return new_value;
    }
    
    switch (type) {
        case INTEGER_TYPE:
            new_value.data.integer = *((const int *)generic_pointer);
            break;
        case STRING_TYPE:
            new_value.data.string = strdup(((const char *)generic_pointer));
            if (new_value.data.string == NULL) {
                perror("could not malloc with strdup in to_key function!\n");
            }
            break;
        case FLOAT_TYPE:
            new_value.data.float_value = *((const float *)generic_pointer);
            break;
        case DOUBLE_TYPE:
            new_value.data.double_value = *((const double *)generic_pointer);
            break;
        default:
            perror("Invalid data type for key/value conversion!\n");
            new_value.type = INVALID_TYPE;
            break;
    }
    return new_value;
}

// deletes a STATICALLY ALLOCATED key with a malloc'd string
void delete_key(Key key_to_delete) {
    // free any strings if present
    if (key_to_delete.type == STRING_TYPE) {
        free(key_to_delete.data.string);
    }
    return;
}

// deletes a STATICALLY ALLOCATED value with a malloc'd string
void delete_value(Value value_to_delete) {
    // free any strings if present
    if (value_to_delete.type == STRING_TYPE) {
        free(value_to_delete.data.string);
    }
    return;
}

DATA_TYPE hash_table_get_key_type(const HashMap *map) {
    if (map == NULL) {
        perror("map is NULL in hash_table_get_key_type() function!\n");
        return INVALID_TYPE;
    }
    return map->key_type;
}

/* batch inserts a list of keys and list of corresponding values. True on success, else false
   assumes keys are unique, and if not the most recent key (lastest in the list of keys) will replace any old key that is idenctical
*/
bool hash_table_batch_insert(HashMap *map, void *array_of_keys, void *array_of_values, size_t number_of_elements, const DATA_TYPE key_type, const DATA_TYPE value_type) {
    // assume the two arrays have the same size. If not, problems will occur
    if (!map) {
        perror("map is NULL in hash_table_batch_insert() function!\n");
        return false;
    }
    if (number_of_elements == 0) {
        perror("tried to insert an array of size 0 in hash_table_batch_insert() function!\n");
        return false;
    }
    if (map->bucket_count == 0) {
        perror("cannot insert into a map with 0 buckets!\n");
        return false;
    }
    if (array_of_keys == NULL || array_of_values == NULL) {
        perror("passed in NULL arrays into hash_table_batch_insert() function!\n");
        return false;
    }
    if (key_type != map->key_type) {
        fprintf(stderr, "key type mismatch in hash_table_batch_insert() function! Expected %d, got %d\n", map->key_type, key_type);
        return false;
    }
    // note: these are malloc'd and must be free'd later
    Key *keys = convert_array_to_keys(array_of_keys, number_of_elements, key_type);
    Value *values = convert_array_to_values(array_of_values, number_of_elements, value_type);

    if (!keys || !values) {
        perror("failed to convert arrays into key/value arrays inside hash_table_batch_insert() function!");
        return false;
    }

    for (size_t i = 0; i < number_of_elements; i++) {
        if (!hash_table_insert(map, &(keys[i]), &(values[i]))) {
            fprintf(stderr, "Failed insertion for element %zu in hash_table_batch_insert() function!\n", i);
            delete_key((keys[i]));
            delete_value((values[i]));
            return false;
        }
        // if there are strings, make sure to free them inside the key/value structs
        delete_key((keys[i]));
        delete_value((values[i]));
    }
    free(keys);
    free(values);
    return true;
}

// batch deletions using a list of keys. The "strict_mode" parameter determines if the function returns false if any key is not deleted (was never in the hashmap)
bool hash_table_batch_delete(HashMap *map, void *array_of_keys, size_t number_of_elements, const DATA_TYPE key_type, const bool strict_mode) {
    if (!map || !array_of_keys) {
        perror("NULL argument in batch delete!");
        return false;
    }
    if (number_of_elements == 0) {
        perror("tried to insert an array of size 0 in hash_table_batch_delete() function!\n");
        return false;
    }
    if (key_type != map->key_type) {
        fprintf(stderr, "Key type mismatch in batch delete! Expected %d, got %d.\n", map->key_type, key_type);
        return false;
    }

    Key *keys = convert_array_to_keys(array_of_keys, number_of_elements, key_type);
    if (!keys) {
        perror("Failed to convert array to keys!");
        return false;
    }

    bool success = true;
    for (size_t i = 0; i < number_of_elements; i++) {
        if (!hash_table_entry_delete(map, &keys[i])) {
            if (strict_mode) {
                fprintf(stderr, "Key at index %zu not found in batch delete!\n", i);
                success = false;  // Fail if strict mode is enabled and key was never in table
            }
        }
        delete_key((keys[i]));
    }
    free(keys);
    return success;
}

// a function that returns the number of keys currently in the table (meant to be invoked by the user)
size_t hash_table_key_count(const HashMap *map) {
    if (map == NULL) {
        perror("passed in NULL hashmap into hash_table_count() function!\n");
        return 0;
    }
    return map->key_count;
}

// a function that returns the key type of the hash table (meant to be invoked by the user)
DATA_TYPE hash_table_get_key_type(const HashMap *map) {
    if (map == NULL) {
        perror("passed in NULL hashmap into hash_table_get_type() function!\n");
        return INVALID_TYPE;
    }
    return map->key_type;  
}

// debugging function that prints basic info about a hashmap and entries per bucket
void hash_table_debug_print(const HashMap *map) {
    if (!map) {
        printf("hashmap passed into hash_table_debug_print() is NULL!\n");
        return;
    }

    printf("=== HASH TABLE DEBUG INFO ===\n");
    printf("Bucket count: %zu\n", map->bucket_count);
    printf("Key count: %zu\n", map->key_count);
    printf("Load factor: %.2f\n", get_hash_table_load_factor(map));

    for (size_t i = 0; i < (map->bucket_count); i++) {
        Entry *current = map->buckets[i];
        size_t bucket_size = 0;

        printf("Bucket[%zu]: ", i);
        while (current) {
            bucket_size++;
            current = current->next;
        }
        printf("%zu entries\n", bucket_size);
    }
    printf("=== END OF DEBUG INFO ===\n");
    return;
}

// debugging function that prints basic info about a hashmap
void hash_table_info_print(const HashMap *map) {
    if (!map) {
        printf("hashmap passed into hash_table_info_print() is NULL!\n");
        return;
    }
    printf("=== HASH TABLE INFO ===\n");
    printf("Bucket count: %zu\n", map->bucket_count);
    printf("Key count: %zu\n", map->key_count);
    printf("Load factor: %.2f\n", get_hash_table_load_factor(map));
    return;
}

#endif /* HASHMAP_H */