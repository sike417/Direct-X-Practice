#include "pch.h"
#include "FFMPEGEncoder.h"

extern "C"
{
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

namespace MediaUtils
{
    // a wrapper around a single output AVStream
    struct OutputStream
    {
        AVStream* pStream;
        AVCodec* pCodec;
        AVCodecContext* pEncoder;

        /* pts of the next frame that will be generated */
        int64_t next_pts;

        AVFrame* frame;
        AVFrame* tmp_frame;

        struct SwsContext* sws_ctx;
    };

    static void avlog_cb(void* test, int level, const char* szFmt, va_list varg) 
    {
        std::string formattedString;

        const int outputSize = _vscprintf(szFmt, varg);
        formattedString.resize(outputSize + 1);

        vsprintf_s(&formattedString[0], formattedString.size(), szFmt, varg);
        OutputDebugStringA(formattedString.c_str());
    }
}

using namespace MediaUtils;

static constexpr AVPixelFormat INPUT_PIXELFORMAT = AV_PIX_FMT_RGB32;
static constexpr AVPixelFormat EXPECTED_PIXELFORMAT = AV_PIX_FMT_YUV420P;
static constexpr AVPixelFormat THUMBNAIL_PIXELFORMAT = AV_PIX_FMT_RGB24;

FFMPEGEncoder::FFMPEGEncoder(const std::string& fileName,
    const int width,
    const int height,
    const int frameRate)
    : m_fileName(fileName)
    , m_iDesiredWidth(width)
    , m_iDesiredHeight(height)
    , m_iFrameRate(frameRate)
    , m_bIsFirstFrame(true)
    , m_pVideoStream(nullptr)
    , m_pThumbnailStream(nullptr)
    , m_pFormatContext(nullptr)
    , m_pOutputFormat(nullptr)
{
    av_log_set_callback(avlog_cb);
    av_log_set_level(AV_LOG_VERBOSE);
    
    // adjust the height to be divisible by 2.
    m_iDesiredHeight = m_iDesiredHeight % 2 ? m_iDesiredHeight + 1 : m_iDesiredHeight;
}


FFMPEGEncoder::~FFMPEGEncoder()
{
    close_stream(m_pVideoStream);
    close_stream(m_pThumbnailStream);

    if (m_pFormatContext)
    {
        avio_closep(&m_pFormatContext->pb);
        avformat_free_context(m_pFormatContext);
    }
}

bool FFMPEGEncoder::AddFrame(char* srcCharArray)
{
    if (!m_bInitializationComplete)
    {
        return false;
    }

    if (!generateInternalFrame(srcCharArray, m_pVideoStream))
    {
        return false;
    }

    if (m_bIsFirstFrame)
    {
        m_bIsFirstFrame = false;
        generateInternalFrame(srcCharArray, m_pThumbnailStream);
        encodeFrameToContext(m_pThumbnailStream->pEncoder, m_pThumbnailStream, m_pFormatContext);
    }

    return encodeFrameToContext(m_pVideoStream->pEncoder, m_pVideoStream, m_pFormatContext);
}

bool FFMPEGEncoder::FinalizeEncoding()
{
    if (!m_bInitializationComplete)
    {
        return false;
    }

    AVPacket* pkt = av_packet_alloc();

    int ret = avcodec_send_frame(m_pVideoStream->pEncoder, nullptr);

    while (ret >= 0)
    {
        ret = avcodec_receive_packet(m_pVideoStream->pEncoder, pkt);
        if (ret >= 0)
        {
            ret = write_frame(m_pFormatContext, &m_pVideoStream->pEncoder->time_base, m_pVideoStream->pStream, pkt);
        }
    }

    av_packet_unref(pkt);

    if (m_pFormatContext)
    {
        /* Write the trailer, if any. The trailer must be written before you
         * close the CodecContexts open when you wrote the header; otherwise
         * av_write_trailer() may try to use memory that was freed on
         * av_codec_close(). */

        av_write_trailer(m_pFormatContext);
    }

    return true;
}

bool FFMPEGEncoder::InitializeEncoder()
{
    int ret = 0;

    avformat_alloc_output_context2(&m_pFormatContext, nullptr, nullptr, m_fileName.c_str());
    if (!m_pFormatContext)
    {
        OutputDebugStringA("Could not deduce output format from file extension: using MPEG.");
        avformat_alloc_output_context2(&m_pFormatContext, nullptr, "mpeg", m_fileName.c_str());
    }

    if (!m_pFormatContext)
    {
        OutputDebugStringA("Could not create format context");
        return m_bInitializationComplete;
    }

    m_pOutputFormat = m_pFormatContext->oformat;

    if (m_pOutputFormat->video_codec == AV_CODEC_ID_NONE)
    {
        return m_bInitializationComplete;
    }

    if (!add_stream(&m_pVideoStream, m_pFormatContext, m_pOutputFormat->video_codec))
    {
        return m_bInitializationComplete;
    }

    if (!add_stream(&m_pThumbnailStream, m_pFormatContext, AV_CODEC_ID_PNG, true))
    {
        return m_bInitializationComplete;
    }

    ret = open_video(m_pVideoStream->pCodec, m_pVideoStream, nullptr);

    if (ret < 0)
    {
        return m_bInitializationComplete;
    }

    ret = open_video(m_pThumbnailStream->pCodec, m_pThumbnailStream, nullptr);

    if (ret < 0)
    {
        return m_bInitializationComplete;
    }

    ret = avio_open(&m_pFormatContext->pb, m_fileName.c_str(), AVIO_FLAG_WRITE);
    if (ret < 0)
    {
        int outputsize = snprintf(nullptr, 0, "Could not open '%s'", m_fileName.c_str());

        std::string outputBuffer;
        outputBuffer.resize(outputsize);
        sprintf(&outputBuffer[0], "Could not open '%s'\n", m_fileName.c_str());

        OutputDebugStringA(outputBuffer.c_str());

        return m_bInitializationComplete;
    }

    ret = avformat_write_header(m_pFormatContext, nullptr);
    if (ret >= 0)
    {
        m_bInitializationComplete = true;
    }

    return m_bInitializationComplete;
}

#pragma region private methods

bool FFMPEGEncoder::generateInternalFrame(char* srcCharArray, OutputStream* pVideoStream) const
{
    AVCodecContext* codecContext = pVideoStream->pEncoder;

    if (av_frame_make_writable(pVideoStream->frame) < 0)
    {
        return false;
    }

    // transferring ownership of the memory allocated to pConvertedFrameBuffer, do not free the memory yet.
    av_image_fill_arrays(pVideoStream->tmp_frame->data,
        pVideoStream->tmp_frame->linesize,
        reinterpret_cast<uint8_t*>(srcCharArray),
        INPUT_PIXELFORMAT,
        m_iDesiredWidth,
        m_iDesiredHeight,
        1);

    if (codecContext->pix_fmt != INPUT_PIXELFORMAT)
    {

        /* Since our encoder only accepts a different pixel format than our input, we need to convert from one space to the next*/
        if (!pVideoStream->sws_ctx)
        {
            pVideoStream->sws_ctx = sws_getContext(codecContext->width, codecContext->height,
                INPUT_PIXELFORMAT,
                codecContext->width, codecContext->height,
                codecContext->pix_fmt,
                SWS_BICUBIC, nullptr, nullptr, nullptr);

            if (!pVideoStream->sws_ctx)
            {
                OutputDebugStringA("Could not initialize the conversion context");
                return false;
            }
        }

        sws_scale(pVideoStream->sws_ctx, pVideoStream->tmp_frame->data,
            pVideoStream->tmp_frame->linesize, 0, codecContext->height, pVideoStream->frame->data,
            pVideoStream->frame->linesize);
    }
    else
    {
        //TODO: If this else statement is ever expected to be true, verify that this works.
        av_frame_copy(pVideoStream->frame, pVideoStream->tmp_frame);
    }

    pVideoStream->frame->pts = pVideoStream->next_pts++;

    return true;
}

bool FFMPEGEncoder::encodeFrameToContext(AVCodecContext* codecContext, OutputStream* videoStream, AVFormatContext* formatContext)
{
    int ret = 0;
    AVPacket* pkt = nullptr;
    ret = avcodec_send_frame(codecContext, videoStream->frame);

    auto _ = gsl::finally([&]
        {
            if (pkt != nullptr)
            {
                av_packet_free(&pkt);
            }
        });

    while (ret >= 0)
    {
        pkt = av_packet_alloc();
        ret = avcodec_receive_packet(codecContext, pkt);

        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            //Expected condition. No more encoded frames to write.
            return true;
        }

        if (ret >= 0)
        {
            ret = write_frame(formatContext, &codecContext->time_base, videoStream->pStream, pkt);
            av_packet_free(&pkt);
        }
    }

    return false;
}

