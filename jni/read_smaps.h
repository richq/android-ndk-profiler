#ifndef read_smaps_h_seen
#define read_smaps_h_seen

struct smap {
	unsigned int base;
	unsigned int lo;
	unsigned int hi;
	struct smap *next;
};

struct smap *read_smaps(FILE *fp, const char *lname);
void free_smaps(struct smap *s);
unsigned int get_real_address(const struct smap *smaps, unsigned int fake);

#endif
