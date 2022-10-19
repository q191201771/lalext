#include <stdio.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavformat/avformat.h>
#include <libavutil/timestamp.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libswresample/swresample.h>
#include <libavutil/audio_fifo.h>
#include <libavutil/samplefmt.h>

typedef struct AudioPrcessContex
{
    // decoder
    AVCodecContext *decodeContex;
    int in_samplerate;
    int in_channels;
    int in_format;

    // encoder
    AVCodecContext *encoderContex;
    int out_samplerate;
    int out_channels;
    int out_format;

    // resample
    struct SwrContext *resample_context;
    AVFrame *converted_frame;
    int out_nb_samples;

    // fifo
    AVAudioFifo *fifo;

    // dump
    AVFormatContext *output_format_context;
    FILE *dump;
    int enable_dump;
} AudioPrcessContex;

char errorStr[AV_ERROR_MAX_STRING_SIZE];

static int init_codec_contex(AVCodec *codec, AVCodecContext **ctx, int samplerate, int channels, enum AVSampleFormat format)
{
    AVCodecContext *contex = avcodec_alloc_context3(codec);
    if (!contex)
    {
        fprintf(stderr, "aac codec context alloc failed\n");
        return AVERROR_EXIT;
    }

    contex->codec_type = codec->type;
    contex->bit_rate = 0; // default by decoder
    contex->sample_fmt = format;
    contex->sample_rate = samplerate;
    contex->channels = channels;
    contex->channel_layout = av_get_default_channel_layout(channels);

    int ret = avcodec_open2(contex, contex->codec, NULL);
    if (ret < 0)
    {
        fprintf(stderr, "Could not open input codec (error '%s')\n", av_make_error_string(errorStr, AV_ERROR_MAX_STRING_SIZE, ret));
        return ret;
    }

    *ctx = contex;

    return 0;
}

static int init_aac_decoder(AVCodecContext **ctx, int samplerate, int channels, enum AVSampleFormat format)
{
    AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_AAC);
    if (!codec)
    {
        fprintf(stderr, "Could not find input codec\n");
        return AVERROR_EXIT;
    }

    return init_codec_contex(codec, ctx, samplerate, channels, format);
}

static int init_opus_encoder(AVCodecContext **ctx, int samplerate, int channels, enum AVSampleFormat format)
{
    AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_OPUS);

    if (!codec)
    {
        fprintf(stderr, "Could not find input codec\n");
        return AVERROR_EXIT;
    }

    return init_codec_contex(codec, ctx, samplerate, channels, format);
}

/**
 * Initialize a FIFO buffer for the audio samples to be encoded.
 * @param[out] fifo                 Sample buffer
 * @param      output_codec_context Codec context of the output file
 * @return Error code (0 if successful)
 */
static int init_fifo(AVAudioFifo **fifo, AVCodecContext *output_codec_context)
{
    /* Create the FIFO buffer based on the specified output sample format. */
    if (!(*fifo = av_audio_fifo_alloc(output_codec_context->sample_fmt, output_codec_context->channels, 1)))
    {
        fprintf(stderr, "Could not allocate FIFO\n");
        return AVERROR(ENOMEM);
    }
    return 0;
}

static int init_resampler(AVCodecContext *input_codec_context, AVCodecContext *output_codec_context, SwrContext **resample_context, AVFrame **frame)
{
    int error;

    /*
     * Create a resampler context for the conversion.
     * Set the conversion parameters.
     * Default channel layouts based on the number of channels
     * are assumed for simplicity (they are sometimes not detected
     * properly by the demuxer and/or decoder).
     */
    *resample_context = swr_alloc_set_opts(
        NULL, av_get_default_channel_layout(output_codec_context->channels), output_codec_context->sample_fmt, output_codec_context->sample_rate,
        av_get_default_channel_layout(input_codec_context->channels), input_codec_context->sample_fmt, input_codec_context->sample_rate, 0, NULL);
    if (!*resample_context)
    {
        fprintf(stderr, "Could not allocate resample context\n");
        return AVERROR(ENOMEM);
    }

    /* Open the resampler with the specified parameters. */
    if ((error = swr_init(*resample_context)) < 0)
    {
        fprintf(stderr, "Could not open resample context\n");
        swr_free(resample_context);
        return error;
    }

    // init frame
    AVFrame *tmpFrame = av_frame_alloc();
    tmpFrame->format = output_codec_context->sample_fmt;
    tmpFrame->channel_layout = output_codec_context->channel_layout;
    tmpFrame->sample_rate = output_codec_context->sample_rate;
    tmpFrame->nb_samples = 0;

    return 0;
}

