#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "read_maps.h"

static void test_read_maps(void)
{
	FILE *fp = fopen("maps.txt", "r");
	struct proc_map *result = read_maps(fp, "libc-2.11.1.so");
	fclose(fp);
	assert(result != NULL);

        assert(0 == result->base);
        assert(0x00b16000 == result->lo);
        assert(0x00c69000 == result->hi);

	struct proc_map *next = result->next;
	assert(next != NULL);
        assert(0x00153000 == next->base);
        assert(0x00c69000 == next->lo);
        assert(0x00c6a000 == next->hi);

	next = next->next;
	assert(next != NULL);
        assert(0x00153000 == next->base);
        assert(0x00c6a000 == next->lo);
        assert(0x00c6c000 == next->hi);

	next = next->next;
	assert(next != NULL);
        assert(0x00155000 == next->base);
        assert(0x00c6c000 == next->lo);
        assert(0x00c6d000 == next->hi);

	unsigned int fake = 0x00b16000 + 0x000f7e30;
	unsigned int realaddr = get_real_address(result, fake);
	assert(realaddr == 0x000f7e30);

	free_maps(result);
}

int main(int argc, const char *argv[])
{
	test_read_maps();
	return 0;
}
