#include <iostream>
#include <fstream>
#include <vector>
#include "helper.h"

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
    #include <libavutil/opt.h>
}

using namespace std;

int main() {
    const string input_file = "../input/input_1080.bgra";
    const string output_file = "../output/output_1080.h264";
    const int width = 1920;
    const int height = 1080;

    // 读取BGRA数据
    ifstream input(input_file, ios::binary);
    if (!input.is_open()) {
        cerr << "Failed to open input file" << endl;
        return -1;
    }
    vector<uint8_t> bgra((istreambuf_iterator<char>(input)), istreambuf_iterator<char>());
    input.close();

    // 初始化日志开始时间
    startTime = get_cpu_cycle();

    // 创建AVCodecContext对象，选择编码器
    NOW(CYCLE_START, 1);
    AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec) {
        cerr << "Failed to find H.264 encoder" << endl;
        return -1;
    }

    AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        cerr << "Failed to allocate AVCodecContext object" << endl;
        return -1;
    }
    NOW(CYCLE_END, 1);

    // 设置编码参数
    NOW(CYCLE_START, 2);
    // 初始为400000，效果不太佳
    codec_ctx->bit_rate = 2000000;
    codec_ctx->width = width;
    codec_ctx->height = height;
    codec_ctx->time_base = {1, 25};
    codec_ctx->gop_size = 10;
    codec_ctx->max_b_frames = 1;
    codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;

    av_opt_set(codec_ctx->priv_data, "preset", "slow", 0);
    // 这个修改延迟增加很大
    // av_opt_set(codec_ctx->priv_data, "crf", "0", 0);
    // av_opt_set(codec_ctx->priv_data, "preset", "veryslow", 0);


    if (avcodec_open2(codec_ctx, codec, nullptr) < 0) {
        cerr << "Failed to open H.264 encoder" << endl;
        avcodec_free_context(&codec_ctx);
        return -1;
    }
    NOW(CYCLE_END, 2);

    // 创建AVFrame对象
    NOW(CYCLE_START, 3);
    AVFrame *frame = av_frame_alloc();
    if (!frame) {
        cerr << "Failed to allocate AVFrame object" << endl;
        avcodec_free_context(&codec_ctx);
        return -1;
    }
    frame->format = codec_ctx->pix_fmt;
    frame->width = codec_ctx->width;
    frame->height = codec_ctx->height;

    int ret = av_image_alloc(frame->data, frame->linesize, codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt, 32);
    if (ret < 0) {
        cerr << "Failed to allocate picture buffer" << endl;
        av_frame_free(&frame);
        avcodec_free_context(&codec_ctx);
        return -1;
    }
    NOW(CYCLE_END, 3);

    // 创建SwsContext对象
    NOW(CYCLE_START, 4);
    SwsContext *sws_ctx = sws_getContext(width, height, AV_PIX_FMT_BGRA, codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt, SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!sws_ctx) {
        cerr << "Failed to create SwsContext object" << endl;
        av_freep(&frame->data[0]);
        av_frame_free(&frame);
        avcodec_free_context(&codec_ctx);
        return -1;
    }
    NOW(CYCLE_END, 4);

    // 打开输出文件
    FILE *outfile = fopen(output_file.c_str(), "wb");
    if (!outfile) {
        cerr << "Failed to open output file" << endl;
        sws_freeContext(sws_ctx);
        av_freep(&frame->data[0]);
        av_frame_free(&frame);
        avcodec_free_context(&codec_ctx);
        return -1;
    }

    // 创建AVPacket对象
    NOW(CYCLE_START, 5);
    AVPacket *pkt = av_packet_alloc();
    if (!pkt) {
        cerr << "Failed to allocate AVPacket object" << endl;
        fclose(outfile);
        sws_freeContext(sws_ctx);
        av_freep(&frame->data[0]);
        av_frame_free(&frame);
        avcodec_free_context(&codec_ctx);
        return -1;
    }
    NOW(CYCLE_END, 5);

    // 将BGRA数据转换为YUV并编码
    NOW(CYCLE_START, 6);
    int frame_count = bgra.size() / (width * height * 4); // 每帧BGRA数据大小为 width * height * 4 字节
    for (int i = 0; i < frame_count; ++i) {
        uint8_t *src_slices[1] = {bgra.data() + i * width * height * 4};
        int src_stride[1] = {4 * width};
        sws_scale(sws_ctx, src_slices, src_stride, 0, height, frame->data, frame->linesize);

        frame->pts = i;

        ret = avcodec_send_frame(codec_ctx, frame);
        if (ret < 0) {
            cerr << "Error sending a frame to the encoder: " << ret << endl;
            break;
        }

        while (ret >= 0) {
            ret = avcodec_receive_packet(codec_ctx, pkt);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                break;
            else if (ret < 0) {
                cerr << "Error encoding a frame: " << ret << endl;
                break;
            }

            fwrite(pkt->data, 1, pkt->size, outfile);
            av_packet_unref(pkt);
        }
    }
    NOW(CYCLE_END, 6);

    // 编码过程
    NOW(CYCLE_START, 7);
    avcodec_send_frame(codec_ctx, nullptr);
    while (avcodec_receive_packet(codec_ctx, pkt) >= 0) {
        fwrite(pkt->data, 1, pkt->size, outfile);
        av_packet_unref(pkt);
    }
    NOW(CYCLE_END, 7);

    // 清理资源
    NOW(CYCLE_START, 8);
    fclose(outfile);
    av_packet_free(&pkt);
    sws_freeContext(sws_ctx);
    av_freep(&frame->data[0]);
    av_frame_free(&frame);
    avcodec_free_context(&codec_ctx);
    NOW(CYCLE_END, 8);

    // 保存日志
    string fileName = "../log/1080/encode_log";
    exportLog(fileName);
    return 0;
}