int FFMPEGEncoder::open_video(AVCodec* codec, OutputStream* videoStream, AVDictionary* opt_arg)
{
    AVCodecContext* c = videoStream->pEncoder;
    AVDictionary* opt = nullptr;

    av_dict_copy(&opt, opt_arg, 0);

    /* open the codec */
    int ret = avcodec_open2(c, codec, &opt);
    av_dict_free(&opt);
    if (ret < 0)
    {
        char buf[256];
        av_strerror(ret, buf, sizeof(buf));

        int outputsize = snprintf(nullptr, 0, "Could not open video codec: %s", buf);

        std::string outputBuffer;
        outputBuffer.resize(outputsize);
        sprintf(&outputBuffer[0], "Could not open video codec: %s", buf);

        OutputDebugStringA(outputBuffer.c_str());

        return ret;
    }

    /* allocate and init a re-usable frame */
    videoStream->frame = alloc_frame(c->pix_fmt, c->width, c->height);
    if (!videoStream->frame)
    {
        OutputDebugStringA("Could not allocate video frame");
        return -1;
    }


    // Note: The tmp_frame variable used to only get created when the INPUT_PIXELFORMAT was different
    // than the encoder pixel format, but now that we are working with the srcCharArray directly, we will
    // always write to the tmp_frame and then copy to the permanent frame, in order to ensure
    // that the SrcCharArray doesn't get deleted while it is still being used.

    videoStream->tmp_frame = nullptr;
    videoStream->tmp_frame = alloc_frame(INPUT_PIXELFORMAT, c->width, c->height);
    if (!videoStream->tmp_frame)
    {
        OutputDebugStringA("Could not allocate temporary picture");
        return -1;
    }

    /* copy the stream parameters to the muxer */
    ret = avcodec_parameters_from_context(videoStream->pStream->codecpar, c);
    if (ret < 0)
    {
        OutputDebugStringA("Could not copy the stream parameters");
    }
    return ret;
}

