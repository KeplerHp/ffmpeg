#include <iostream>
#include <fstream>
#include <vector>

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
}

using namespace std;

int main() {
    const string input_file = "../output/output_1080.h264";
    const string output_file = "../output/output_1080.bgra";
    const int width = 1920;
    const int height = 1080;

    // 打开输入文件
    AVFormatContext *format_ctx = nullptr;
    if (avformat_open_input(&format_ctx, input_file.c_str(), nullptr, nullptr) < 0) {
        cerr << "Failed to open input file" << endl;
        return -1;
    }

    // 查找流信息
    if (avformat_find_stream_info(format_ctx, nullptr) < 0) {
        cerr << "Failed to retrieve input stream information" << endl;
        avformat_close_input(&format_ctx);
        return -1;
    }

    // 查找视频流
    int video_stream_index = -1;
    for (unsigned int i = 0; i < format_ctx->nb_streams; ++i) {
        if (format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        }
    }
    if (video_stream_index == -1) {
        cerr << "Failed to find a video stream" << endl;
        avformat_close_input(&format_ctx);
        return -1;
    }

    // 获取视频流的解码器
    AVCodecParameters *codec_params = format_ctx->streams[video_stream_index]->codecpar;
    AVCodec *codec = avcodec_find_decoder(codec_params->codec_id);
    if (!codec) {
        cerr << "Failed to find codec" << endl;
        avformat_close_input(&format_ctx);
        return -1;
    }

    AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        cerr << "Failed to allocate AVCodecContext" << endl;
        avformat_close_input(&format_ctx);
        return -1;
    }

    if (avcodec_parameters_to_context(codec_ctx, codec_params) < 0) {
        cerr << "Failed to copy codec parameters to codec context" << endl;
        avcodec_free_context(&codec_ctx);
        avformat_close_input(&format_ctx);
        return -1;
    }

    if (avcodec_open2(codec_ctx, codec, nullptr) < 0) {
        cerr << "Failed to open codec" << endl;
        avcodec_free_context(&codec_ctx);
        avformat_close_input(&format_ctx);
        return -1;
    }

    // 创建SwsContext对象，将解码后的YUV帧转换为BGRA格式
    SwsContext *sws_ctx = sws_getContext(codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
                                         codec_ctx->width, codec_ctx->height, AV_PIX_FMT_BGRA,
                                         SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!sws_ctx) {
        cerr << "Failed to create SwsContext" << endl;
        avcodec_free_context(&codec_ctx);
        avformat_close_input(&format_ctx);
        return -1;
    }

    // 打开输出文件
    ofstream output(output_file, ios::binary);
    if (!output.is_open()) {
        cerr << "Failed to open output file" << endl;
        sws_freeContext(sws_ctx);
        avcodec_free_context(&codec_ctx);
        avformat_close_input(&format_ctx);
        return -1;
    }

    AVFrame *frame = av_frame_alloc();
    AVFrame *frame_bgra = av_frame_alloc();

    // 分配BGRA图像内存
    int num_bytes = av_image_get_buffer_size(AV_PIX_FMT_BGRA, codec_ctx->width, codec_ctx->height, 32);
    vector<uint8_t> buffer(num_bytes);
    av_image_fill_arrays(frame_bgra->data, frame_bgra->linesize, buffer.data(), AV_PIX_FMT_BGRA, codec_ctx->width, codec_ctx->height, 32);

    AVPacket *packet = av_packet_alloc();

    // 读取帧并解码
    while (av_read_frame(format_ctx, packet) >= 0) {
        if (packet->stream_index == video_stream_index) {
            if (avcodec_send_packet(codec_ctx, packet) == 0) {
                while (avcodec_receive_frame(codec_ctx, frame) == 0) {
                    sws_scale(sws_ctx, frame->data, frame->linesize, 0, codec_ctx->height,
                              frame_bgra->data, frame_bgra->linesize);

                    output.write(reinterpret_cast<char *>(frame_bgra->data[0]), num_bytes);
                }
            }
        }
        av_packet_unref(packet);
    }

    // Flush the decoder to get all remaining frames
    avcodec_send_packet(codec_ctx, nullptr);
    while (avcodec_receive_frame(codec_ctx, frame) == 0) {
        sws_scale(sws_ctx, frame->data, frame->linesize, 0, codec_ctx->height,
                  frame_bgra->data, frame_bgra->linesize);

        output.write(reinterpret_cast<char *>(frame_bgra->data[0]), num_bytes);
    }

    // 清理资源
    output.close();
    av_packet_free(&packet);
    av_frame_free(&frame_bgra);
    av_frame_free(&frame);
    sws_freeContext(sws_ctx);
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&format_ctx);

    return 0;
}
