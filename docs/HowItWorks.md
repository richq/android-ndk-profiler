### Design decisions, how it works

When you compile C code using GCC and pass the `-pg` flag, the compiler inserts a call to the function `__gnu_mcount_nc` at the start of every function.
Normally you link with `-pg`, the implementation of this function is linked into your program or shared library.
The Android platform doesn't provide an implementation of `__gnu_mcount_nc`, and it also lacks the C library calls necessary for profiling.

This library implements `__gnu_mcount_nc` in a few lines of thumb code that extract the required function addresses from the stack and call a C profiling function.

The C profiling code is adapted from Forgotten's Visual Boy Advance, which itself was derived from gprof, and this is why this library has to be licenced GPL v2+.
You probably cannot distribute applications linked with profiling information in the Android Market, but you shouldn't be doing that anyway.

Linux uses address space layout randomization, so a function that should be at the absolute address 0x6b48 is really at the virtual address e.g. 0x8b006b48.
The running application knows how to convert from the virtual address to the real one.
The profiling library figures out the address-munging by parsing `/proc/self/maps`.
This is why you need to tell the setup function the name of your shared library.

Timing information is collected using an interval timer, which decrements both when the process executes and when the system is executing on behalf of the process.
When the timer finishes it sends a `SIGPROF` signal.
This is handled in a suitable handler routine.
In order to get the current address, the `context` argument of the routine is used.
This method is thread safe and should work on all versions of Android (tested on 2.2).