/**
 * Open an output file and the required encoder.
 * Also set some basic encoder parameters.
 * Some of these parameters are based on the input file's parameters.
 * @param      filename              File to be opened
 * @param      input_codec_context   Codec context of input file
 * @param[out] output_format_context Format context of output file
 * @param[out] output_codec_context  Codec context of output file
 * @return Error code (0 if successful)
 */
static int open_output_file(const char *filename, AVFormatContext **output_format_context, AVCodecContext *output_codec_context)
{
    AVIOContext *output_io_context = NULL;
    AVStream *stream = NULL;
    int error;

    /* Open the output file to write to it. */
    if ((error = avio_open(&output_io_context, filename, AVIO_FLAG_WRITE)) < 0)
    {
        fprintf(stderr, "Could not open output file '%s' (error '%s')\n", filename, av_make_error_string(errorStr, AV_ERROR_MAX_STRING_SIZE, error));
        return error;
    }

    /* Create a new format context for the output container format. */
    if (!(*output_format_context = avformat_alloc_context()))
    {
        fprintf(stderr, "Could not allocate output format context\n");
        return AVERROR(ENOMEM);
    }

    /* Associate the output file (pointer) with the container format context. */
    (*output_format_context)->pb = output_io_context;

    /* Guess the desired container format based on the file extension. */
    if (!((*output_format_context)->oformat = av_guess_format(NULL, filename, NULL)))
    {
        fprintf(stderr, "Could not find output file format\n");
        goto cleanup;
    }

    if (!((*output_format_context)->url = av_strdup(filename)))
    {
        fprintf(stderr, "Could not allocate url.\n");
        error = AVERROR(ENOMEM);
        goto cleanup;
    }

    /* Create a new audio stream in the output file container. */
    if (!(stream = avformat_new_stream(*output_format_context, NULL)))
    {
        fprintf(stderr, "Could not create new stream\n");
        error = AVERROR(ENOMEM);
        goto cleanup;
    }

    /* Set the sample rate for the container. */
    stream->time_base.den = output_codec_context->sample_rate;
    stream->time_base.num = 1;

    error = avcodec_parameters_from_context(stream->codecpar, output_codec_context);
    if (error < 0)
    {
        fprintf(stderr, "Could not initialize stream parameters\n");
        goto cleanup;
    }

    /* Some container formats (like MP4) require global headers to be present.
     * Mark the encoder so that it behaves accordingly. */
    if ((*output_format_context)->oformat->flags & AVFMT_GLOBALHEADER)
        output_codec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    return 0;

cleanup:
    avio_closep(&(*output_format_context)->pb);
    avformat_free_context(*output_format_context);
    *output_format_context = NULL;
    return error < 0 ? error : AVERROR_EXIT;
}

static int write_output_file_header(AVFormatContext *output_format_context);

static int init_aac_2_opus_contex(struct AudioPrcessContex *contex)
{
    int ret = 0;
    if (ret = init_aac_decoder(&contex->decodeContex, contex->in_samplerate, contex->in_channels, (enum AVSampleFormat)contex->in_format))
        return ret;

    if (ret = init_opus_encoder(&contex->encoderContex, contex->out_samplerate, contex->out_channels, (enum AVSampleFormat)contex->out_format))
        return ret;

    if (ret = init_resampler(contex->decodeContex, contex->encoderContex, &contex->resample_context, &contex->converted_frame))
        return ret;

    if (ret = init_fifo(&contex->fifo, contex->encoderContex))
        return ret;

    if (contex->enable_dump)
    {
        if (ret = open_output_file("opus_48000_2.ogg", &contex->output_format_context, contex->encoderContex))
            return ret;

        if (ret = write_output_file_header(contex->output_format_context))
            return ret;
    }

    // contex->dump = fopen("c_raw_data.bin", "wb+");

    return 0;
}

