#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "list.h"


struct feature_pair {
    float weight;
    const char *feature;
    uint64_t hash;
    struct list_head node;
};

struct list_head *_extract(const char *text)
{
    int i;
    struct list_head *list;
    struct feature_pair *pair;
	char *str = strdup(text);
	char *p;

    list = malloc(sizeof(struct list_head));
    INIT_LIST_HEAD(list);

	p = strtok(str, " ");
	while (p != NULL) {
        pair = malloc(sizeof(struct feature_pair));
        pair->weight = 1;
        pair->feature = p;
        list_append(list, &pair->node);
		p = strtok(NULL, " ");
    }

    free(str);

    return list;
}

static inline unsigned int BKDRHash(const char *str)
{
    unsigned int seed = 131;
    unsigned int hash = 0;

    while (*str) {
        hash = hash * seed + (*str++);
    }

    return (hash & 0x7FFFFFFF);
}

static inline unsigned int DJBHash(const char *str)
{
    unsigned int hash = 5381;

    while (*str) {
        hash += (hash << 5) + (*str++);
    }

    return (hash & 0x7FFFFFFF);
}

static inline void _hash(struct list_head *head)
{
	struct feature_pair *entry;

	list_for_each_entry(entry, head, node) {
		entry->hash = (uint64_t)BKDRHash(entry->feature) << 32 |
            DJBHash(entry->feature);
	}
}

static inline uint64_t _reduction(float set[64])
{
	int i;
	uint64_t hash = 0;

	for (i = 0; i < 64; i++) {
		hash |= (uint64_t)(set[i] > 0.0) << i;
	}

	return hash;
}

static inline uint64_t _merge(struct list_head *head)
{
	float set[64];
	struct feature_pair *entry;
	uint64_t hash;
	float weight;
	int i;

	for (i = 0; i < 64; i++)
		set[i] = 0.0;

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
        free(entry);
    }
    free(head);

    return hash;
}

int sim_distance(uint64_t a, uint64_t b)
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
    return sim_distance(a, b) <= 3;
}
