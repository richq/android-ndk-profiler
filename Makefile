
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
	$(NDK)/ndk-build $(PROJECT) V=1

ifeq ($(VERSION),)
dist:
	@echo "Set VERSION to create dist zip"
	@exit 1
srcdist:
	@echo "Set VERSION to create dist tarball"
	@exit 1
else
dist: andprof
	mkdir -p $(PROJECT)/armeabi $(PROJECT)/armeabi-v7a
	cp -pv jni/prof.h $(PROJECT)
	cp -pv obj/local/armeabi/*.a $(PROJECT)/armeabi
	cp -pv obj/local/armeabi-v7a/*.a $(PROJECT)/armeabi-v7a
	cp jni/Android-module.mk $(PROJECT)/Android.mk
	zip -r $(PROJECT)-$(VERSION).zip $(PROJECT)
	rm -rf $(PROJECT)

srcdist:
	git archive --format=tar.gz --prefix=$(PROJECT)-$(VERSION)/ HEAD > $(PROJECT)-$(VERSION).tar.gz

endif

check:
	$(MAKE) -C test check

clean:
	$(NDK)/ndk-build clean
	$(MAKE) -C test clean