static int init_packet(AVPacket **packet)
{
    if (!(*packet = av_packet_alloc()))
    {
        fprintf(stderr, "Could not allocate packet\n");
        return AVERROR(ENOMEM);
    }
    return 0;
}

static int flush_swr_context(struct AudioPrcessContex *ctx);

static void clean_up_audio_process_contex(struct AudioPrcessContex *ctx)
{
    // 先清理resample缓存
    flush_swr_context(ctx);

    avcodec_free_context(&ctx->decodeContex);

    avcodec_free_context(&ctx->encoderContex);

    swr_free(&ctx->resample_context);

    av_frame_free(&ctx->converted_frame);

    av_audio_fifo_free(ctx->fifo);

    avformat_free_context(ctx->output_format_context);
}

static int decode_audio_frame(AVFrame *frame, AVCodecContext *ctx, uint8_t *data, int size, int *decoded)
{
    AVPacket *input_packet;
    int error;

    error = init_packet(&input_packet);
    if (error < 0)
        return error;
    av_init_packet(input_packet);
    input_packet->data = data;
    input_packet->size = size;

    if ((error = avcodec_send_packet(ctx, input_packet)) < 0)
    {
        fprintf(stderr, "Could not send packet for decoding (error '%s')\n", av_make_error_string(errorStr, AV_ERROR_MAX_STRING_SIZE, error));
        goto cleanup;
    }

    /* Receive one frame from the decoder. */
    error = avcodec_receive_frame(ctx, frame);
    /* If the decoder asks for more data to be able to decode a frame,
     * return indicating that no data is present. */
    if (error == AVERROR(EAGAIN))
    {
        error = 0;
        goto cleanup;
        /* If the end of the input file is reached, stop decoding. */
    }
    else if (error < 0)
    {
        fprintf(stderr, "Could not decode frame (error '%s')\n", av_make_error_string(errorStr, AV_ERROR_MAX_STRING_SIZE, error));
        goto cleanup;
        /* Default case: Return decoded data. */
    }
    else
    {
        *decoded = 1;
        goto cleanup;
    }

cleanup:
    av_packet_free(&input_packet);

    return 0;
}

static int resample_audio_frame(SwrContext *ctx, AVFrame *in_frame, AVFrame *out_frame)
{
    return 0;
}

static int init_input_frame(AVFrame **frame)
{
    if (!(*frame = av_frame_alloc()))
    {
        fprintf(stderr, "Could not allocate input frame\n");
        return AVERROR(ENOMEM);
    }
    return 0;
}

static int init_converted_samples(uint8_t ***converted_input_samples, AVCodecContext *output_codec_context, int frame_size, struct AudioPrcessContex *ctx)
{
    int error;

    /* Allocate as many pointers as there are audio channels.
     * Each pointer will later point to the audio samples of the corresponding
     * channels (although it may be NULL for interleaved formats).
     */

    int dst_nb_samples = av_rescale_rnd(
        swr_get_delay(ctx->resample_context, ctx->encoderContex->sample_rate) + frame_size, output_codec_context->sample_rate,
        ctx->encoderContex->sample_rate, AV_ROUND_UP);

    if (dst_nb_samples > ctx->out_nb_samples)
    {
        ctx->out_nb_samples = dst_nb_samples;
    }

    if (!(*converted_input_samples = (uint8_t **)calloc(output_codec_context->channels, sizeof(**converted_input_samples))))
    {
        fprintf(stderr, "Could not allocate converted input sample pointers\n");
        return AVERROR(ENOMEM);
    }

    /* Allocate memory for the samples of all channels in one consecutive
     * block for convenience. */
    if ((error = av_samples_alloc(
             *converted_input_samples, NULL, output_codec_context->channels, ctx->out_nb_samples, output_codec_context->sample_fmt, 1)) < 0)
    {
        fprintf(stderr, "Could not allocate converted input samples (error '%s')\n", av_make_error_string(errorStr, AV_ERROR_MAX_STRING_SIZE, error));
        av_freep(&(*converted_input_samples)[0]);
        free(*converted_input_samples);
        return error;
    }
    return 0;
}

