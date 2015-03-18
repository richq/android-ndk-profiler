# Create Example

In the `/example/` directory you will find the script `create-example.sh` and a patch file for the native-activity included in the android-ndk-r5b.
There is also a complete example called `timing-test`.

## native-activity example
Running the `create-example.sh` script should:

1. Compile the profiling library (providing you have the variable NDK set, or it is installed in $HOME/tools/android-ndk-r5b)
1. Copy the native-activity sample from the NDK samples directory
1. Patch the native-activity with the required changes, as outlined on [Usage](Usage.md)
1. Update the native-activity to create the build files
1. Compile the native and Java parts of the example
1. Install the example to the emulator/device

### How to run the script
To run the script, you need to pass it the full paths to the `android` and `ndk-build` tools. If these are in your $PATH, then it should be simply:

    example/create-example.sh `which android` `which ndk-build`

### Legal information
I haven't included the example in the repository due to licensing mismatch, but also in case the examples in the SDK are updated.
The NDK examples are licensed under the Apache License, Version 2.0. All code in this project is GNU GPL v2+.

## Run the Native Activity Example

Run the example on your phone or emulator.
Press the screen or tilt the device for a few seconds.
Make sure you trigger the "destroy" event by pressing the Back button.
This will return you to the Android home screen.

## Seeing the results

Once you have run the test, pull the gmon.out file from the device/emulator with adb and run the gprof program against the compiled library:

    cd example/native-activity
    adb pull /sdcard/gmon.out .
    echo "Use the either armeabi or armeabi-v7a depending on which you profiled with, so only one of these:"
    arm-linux-androideabi-gprof obj/local/armeabi-v7a/libnative-activity.so
    arm-linux-androideabi-gprof obj/local/armeabi/libnative-activity.so

The results will hopefully look something like this:

    Flat profile:
    
    Each sample counts as 0.01 seconds.
      %   cumulative   self              self     total
     time   seconds   seconds    calls  ms/call  ms/call  name
     74.16      3.33     3.33                             android_main
     23.16      4.37     1.04                             process_cmd
      2.45      4.48     0.11        7    15.71    15.71  engine_handle_cmd
      0.22      4.49     0.01                             process_input
      0.00      4.49     0.00       71     0.00     0.00  engine_draw_frame
      0.00      4.49     0.00        8     0.00     0.00  engine_handle_input
      0.00      4.49     0.00        1     0.00     0.00  engine_term_display

The `android_main` and 2 `process_` calls are called from JNI directly, which is why they have no calling information.

## timing-test example
The timing-test example is a simple Android application with 3 buttons. To compile it, unpack the source tarball, build the main library with make, then build the example with ant:

    tar xf android-ndk-profiler-VERSION-src.tar.gz
    cd android-ndk-profiler-VERSION
    make
    cd examples/timing-test
    ant
    # optionally install to the device
    ant install

### Run the timing-test example
This is a simple Android app.
Press the 3 buttons in order, starting at the top.
It is quite likely to force close if you mistreat it!

The timing test example has 3 functions with calls to sleep.
It shows very little time in the JNI functions, since most of the time is spent in the sleep system calls.
