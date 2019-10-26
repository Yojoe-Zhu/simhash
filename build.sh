#!/bin/sh

gcc -o simhash simhash.c lookup3.c test.c -O3
gcc -o test_hash hash.c test_hash.c -O3
