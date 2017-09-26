package com.xiukun.lxkaudio;

import android.content.Context;
import android.content.res.AssetManager;
import android.media.AudioManager;
import android.os.Build;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity implements View.OnClickListener{

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }
    static boolean isPlayingAsset = false;
    boolean created = false;
    private AssetManager assetsManager;
    private Button btn;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        btn = (Button) findViewById(R.id.btn);
        btn.setOnClickListener(this);
        assetsManager = getAssets();
        createEngine();

    }




    public  native void setPlayingAssetAudioPlayer(boolean isPlaying);
    public  native void shutdown();
    public native void createEngine();
    public native boolean player(AssetManager assetManager, String filename);

    @Override
    public void onClick(View view) {
        switch (view.getId()){

            case R.id.btn:
                if (!created) {
                    created = player(assetsManager,"王菲 - 匆匆那年.mp3");
                }
                if (created) {

                    isPlayingAsset = !isPlayingAsset;
                    if (isPlayingAsset){
                        btn.setText("暂停");
                    }else{
                        btn.setText("播放");
                    }
                    setPlayingAssetAudioPlayer(isPlayingAsset);

                }
                break;



        }

    }



    @Override
    protected void onDestroy() {

        shutdown();
        super.onDestroy();
    }
}
