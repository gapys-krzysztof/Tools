UNAME := $(shell uname)

ifeq ($(UNAME), Darwin)
ARCH=-arch i386
FLAGS=-stdlib=libc++
endif

SRCS = \
  ../src/libpng/png.c \
  ../src/libpng/pngerror.c \
  ../src/libpng/pngget.c \
  ../src/libpng/pngmem.c \
  ../src/libpng/pngpread.c \
  ../src/libpng/pngread.c \
  ../src/libpng/pngrio.c \
  ../src/libpng/pngrtran.c \
  ../src/libpng/pngrutil.c \
  ../src/libpng/pngset.c \
  ../src/libpng/pngtrans.c \
  ../src/libpng/pngwio.c \
  ../src/libpng/pngwrite.c \
  ../src/libpng/pngwtran.c \
  ../src/libpng/pngwutil.c \
  ../src/zlib/adler32.c \
  ../src/zlib/compress.c \
  ../src/zlib/crc32.c \
  ../src/zlib/deflate.c \
  ../src/zlib/gzlib.c \
  ../src/zlib/infback.c \
  ../src/zlib/inffast.c \
  ../src/zlib/inflate.c \
  ../src/zlib/inftrees.c \
  ../src/zlib/trees.c \
  ../src/zlib/uncompr.c \
  ../src/zlib/zutil.c

SRCSC = \
  ../src/Node.cpp \
  ../src/Application.cpp \
  ../src/Bitmap.cpp \
  ../src/Processing.cpp \
  ../src/String.cpp

OBJS = $(addsuffix .o,$(basename $(notdir $(SRCS))))

all: ../../../bin/TextureAtlas

clean:
	@echo * Clean
	@rm -f TextureAtlas
	@rm -f ../../../bin/TextureAtlas
	@rm -f *.o

TextureAtlas:
	@echo "* Build"
	clang -c $(SRCS)  -I ../../src/zlib  -I ../../src -I ../../src/libpng -O2  $(ARCH)
	clang++ -o $@ -I../../src -I../../contrib -D__UNIX__ $(SRCSC) $(OBJS) -I ../../src/zlib  -I ../../src -I ../../src/libpng -O2 -std=c++14 $(FLAGS) $(ARCH)

../../../bin/TextureAtlas : TextureAtlas
	@echo "* Copy"
	@cp $< $@
