#ifndef _HASHMAP_H
#define _HASHMAP_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define HASHMAP_INITIAL_CAPACITY 16

typedef struct Hashmap {
  void *data;
  size_t capacity;
  size_t sizeof_data;
  size_t available_positions;
} Hashmap;

uint64_t hashmap_hash_function(char *key); // This hash function will be
                                           // following the FNV-1a algorithm.
Hashmap *
hashmap_default_value(Hashmap *original,
                      size_t sizeof_data); // A function that initializes the
                                           // hashmap to a proper default value

#endif // !_HASHMAP_H
