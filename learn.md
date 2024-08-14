# ffmpeg in C++ learning
learn to use ffmpeg

简单学习FFmpeg in C++的使用

要使用 FFmpeg 将 `.png` 图片压缩成 H.264 格式，你可以使用以下命令：
```bash
ffmpeg -i input.png -c:v libx264 -crf 23 output.h264
```
这里 `-i` 表示输入文件，`input.png` 是你要压缩的图片文件。`-c:v libx264` 指定使用 x264 编码器进行视频编码，`-crf 23` 是设置常量速率因子（CRF）为 23，这是一个质量和压缩率的平衡值，值越小质量越高，但文件大小也会越大。`output.h264` 是输出的 H.264 编码视频文件。

如果你想要将 H.264 视频还原成 `.png` 图片，可以使用以下命令：
```bash
ffmpeg -i output.h264 output%03d.png
```
这个命令会从 `input.h264` 视频中提取每一帧，并将其保存为 `output001.png`、`output002.png` 等格式的图片。`%03d` 是一个格式化字符串，表示数字将被格式化为三位数，如果需要更多位数，可以相应增加数字的个数。

从jpeg得到bgra文件
```bash
ffmpeg -i input.jpeg -pix_fmt bgra -f rawvideo input.bgra
```

将bgra还原为jpeg (宽高未知？)
```bash
ffmpeg -f rawvideo -pixel_format bgra -video_size 1920x1080 -i ../output/output_1080.bgra ../output/output_1080.jpg // 这里可行
```

将bgra转换为h264
```bash
ffmpeg -f rawvideo -pix_fmt bgra -s 1920x1080 -i ../input/input_1080.bgra -c:v libx264 ../output/output_1080.h264
```

将h264还原为bgra
```bash
ffmpeg -i ../output/output_1080.h264 -pix_fmt bgra -f rawvideo ../output/output_1080.bgra
```


src文件夹中包含C++代码，调用ffmpeg的库以实现上述命令行相同的功能

版本信息如下：
```bash
$ ffmpeg -version
ffmpeg version 4.4.2-0ubuntu0.22.04.1 Copyright (c) 2000-2021 the FFmpeg developers
built with gcc 11 (Ubuntu 11.2.0-19ubuntu1)
configuration: --prefix=/usr --extra-version=0ubuntu0.22.04.1 --toolchain=hardened --libdir=/usr/lib/x86_64-linux-gnu --incdir=/usr/include/x86_64-linux-gnu --arch=amd64 --enable-gpl --disable-stripping --enable-gnutls --enable-ladspa --enable-libaom --enable-libass --enable-libbluray --enable-libbs2b --enable-libcaca --enable-libcdio --enable-libcodec2 --enable-libdav1d --enable-libflite --enable-libfontconfig --enable-libfreetype --enable-libfribidi --enable-libgme --enable-libgsm --enable-libjack --enable-libmp3lame --enable-libmysofa --enable-libopenjpeg --enable-libopenmpt --enable-libopus --enable-libpulse --enable-librabbitmq --enable-librubberband --enable-libshine --enable-libsnappy --enable-libsoxr --enable-libspeex --enable-libsrt --enable-libssh --enable-libtheora --enable-libtwolame --enable-libvidstab --enable-libvorbis --enable-libvpx --enable-libwebp --enable-libx265 --enable-libxml2 --enable-libxvid --enable-libzimg --enable-libzmq --enable-libzvbi --enable-lv2 --enable-omx --enable-openal --enable-opencl --enable-opengl --enable-sdl2 --enable-pocketsphinx --enable-librsvg --enable-libmfx --enable-libdc1394 --enable-libdrm --enable-libiec61883 --enable-chromaprint --enable-frei0r --enable-libx264 --enable-shared
libavutil      56. 70.100 / 56. 70.100
libavcodec     58.134.100 / 58.134.100
libavformat    58. 76.100 / 58. 76.100
libavdevice    58. 13.100 / 58. 13.100
libavfilter     7.110.100 /  7.110.100
libswscale      5.  9.100 /  5.  9.100
libswresample   3.  9.100 /  3.  9.100
libpostproc    55.  9.100 / 55.  9.100
```

image文件夹中包含使用的图片，分别为1080、720、480、360规格；

input文件夹中为对应的图片转换为的bgra格式文件；

output文件夹中包含对应bgra压缩成的h264文件。
