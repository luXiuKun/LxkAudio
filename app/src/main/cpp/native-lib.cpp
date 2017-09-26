#include <jni.h>
#include <string>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>

#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"jason",FORMAT,##__VA_ARGS__);

SLObjectItf   engineObject = NULL;
SLEngineItf engineEngine;
 SLObjectItf outputMixObject = NULL;
 SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;

SLEnvironmentalReverbSettings reverbSettings =
        SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

 pthread_mutex_t  audioEngineLock = PTHREAD_MUTEX_INITIALIZER;
 SLObjectItf fdPlayerObject = NULL;
 SLPlayItf fdPlayerPlay;
 SLSeekItf fdPlayerSeek;

 SLMuteSoloItf fdPlayerMuteSolo;
 SLVolumeItf fdPlayerVolume;

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_xiukun_lxkaudio_MainActivity_player(JNIEnv *env, jobject instance, jobject assetManager,
                                             jstring filename_) {
    const char *filename = env->GetStringUTFChars(filename_, 0);

    // TODO
    // 读取assets文件
    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
    AAsset* asset = AAssetManager_open(mgr, filename, AASSET_MODE_UNKNOWN);

    if (NULL == asset) {
        LOGE("读取失败。。。");
        return JNI_FALSE;
    }


    // open asset as file descriptor
    off_t start, length;
    int fd = AAsset_openFileDescriptor(asset, &start, &length);
    AAsset_close(asset);

    // 配置信息
    SLDataLocator_AndroidFD loc_fd = {SL_DATALOCATOR_ANDROIDFD, fd, start, length};
    SLDataFormat_MIME format_mime = {SL_DATAFORMAT_MIME, NULL, SL_CONTAINERTYPE_UNSPECIFIED};
    SLDataSource audioSrc1 = {&loc_fd, &format_mime};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix1 = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk1 = {&loc_outmix1, NULL};

    // 创建播放器
    const SLInterfaceID audio_ids1[3] = {SL_IID_SEEK, SL_IID_MUTESOLO, SL_IID_VOLUME};
    const SLboolean audio_req1[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

    (*engineEngine)->CreateAudioPlayer(engineEngine, &fdPlayerObject, &audioSrc1, &audioSnk1,
                                       3, audio_ids1, audio_req1);

    (*fdPlayerObject)->Realize(fdPlayerObject, SL_BOOLEAN_FALSE);

    // 获取播放器接口
    (*fdPlayerObject)->GetInterface(fdPlayerObject, SL_IID_PLAY, &fdPlayerPlay);

    // 定位
    (*fdPlayerObject)->GetInterface(fdPlayerObject, SL_IID_SEEK, &fdPlayerSeek);

    // 静音独奏
    (*fdPlayerObject)->GetInterface(fdPlayerObject, SL_IID_MUTESOLO, &fdPlayerMuteSolo);

    // 音量
    (*fdPlayerObject)->GetInterface(fdPlayerObject, SL_IID_VOLUME, &fdPlayerVolume);

    // 开启重复播放
    (*fdPlayerSeek)->SetLoop(fdPlayerSeek, SL_BOOLEAN_TRUE, 0, SL_TIME_UNKNOWN);


    env->ReleaseStringUTFChars(filename_, filename);

    return JNI_TRUE;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_xiukun_lxkaudio_MainActivity_createEngine(JNIEnv *env, jobject instance) {

    // TODO
    SLresult result;
    //初始化引擎
    slCreateEngine(&engineObject,0,NULL,0,NULL,NULL);

    (*engineObject)->Realize(engineObject,SL_BOOLEAN_FALSE);
    //获取引擎接口
    (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);

    //创建音频输出，配置相关参数
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, ids, req);

    (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);

    //环境混响
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                              &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &reverbSettings);
    }


}

extern "C"
JNIEXPORT void JNICALL
Java_com_xiukun_lxkaudio_MainActivity_shutdown(JNIEnv *env, jobject instance) {


    if (fdPlayerObject != NULL) {
        (*fdPlayerObject)->Destroy(fdPlayerObject);
        fdPlayerObject = NULL;
        fdPlayerPlay = NULL;
        fdPlayerSeek = NULL;
        fdPlayerMuteSolo = NULL;
        fdPlayerVolume = NULL;
    }



    // destroy output mix object, and invalidate all associated interfaces
    if (outputMixObject != NULL) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
        outputMixEnvironmentalReverb = NULL;
    }

    // destroy engine object, and invalidate all associated interfaces
    if (engineObject != NULL) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }

    pthread_mutex_destroy(&audioEngineLock);

}

extern "C"
JNIEXPORT void JNICALL
Java_com_xiukun_lxkaudio_MainActivity_setPlayingAssetAudioPlayer(JNIEnv *env, jobject instance,
                                                                 jboolean isPlaying) {


    //设置播放状态
    if (NULL != fdPlayerPlay) {

       (*fdPlayerPlay)->SetPlayState(fdPlayerPlay, isPlaying ?
                                                             SL_PLAYSTATE_PLAYING : SL_PLAYSTATE_PAUSED);
    }

}