static int convert_samples(
    const uint8_t **input_data, uint8_t **converted_data, const int frame_size, SwrContext *resample_context, AudioPrcessContex *ctx)
{
    int samples;

    /* Convert the samples using the resampler. */
    if ((samples = swr_convert(resample_context, converted_data, ctx->out_nb_samples, input_data, frame_size)) < 0)
    {
        fprintf(stderr, "Could not convert input samples (error '%s')\n", av_make_error_string(errorStr, AV_ERROR_MAX_STRING_SIZE, samples));
        return samples;
    }

    return samples;
}

/**
 * Add converted input audio samples to the FIFO buffer for later processing.
 * @param fifo                    Buffer to add the samples to
 * @param converted_input_samples Samples to be added. The dimensions are channel
 *                                (for multi-channel audio), sample.
 * @param frame_size              Number of samples to be converted
 * @return Error code (0 if successful)
 */
static int add_samples_to_fifo(AVAudioFifo *fifo, uint8_t **converted_input_samples, const int frame_size)
{
    int error;

    /* Make the FIFO as large as it needs to be to hold both,
     * the old and the new samples. */
    if ((error = av_audio_fifo_realloc(fifo, av_audio_fifo_size(fifo) + frame_size)) < 0)
    {
        fprintf(stderr, "Could not reallocate FIFO\n");
        return error;
    }

    /* Store the new samples in the FIFO buffer. */
    if (av_audio_fifo_write(fifo, (void **)converted_input_samples, frame_size) < frame_size)
    {
        fprintf(stderr, "Could not write data to FIFO\n");
        return AVERROR_EXIT;
    }
    return 0;
}

static int read_decode_convert_and_store(
    AVAudioFifo *fifo, AVCodecContext *input_codec_context, AVCodecContext *output_codec_context, SwrContext *resampler_context, uint8_t *data,
    int size, AudioPrcessContex *ctx)
{
    /* Temporary storage of the input samples of the frame read from the file. */
    AVFrame *input_frame = NULL;
    /* Temporary storage for the converted input samples. */
    uint8_t **converted_input_samples = NULL;
    int data_present = 0;
    int ret = AVERROR_EXIT;

    /* Initialize temporary storage for one input frame. */
    if (init_input_frame(&input_frame))
        goto cleanup;
    /* Decode one frame worth of audio samples. */
    if (decode_audio_frame(input_frame, input_codec_context, data, size, &data_present))
        goto cleanup;

    /* If there is decoded data, convert and store it. */
    if (data_present)
    {
        /* Initialize the temporary storage for the converted input samples. */
        if (init_converted_samples(&converted_input_samples, output_codec_context, input_frame->nb_samples, ctx))
            goto cleanup;

        /* Convert the input samples to the desired output sample format.
         * This requires a temporary storage provided by converted_input_samples. */
        int conver_samples =
            convert_samples((const uint8_t **)input_frame->extended_data, converted_input_samples, input_frame->nb_samples, resampler_context, ctx);
        if (conver_samples < 0)
            goto cleanup;

        /* Add the converted input samples to the FIFO buffer for later processing. */
        if (add_samples_to_fifo(fifo, converted_input_samples, conver_samples))
            goto cleanup;

        ret = 0;
    }
    ret = 0;

cleanup:
    if (converted_input_samples)
    {
        av_freep(&converted_input_samples[0]);
        free(converted_input_samples);
    }
    av_frame_free(&input_frame);

    return ret;
}

/**
 * Initialize one input frame for writing to the output file.
 * The frame will be exactly frame_size samples large.
 * @param[out] frame                Frame to be initialized
 * @param      output_codec_context Codec context of the output file
 * @param      frame_size           Size of the frame
 * @return Error code (0 if successful)
 */
