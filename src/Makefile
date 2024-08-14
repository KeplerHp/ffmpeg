# 编码部分
ENCODE_PROGRAM = encode
ENCODE_SOURCES = bgra_encode.cpp

# 解码部分
DECODE_PROGRAM = decode
DECODE_SOURCES = bgra_decode.cpp

CC = g++
CFLAGS = -Wall -Wextra -O2 -std=c++11 -I/usr/local/include -I. -D__STDC_CONSTANT_MACROS
LDFLAGS = -lavcodec -lavformat -lavutil -lswscale -lswresample 

all: $(ENCODE_PROGRAM) $(DECODE_PROGRAM)

$(ENCODE_PROGRAM): $(ENCODE_SOURCES) helper.h
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

$(DECODE_PROGRAM): $(DECODE_SOURCES) helper.h
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(ENCODE_PROGRAM) $(DECODE_PROGRAM)
	rm -f ../output/*.jpg ../output/*.bgra ../output/*.h264
	rm -f ../log/log
.PHONY: all clean
