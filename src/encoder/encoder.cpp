#include "encoder.h"

AVCodecContext *Encoder::codec_ctx = NULL;

Encoder::Encoder(enum AVCodecID encoder_id, int bit_rate, enum AVSampleFormat sample_fmt) {
   codec = avcodec_find_encoder(encoder_id);
   if(!codec) av_log(NULL, AV_LOG_ERROR, "Unable to find codec\n");

   codec_ctx = avcodec_alloc_context3(codec);
   if(!codec_ctx) av_log(NULL, AV_LOG_ERROR, "Failed to allocate codec context\n");

   codec_ctx->bit_rate = bit_rate;
   codec_ctx->sample_fmt = sample_fmt;
    
   
   if(!check_sample_fmt(codec, codec_ctx->sample_fmt))
       av_log(NULL, AV_LOG_ERROR, "Codec does not support this sample format!\n");

   codec_ctx->sample_rate = select_sample_rate(codec);
   codec_ctx->channel_layout = (uint64_t)3;
   codec_ctx->channels = 2;   

   if(avcodec_open2(codec_ctx, codec, NULL) < 0) 
       av_log(NULL, AV_LOG_ERROR, "Could not open codec\n"); 
 

   buffer_size = av_samples_get_buffer_size(NULL, codec_ctx->channels,
           codec_ctx->frame_size, codec_ctx->sample_fmt, 0);

   if(buffer_size < 0)
       av_log(NULL, AV_LOG_ERROR, "Could not determine buffer size\n");
    
   av_malloc(buffer_size);
}

int Encoder::check_sample_fmt(AVCodec *codec, enum AVSampleFormat sample_fmt) {
    const enum AVSampleFormat *p = codec->sample_fmts;
    while(*p != AV_SAMPLE_FMT_NONE) {
        if(*p == sample_fmt) 
            return 1;
        p++;
    }
    return 0;
}


int Encoder::select_sample_rate(AVCodec *codec) {
    const int *p;
    int best_samplerate = 48000;

    /*if(!codec->supported_samplerates)
        return 44100;

    p = codec->supported_samplerates;
    while(*p) {
        best_samplerate = FFMAX(*p, best_samplerate);
        p++;
    }
    */
    return best_samplerate;
}

AVFrame *Encoder::new_frame(AVCodecContext *codec_ctx, uint8_t *extended_data_l,
        uint8_t *extended_data_r) {

    AVFrame *frame;

    frame = av_frame_alloc();
    frame->nb_samples = codec_ctx->frame_size;
    frame->format = AV_SAMPLE_FMT_FLTP;
    frame->channel_layout = codec_ctx->channel_layout;
    frame->channels = codec_ctx->channels;

    int ret = avcodec_fill_audio_frame(frame, frame->channels, codec_ctx->sample_fmt, 
            extended_data_r, this->buffer_size, 0);
    if(ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Could not fill frame\n");
        return NULL;
    }

    frame->extended_data[1] = extended_data_l;
    return frame; 
}

