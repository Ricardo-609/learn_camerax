package com.ricardo.learn_camerax;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.camera.core.CameraSelector;
import androidx.camera.core.ImageAnalysis;
import androidx.camera.core.ImageAnalysis.Analyzer;
import androidx.camera.core.ImageCapture;
import androidx.camera.core.ImageCaptureException;
import androidx.camera.core.ImageProxy;
import androidx.camera.core.Preview;
import androidx.camera.lifecycle.ProcessCameraProvider;
import androidx.camera.view.PreviewView;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.widget.Button;

import com.google.common.util.concurrent.ListenableFuture;

import java.io.File;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import android.Manifest;
import android.widget.ImageView;
import android.widget.RadioGroup;
import android.widget.Toast;
import java.text.SimpleDateFormat;

import static android.graphics.PixelFormat.*;
import static java.util.Locale.*;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.SeekBar;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import com.ricardo.learn_camerax.base.GPUImageFilterType;


public class MainActivity extends AppCompatActivity {
    // Used to load the 'opengl' library on application startup.
    static {
        System.loadLibrary("opengl");
    }

    private ImageCapture imageCapture;
    private File outputDirectory;
    private ExecutorService cameraExecutor;
    private ImageView imageView;
    private RadioGroup fragmentAdjustRadiogroup;
    private SeekBar seekBar;
    private  RadioButton fragmentRadioInvert;
    private RadioButton fragmentRadioContrast;
    private RadioButton fragmentRadioExposure;
    private RadioButton fragmentRadioSaturation;
    private RadioButton fragmentRadioSharpness;
    private RadioButton fragmentRadioBright;
    private RadioButton fragmentRadioHue;
    private LinearLayout filterAdjust;
    private SurfaceView surfaceView;

    private GPUImageFilterType imageFilterType;
    private int player = -1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        seekBar = findViewById(R.id.seekBar);
        fragmentRadioInvert = findViewById(R.id.fragment_radio_invert);
        fragmentRadioContrast = findViewById(R.id.fragment_radio_contrast);
        fragmentRadioExposure = findViewById(R.id.fragment_radio_exposure);
        fragmentRadioSaturation = findViewById(R.id.fragment_radio_saturation);
        fragmentRadioSharpness = findViewById(R.id.fragment_radio_sharpness);
        fragmentRadioBright = findViewById(R.id.fragment_radio_bright);
        fragmentRadioHue = findViewById(R.id.fragment_radio_hue);
        filterAdjust = findViewById(R.id.filter_adjust);
        fragmentAdjustRadiogroup = findViewById(R.id.fragment_adjust_radiogroup);
        surfaceView = findViewById(R.id.surfaceView);

        // 请求相机权限
        if (allPermissionGranted()) {
            startCamera();
        } else {
            ActivityCompat.requestPermissions(this, Configuration.REQUIRED_PERMISSIONS,
                    Configuration.REQUEST_CODE_PERMISSIONS);
        }

        // 设置拍照按钮监听
        Button camera_capture_button = findViewById(R.id.camera_capture_button);
        camera_capture_button.setOnClickListener(v -> takePhoto());

        // 设置照片保存位置
        outputDirectory = getOutputDirectory();
        cameraExecutor = Executors.newSingleThreadExecutor();

