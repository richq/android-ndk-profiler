#ifndef read_maps_h_seen
#define read_maps_h_seen

struct proc_map {
	unsigned int base;
	unsigned int lo;
	unsigned int hi;
	struct proc_map *next;
};

struct proc_map *read_maps(FILE *fp, const char *lname);
void free_maps(struct proc_map *s);
unsigned int get_real_address(const struct proc_map *maps, unsigned int fake);

#endif
