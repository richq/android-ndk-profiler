
NDK ?= $(HOME)/tools/android-ndk-r5b/

.PHONY: andprof dist check clean

andprof:
	$(NDK)/ndk-build andprof V=1

dist: andprof
	mkdir -p andprof/armeabi andprof/armeabi-v7a
	cp -pv jni/prof.h andprof
	cp -pv obj/local/armeabi/*.a andprof/armeabi
	cp -pv obj/local/armeabi-v7a/*.a andprof/armeabi-v7a
	cp android-ndk-profiler.mk andprof
	tar czvf andprof.tar.gz andprof
	cd andprof && zip -r ../android-ndk-profiler.zip *
	cd ..
	rm -rf andprof

check:
	$(MAKE) -C test check

clean:
	$(NDK)/ndk-build clean
	$(MAKE) -C test clean
