all: test

read_smaps.o: ../jni/read_smaps.c ../jni/read_smaps.h
	gcc -o read_smaps.o ../jni/read_smaps.c -Wall -c -g -O0

test: read_smaps.o test_read_smaps.c
	gcc -o test read_smaps.o test_read_smaps.c -I ../jni -Wall -g -O0

.PHONY: clean check all
check: test
	./test

clean:
	-rm *.o test -f
