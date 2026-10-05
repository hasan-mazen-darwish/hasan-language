#ifndef _HASHMAP_H
#define _HASHMAP_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define HASHMAP_INITIAL_CAPACITY 16

typedef struct HashmapEntry {
  void *data;
  char *key;
  unsigned int is_occupied : 1;
  uint64_t hash;
} HashmapEntry;

typedef struct Hashmap {
  HashmapEntry *data;
  size_t capacity;
  size_t available_positions;
} Hashmap;

void hashmap_clean_hashmap(Hashmap *hashmap);
static inline uint64_t hashmap_hash_function(char *key)
    __attribute__((pure)); // This hash function will be
                           // following the FNV-1a algorithm.
Hashmap *
hashmap_default_value(Hashmap *original); // A function that initializes the
                                          // hashmap to a proper default value
int hashmap_add_key(
    Hashmap *original, char *key,
    void *value); // Returns 0 for allocating or general failures, 1 for
                  // success, and 2 for existing values.
Hashmap hashmap_new();
int hashmap_set_key(Hashmap *original, char *key,
                    void *value); // Returns 0 for allocating or general
                                  // failuresa and 1 for success
void *hashmap_get_value(Hashmap *hashmap, char *key);

#endif // !_HASHMAP_H
