# Build Changes
From version 3.2 the profiling library uses the Android NDK modules system.
Unpack the android-ndk-profiler.zip file into a suitable place.


    cd $HOME/tools
    unzip android-ndk-profiler.zip

Modify your Android.mk to import the android-ndk-profiler module, link with the library, and compile your code with profiling information

    # compile with profiling
    LOCAL_CFLAGS := -pg
    LOCAL_STATIC_LIBRARIES := android-ndk-profiler
    
    # at the end of Android.mk
    $(call import-module,android-ndk-profiler)

The above requires NDK r5b or later, as previous versions did not support pre-built 3rd party libraries.
But you should always try and use the latest NDK release.

When you compile, you need to set the `NDK_MODULE_PATH` variable to the directory above where you unpacked the android-ndk-profiler.zip earlier.
So if you did it exactly as above, that would be:

    ndk-build NDK_MODULE_PATH=$HOME/tools

You can also set that with `export`, whatever is easiest. See the IMPORT-MODULE.HTML file in the NDK docs directory for more details.

# Code Changes
Add calls to monstartup and moncleanup to your start and shutdown functions.

    /* in the start-up code */
    monstartup("your_lib.so");
    
    /* in the onPause or shutdown code */
    moncleanup();

The `monstartup` call expects the name of your library.
It needs this to find out what the real addresses are of the functions in the library,
since Linux uses [ASLR](http://en.wikipedia.org/wiki/Address_space_layout_randomization) to change the function addresses for added security.

Add write permissions to the `AndroidManifest`.xml file,
so your application may write to the sdcard.
The gmon.out file is saved in `/sdcard/gmon.out`.
These are the lines to add to `AndroidManifest`.xml


    <uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE" />

If you do not want to modify the AndroidManifest.xml file to add write permissions, you can save gmon.out to a file in your application's private storage area.
To do this, set the environment variable `CPUPROFILE` to the gmon.out file name any time before calling the `moncleanup` function:


    setenv("CPUPROFILE", "/data/data/com.example.application/files/gmon.out", 1);
    moncleanup();

Things to note:

* Replace `com.example.application` with the package name of your application.
* This should be the full path to the gmon.out file, not a directory name.
* This works fine for current versions of Android, but...
* ... the recommended way to generate the `/data/data/<application>/files` directory name is to use `getFilesDir()` in the Java part of your application.

# Execution
Once you have made the modifications above you can compile and run your application.
Run your code, profile it, then do something to trigger the `moncleanup()` call.
This will create or overwrite `/sdcard/gmon.out` on the Android device or emulator.
You shouldn't call `moncleanup()` nor `monstartup()` more than once.

Back on your PC, pull the gmon.out file from your Android device or emulator using one of the following, depending on which storage strategy you used.

    adb pull /sdcard/gmon.out .
    adb pull /data/data/com.example.application/files/gmon.out .

Run the gprof tool, passing it the non-stripped library (usually in $PROJECT/obj/local/armeabi-v7a/libXXXX.so or $PROJECT/obj/local/armeabi/libXXXX.so).
This is for NDK version r5b onwards, in earlier versions the path to gprof is different but should still work.

    $ANDROID_NDK/toolchains/arm-linux-androideabi-4.6/prebuilt/linux-x86/bin/arm-linux-androideabi-gprof your_lib.so

The output is the profile information.
To interpret this data you may like to take a look at [this guide](http://www.linuxselfhelp.com/gnu/gprof/html_chapter/gprof_5.html).

There's a slight bug in version 3 where the profiling functions will appear in the timing information.
To ignore these and get just your code's information, use the `-P` and `-Q` options to gprof like this:

    arm-linux-androideabi-gprof your_lib.so \
        -PprofCount -QprofCount -P__gnu_mcount_nc -Q__gnu_mcount_nc

If you see the error `file 'libNNNN.so' has no symbols` then this usually means that you have passed in the stripped library.
You need to use the one that is generated under the obj directory rather than the libs directory.

# Environment Variables

The profiling library will read the following environment variables:

## CPUPROFILE
This is the path to the gmon.out file that is generated when your code calls `moncleanup`. To change the location, set the variable any time before calling moncleanup. Example:

    setenv("CPUPROFILE", "/data/data/com.example.application/files/gmon.out", 1);

## CPUPROFILE\_FREQUENCY
This controls how many interrupts per second the profiler samples. The default is 100 per second. Set this variable before calling the `monstartup` function. Example:

    setenv("CPUPROFILE_FREQUENCY", "500", 1); /* Change to 500 interrupts per second */
    monstartup("your_lib.so");

# Solving Problems

If you have problems, check the logcat output and grep for PROFILING.
Possible problems are being unable to write to the sdcard or unexpected infinite recursion while profiling.
