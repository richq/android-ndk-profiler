#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "read_smaps.h"

static char s_line[256];

void free_smaps(struct smap *s)
{
    struct smap *next = s->next;
    while (next != NULL) {
        struct smap *tmp = next;
        next = next->next;
        free(tmp);
    }
    free(s);
}

struct smap *read_smaps(FILE *fp, const char *lname)
{
    struct smap *results = NULL;
    struct smap *current = NULL;
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
                current = malloc(sizeof(struct smap));
                current->next = NULL;
                results = current;
            } else {
                current->next = malloc(sizeof(struct smap));
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

unsigned int get_real_address(const struct smap *smaps, unsigned int fake)
{
    const struct smap *mp = smaps;
    while (mp) {
        if (fake >= mp->lo && fake <= mp->hi) {
            return fake - mp->lo;
        }
        mp = mp->next;
    }
    return fake;
}
