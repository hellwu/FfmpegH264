package com.hellw.ffmpegh264;

import android.app.Activity;
import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;

import java.io.IOException;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import static android.hardware.Camera.Parameters.FLASH_MODE_AUTO;
import static android.hardware.Camera.Parameters.PREVIEW_FPS_MAX_INDEX;
import static android.hardware.Camera.Parameters.PREVIEW_FPS_MIN_INDEX;

public class MainActivity extends AppCompatActivity implements SurfaceHolder.Callback, Camera.PreviewCallback {
    private static final String TAG = "MainActivity";
    private Button mBt_init;
    private SurfaceView mSurfaceView;
    private SurfaceHolder mSurfaceHolder;
    private Camera mCamera;
    private Camera.Size size;
    private boolean isPreview;
    private boolean isPublish;
    private boolean isStarted;
    private H264Encoder mH264Encoder;
    private ExecutorService excutor;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        initView();
        mH264Encoder = new H264Encoder();
        excutor = Executors.newSingleThreadExecutor();
    }

    private void initView() {
        mBt_init = (Button) findViewById(R.id.bt_init);
        mSurfaceView = (SurfaceView) findViewById(R.id.sv);
        mSurfaceHolder = mSurfaceView.getHolder();
        mSurfaceHolder.addCallback(this);
        mBt_init.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                start();
            }
        });
    }

    @Override
    public void onPreviewFrame(final byte[] data, Camera camera) {
        if (isPublish) {
            excutor.execute(new Runnable() {
                @Override
                public void run() {
                    mH264Encoder.encoder(data, size.width, size.height);
                }
            });
        }

        int buffSize = size.width * size.height * ImageFormat.getBitsPerPixel(ImageFormat.NV21) / 8;
        if (data == null) {
            mCamera.addCallbackBuffer(new byte[buffSize]);
        } else {
            mCamera.addCallbackBuffer(data);
        }
    }


    @Override
    public void surfaceCreated(SurfaceHolder surfaceHolder) {

    }

    @Override
    public void surfaceChanged(SurfaceHolder surfaceHolder, int i, int i1, int i2) {
        initCamera();
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder surfaceHolder) {

    }

    private void start() {
        if (isStarted) {
            stop();
            isStarted = false;

        } else {
            isStarted = true;
            isPublish = true;
            mH264Encoder.init("/sdcard/camera.h264", size.width, size.height);
        }
    }

    private void stop() {
        isPublish = false;
        mH264Encoder.destroy();
    }

    private void openCamera() {
        try {
            this.mCamera = Camera.open();
        } catch (RuntimeException e) {
            throw new RuntimeException("打开摄像头失败", e);
        }
    }

    private void initCamera() {
        if (this.mCamera == null) {
            openCamera();
        }

        setParameters();
        setCameraDisplayOrientation(this, Camera.CameraInfo.CAMERA_FACING_BACK, mCamera);

        int buffSize = size.width * size.height * ImageFormat.getBitsPerPixel(ImageFormat.NV21) / 8;
        mCamera.addCallbackBuffer(new byte[buffSize]);
        mCamera.setPreviewCallbackWithBuffer(this);
        try {
            mCamera.setPreviewDisplay(mSurfaceHolder);
        } catch (IOException e) {
            e.printStackTrace();
        }
        if (isPreview) {
            mCamera.stopPreview();
            isPreview = false;
        }
        mCamera.startPreview();
        isPreview = true;
    }

    public static void setCameraDisplayOrientation(Activity activity,
                                                   int cameraId, android.hardware.Camera camera) {
        android.hardware.Camera.CameraInfo info =
                new android.hardware.Camera.CameraInfo();
        android.hardware.Camera.getCameraInfo(cameraId, info);
        int rotation = activity.getWindowManager().getDefaultDisplay()
                .getRotation();
        int degrees = 0;
        switch (rotation) {
            case Surface.ROTATION_0:
                degrees = 0;
                break;
            case Surface.ROTATION_90:
                degrees = 90;
                break;
            case Surface.ROTATION_180:
                degrees = 180;
                break;
            case Surface.ROTATION_270:
                degrees = 270;
                break;
        }

        int result;
        if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
            result = (info.orientation + degrees) % 360;
            result = (360 - result) % 360;  // compensate the mirror
        } else {  // back-facing
            result = (info.orientation - degrees + 360) % 360;
        }
        camera.setDisplayOrientation(result);
    }


    private void setParameters() {
        Camera.Parameters parameters = mCamera.getParameters();
        List<Camera.Size> supportedPreviewSizes = parameters.getSupportedPreviewSizes();
        for (Camera.Size supportSize : supportedPreviewSizes) {
            if (supportSize.width >= 160 && supportSize.width <= 240) {
                this.size = supportSize;
                Log.i(TAG, "setParameters: width:" + size.width + " ,height:" + size.height);
                break;
            }
        }

        int defFPS = 20 * 1000;
        List<int[]> supportedPreviewFpsRange = parameters.getSupportedPreviewFpsRange();

        int[] destRange = null;
        for (int i = 0; i < supportedPreviewFpsRange.size(); i++) {
            int[] range = supportedPreviewFpsRange.get(i);
            if (range[PREVIEW_FPS_MAX_INDEX] >= defFPS) {
                destRange = range;
                Log.i(TAG, "setParameters: destRange:" + Arrays.toString(range));
                break;
            }
        }

        parameters.setPreviewFpsRange(destRange[PREVIEW_FPS_MIN_INDEX],
                destRange[PREVIEW_FPS_MAX_INDEX]);
        parameters.setPreviewSize(size.width, size.height);
        parameters.setFlashMode(FLASH_MODE_AUTO);
        parameters.setPreviewFormat(ImageFormat.NV21);
        mCamera.setParameters(parameters);
    }
}
