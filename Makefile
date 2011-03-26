
NDK ?= $(HOME)/tools/android-ndk-r5b/

.PHONY: andprof dist check clean

andprof:
	$(NDK)/ndk-build andprof

dist: andprof
	mkdir andprof
	cp -pv jni/prof.h obj/local/armeabi/libandprof.a andprof
	tar czvf andprof.tar.gz andprof
	rm -rf andprof

check:
	$(MAKE) -C test check

clean:
	$(NDK)/ndk-build clean
	$(MAKE) -C test clean
