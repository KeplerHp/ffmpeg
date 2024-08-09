#include <iostream>
#include <fstream>
#include <vector>
#include <png.h>

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
    #include <libavutil/opt.h>
}

using namespace std;

bool read_png(const string &file_name, vector<uint8_t> &rgba, int &width, int &height) {
    FILE *fp = fopen(file_name.c_str(), "rb");
    if (!fp) {
        cerr << "Failed to open PNG file: " << file_name << endl;
        return false;
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png) {
        cerr << "Failed to create PNG read struct" << endl;
        fclose(fp);
        return false;
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        cerr << "Failed to create PNG info struct" << endl;
        png_destroy_read_struct(&png, nullptr, nullptr);
        fclose(fp);
        return false;
    }

    if (setjmp(png_jmpbuf(png))) {
        cerr << "Failed to set PNG jump buffer" << endl;
        png_destroy_read_struct(&png, &info, nullptr);
        fclose(fp);
        return false;
    }

    png_init_io(png, fp);
    png_read_info(png, info);

    width = png_get_image_width(png, info);
    height = png_get_image_height(png, info);
    png_byte color_type = png_get_color_type(png, info);
    png_byte bit_depth = png_get_bit_depth(png, info);

    if (bit_depth == 16)
        png_set_strip_16(png);

    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png);

    if (png_get_valid(png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png);

    if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png);

    png_read_update_info(png, info);

    int rowbytes = png_get_rowbytes(png, info);
    rgba.resize(rowbytes * height);
    vector<png_bytep> row_pointers(height);
    for (int i = 0; i < height; ++i)
        row_pointers[i] = &rgba[i * rowbytes];

    png_read_image(png, row_pointers.data());
    png_destroy_read_struct(&png, &info, nullptr);
    fclose(fp);

    return true;
}

int main() {
    string input_file = "input.png";
    vector<uint8_t> rgba;
    int width, height;

    if (!read_png(input_file, rgba, width, height)) {
        cerr << "Failed to read input PNG file" << endl;
        return -1;
    }

    AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec) {
        cerr << "Failed to find H.264 encoder" << endl;
        return -1;
    }

    AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        cerr << "Failed to allocate AVCodecContext" << endl;
        return -1;
    }

    codec_ctx->bit_rate = 400000;
    codec_ctx->width = width;
    codec_ctx->height = height;
    codec_ctx->time_base = {1, 30};
    codec_ctx->gop_size = 10;
    codec_ctx->max_b_frames = 1;
    codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;

    if (codec->id == AV_CODEC_ID_H264) {
        av_opt_set(codec_ctx->priv_data, "preset", "slow", 0);
        av_opt_set(codec_ctx->priv_data, "crf", "23", 0); // 设置CRF值为23
    }

    if (avcodec_open2(codec_ctx, codec, nullptr) < 0) {
        cerr << "Failed to open codec" << endl;
        avcodec_free_context(&codec_ctx);
        return -1;
    }

    AVFrame *frame = av_frame_alloc();
    if (!frame) {
        cerr << "Failed to allocate frame" << endl;
        avcodec_free_context(&codec_ctx);
        return -1;
    }

    frame->format = codec_ctx->pix_fmt;
    frame->width = codec_ctx->width;
    frame->height = codec_ctx->height;

    int ret = av_image_alloc(frame->data, frame->linesize, codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt, 32);
    if (ret < 0) {
        cerr << "Failed to allocate image" << endl;
        av_frame_free(&frame);
        avcodec_free_context(&codec_ctx);
        return -1;
    }

    SwsContext *sws_ctx = sws_getContext(width, height, AV_PIX_FMT_RGBA, codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt, SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!sws_ctx) {
        cerr << "Failed to create SwsContext" << endl;
        av_freep(&frame->data[0]);
        av_frame_free(&frame);
        avcodec_free_context(&codec_ctx);
        return -1;
    }

    uint8_t *src_slices[1] = { rgba.data() };
    int src_stride[1] = { 4 * width };
    sws_scale(sws_ctx, src_slices, src_stride, 0, height, frame->data, frame->linesize);
    sws_freeContext(sws_ctx);

    AVPacket *pkt = av_packet_alloc();
    if (!pkt) {
        cerr << "Failed to allocate packet" << endl;
        av_freep(&frame->data[0]);
        av_frame_free(&frame);
        avcodec_free_context(&codec_ctx);
        return -1;
    }

    FILE *outfile = fopen("output.h264", "wb");
    if (!outfile) {
        cerr << "Failed to open output file" << endl;
        av_packet_free(&pkt);
        av_freep(&frame->data[0]);
        av_frame_free(&frame);
        avcodec_free_context(&codec_ctx);
        return -1;
    }

    ret = avcodec_send_frame(codec_ctx, frame);
    if (ret < 0) {
        cerr << "Error sending a frame for encoding" << endl;
        av_packet_free(&pkt);
        av_freep(&frame->data[0]);
        av_frame_free(&frame);
        avcodec_free_context(&codec_ctx);
        fclose(outfile);
        return -1;
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(codec_ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        else if (ret < 0) {
            cerr << "Error during encoding" << endl;
            break;
        }

        fwrite(pkt->data, 1, pkt->size, outfile);
        av_packet_unref(pkt);
    }

    ret = avcodec_send_frame(codec_ctx, nullptr);
    while (ret >= 0) {
        ret = avcodec_receive_packet(codec_ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        else if (ret < 0) {
            cerr << "Error during encoding flush" << endl;
            break;
        }

        fwrite(pkt->data, 1, pkt->size, outfile);
        av_packet_unref(pkt);
    }

    fclose(outfile);
    av_packet_free(&pkt);
    av_freep(&frame->data[0]);
    av_frame_free(&frame);
    avcodec_free_context(&codec_ctx);

    return 0;
}
