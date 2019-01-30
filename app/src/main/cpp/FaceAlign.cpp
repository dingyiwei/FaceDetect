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

#define TAG "dyw_fa"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

std::vector<dlib::rectangle> getRects(JNIEnv *env, jobject listObj)
{
    jclass listClass = env->FindClass("java/util/ArrayList");
    jmethodID listGet = env->GetMethodID(listClass, "get", "(I)Ljava/lang/Object;");
    jmethodID listSize = env->GetMethodID(listClass, "size", "()I");
    jint size = env->CallIntMethod(listObj, listSize);

    jclass rectClass = env->FindClass("android/graphics/Rect");
    jfieldID rectLeft = env->GetStaticFieldID(rectClass, "left", "I");
    jfieldID rectTop = env->GetStaticFieldID(rectClass, "top", "I");
    jfieldID rectRight = env->GetStaticFieldID(rectClass, "right", "I");
    jfieldID rectBottom = env->GetStaticFieldID(rectClass, "bottom", "I");

    std::vector<dlib::rectangle> rects;
    for (int i = 0; i < size; i++)
    {
        jobject rectObj = env->CallObjectMethod(listObj, listGet, i);
        jint left = env->GetStaticIntField(rectClass, rectLeft);
        jint top = env->GetStaticIntField(rectClass, rectTop);
        jint right = env->GetStaticIntField(rectClass, rectRight);
        jint bottom = env->GetStaticIntField(rectClass, rectBottom);
        rects.push_back(dlib::rectangle(left, top, right, bottom));
    }

    return rects;
}

dlib::array2d<dlib::rgb_pixel> convertBitmapToDlibImg(JNIEnv *env, jobject bitmap)
{
    // convert bitmap to cv::Mat
    AndroidBitmapInfo info;
    void *pixels = nullptr;
    int ret = 0;
    if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) < 0)
    {
        LOGE("AndroidBitmap_getInfo failed");
    }
    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888)
    {
        LOGE("Bitmap format is not RGBA_8888");
    }
    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &pixels)) < 0)
    {
        LOGE("AndroidBitmap_lockPixels failed");
    }
    cv::Mat imgMat(info.height, info.width, CV_8UC4, pixels);
    AndroidBitmap_unlockPixels(env, bitmap);

    // convert cv::Mat to dlib::cv_image
    cv::Mat imgBGR;
    cv::cvtColor(imgMat, imgBGR, CV_RGBA2BGR);
    dlib::cv_image<dlib::bgr_pixel> imgDlib(imgBGR);

    dlib::array2d<dlib::rgb_pixel> img;
    assign_image(img, imgDlib);

    return img;
}

dlib::array<dlib::array2d<dlib::rgb_pixel>> faceAlign(const dlib::array2d<dlib::rgb_pixel> &img,
                                                      const std::vector<dlib::rectangle> &rects,
                                                      const long long spPtr)
{
    dlib::shape_predictor *sp = (dlib::shape_predictor*)spPtr;

    std::vector<dlib::full_object_detection> shapes;
    for (size_t i = 0; i < rects.size(); i++)
    {
        dlib::full_object_detection shape = (*sp)(img, rects[i]);
        shapes.push_back(shape);
    }

    dlib::array<dlib::array2d<dlib::rgb_pixel>> faceChips;
    dlib::extract_image_chips(img, dlib::get_face_chip_details(shapes), faceChips);

    return faceChips;
}

jobject convertFacesToList(JNIEnv *env,
                           dlib::array<dlib::array2d<dlib::rgb_pixel>> &faceChips,
                           jobject bitmap)
{
    // get android.graphics.Bitmap
    jclass bitmapClass = env->FindClass("android/graphics/Bitmap");
    jmethodID bitmapCreateBitmap =
        env->GetStaticMethodID(bitmapClass, "createBitmap",
                               "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
    jmethodID bitmapGetConfig = env->GetMethodID(bitmapClass,
                                                 "getConfig",
                                                 "()Landroid/graphics/Bitmap$Config;");
    jobject bitmapConfigObj = env->CallObjectMethod(bitmap, bitmapGetConfig);

    // get java.util.ArrayList
    jclass listClass = env->FindClass("java/util/ArrayList");
    jmethodID listConstruction = env->GetMethodID(listClass, "<init>", "()V");
    jmethodID listAdd  = env->GetMethodID(listClass, "add", "(Ljava/lang/Object;)Z");

    // instantiate an object of ArrayList
    jobject listObj = env->NewObject(listClass, listConstruction);
    for (size_t i = 0; i < faceChips.size(); i++)
    {
        // convert a dlib result to cv::Mat
        cv::Mat faceMat = dlib::toMat(faceChips[i]);

        void *facePixels = nullptr;

        // create a bitmap object
        jobject bitmapObj = env->CallStaticObjectMethod(bitmapClass,
                                                        bitmapCreateBitmap,
                                                        faceChips[i].nc(),
                                                        faceChips[i].nr(),
                                                        bitmapConfigObj);
        AndroidBitmap_lockPixels(env, bitmapObj, &facePixels);
        cv::Mat faceRGBA(faceChips[i].nc(), faceChips[i].nr(), CV_8UC4, facePixels);
        cv::cvtColor(faceMat, faceRGBA, CV_RGB2RGBA);
        AndroidBitmap_unlockPixels(env, bitmapObj);

        // add bitmap into ArrayList
        env->CallBooleanMethod(listObj, listAdd, bitmapObj);
    }

    return listObj;
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_dyw_facedetect_FaceAlign_faceAlign(JNIEnv *env, jobject, jobject bitmap, jlong spPtr)
{
    dlib::array2d<dlib::rgb_pixel> img = convertBitmapToDlibImg(env, bitmap);

    // get rectangles of faces
    dlib::frontal_face_detector detector = dlib::get_frontal_face_detector();
    std::vector<dlib::rectangle> rects = detector(img);

    // get faces pixels
    dlib::array<dlib::array2d<dlib::rgb_pixel>> faceChips = faceAlign(img, rects, spPtr);

    // convert face_chips to a list of bitmaps
    jobject listObj = convertFacesToList(env, faceChips, bitmap);

    return listObj;
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_dyw_facedetect_FaceAlign_faceAlignWithRects(JNIEnv *env, jobject, jobject bitmap, jobject rectList, jlong spPtr)
{
    dlib::array2d<dlib::rgb_pixel> img = convertBitmapToDlibImg(env, bitmap);

    std::vector<dlib::rectangle> rects = getRects(env, rectList);

    dlib::array<dlib::array2d<dlib::rgb_pixel>> faceChips = faceAlign(img, rects, spPtr);

    jobject listObj = convertFacesToList(env, faceChips, bitmap);

    return listObj;
}
