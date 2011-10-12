
NDK ?= $(HOME)/tools/android-ndk-r6

.PHONY: andprof dist check clean

REP := http://android-ndk-profiler.googlecode.com/svn

andprof:
	$(NDK)/ndk-build andprof V=1

ifeq ($(VERSION),)
dist:
	@echo "Set VERSION to create dist zip"
	@exit 1
else
dist: andprof
	mkdir -p andprof/armeabi andprof/armeabi-v7a
	cp -pv jni/prof.h andprof
	cp -pv obj/local/armeabi/*.a andprof/armeabi
	cp -pv obj/local/armeabi-v7a/*.a andprof/armeabi-v7a
	cp android-ndk-profiler.mk andprof
	tar czvf andprof.tar.gz andprof
	cd andprof && zip -r ../android-ndk-profiler-$(VERSION).zip *
	cd ..
	rm -rf andprof
endif

srcdist:
	test -z "$(VERSION)" || (svn export $(REP)/trunk android-ndk-profiler-$(VERSION) && \
	tar czf android-ndk-profiler-$(VERSION)-src.tar.gz android-ndk-profiler-$(VERSION) ) ;

check:
	$(MAKE) -C test check

clean:
	$(NDK)/ndk-build clean
	$(MAKE) -C test clean
