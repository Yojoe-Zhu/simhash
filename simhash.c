#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "list.h"

LIST_HEAD_DEFINE(feature_pair_list);
LIST_HEAD_DEFINE(hash_pair_list);

struct feature_pair {
    float weight;
    const char *feature;
    struct list_head node;
};

struct hash_pair {
    float weight;
    uint64_t hash;
    struct list_head node;
};

static uint64_t do_hash(const char *);

struct list_head *sim_extract(const char *text)
{
    int i;
    struct list_head *list = &feature_pair_list;
    struct feature_pair *pair;
	char *str = strdup(text);
	char *p;

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

struct list_head *sim_hash(struct list_head *head)
{
	struct list_head *list = &hash_pair_list;
	struct feature_pair *entry;
	struct hash_pair *pair;

	list_for_each_entry(entry, head, node) {
		pair = malloc(sizeof(struct hash_pair));
		pair->weight = entry->weight;
		pair->hash = do_hash(entry->feature);
		list_append(list, &pair->node);
	}

	return list;
}

static unsigned int BKDRHash(const char *str)
{
    unsigned int seed = 131;
    unsigned int hash = 0;

    while (*str) {
        hash = hash * seed + (*str++);
    }

    return (hash & 0x7FFFFFFF);
}

static unsigned int DJBHash(const char *str)
{
    unsigned int hash = 5381;

    while (*str) {
        hash += (hash << 5) + (*str++);
    }

    return (hash & 0x7FFFFFFF);
}

static uint64_t do_hash(const char *str)
{
    long hi = BKDRHash(str);
    long low = DJBHash(str);

    return ((uint64_t)hi << 32) | (uint64_t)low;
}

static uint64_t sim_reduction(float set[64])
{
	int i;
	uint64_t hash = 0;

	for (i = 0; i < 64; i++) {
		hash |= (set[i] > 0.0);
		hash <<= 1;
	}

	return hash;
}

uint64_t sim_merge(struct list_head *head)
{
	float set[64];
	struct hash_pair *entry;
	uint64_t hash;
	float weight;
	int i;

	for (i = 0; i < 64; i++)
		set[i] = 0.0;

	list_for_each_entry(entry, head, node) {
		hash = entry->hash;
		weight = entry->weight;

		for (i = 0; i < 64; i++) {
			if (hash & 0x01) {
				set[i] += weight;
			} else {
				set[i] -= weight;
			}

			hash >>= 1;
		}
	}

	return sim_reduction(set);
}

uint64_t simhash(const char *text)
{
    struct list_head *f;
    struct list_head *h;
    struct feature_pair *feature;
    struct feature_pair *feature_n;
    struct hash_pair *entry;
    struct hash_pair *entry_n;
    uint64_t hash;

    f = sim_extract(text);
    h = sim_hash(f);
	hash = sim_merge(h);

    list_for_each_entry_safe(feature, feature_n, f, node) {
        list_del(&feature->node);
        free(feature);
    }

    list_for_each_entry_safe(entry, entry_n, h, node) {
        list_del(&entry->node);
        free(entry);
    }

    return hash;
}

int sim_distance(uint64_t a, uint64_t b)
{
    int cnt = 0;
    uint64_t c = a ^ b;
    static const int map[16] = {
        0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4
    };

    /* Almost the same performance as __builtin_popcountll()
     * when compiled with -O3.
     */
    for (; c != 0; c = c >> 4) {
        cnt += map[c & 0x0F];
    }

    return cnt;
}

bool is_equal(uint64_t a, uint64_t b)
{
    return sim_distance(a, b) <= 3;
}
