#!/bin/sh

# This script copies the san-angeles sample from the ndk package
# patches it with the required changes, and compiles the code.

cd $(dirname $0)

if [ $# -ne 2 ] ; then
    echo "Usage: $0 \`which android\` \`which ndk-build\`"
    echo
    echo "Arg 1: the full path to the 'android' application"
    echo "Arg 2: the full path to the 'ndk-build' script"
    exit 1
fi

die() {
  echo >&2 "$@"
  exit 1
}

# build the library and copy it to the correct place
cd ..
make dist VERSION=example|| die "Error building profiling code"
unzip android-ndk-profiler-example.zip
export NDK_MODULE_PATH=$(pwd)
cd -

android=$1
ndk_build=$2

sdk=$(dirname $(dirname $android))
ndk=$(dirname $ndk_build)

if [ -d san-angeles ] ; then
    rm san-angeles -rf
fi

# copy example from ndk
cp -r $ndk/samples/san-angeles/ . || die "Could not copy the example"
# update the example to get the build files
$android update project -p ./san-angeles -n com.example.SanAngeles --target android-10 || die "Unable to create build files for san-angeles"

# patch the makefile so it has profiling
sed -i 's/^include \$(BUILD.*$/LOCAL_CFLAGS += -pg\nLOCAL_STATIC_LIBRARIES := android-ndk-profiler\n&\n$(call import-module,android-ndk-profiler)/g' \
    san-angeles/jni/Android.mk

# add monstartup/cleanup to the start/pause methods
sed -i 's/^ \+sWindowW.*$/&\n    monstartup("sanangeles.so");/g' san-angeles/jni/app-android.c
sed -i 's/^    _pause();/&\n    moncleanup();/g' san-angeles/jni/app-android.c

# add WRITE_EXTERNAL_STORAGE so gmon.out can be written
sed -i 's@^</manifest>@<uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE" />\n&@g' san-angeles/AndroidManifest.xml

# build
cd san-angeles
$ndk_build || die "Error building with ndk-build"
ant debug || die "Error building with ant"

echo "Run the following to install the example"
echo "  \$ cd san-angeles && ant debug install"
echo "After you run the activity:"
echo "  \$ adb pull /sdcard/gmon.out ."
echo "  \$ arm-linux-androideabi-gprof obj/local/armeabi/libsanangeles.so"
