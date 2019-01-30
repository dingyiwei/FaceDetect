//
// Created by Ding Yiwei on 19-1-29.
//

#include <jni.h>
#include <string>
#include <android/log.h>
#include <android/bitmap.h>
#include <opencv2/opencv.hpp>
#include <dlib/opencv.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>
#include <dlib/image_io.h>

#define TAG "dyw_sp"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

extern "C" JNIEXPORT jlong JNICALL
Java_com_dyw_facedetect_ShapePredictor_initSp(JNIEnv *env, jobject)
{
    dlib::shape_predictor *sp = new dlib::shape_predictor();
    dlib::deserialize("/storage/emulated/0/Download/shape_predictor_68_face_landmarks.dat") >> (*sp);
    jlong spPtr = (long long)sp;
    return spPtr;
}

extern "C" JNIEXPORT void JNICALL
Java_com_dyw_facedetect_ShapePredictor_decontructSp(JNIEnv *env, jobject, jlong spPtr)
{
    dlib::shape_predictor *sp = (dlib::shape_predictor*)spPtr;
    delete sp;
}
