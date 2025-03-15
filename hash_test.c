#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashmap.h"

int main() {
    printf("Start of main test....\n");
    // 1️⃣ Create the hash table
    size_t initial_size = 4;
    HashMap *map = hash_table_create(initial_size, STRING_TYPE);
    if (!map) {
        printf("Failed to create hash map!\n");
        return EXIT_FAILURE;
    }
    // 2️⃣ Insert integer keys with string values
    char *str_keys[] = {"one", "two", "three", "four", "five"};
    char *values[] = {"Apple", "Banana", "Cherry", "Date", "Elderberry"};

    printf("inserting values....\n");
    for (size_t i = 0; i < 5; i++) {
        Key key = to_key((str_keys[i]), STRING_TYPE);
        Value value = to_value(values[i], STRING_TYPE);
        if (key.type == INVALID_TYPE || value.type == INVALID_TYPE) {
            printf("key and/or value conversion functions failed!\n");
            return EXIT_FAILURE;
        }

        if (!hash_table_insert(map, &key, &value)) {
            printf("Insertion failed for key: %s\n", str_keys[i]);
            hash_table_destroy(&map);
        }
        delete_key(key); // not actually needed since they are ints, but good to have
        delete_value(value); // need this since values are strings
    }

    // // 3️⃣ Print the hash table

    
    printf("\n--- Hash Table Contents ---\n");
    hash_table_print(map);

    char *strings_k[] = {"I", "am", "testing", "something", "with", "the", "batch", "insert"};
    char *strings_v[] = {"I_v", "am_v", "testing_v", "something_v", "with_v", "the_v", "batch_v", "insert_v"};

    size_t size = (sizeof(strings_k) / sizeof(char *));

    printf("\nAttempting batch insert...\n\n");
    if (!hash_table_batch_insert(map, strings_k, strings_v, size, STRING_TYPE, STRING_TYPE)) {
        printf("error with batch insert\n");
        return EXIT_FAILURE;
    }
    printf("\ntesting batch delete now\n\n");
    bool success = hash_table_batch_delete(map, strings_k, size, STRING_TYPE, true);
    printf("batch insert result : %s\n", (success == true) ? "success" : "failure");
    printf("\n--- Hash Table Contents post deletion---\n");
    hash_table_print(map);
    hash_table_debug_print(map);
    hash_table_destroy(&map);
    return 0;
    

    // // 4️⃣ Test lookup
    // // 5️⃣ 
    printf("\n--- Lookup Tests ---\n");
    Key lookup_key = to_key(str_keys[2], STRING_TYPE);
    printf("searching for key \"%s\" in hash map...\n", str_keys[2]);
    Entry *found_entry = hash_table_entry_lookup(map, &lookup_key);
    if (found_entry) {
        printf("Found key %s -> Value: %s (string reading)\n", str_keys[2], found_entry->value.data.string);
        delete_key(lookup_key);
    } else {
        printf("Key %s not found\n", str_keys[2]);
    }

    // add a new entry with a different type

    printf("adding integer number to hash map...\n");
    // Ensure the KEY TYPE matches here
    Key temp_key = {.data.string = "new", .type = STRING_TYPE};
    Value temp_value = {.data.integer = 10001, .type = INTEGER_TYPE};
    hash_table_insert(map, &temp_key, &temp_value);
    
    printf("current hashmap is now:\n");
    hash_table_print(map);

    // // 5️⃣ Get all keys
    printf("\n--- Get All Keys ---\n");
    Key *keys = get_hash_table_keys(map);
    if (keys) {
        size_t key_count = map->key_count;
        for (size_t i = 0; i < key_count; i++) {
            if (keys[i].type == STRING_TYPE) {
                printf("Key: %s\n", keys[i].data.string);
                free(keys[i].data.string);  // Free strdup'd strings
            } else {
                printf("Key: %d\n", keys[i].data.integer);
            }
        }
        free(keys);
    }

    // // 6️⃣ Delete an element 
    printf("\n--- Deleting Key: %s ---\n", str_keys[1]);
    Key deletion_key = to_key(str_keys[1], STRING_TYPE);
    if (hash_table_entry_delete(map, &deletion_key)) {
        printf("Key %s deleted successfully.\n", str_keys[1]);
    } else {
        printf("Failed to delete key %s\n", str_keys[1]);
    }
    delete_key(deletion_key);

    // // 7️⃣ print after deletion
    printf("\n--- Hash Table After Deletion ---\n");
    hash_table_print(map);

    // try to replace an entry that is already present
    printf("attempting to replace a key...\n");
    Value temp_value_2 = {.data.string = "I AM A NEW ENTRY REPLACING THE INTEGER", .type = STRING_TYPE};  // Same here
    hash_table_insert(map, &temp_key, &temp_value_2);
    printf("\n--- Hash Table After replacing a key ---\n");
    hash_table_print(map);

    printf("clearing map....\n");
    if (!hash_table_clear(map)) {
        printf("could not clear hash map\n");
    }

    // force hash map resize by adding a lot of stuff
    // 2️⃣ Insert integer keys with string values
    char str_keys_2[50][20];
    int values_2[50];

    for (int i = 0; i < 10; i++) {
        sprintf(str_keys_2[i], "%03d", i);
        values_2[i] = i;
    }
    for (size_t i = 0; i < 10; i++) {
        Key key = to_key(&(str_keys_2[i]), STRING_TYPE);
        Value value = to_value(&(values_2[i]), INTEGER_TYPE);
        if (key.type == INVALID_TYPE || value.type == INVALID_TYPE) {
            printf("key and/or value conversion functions failed!\n");
            return EXIT_FAILURE;
        }

        if (!hash_table_insert(map, &key, &value)) {
            printf("Insertion failed for key: %s\n", str_keys_2[i]);
            hash_table_destroy(&map);
        }
        delete_key(key);
        delete_value(value); // need this since values are strings
    }

    hash_table_print(map);

        // try to replace an entry that is already present (again)
        printf("attempting to replace a key...\n");
        Value temp_value_3 = {.data.string = "I AM A NEW ENTRY REPLACING THE INTEGER", .type = STRING_TYPE};  // Same here
        hash_table_insert(map, &temp_key, &temp_value_3);
        printf("\n--- Hash Table After replacing a key (again) ---\n");
        hash_table_print(map);

    // // 8️⃣Destroy the hash map
    printf("final size of hash map is: %zu buckets and %zu keys\n", map->bucket_count, map->key_count);

    hash_table_destroy(&map);
    printf("\nHash map destroyed successfully.\n");

    return EXIT_SUCCESS;
}
