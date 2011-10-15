all: test

read_maps.o: ../jni/read_maps.c ../jni/read_maps.h
	gcc -o read_maps.o ../jni/read_maps.c -Wall -c -g -O0

test: read_maps.o test_read_maps.c
	gcc -o test read_maps.o test_read_maps.c -I ../jni -Wall -g -O0

.PHONY: clean check all
check: test
	./test

clean:
	-rm *.o test -f
