package com.example.timingtest;

import android.app.Activity;
import android.view.View;
import android.widget.Button;
import android.os.Bundle;

public class TimingTest extends Activity {

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);
		{
			final Button button = (Button) findViewById(R.id.native_call);
			button.setOnClickListener(new View.OnClickListener() {
				public void onClick(View v) {
					doNative();
				}
			});
		}

		{
			final Button button = (Button) findViewById(R.id.monstart);
			button.setOnClickListener(new View.OnClickListener() {
				public void onClick(View v) {
					startProfiler();
				}
			});
		}

		{
			final Button button = (Button) findViewById(R.id.moncleanup);
			button.setOnClickListener(new View.OnClickListener() {
				public void onClick(View v) {
					cleanupProfiler();
				}
			});
		}
	}

	public native String startProfiler();
	public native String doNative();
	public native String cleanupProfiler();

	static {
		System.loadLibrary("timing_jni");
	}
}
