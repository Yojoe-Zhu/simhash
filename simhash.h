#include <stdbool.h>
#include <stdint.h>

uint64_t simhash(const char *text);
int hamming_distance(uint64_t a, uint64_t b);
bool is_equal(uint64_t a, uint64_t b);

