PROGRAM = ffmpeg

SOURCES = bgra_encode_and_decode.cpp

CC = g++

CFLAGS = -Wall -Wextra -O2 -std=c++11 -I/usr/local/include -D__STDC_CONSTANT_MACROS

LDFLAGS = -lavcodec -lavformat -lavutil -lswscale -lswresample 

all: $(PROGRAM)

$(PROGRAM): $(SOURCES)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(PROGRAM)
	rm -f ../output/*.jpg ../output/*.bgra ../output/*.h264

.PHONY: all clean