int FFMPEGEncoder::write_frame(AVFormatContext* formatContext, const AVRational* time_base, AVStream* avStream, AVPacket* pkt)
{
    /* rescale output packet timestamp values from codec to stream timebase */
    av_packet_rescale_ts(pkt, *time_base, avStream->time_base);
    pkt->stream_index = avStream->index;

    /* Write the compressed frame to the media file. */
    return av_interleaved_write_frame(formatContext, pkt);
}

AVFrame* FFMPEGEncoder::alloc_frame(AVPixelFormat pix_fmt, int width, int height)
{
    AVFrame* frame = av_frame_alloc();
    if (!frame)
    {
        return nullptr;
    }

    frame->format = pix_fmt;
    frame->width = width;
    frame->height = height;

    /* allocate the buffers for the frame data */
    const int ret = av_frame_get_buffer(frame, EXPECTED_BPP);
    if (ret < 0)
    {
        av_frame_unref(frame);

        OutputDebugStringA("Could not allocate frame data.");

        return nullptr;
    }

    return frame;
}

bool FFMPEGEncoder::add_stream(OutputStream** dpVideoStream, AVFormatContext* formatContext, AVCodecID codecId, bool isThumbnailStream) const
{
    if (*dpVideoStream == nullptr)
    {
        *dpVideoStream = new OutputStream();
    }


    OutputStream* pVideoStream = *dpVideoStream;

    // Find the requested codec
    pVideoStream->pCodec = avcodec_find_encoder(codecId);
    //*codec = avcodec_find_encoder_by_name("libx264rgb");
    if (!pVideoStream->pCodec)
    {
        int outputsize = snprintf(nullptr, 0, "Could not find pEncoder for '%s'\n", avcodec_get_name(codecId));

        std::string outputBuffer;
        outputBuffer.resize(outputsize);
        sprintf(&outputBuffer[0], "Could not find pEncoder for '%s'\n", avcodec_get_name(codecId));

        OutputDebugStringA(outputBuffer.c_str());

        return false;
    }

    if (pVideoStream->pCodec->type != AVMEDIA_TYPE_VIDEO)
    {
        OutputDebugStringA("Only video codec types are currently supported.");
        return false;
    }

    // Create the video stream
    pVideoStream->pStream = avformat_new_stream(formatContext, pVideoStream->pCodec);
    if (!pVideoStream->pStream)
    {
        OutputDebugStringA("Could not allocate stream.");

        return false;
    }
    pVideoStream->pStream->id = formatContext->nb_streams - 1;
    pVideoStream->pStream->disposition = isThumbnailStream ? AV_DISPOSITION_ATTACHED_PIC : 0;

    // create the context associated with the codec.
    AVCodecContext* pCodecContext = avcodec_alloc_context3(pVideoStream->pCodec);
    if (!pCodecContext)
    {
        OutputDebugStringA("Could not alloc an encoding context.");

        return false;
    }
    pVideoStream->pEncoder = pCodecContext;

    // Set up the codec context and the stream.
    if (isThumbnailStream)
    {
        setupStream(pCodecContext, pVideoStream->pStream, THUMBNAIL_PIXELFORMAT);
    }
    else
    {
        setupVideoStream(pCodecContext, pVideoStream->pStream, codecId);
    }

    /* Some formats want stream headers to be separate. */
    if (formatContext->oformat->flags & AVFMT_GLOBALHEADER)
    {
        pCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    return true;
}

void FFMPEGEncoder::close_stream(OutputStream* videoStream)
{
    if (videoStream != nullptr)
    {
        avcodec_free_context(&videoStream->pEncoder);
        av_frame_free(&videoStream->frame);
        av_frame_free(&videoStream->tmp_frame);
        sws_freeContext(videoStream->sws_ctx);
    }
}

void FFMPEGEncoder::setupVideoStream(AVCodecContext* pCodecContext, AVStream* pAVStream, AVCodecID codecId) const
{
    setupStream(pCodecContext, pAVStream, EXPECTED_PIXELFORMAT);

    pCodecContext->codec_id = codecId;

    //TODO: Update Gob size
    pCodecContext->gop_size = 12; /* emit one intra frame every twelve frames at most */
}

void FFMPEGEncoder::setupStream(AVCodecContext* pCodecContext, AVStream* pAVStream, AVPixelFormat desiredPixelFormat) const
{
    /* Resolution must be a multiple of two. */
    pCodecContext->width = m_iDesiredWidth;
    pCodecContext->height = m_iDesiredHeight;

    /* timebase: This is the fundamental unit of time (in seconds) in terms
     * of which frame timestamps are represented. For fixed-fps content,
     * timebase should be 1/framerate and timestamp increments should be
     * identical to 1. */
    pAVStream->time_base = { 1, m_iFrameRate };
    pCodecContext->time_base = pAVStream->time_base;

    pCodecContext->pix_fmt = desiredPixelFormat;
}

#pragma endregion