static int init_output_frame(AVFrame **frame, AVCodecContext *output_codec_context, int frame_size)
{
    int error;

    /* Create a new frame to store the audio samples. */
    if (!(*frame = av_frame_alloc()))
    {
        fprintf(stderr, "Could not allocate output frame\n");
        return AVERROR_EXIT;
    }

    /* Set the frame's parameters, especially its size and format.
     * av_frame_get_buffer needs this to allocate memory for the
     * audio samples of the frame.
     * Default channel layouts based on the number of channels
     * are assumed for simplicity. */
    (*frame)->nb_samples = frame_size;
    (*frame)->channel_layout = output_codec_context->channel_layout;
    (*frame)->format = output_codec_context->sample_fmt;
    (*frame)->sample_rate = output_codec_context->sample_rate;

    /* Allocate the samples of the created frame. This call will make
     * sure that the audio frame can hold as many samples as specified. */
    if ((error = av_frame_get_buffer(*frame, 0)) < 0)
    {
        fprintf(stderr, "Could not allocate output frame samples (error '%s')\n", av_make_error_string(errorStr, AV_ERROR_MAX_STRING_SIZE, error));
        av_frame_free(frame);
        return error;
    }

    return 0;
}

/**
 * Write the header of the output file container.
 * @param output_format_context Format context of the output file
 * @return Error code (0 if successful)
 */
static int write_output_file_header(AVFormatContext *output_format_context)
{
    int error;
    if ((error = avformat_write_header(output_format_context, NULL)) < 0)
    {
        fprintf(stderr, "Could not write output file header (error '%s')\n", av_make_error_string(errorStr, AV_ERROR_MAX_STRING_SIZE, error));
        return error;
    }
    return 0;
}

// flush resample
static int flush_swr_context(AudioPrcessContex *ctx)
{
    AVFrame *frame;

    if (init_output_frame(&frame, ctx->encoderContex, ctx->encoderContex->frame_size))
        return -1;

    int out_samples = 0;

    do
    {
        out_samples = swr_convert_frame(ctx->resample_context, frame, NULL);
        printf("flush resample cache: %d", out_samples);
    } while (out_samples);

    av_frame_free(&frame);

    return 0;
}
/**
 * Encode one frame worth of audio to the output file.
 * @param      frame                 Samples to be encoded
 * @param      output_format_context Format context of the output file
 * @param      output_codec_context  Codec context of the output file
 * @param[out] data_present          Indicates whether data has been
 *                                   encoded
 * @return Error code (0 if successful)
 */
static int encode_audio_frame(AVFrame *frame, AVFormatContext *output_format_context, AVCodecContext *output_codec_context, int *data_present, AVPacket *output_packet, int enable_dump)
{
    ///* Set a timestamp based on the sample rate for the container. */
    /*if (frame)
    {
        frame->pts = pts;
        pts += frame->nb_samples;
    }*/

    int error = 0;

    /* Send the audio frame stored in the temporary packet to the encoder.
     * The output audio stream encoder is used to do this. */
    error = avcodec_send_frame(output_codec_context, frame);
    /* The encoder signals that it has nothing more to encode. */
    if (error == AVERROR_EOF)
    {
        error = 0;
        goto cleanup;
    }
    else if (error < 0)
    {
        fprintf(stderr, "Could not send packet for encoding (error '%s')\n", av_make_error_string(errorStr, AV_ERROR_MAX_STRING_SIZE, error));
        goto cleanup;
    }

    /* Receive one encoded frame from the encoder. */
    error = avcodec_receive_packet(output_codec_context, output_packet);
    /* If the encoder asks for more data to be able to provide an
     * encoded frame, return indicating that no data is present. */
    if (error == AVERROR(EAGAIN))
    {
        error = 0;
        goto cleanup;
        /* If the last frame has been encoded, stop encoding. */
    }
    else if (error == AVERROR_EOF)
    {
        error = 0;
        goto cleanup;
    }
    else if (error < 0)
    {
        fprintf(stderr, "Could not encode frame (error '%s')\n", av_make_error_string(errorStr, AV_ERROR_MAX_STRING_SIZE, error));
        goto cleanup;
        /* Default case: Return encoded data. */
    }
    else
    {
        *data_present = 1;
    }

    // for debug
    /* Write one audio frame from the temporary packet to the output file. */
    if (enable_dump && *data_present && output_format_context != NULL)
    {
        if (error = av_write_frame(output_format_context, output_packet) < 0)
        {
            fprintf(stderr, "Could not write frame (error '%s')\n", av_make_error_string(errorStr, AV_ERROR_MAX_STRING_SIZE, error));
            goto cleanup;
        }
    }

cleanup:

    return error;
}

