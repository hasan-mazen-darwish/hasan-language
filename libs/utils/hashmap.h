#ifndef _HASHMAP_H
#define _HASHMAP_H

#include <stdint.h>

uint64_t hashmap_hash_function(char *key); // This hash function will be
                                           // following the FNV-1a algorithm.

#endif // !_HASHMAP_H
