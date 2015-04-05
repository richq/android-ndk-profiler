#!/bin/sh

# This script copies the native-activity sample from the ndk package
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

if [ ! -d native-activity ] ; then
    # copy example from ndk
    cp -r $ndk/samples/native-activity/ . || die "Could not copy the example"
    # find the oldest target available that's 10 or higher
    exampletarget=$($android list targets | awk '/android-[0-9]/{gsub("\"", ""); split($4, a, "-"); if (a[2] >= 10) { print $4}}' | head -n1)
    test -z "$exampletarget" && die "unable to find a suitable android target android-10 or up"
    # update the example to get the build files
    $android update project -p ./native-activity -s --target $exampletarget || die "Unable to create build files for native-activity"
    # patch it so it has profiling
    patch -i native-activity.patch -p0 || die "Could not patch native-activity for profiling"
fi

# build
cd native-activity
$ndk_build APP_ABI=armeabi || die "Error building with ndk-build"
ant debug || die "Error building with ant"

echo "Run the following to install the example"
echo "  \$ cd native-activity && ant debug install"
echo "After you run the activity:"
echo "  \$ adb pull /sdcard/gmon.out ."
echo "  \$ arm-linux-androideabi-gprof obj/local/armeabi/libnative-activity.so"
