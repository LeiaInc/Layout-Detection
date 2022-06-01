package com.leiainc.androidsdk.layoutdetection;

import android.content.Context;
import android.graphics.Bitmap;

import androidx.annotation.NonNull;

import com.leiainc.androidsdk.photoformat.ImageLayoutType;

import java.util.Objects;

class SupportVectorMachineLayoutDetector extends ImageLayoutDetector {

    private static volatile SupportVectorMachineLayoutDetector INSTANCE;

    public static SupportVectorMachineLayoutDetector getInstance(final Context context) {
        if (INSTANCE == null) {
            synchronized (SupportVectorMachineLayoutDetector.class) {
                if (INSTANCE == null) {
                    INSTANCE = new SupportVectorMachineLayoutDetector(context);
                }
            }
        }

        return INSTANCE;
    }

    private SupportVectorMachineLayoutDetector(Context context) {
        String svm_model = loadSVMfileFromAsset(context, "svm");
        int success = loadSVM( svm_model );
        if (success <= 0) {
            throw new RuntimeException("Failed to load SVM Model");
        }
    }

    @Override
    public ImageLayoutType detectLayout(@NonNull Bitmap bitmap) {
        Objects.requireNonNull(bitmap);

        // The c++ code supports only ARGB_8888
        if (bitmap.getConfig() != Bitmap.Config.ARGB_8888) {
            bitmap = bitmap.copy(Bitmap.Config.ARGB_8888, false);
        }

        String detectionCode = detection(bitmap);
        return getImageLayoutTypeFromCode(detectionCode);
    }
}
