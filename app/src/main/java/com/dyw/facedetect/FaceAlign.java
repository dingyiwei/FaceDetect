package com.dyw.facedetect;

import android.graphics.Bitmap;
import android.graphics.Rect;

import java.util.ArrayList;

public class FaceAlign {
    public static ArrayList<Bitmap> faceAlign(Bitmap bitmap) {
        long spPtr = ShapePredictor.getSpPtr();
        return faceAlign(bitmap, spPtr);
    }

    public static ArrayList<Bitmap> faceAlignWithRects(Bitmap bitmap, ArrayList<Rect> rects) {
        long spPtr = ShapePredictor.getSpPtr();
        return faceAlignWithRects(bitmap, rects, spPtr);
    }

    private static native ArrayList<Bitmap> faceAlign(Bitmap bitmap, long spPtr);

    public static native ArrayList<Bitmap> faceAlignWithRects(Bitmap bitmap, ArrayList<Rect> rects, long spPtr);
}
