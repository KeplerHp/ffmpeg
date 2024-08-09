# 定义程序名称
PROGRAM = ffmpeg

# 定义源文件
SOURCES = bgra_encode_and_decode.cpp

# 使用g++编译器
CC = g++

# 定义编译选项
CFLAGS = -Wall -Wextra -O2 -std=c++11 -I/usr/local/include -D__STDC_CONSTANT_MACROS

# 定义链接选项
LDFLAGS = -lavcodec -lavformat -lavutil -lswscale -lswresample 

# 默认目标
all: $(PROGRAM)

# 直接编译源文件到可执行文件
$(PROGRAM): $(SOURCES)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# 清理编译生成的文件
clean:
	rm -f $(PROGRAM)
	rm -f ../output/*.jpg ../output/*.bgra ../output/*.h264

# 声明伪目标
.PHONY: all clean