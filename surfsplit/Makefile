UNAME := $(shell uname)
CFLAGS = -O3 -s -DPNG_INTEL_SSE

# XCode cannot into std=c++17 yet (Aug 2018)
ifeq ($(UNAME), Darwin)
CPPSTD = --std=c++1z
else
CPPSTD = --std=c++17
endif

CXXFLAGS = $(CFLAGS) $(CPPSTD)
INCLUDES = -I../common
LIBS = -lpthread
ifeq ($(UNAME), Darwin)
INCLUDES += -I/Library/Frameworks/SDL2.Framework/Headers
LIBS += -F/Library/Frameworks -framework OpenGL -framework SDL2
else
INCLUDES += $(shell pkg-config --cflags sdl2)
LIBS += $(shell pkg-config --libs sdl2) -lGL
endif

IMAGE = SurfSplit

SRC = $(shell egrep 'ClCompile.*cpp"' build/$(IMAGE).vcxproj | sed -e 's/.*\"\(.*\)\".*/\1/' | sed -e 's@\\@/@g' | sed -e 's@../@@')
CSRC = $(shell egrep 'ClCompile.*c"' build/$(IMAGE).vcxproj | sed -e 's/.*\"\(.*\)\".*/\1/' | sed -e 's@\\@/@g' | sed -e 's@../@@')
OBJ = $(SRC:%.cpp=%.o) $(CSRC:%.c=%.o)


all: $(IMAGE)

%.o: %.cpp
	$(CXX) -c $(INCLUDES) $(CXXFLAGS) $< -o $@

%.o: %.c
	$(CC) -c $(INCLUDES) $(CFLAGS) $< -o $@

%.d : %.cpp
	@echo Resolving dependencies of $<
	@mkdir -p $(@D)
	@$(CXX) -MM $(INCLUDES) $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,.*\.o[ :]*,$(<:.cpp=.o) $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

%.d : %.c
	@echo Resolving dependencies of $<
	@mkdir -p $(@D)
	@$(CC) -MM $(INCLUDES) $(CFLAGS) $< > $@.$$$$; \
	sed 's,.*\.o[ :]*,$(<:.c=.o) $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(IMAGE): $(OBJ)
	$(CXX) $(CFLAGS) $(OBJ) $(LIBS) -o $@
	cp $(IMAGE) ../../bin

-include $(SRC:.cpp=.d) $(CSRC:.c=.d)

clean:
	rm -f $(OBJ) $(SRC:.cpp=.d) $(CSRC:.c=.d) $(IMAGE) ../../bin/$(IMAGE)

.PHONY: clean all
