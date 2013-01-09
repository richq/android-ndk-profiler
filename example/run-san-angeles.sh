#!/bin/sh

cd san-angeles
ant debug install

# start the activity
adb shell am start -n com.example.SanAngeles/.DemoActivity

# wait for a while
sleep 20

# send HOME key
adb shell input keyevent 3

adb pull /sdcard/gmon.out .

ANDROID_NDK=$(ls -1d ~/tools/android-ndk* | tail -n1)
abi=arm-linux-androideabi
version=4.4.3
host=linux-x86
SUPPRESS='-PprofCount -QprofCount -P__gnu_mcount_nc -Q__gnu_mcount_nc'
$ANDROID_NDK/toolchains/$abi-$version/prebuilt/$host/bin/$abi-gprof $SUPPRESS obj/local/armeabi/libsanangeles.so