        init();
        initSetSurFaceViewHolder(surfaceView);
    }

    public void init() {
        surfaceView.getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
                if (player == -1) {
                    player = createGLRender(holder.getSurface());
                }

                showBitmap(player, BitmapFactory.decodeResource(getResources(), R.mipmap.ic_qxx));

            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {
                if (player != -1) {
                    releaseGLRender(player);
                    player = -1;
                }

            }
        });

        fragmentAdjustRadiogroup.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {


            @Override
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                if (checkedId == -1) {
                    seekBar.setVisibility(View.GONE);
                    return;
                }

                seekBar.setVisibility(View.VISIBLE);
                //image adjust filter

                imageFilterType = GPUImageFilterType.NONE;

                if (checkedId == R.id.fragment_radio_default) {
                    seekBar.setVisibility(View.INVISIBLE);
                    imageFilterType = GPUImageFilterType.NONE;
                } else if (checkedId == R.id.fragment_radio_invert) {
                    seekBar.setVisibility(View.INVISIBLE);
                    imageFilterType = GPUImageFilterType.INVERT;
                } else if (checkedId == R.id.fragment_radio_contrast) {
                    imageFilterType = GPUImageFilterType.CONTRAST;
                } else if (checkedId == R.id.fragment_radio_saturation) {
                    imageFilterType = GPUImageFilterType.SATURATION;
                } else if (checkedId == R.id.fragment_radio_exposure) {
                    imageFilterType = GPUImageFilterType.EXPOSURE;
                } else if (checkedId == R.id.fragment_radio_sharpness) {
                    imageFilterType = GPUImageFilterType.SHARPEN;
                } else if (checkedId == R.id.fragment_radio_bright) {
                    imageFilterType = GPUImageFilterType.BRIGHTNESS;
                } else if (checkedId == R.id.fragment_radio_hue) {
                    imageFilterType = GPUImageFilterType.HUE;
                }
                switchToFilterNative(player, imageFilterType.ordinal());

                seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                    @Override
                    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                        adjust(player, getAdjustValue(progress, imageFilterType), imageFilterType.ordinal());
                    }

                    @Override
                    public void onStartTrackingTouch(SeekBar seekBar) {

                    }

                    @Override
                    public void onStopTrackingTouch(SeekBar seekBar) {

                    }
                });

            }
        });

    }

    // surfaceView是独立的window窗口，即使在layout中设置好显示的层级关系，但是surfaceView不受Activity的窗口控制，
    // 所以在显示的时候要需要代码设置，指定哪个优先在上层
    private void initSetSurFaceViewHolder(SurfaceView surfaceView) {
        SurfaceHolder holder = surfaceView.getHolder();
        if (holder == null)
            return;
        holder.setKeepScreenOn(true);
        holder.setFormat(TRANSPARENT);
        surfaceView.setZOrderOnTop(true);
        surfaceView.setZOrderMediaOverlay(true);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        cameraExecutor.shutdown();
    }

    private void startCamera() {
        // 将camera的生命周期和Activity绑定在一起（设定生命周期所有者，可不用手动控制）
        ListenableFuture<ProcessCameraProvider> cameraProviderListenableFuture = ProcessCameraProvider.getInstance(this);

        cameraProviderListenableFuture.addListener(() -> {
            try {
                // 将相机和当前生命周期的所有者绑定所需的对象
                ProcessCameraProvider processCameraProvider = cameraProviderListenableFuture.get();
                // 创建一个Preview 实例，并设置该实例的 surface 提供者（provider）
                PreviewView viewFinder = (PreviewView)findViewById(R.id.viewFinder);
                Preview preview = new Preview.Builder().build();
                preview.setSurfaceProvider(viewFinder.getSurfaceProvider());

                // 选择后置摄像头作为默认摄像头
                CameraSelector cameraSelector = CameraSelector.DEFAULT_BACK_CAMERA;

                // 创建拍照所需的实例
                imageCapture = new ImageCapture.Builder().build();

                // 设置预览帧分析
                ImageAnalysis imageAnalysis = new ImageAnalysis.Builder()
                        .build();
                imageAnalysis.setAnalyzer(cameraExecutor, new MyAnalyzer());


                // 重新绑定用例前先解绑
                processCameraProvider.unbindAll();
                // 绑定用例至相机
//                processCameraProvider.bindToLifecycle(MainActivity.this, cameraSelector, preview);

                // 绑定拍照用例
                processCameraProvider.bindToLifecycle(MainActivity.this, cameraSelector,
                        preview,
                        imageCapture,
                        imageAnalysis);
            } catch (Exception e) {
                Log.e(Configuration.TAG, "绑定失败");
            }
        }, ContextCompat.getMainExecutor(this));

    }

    private void takePhoto() {
        // 确保imageCapture 已经被实例化, 否则程序将可能崩溃
        if (imageCapture != null) {
            // 创建带时间戳的输出文件以保存图片，带时间戳是为了保证文件名唯一
            File photoFile = new File(outputDirectory,
                    new SimpleDateFormat(Configuration.FILENAME_FORMAT,
                            SIMPLIFIED_CHINESE).format(System.currentTimeMillis())
                            + ".jpg");

            // 创建 output option 对象，用以指定照片的输出方式
            ImageCapture.OutputFileOptions outputFileOptions = new ImageCapture.OutputFileOptions
                    .Builder(photoFile)
                    .build();

            // 执行takePicture（拍照）方法
            imageCapture.takePicture(outputFileOptions,
                    ContextCompat.getMainExecutor(this),
                    new ImageCapture.OnImageSavedCallback() {       // 保存照片时的回调
                        @Override
                        public void onImageSaved(ImageCapture.OutputFileResults outputFileResults) {
                            Uri savedUri = Uri.fromFile(photoFile);
                            String msg = "照片捕获成功! " + savedUri;
                            Toast.makeText(getBaseContext(), msg, Toast.LENGTH_SHORT).show();
                            Log.d(Configuration.TAG, msg);
                        }

                        @Override
                        public void onError(@NonNull ImageCaptureException exception) {
                            Log.e(Configuration.TAG, "Photo capture failed: " + exception.getMessage());

                        }
                    });
        }
    }

    private File getOutputDirectory() {
        File mediaDir = new File(getExternalMediaDirs()[0], getString(R.string.app_name));
        boolean isExist = mediaDir.exists() || mediaDir.mkdir();
        return isExist ? mediaDir : null;
    }

    private boolean allPermissionGranted() {
        for (String permission : Configuration.REQUIRED_PERMISSIONS) {
            if (ContextCompat.checkSelfPermission(this, permission)
                    != PackageManager.PERMISSION_GRANTED) {
                return false;
            }
        }
        return true;
    }

    static class Configuration {
        public static final String TAG = "learn_camerax";
        public static final String FILENAME_FORMAT = "yyyy-MM-dd-HH-mm-ss-SSS";
        public static final int REQUEST_CODE_PERMISSIONS = 10;
        public static final String[] REQUIRED_PERMISSIONS = new String[]{
                Manifest.permission.CAMERA,
                Manifest.permission.READ_EXTERNAL_STORAGE,
                Manifest.permission.WRITE_EXTERNAL_STORAGE
                };
    }

    private static class MyAnalyzer implements Analyzer{
        @Override
        public void analyze(@NonNull ImageProxy image) {
            Log.d(Configuration.TAG, "Image's stamp is " + image.getImage().getTimestamp());
            image.close();
        }
    }

    public float getAdjustValue(int progress, GPUImageFilterType gpuImageFilterType) {
        float value = 0;
        switch (gpuImageFilterType) {
            case CONTRAST:
                value = range(progress, 0.0f, 2.0f);
                break;
            case BRIGHTNESS:
                value = range(progress, -1.0f, 1.0f);
                break;
            case EXPOSURE:
                value = range(progress, -2.0f, 2.0f);
                break;
            case SATURATION:
                value = range(progress, 0.0f, 2.0f);
                break;
            case HUE:
                value = range(progress, 0.0f, 360.0f);
                break;
            case SHARPEN:
                value = range(progress, -4.0f, 4.0f);
                break;
        }
        return value;
    }

    static float range(int percentage, float start, float end) {
        return (end - start) * percentage / 100.0f + start;
    }

    static int range(int percentage, int start, int end) {
        return (end - start) * percentage / 100 + start;
    }

    public native void switchToFilterNative(int render, int filterType);

    public native void adjust(int render, float value, int filterType);

    public native void releaseGLRender(int render);

    public native void showBitmap(int player, Bitmap bitmap);

    public native int createGLRender(Surface surface);
}

