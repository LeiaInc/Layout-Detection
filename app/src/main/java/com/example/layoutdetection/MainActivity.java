package com.example.layoutdetection;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.text.method.ScrollingMovementMethod;
import android.widget.TextView;

import com.leiainc.androidsdk.layoutdetection.ImageLayoutDetector;
import com.leiainc.androidsdk.photoformat.ImageLayoutType;

import java.io.IOException;
import java.io.InputStream;

public class MainActivity extends AppCompatActivity {

    private Bitmap loadBitmapFromAsset(@NonNull String name) {
        AssetManager assetManager = getAssets();
        try (InputStream is = assetManager.open(name)) {
            return BitmapFactory.decodeStream(is);
        } catch (IOException e) {
            throw new IllegalStateException(e);
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        TextView text_output = findViewById(R.id.textOutput);
        text_output.setMovementMethod(new ScrollingMovementMethod());
        String result_text = "";

        ImageLayoutDetector imageLayoutDetector = ImageLayoutDetector.getInstance(this);
        imageLayoutDetector.setThresConfidenceScore(0.90f);

        double avg_runtime = 0.0;

        for (int i = 0; i < 56; i++) {
            Bitmap bitmap = loadBitmapFromAsset(i + ".jpg");
            long start = System.currentTimeMillis();
            // Run the layout detection
            ImageLayoutType layout = imageLayoutDetector.detectLayout(bitmap);
            long elapsed_time = System.currentTimeMillis() - start;

            avg_runtime += elapsed_time;

            result_text += String.format("%d.jpg : %s (%s msec)\n", i, layout.name(), elapsed_time);
            text_output.setText(result_text);

        }
        avg_runtime /= 50.0;

        result_text += String.format("Avg. Runtime: %s msec", avg_runtime);

        text_output.setText(result_text);
    }
}
