package com.dyw.facedetect;

import android.Manifest;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Environment;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;

import java.nio.ByteBuffer;
import java.util.ArrayList;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = (TextView) findViewById(R.id.sample_text);
        String path = Environment.getExternalStorageDirectory().toString() + "/Download/Obama.jpeg";
        tv.setText(path);
        if (ContextCompat.checkSelfPermission(MainActivity.this, Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            String[] permissions = { Manifest.permission.READ_EXTERNAL_STORAGE };
            ActivityCompat.requestPermissions(MainActivity.this, permissions, 1);
        }
        Bitmap bitmap = BitmapFactory.decodeFile(path);

        //ArrayList<Bitmap> faces = faceDetect(bitmap, spPtr);
        //faces = faceDetect(bitmap, spPtr);
        ArrayList<Bitmap> faces = FaceAlign.faceAlign(bitmap);
        Log.d(  "DYW MA", "begin");
        for (int i = 0; i < 100; i++) {
            faces = FaceAlign.faceAlign(bitmap);
        }
        Log.d(  "DYW MA", "end");
    }

    //public native ArrayList<Bitmap> faceDetect(Bitmap img, long spPtr);
}
