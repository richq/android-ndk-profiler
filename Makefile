
NDK ?= $(HOME)/tools/android-ndk-r5b/

.PHONY: andprof dist check clean

andprof:
	$(NDK)/ndk-build andprof V=1

dist: andprof
	mkdir andprof
	cp -pv jni/prof.h obj/local/armeabi/libandprof.a andprof
	tar czvf andprof.tar.gz andprof
	cd andprof && zip ../android-ndk-profiler.zip *
	cd ..
	rm -rf andprof

check:
	$(MAKE) -C test check

clean:
	$(NDK)/ndk-build clean
	$(MAKE) -C test clean
