#include "./hashmap.h"
#include <stdint.h>
#include <string.h>

#if defined(__GNUC__) || defined(__clang__)
#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define LIKELY(x) (x)
#define UNLIKELY(x) (x)
#endif

#define FNV_PRIME_64 0x100000001b3ULL // 240 + 28 + 0xb3 = 1099511628211
#define FNV_OFFSET_BASIS_64 0xcbf29ce484222325ULL // 14695981039346656037

static Hashmap *hashmap_rehash_data(Hashmap *hashmap) {
  Hashmap copy = *hashmap;
  HashmapEntry *temp = calloc(hashmap->capacity, sizeof(HashmapEntry));
  if (temp == NULL)
    return NULL;
  free(copy.data);
  copy.data = temp;
  for (size_t i = 0; i < hashmap->capacity; i++) {
    if (hashmap->data[i].key != NULL) {
      uint64_t hash = hashmap->data[i].hash;
      size_t index =
          hash & (hashmap->capacity -
                  1); // This is the same as hash % hashmap->capacity since the
                      // capacity is guaranteed to be a power of two
      if (copy.data[index].is_occupied == 0)
        copy.data[index] = hashmap->data[i];

      // The bucket is occupied! this means either we hit the same key or we
      // have a collision.
      else if (copy.data[index].hash != hash ||
               strcmp(copy.data[index].key, hashmap->data[i].key) != 0) {
        // Here, we have a collision. A previous key took the hash position of
        // this key
        int is_occupied = 1;
        while (is_occupied == 1) {
          index++;
          if (index >= copy.capacity)
            break;
          if (copy.data[index].hash == hash ||
              strcmp(copy.data[index].key, hashmap->data[i].key) == 0) {
            is_occupied = 0;
            copy.data[index] = hashmap->data[i];
            break;
          }
          is_occupied = copy.data[index].is_occupied;
          if (is_occupied == 0) {
            copy.data[index] = hashmap->data[i];
            break;
          }
        }

        // This loop ended and we have two cases: either is_occupied = 0, then
        // in this case, we successfully inserted the element into the hashmap.
        // Otherwise, if is_occupied = 1, then the loop ended after exceeding
        // the capacity. That means that the empty slots are in the beginning of
        // the array. And we are sure that the array is empty, because there are
        // maximum capacity/2 elements, and it is never out of capacity.

        if (is_occupied == 1) {
          index = 0;
          while (is_occupied == 1) {
            if (index >= copy.capacity)
              break;
            if (copy.data[index].hash == hash ||
                strcmp(copy.data[index].key, hashmap->data[i].key) == 0) {
              is_occupied = 0;
              copy.data[index] = hashmap->data[i];
              break;
            }
            is_occupied = copy.data[index].is_occupied;
            if (is_occupied == 0) {
              copy.data[index] = hashmap->data[i];
              break;
            }
            index++;
          }
          if (is_occupied == 1)
            return NULL; // Fatal error. This must not happen.
        }
      }
    }
  }
  *hashmap = copy;
  return hashmap;
}

static Hashmap *hashmap_expand_capacity(Hashmap *original) {
  if (original->capacity > SIZE_MAX / 2)
    return NULL;
  original->capacity *= 2;
  void *temp =
      realloc(original->data, original->capacity * sizeof(HashmapEntry));
  if (temp == NULL)
    return NULL;
  original->data = temp;
  if (hashmap_rehash_data(original) == NULL)
    return NULL;
  return original;
}

void hashmap_clean_hashmap(Hashmap *hashmap) {
  for (size_t i = 0; i < hashmap->capacity; i++) {
    if (hashmap->data[i].key != NULL) {
      free(hashmap->data[i].data);
      free(hashmap->data[i].key);
    }
  }
  free(hashmap->data);
  free(hashmap);
}

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

Hashmap *hashmap_default_value(Hashmap *original) {
  if (original == NULL)
    return NULL;
  if (original->data != NULL) {
    for (size_t i = 0; i < original->capacity; i++) {
      if (original->data[i].key != NULL) {
        free(original->data[i].data);
        original->data[i].data = NULL;
        free(original->data[i].key);
        original->data[i].key = NULL;
      }
    }
  }
  free(original->data);
  original->data = NULL;
  void *temp = calloc(HASHMAP_INITIAL_CAPACITY, sizeof(HashmapEntry));
  if (temp == NULL)
    return NULL;

  original->data = temp;
  original->capacity = HASHMAP_INITIAL_CAPACITY;
  original->available_positions = HASHMAP_INITIAL_CAPACITY;
  return original;
}

void *hashmap_get_value(Hashmap *hashmap, char *key) {
  uint64_t hash = hashmap_hash_function(key);
  size_t index = hash & (hashmap->capacity - 1);
  if (hash == hashmap->data[index].hash &&
      strcmp(key, hashmap->data[index].key) == 0)
    return hashmap->data[index].data;

  for (size_t i = index + 1; i < hashmap->capacity; i++) {
    if (hashmap->data[i].is_occupied == 0)
      return NULL;
    else if (hashmap->data[i].hash == hash &&
             strcmp(hashmap->data[i].key, key) == 0)
      return hashmap->data[i].data;
    // Otherwise, the bucket is both occupied and is not equal to the key. We
    // will continue.
  }

  // Now, we will run another loop, just checking if the key is somewhere in the
  // beginning of the array
  for (size_t i = 0; i < index; i++) {
    if (hashmap->data[i].is_occupied == 0)
      return NULL;
    else if (hashmap->data[i].hash == hash &&
             strcmp(hashmap->data[i].key, key) == 0)
      return hashmap->data[i].data;
  }
  // No data found. Returning null.
  return NULL;
}

int hashmap_add_key(Hashmap *original, char *key, void *value) {
  // Returning values:
  // 0 for allocating failures or just failures
  // 1 for success
  // 2 for existing values

  // Expanding the hashmap if we already know that the size cap got hit (though
  // rare):
  if (UNLIKELY(original->available_positions == 0)) {
    if (hashmap_expand_capacity(original) == NULL) {
      hashmap_clean_hashmap(original);
      return 0;
    }
  }

  // Now, we will start with hashing the key, looking for it if it exists, and
  // add it to the hash map.
  if (hashmap_get_value(original, key) != NULL)
    return 2;

  uint64_t hash = hashmap_hash_function(key);
  size_t index = hash & (original->capacity - 1);

  // If this index is not occupied, perfect.
  if (original->data[index].is_occupied == 0) {
    original->data[index].is_occupied = 1;
    original->data[index].data = value;
    original->data[index].key = strdup(key);
    original->data[index].hash = hash;
    original->available_positions -= 1;
    return 1;
  }

  // Otherwise, we will start looping through the array. Of course, if the array
  // is already full and out of capacity, we will instantly increase it's
  // capacity.

  for (size_t i = index + 1; i < original->capacity; i++) {
    if (original->data[i].is_occupied == 0) {
      original->data[i].is_occupied = 1;
      original->data[i].data = value;
      original->data[i].key = strdup(key);
      original->data[i].hash = hash;
      original->available_positions -= 1;
      return 1;
    }
  }

  for (size_t i = 0; i < index; i++) {
    if (original->data[i].is_occupied == 0) {
      original->data[i].is_occupied = 1;
      original->data[i].data = value;
      original->data[i].key = strdup(key);
      original->data[i].hash = hash;
      original->available_positions -= 1;
      return 1;
    }
  }

  // Otherwise, there is no available positions for some reason.
  return 0;
}
