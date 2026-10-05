#include "./libs/utils/hashmap.h"
#include <stdio.h>

int main() {
  printf("Testing the hashmaps.\n");
  printf("---------------------\n\n");

  printf("1. Testing initialization...\n");
  Hashmap hashmap = hashmap_new();
  printf(
      "Testing initialization succeed! we initialized the hashmap values!\n");

  printf("2. Testing setting one value (number 10) to key 'hello world'...\n");
  int num = 10;
  int add = hashmap_set_key(&hashmap, "hello world", &num);
  printf("Setting value test passed! got code %d\n", add);

  printf("3. Testing getting the value of the key 'hello world'...\n");
  int *number = (int *)hashmap_get_value(&hashmap, "hello world");
  printf("Testing completed. Got value %d. Original value: %d\n", *number, num);
  printf(
      "4. Setting the hash map to 33 different values... The values are:\n\n");

  char *values[33] = {
      "value 1",   "value 2",    "value 3",      "value 4",    "thirsty",
      "Ramadan",   "ramadan",    "hasan",        "HASAN",      "Dudes",
      "_integer",  "Randomizer", "six",          "seven",      "Row",
      "cOlimn",    "rudeness",   "stuff",        "biocvh",     "haha",
      "sept",      "kkakak",     "hoorrible",    "skjs",       "Tikitikitik",
      "kls",       "kka",        "wow",          "twentyNine", "thirty",
      "thirtyOne", "thirtyTwo",  "thirtyThree33"};

  for (int i = 0; i < 33; i++) {
    printf("-%d- \"%s\". Capacity: %zu, Available positions: %zu.\n", i + 1,
           values[i], hashmap.capacity, hashmap.available_positions);
    hashmap_set_key(&hashmap, values[i], values[i]);
  }

  printf("\n5. Overriding the value of a hashmap. Picking key \"%s\"...\n",
         values[10]);
  hashmap_set_key(&hashmap, values[10], "\033[1;34mOverrided value\033[0m");
  printf("Overriding succeed!\n");

  printf("6. Getting the values of the inserted data...\n\n");
  for (int i = 0; i < 33; i++) {
    char *key = values[i];
    char *value = hashmap_get_value(&hashmap, key);
    if (value == NULL) {
      printf("NULL value for key \"%s\"!!!!!\n", key);
      continue;
    }
    printf("Key \033[1;32m\"%s\"\033[0m - \033[1;33m%s\033[0m\n", key, value);
  }

  printf("\n\n\nYour hashmap lib is working! congrats!\n\n\n");
  return 0;
}
