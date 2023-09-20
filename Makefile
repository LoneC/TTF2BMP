CC = gcc
CFLAGS = -I/usr/include -I/usr/include/freetype2 -fPIC -Wall -Werror -Wpedantic -Wno-unused -std=c11
LFLAGS = -L/usr/lib -lm -lfreetype -lpng
TARGET = ttf2bmp

SOURCES = $(wildcard *.c)
OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c))

all: $(OBJECTS) $(TARGET) clean

$(OBJECTS): $(SOURCES)
	$(CC) -c $(SOURCES) $(CFLAGS)

$(TARGET): $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS) $(LFLAGS)

clean:
	rm *.o
