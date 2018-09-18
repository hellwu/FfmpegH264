package com.hellw.ffmpegh264;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.Button;

public class MainActivity extends AppCompatActivity {
    private Button mBt_init;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mBt_init = (Button) findViewById(R.id.bt_init);

        mBt_init.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                int result = init();
                Log.i("hellw", "init reulst = "+result);
            }
        });
    }

    public native int init();
    static {
        System.loadLibrary("x264");
        System.loadLibrary("avcodec");
        System.loadLibrary("avfilter");
        System.loadLibrary("avformat");
        System.loadLibrary("avutil");
        System.loadLibrary("swresample");
        System.loadLibrary("swscale");

        System.loadLibrary("video_play");
    }
}