/**
 * Load one audio frame from the FIFO buffer, encode and write it to the
 * output file.
 * @param fifo                  Buffer used for temporary storage
 * @param output_format_context Format context of the output file
 * @param output_codec_context  Codec context of the output file
 * @return Error code (0 if successful)
 */
static int load_encode_and_write(AVAudioFifo *fifo, AVFormatContext *output_format_context, AVCodecContext *output_codec_context, uint8_t *out_data, int *out_len, int enable_dump)
{
    /* Temporary storage of the output samples of the frame written to the file. */
    AVFrame *output_frame;
    /* Use the maximum number of possible samples per frame.
     * If there is less than the maximum possible frame size in the FIFO
     * buffer use this number. Otherwise, use the maximum possible frame size. */
    const int frame_size = FFMIN(av_audio_fifo_size(fifo), output_codec_context->frame_size);
    int data_written;

    /* Initialize temporary storage for one output frame. */
    if (init_output_frame(&output_frame, output_codec_context, frame_size))
        return AVERROR_EXIT;

    /* Packet used for temporary storage. */
    AVPacket *output_packet;
    int error;

    error = init_packet(&output_packet);
    if (error < 0)
        return error;

    /* Read as many samples from the FIFO buffer as required to fill the frame.
     * The samples are stored in the frame temporarily. */
    if (av_audio_fifo_read(fifo, (void **)output_frame->data, frame_size) < frame_size)
    {
        fprintf(stderr, "Could not read data from FIFO\n");
        av_frame_free(&output_frame);
        return AVERROR_EXIT;
    }

    /* Encode one frame worth of audio samples. */
    if (encode_audio_frame(output_frame, output_format_context, output_codec_context, &data_written, output_packet, enable_dump))
    {
        av_frame_free(&output_frame);
        return AVERROR_EXIT;
    }

    if (data_written && out_data)
    {
        memcpy(out_data, output_packet->data, output_packet->size);
        *out_len = output_packet->size;
    }

    av_frame_free(&output_frame);
    av_packet_free(&output_packet);
    return 0;
}

static int fifo_enough_to_encode(AudioPrcessContex *ctx)
{
    int ret = 0;
    if (av_audio_fifo_size(ctx->fifo) > ctx->encoderContex->frame_size)
        ret = 1;

    return ret;
}

static int decode_resample_and_store(int8_t *in_data, int in_len, AudioPrcessContex *ctx)
{
    int ret = 0;
    if (ret = read_decode_convert_and_store(ctx->fifo, ctx->decodeContex, ctx->encoderContex, ctx->resample_context, (uint8_t *)in_data, in_len, ctx))
    {
        printf("read_decode_convert_and_store failed\n");
        return ret; // decode or resample failed
    }

    return 0;
}

static int encode_and_read(int8_t *out_data, int *out_len, int *encoded_samples, AudioPrcessContex *ctx)
{
    int ret = 0;
    const int output_frame_size = ctx->encoderContex->frame_size;
    if (av_audio_fifo_size(ctx->fifo) > output_frame_size)
    {
        int len = 0;
        if (ret = load_encode_and_write(ctx->fifo, ctx->output_format_context, ctx->encoderContex, out_data, &len, ctx->enable_dump))
            return ret;

        // for dump
        // size_t size = fwrite(out_data, 1, len, ctx->dump);
        // fflush(ctx->dump);

        *encoded_samples = output_frame_size;
        *out_len = len;
    }
    return ret;
}
