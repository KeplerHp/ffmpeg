#include <iostream>
#include <fstream>
#include <vector>
#include "helper.h"

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
    // const int width = 1920;
    // const int height = 1080;

    // 开始日志时间
    startTime = get_cpu_cycle();

    // 打开输入文件
    NOW(CYCLE_START, 1);
    AVFormatContext *format_ctx = nullptr;
    if (avformat_open_input(&format_ctx, input_file.c_str(), nullptr, nullptr) < 0) {
        cerr << "Failed to open input file" << endl;
        return -1;
    }
    NOW(CYCLE_END, 1);

    // 查找流信息
    NOW(CYCLE_START, 2);
    if (avformat_find_stream_info(format_ctx, nullptr) < 0) {
        cerr << "Failed to retrieve input stream information" << endl;
        avformat_close_input(&format_ctx);
        return -1;
    }
    NOW(CYCLE_END, 2);

    // 查找视频流
    NOW(CYCLE_START, 3);
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
    NOW(CYCLE_END, 3);

    // 获取视频流的解码器
    NOW(CYCLE_START, 4);
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
    NOW(CYCLE_END, 4);

    // 创建SwsContext对象，将解码后的YUV帧转换为BGRA格式
    NOW(CYCLE_START, 5);
    SwsContext *sws_ctx = sws_getContext(codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
                                         codec_ctx->width, codec_ctx->height, AV_PIX_FMT_BGRA,
                                         SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!sws_ctx) {
        cerr << "Failed to create SwsContext" << endl;
        avcodec_free_context(&codec_ctx);
        avformat_close_input(&format_ctx);
        return -1;
    }
    NOW(CYCLE_START, 5);

    // 打开输出文件
    NOW(CYCLE_START, 6);
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

    NOW(CYCLE_END, 6);

    // 分配BGRA图像内存
    NOW(CYCLE_START, 7);
    int num_bytes = av_image_get_buffer_size(AV_PIX_FMT_BGRA, codec_ctx->width, codec_ctx->height, 32);
    vector<uint8_t> buffer(num_bytes);
    av_image_fill_arrays(frame_bgra->data, frame_bgra->linesize, buffer.data(), AV_PIX_FMT_BGRA, codec_ctx->width, codec_ctx->height, 32);

    AVPacket *packet = av_packet_alloc();
    NOW(CYCLE_END, 7);

    // 读取帧并解码
    NOW(CYCLE_START, 8);
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
    NOW(CYCLE_END, 8);

    // Flush the decoder to get all remaining frames
    NOW(CYCLE_START, 9);
    avcodec_send_packet(codec_ctx, nullptr);
    while (avcodec_receive_frame(codec_ctx, frame) == 0) {
        sws_scale(sws_ctx, frame->data, frame->linesize, 0, codec_ctx->height,
                  frame_bgra->data, frame_bgra->linesize);

        output.write(reinterpret_cast<char *>(frame_bgra->data[0]), num_bytes);
    }
    NOW(CYCLE_END, 9);

    // 清理资源
    NOW(CYCLE_START, 10);
    output.close();
    av_packet_free(&packet);
    av_frame_free(&frame_bgra);
    av_frame_free(&frame);
    sws_freeContext(sws_ctx);
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&format_ctx);
    NOW(CYCLE_END, 10);

    // 保存日志
    string fileName = "../log/decode_log";
    exportLog(fileName);
    return 0;
}
