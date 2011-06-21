#include <jni.h>

void do_dummy_call_1(void);
void do_dummy_call_2(void);

void Java_com_example_timingtest_TimingTest_doNative(JNIEnv* env, jobject thiz)
{
	sleep(1);
        do_dummy_call_1();
        sleep(1);
        do_dummy_call_2();
        sleep(1);
}

void Java_com_example_timingtest_TimingTest_startProfiler(JNIEnv* env, jobject thiz)
{
        monstartup("timing_jni.so");
}

void Java_com_example_timingtest_TimingTest_cleanupProfiler(JNIEnv* env, jobject thiz)
{
        moncleanup();
}
