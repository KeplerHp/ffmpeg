#### 使用`FFmpeg`对`bgra`格式文件进行压缩解压缩

``image``文件夹内为原始`jpg`图片

在``src``目录下执行``make``即可得到``encode``和``decode``两个可执行文件

可以在``src``目录下执行

```
ffmpeg -f rawvideo -pixel_format bgra -video_size 1920x1080 -i ../output/output_1080.bgra ../output/output_1080.jpg
```

可以将解压缩得到的`bgra`文件还原为`jpg`图片和原始图片对比

`fsnr.py`用于计算`FSNR`，`psnr.py`用于计算`PSNR`，`compare.py`用于比较输入`bgra`和压缩解压缩之后的`bgra`文件



调整``bgra_encode.cpp``中的``设置编码参数``部分代码可以调整压缩比例。


下面是一些可能的调整：

1. 调整编码器的比特率

比特率（bit_rate）是影响压缩质量的一个重要参数。增加比特率可以减少压缩损失，但也会增加文件大小。可以尝试调高比特率，如将codec_ctx->bit_rate设置为更高的值：

``` cpp
codec_ctx->bit_rate = 2000000; // 增加比特率，例如设为 2 Mbps
```

2. 使用无损压缩

如果你希望保留所有的图像信息，可以考虑使用H.264编码的无损模式。可以通过设置crf为0，并将preset设置为veryslow（更高质量但速度较慢）：

``` cpp
av_opt_set(codec_ctx->priv_data, "crf", "0", 0);
av_opt_set(codec_ctx->priv_data, "preset", "veryslow", 0);
```

3. 调整帧间预测和压缩参数

可以调整编码器的其他参数，如gop_size、max_b_frames等，以找到质量与文件大小的最佳平衡。例如：

``` cpp
codec_ctx->gop_size = 50;  // GOP大小，可以根据需要调整
codec_ctx->max_b_frames = 2; // 使用更多的B帧可以提高压缩效率
```

4. 使用高质量像素格式

如果H.264不满足你的质量要求，可以考虑使用更高质量的像素格式或其他编码格式。尽管H.264是一个非常流行的压缩格式，但在某些情况下，可能需要考虑无损的编码格式（如FFV1）或其他高质量的压缩方法。

5. 调整编码模式和压缩配置

可以尝试使用不同的编码模式（如av_opt_set(codec_ctx->priv_data, "tune", "film", 0);）来优化编码质量。这些调整根据具体情况可能对图像质量产生明显影响。

```cpp
av_opt_set(codec_ctx->priv_data, "profile", "high", 0);
av_opt_set(codec_ctx->priv_data, "tune", "film", 0); // 针对高动态范围场景优化
```

6. 使用更高效的色彩空间转换

在压缩过程中，颜色空间的转换可能也会影响质量。确保SwsContext使用最佳的转换参数，如更高精度的滤波器：

```cpp
sws_ctx = sws_getContext(width, height, AV_PIX_FMT_BGRA,
                         codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
                         SWS_LANCZOS, nullptr, nullptr, nullptr);
```
