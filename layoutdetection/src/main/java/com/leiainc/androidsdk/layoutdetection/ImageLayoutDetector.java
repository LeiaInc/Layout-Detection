package com.leiainc.androidsdk.layoutdetection;

import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Bitmap;

import androidx.annotation.NonNull;

import com.leiainc.androidsdk.photoformat.ImageLayoutType;

import java.io.IOException;
import java.io.InputStream;

public abstract class ImageLayoutDetector {

    private float thresConfidenceScore = 0.90f;

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("layoutdetection");
    }

    public static ImageLayoutDetector getInstance(Context context) {
        return SupportVectorMachineLayoutDetector.getInstance(context);
    }

    protected ImageLayoutType getImageLayoutTypeFromCode(String detectionCode) {
        String[] splited = detectionCode.split("\\s+");

        if(splited.length != 2) {
            throw new RuntimeException("The detection code string must contain two numbers.");
        }

        int label = Integer.parseInt(splited[0]);
        float confidenceScore = Float.parseFloat(splited[1]);

        if(confidenceScore < thresConfidenceScore) {
            return ImageLayoutType.UNKNOWN;
        }

        switch (Integer.parseInt(detectionCode.substring(0,1))) {
            case 0:
                return ImageLayoutType.SINGLE;
            case 1:
                return ImageLayoutType.TWO_BY_ONE;
            case 2:
                return ImageLayoutType.TWO_BY_ONE_HALF;
            case 3:
                return ImageLayoutType.ONE_BY_TWO;
            case 4:
                return ImageLayoutType.TWO_BY_TWO;

        }
        return ImageLayoutType.UNKNOWN;
    }

    protected String loadSVMfileFromAsset(@NonNull Context context, @NonNull String name) {
        AssetManager assetManager = context.getAssets();
        InputStream is = null;
        try {
            is = assetManager.open(name);
        } catch (IOException e) {
            e.printStackTrace();
        }
        int length = 0;
        try {
            length = is.available();
        } catch (IOException e) {
            e.printStackTrace();
        }
        byte[] data = new byte[length];
        try {
            is.read(data);
        } catch (IOException e) {
            e.printStackTrace();
        }

        String svm_model = new String(data);

        return svm_model;
    }

    public void setThresConfidenceScore(float val) {
        thresConfidenceScore = val;
    }

    public float getThresConfidenceScore() {
        return thresConfidenceScore;
    }

    public abstract ImageLayoutType detectLayout(Bitmap bitmap);

    native String detection(Bitmap bitmap);
    native int loadSVM(String svm_model);
}
