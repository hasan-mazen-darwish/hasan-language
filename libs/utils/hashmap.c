#include "./hashmap.h"
#include <stdint.h>

#define FNV_PRIME_64 0x100000001b3ULL // 240 + 28 + 0xb3 = 1099511628211
#define FNV_OFFSET_BASIS_64 0xcbf29ce484222325ULL // 14695981039346656037

uint64_t hashmap_hash_function(char *key) {
  uint64_t hash = FNV_OFFSET_BASIS_64;
  char *p = key;
  while (*p) {
    hash ^= (unsigned int)*p;
    hash *= FNV_PRIME_64;
    p++;
  }
  return hash;
}
