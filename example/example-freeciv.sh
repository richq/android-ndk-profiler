#!/bin/sh

# This script checks out freeciv-android from its svn repo,
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

if [ ! -d freeciv-android ] ; then
    svn checkout http://freeciv-android.googlecode.com/svn/trunk/ freeciv-android || die "Could not checkout code"
else
    svn revert -R freeciv-android
    rm freeciv-android/custom_rules.xml freeciv-android/ant.properties
fi
# update the example to get the build files
$android update project -p freeciv-android -n net.hackcasual.freeciv --target android-8 || die "Unable to create build files"
# patch it so it has profiling
patch -i freeciv-android.patch -p0 || die "Could not patch for profiling"

# build
cd freeciv-android
$ndk_build -j3 || die "Error building with ndk-build"
ant debug || die "Error building with ant"

echo "Run the following to install the example"
echo "  \$ cd $(pwd) && ant debug install"
echo "After you run freeciv:"
echo "  \$ adb pull /sdcard/gmon.out ."
echo "  \$ arm-linux-androideabi-gprof obj/local/armeabi/libclient.so"
