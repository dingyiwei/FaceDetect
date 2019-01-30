package com.dyw.facedetect;

public class ShapePredictor {
    private static long spPtr;

    private ShapePredictor() {}

    public static long getSpPtr() {
        if (spPtr == 0) {
            spPtr = initSp();
        }
        return spPtr;
    }

    public static void destroySp() {
        decontructSp(spPtr);
    }

    private native static long initSp();

    private native static void decontructSp(long spPtr);
}
