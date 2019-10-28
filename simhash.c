#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "simhash.h"
#include "list.h"

struct feature_pair {
    int weight;
    const char *feature;
    unsigned int len;
    uint64_t hash;
    struct list_head node;
};

struct feature_pair total_pairs[200];

struct list_head *_extract(const char *text)
{
    int i;
    int cnt = 0;
    struct list_head *list;
    struct feature_pair *pair;
	const char *p = text;
    unsigned int len = strlen(text);
    unsigned int step = 1;

    list = malloc(sizeof(struct list_head));
    INIT_LIST_HEAD(list);

    for (i = 0; i < len; i += step) {
        pair = &total_pairs[cnt++];
        list_append(list, &pair->node);
        pair->weight = 1;
        pair->feature = p;

        if (i + step < len) {
            pair->len = step;
        } else {
            pair->len = len - i;
        }

        p += pair->len;
    }

    return list;
}

extern void hashlittle2(
  const void *key,   /* the key to hash */
  size_t      length,/* length of the key */
  uint32_t   *pc,    /* IN: primary initval, OUT: primary hash */
  uint32_t   *pb);    /* IN: secondary initval, OUT: secondary hash */

static inline uint64_t lookup3_hash(const char *str, unsigned int len)
{
    unsigned int hi = 0;
    unsigned int low = 5381;
    unsigned int seed = 131;

    hashlittle2(str, len, &hi, &low);

    return ((uint64_t)hi << 32) | low;
}

static inline void _hash(struct list_head *head)
{
	struct feature_pair *entry;

	list_for_each_entry(entry, head, node) {
		entry->hash = lookup3_hash(entry->feature, entry->len);
	}
}

static inline uint64_t _reduction(int set[64])
{
	int i;
	uint64_t hash = 0;

	for (i = 0; i < 64; i++) {
		hash |= (uint64_t)(set[i] > 0) << i;
	}

	return hash;
}

static inline uint64_t _merge(struct list_head *head)
{
	int set[64] = {0};
	struct feature_pair *entry;
	uint64_t hash;
	int weight;
	int i;

	list_for_each_entry(entry, head, node) {
		hash = entry->hash;
		weight = entry->weight;

		for (i = 0; i < 64; i++) {
			if ((hash >> i) & 0x01) {
				set[i] += weight;
			} else {
				set[i] -= weight;
			}
		}
	}

	return _reduction(set);
}

/* export */

uint64_t simhash(const char *text)
{
    struct list_head *head;
    struct feature_pair *entry;
    struct feature_pair *n;
    uint64_t hash;

    head = _extract(text);
    _hash(head);
	hash = _merge(head);

    list_for_each_entry_safe(entry, n, head, node) {
        list_del(&entry->node);
    }
    free(head);

    return hash;
}

int hamming_distance(uint64_t a, uint64_t b)
{
    int cnt = 0;
    uint64_t c = a ^ b;
    static const int map[16] = {
        0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4
    };

    for (; c != 0; c = c >> 4) {
        cnt += map[c & 0x0F];
    }

    return cnt;
}

bool is_equal(uint64_t a, uint64_t b)
{
    return hamming_distance(a, b) <= 3;
}
