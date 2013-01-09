/*
 * Part of the android-ndk-profiler library.
 * Copyright (C) Richard Quirk
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "read_maps.h"

static char s_line[256];

void free_maps(struct proc_map *s)
{
    struct proc_map *next = s->next;
    while (next != NULL) {
        struct proc_map *tmp = next;
        next = next->next;
        free(tmp);
    }
    free(s);
}

struct proc_map *read_maps(FILE *fp, const char *lname)
{
    struct proc_map *results = NULL;
    struct proc_map *current = NULL;
    size_t namelen = strlen(lname);
    while (fgets(s_line, sizeof(s_line), fp) != NULL) {
        size_t len = strlen(s_line);
        len--;
        s_line[len] = 0;
        if (namelen < len && strcmp(lname, &s_line[len - namelen]) == 0) {
            char c[1];
            char perm[4];
            int lo, base, hi;
            sscanf(s_line, "%x-%x %4c %x %c", &lo, &hi, perm, &base, c);
            if (results == NULL) {
                current = malloc(sizeof(struct proc_map));
                current->next = NULL;
                results = current;
            } else {
                current->next = malloc(sizeof(struct proc_map));
                current = current->next;
                current->next = NULL;
            }
            current->base = base;
            current->lo = lo;
            current->hi = hi;
        }
    }
    return results;
}

unsigned int get_real_address(const struct proc_map *maps, unsigned int fake)
{
    const struct proc_map *mp = maps;
    while (mp) {
        if (fake >= mp->lo && fake <= mp->hi) {
            return fake - mp->lo;
        }
        mp = mp->next;
    }
    return fake;
}
