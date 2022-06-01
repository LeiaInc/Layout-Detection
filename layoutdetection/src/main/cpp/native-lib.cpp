#include <jni.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include <android/log.h>
#include <android/bitmap.h>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include "feature.h"
#include "svm_util.h"

#define  LOG_TAG    "libBitmapConverter"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

using namespace std;
using namespace cv;

#define MAX_IMG_DIM 64
svm_model* g_model;

Mat4b wrapBitmapInMat(JNIEnv *env, jobject bitmap) {
    AndroidBitmapInfo info;
    int ret;

    if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) < 0) {
        LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
        throw std::exception();
    }

    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        LOGE("Bitmap format is not ARGB_8888!");
        throw std::exception();
    }

    void *pixels;
    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &pixels)) < 0) {
        LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
        throw std::exception();
    }

    // Wrap pixels in OpenCV Mat.
    return Mat(info.height, info.width, CV_8UC4, pixels);
}

extern "C" JNIEXPORT int JNICALL
Java_com_leiainc_androidsdk_layoutdetection_ImageLayoutDetector_loadSVM(
        JNIEnv *env,
        jobject,
        jstring svm_model) {
    const char *data = env->GetStringUTFChars(svm_model , NULL ) ;

    std::string model_content = string(data);

    g_model = loadSVMFromString(model_content);
    if(!g_model) {
        return 0;
    }
    return g_model->l; // return the number of support vectors if success.
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_leiainc_androidsdk_layoutdetection_ImageLayoutDetector_detection(
        JNIEnv *env,
        jobject,
        jobject bitmap) {

    Mat4b img;
    img = wrapBitmapInMat(env, bitmap);
    AndroidBitmap_unlockPixels(env, bitmap);
    Mat img_gray;
    cv::cvtColor(img, img_gray, cv::COLOR_BGR2GRAY);

    float feat[NUM_FEATURES];
    featureExtraction(img_gray, MAX_IMG_DIM, feat);

    int label;
    vector<double> prob;
    svmPrediction(g_model, feat, NUM_FEATURES, label, prob);

    string response = to_string(label) + " " + to_string(prob[label]);

    return env->NewStringUTF(response.c_str());
}

