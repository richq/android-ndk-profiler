
ifeq ($(ANDROID_HOME),)
NDK ?= $(ANDROID_NDK)
else
NDK ?= $(lastword $(sort $(wildcard $(dir $(ANDROID_HOME))/android-ndk*)))
endif

ifeq ($(NDK),)
$(error no ndk found - set ANDROID_HOME or ANDROID_NDK)
endif

.PHONY: andprof dist check clean

PROJECT=android-ndk-profiler

andprof:
	$(NDK)/ndk-build andprof V=1

ifeq ($(VERSION),)
dist:
	@echo "Set VERSION to create dist zip"
	@exit 1
srcdist:
	@echo "Set VERSION to create dist tarball"
	@exit 1
else
dist: andprof
	mkdir -p andprof/armeabi andprof/armeabi-v7a
	cp -pv jni/prof.h andprof
	cp -pv obj/local/armeabi/*.a andprof/armeabi
	cp -pv obj/local/armeabi-v7a/*.a andprof/armeabi-v7a
	cp android-ndk-profiler.mk andprof
	cd andprof && zip -r ../$(PROJECT)-$(VERSION).zip *
	cd ..
	rm -rf andprof

srcdist:
	git archive --format=tar.gz --prefix=$(PROJECT)-$(VERSION)/ HEAD > $(PROJECT)-$(VERSION).tar.gz

endif

check:
	$(MAKE) -C test check

clean:
	$(NDK)/ndk-build clean
	$(MAKE) -C test clean
