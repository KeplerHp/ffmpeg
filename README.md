# ffmpeg
learn to use ffmpeg

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
ffmpeg -f rawvideo -pixel_format bgra -video_size 300x168 -i input.bgra output.jpeg  // 这里可行
```

将bgra转换为h264
```bash
ffmpeg -f rawvideo -pix_fmt bgra -s 300x168 -i input.bgra -c:v libx264 output.h264
```

将h264还原为bgra
```bash
ffmpeg -i output.h264 -pix_fmt bgra -f rawvideo output.bgra
```