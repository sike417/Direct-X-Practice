#pragma once

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}

namespace MediaUtils
{
    struct OutputStream;

    class FFMPEGEncoder final
    {
    public:
        FFMPEGEncoder(const std::string & fileName, const int width, const int height, const int frameRate);
        ~FFMPEGEncoder();

        FFMPEGEncoder() = delete;
        FFMPEGEncoder(const FFMPEGEncoder&) = delete;
        FFMPEGEncoder(const FFMPEGEncoder&&) = delete;
        const FFMPEGEncoder& operator=(const FFMPEGEncoder&) = delete;
        const FFMPEGEncoder& operator=(const FFMPEGEncoder&&) = delete;


        bool AddFrame(char* srcCharArray);
        bool FinalizeEncoding();
        bool InitializeEncoder();

    private:
        bool generateInternalFrame(char* srcCharArray, OutputStream* pVideoStream) const;
        static bool encodeFrameToContext(AVCodecContext* codecContext, OutputStream* videoStream, AVFormatContext* formatContext);
        static int open_video(AVCodec* codec, OutputStream* videoStream, AVDictionary* opt_arg);
        static AVFrame* alloc_frame(enum AVPixelFormat pix_fmt, int width, int height);
        static int write_frame(AVFormatContext* formatContext, const AVRational* time_base, AVStream* avStream, AVPacket* pkt);

        bool add_stream(OutputStream** videoStream, AVFormatContext* formatContext, enum AVCodecID codec_id, bool isThumbnailStream = false) const;
        static void close_stream(OutputStream* videoStream);

        void setupVideoStream(AVCodecContext* pCodecContext, AVStream* pAVStream, AVCodecID id) const;
        void setupStream(AVCodecContext* pCodecContext, AVStream* pAVStream, AVPixelFormat desiredPixelFormat) const;

    private:
        bool m_bInitializationComplete = false;
        static const int EXPECTED_BPP = 32;
        static const int EXPECTED_BYTEPP = 4;
        static const size_t DEFAULT_BITRATE = 4000000; //PHX_TODO: Update the bitrate.

        const std::string m_fileName;
        const int m_iDesiredWidth;
        int m_iDesiredHeight;
        const int m_iFrameRate;

        bool m_bIsFirstFrame;

        OutputStream* m_pVideoStream;
        OutputStream* m_pThumbnailStream;
        AVFormatContext* m_pFormatContext;
        AVOutputFormat* m_pOutputFormat;
    };
}
