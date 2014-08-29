CFLAGS = -g3 -O3
CXXFLAGS = $(CFLAGS) --std=c++0x
DEFINES =
INCLUDES = $(shell pkg-config --cflags sdl2 libpng)
LIBS = $(shell pkg-config --libs sdl2 libpng) -lGL

IMAGE = SurfSplit

SRC = $(shell egrep 'ClCompile.*cpp"' build/$(IMAGE).vcxproj | grep -v common | sed -e 's/.*\"\(.*\)\".*/\1/' | sed -e 's@\\@/@g' | sed -e 's@../@@')
CSRC = $(shell egrep 'ClCompile.*c"' build/$(IMAGE).vcxproj | grep -v common | sed -e 's/.*\"\(.*\)\".*/\1/' | sed -e 's@\\@/@g' | sed -e 's@../@@')
OBJ = $(SRC:%.cpp=%.o) $(CSRC:%.c=%.o)

all: $(IMAGE)

%.o: %.cpp
	$(CXX) -c $(INCLUDES) $(CXXFLAGS) $(DEFINES) $< -o $@

%.o: %.c
	$(CC) -c $(INCLUDES) $(CFLAGS) $(DEFINES) $< -o $@

%.d : %.cpp
	@echo Resolving dependencies of $<
	@mkdir -p $(@D)
	@$(CXX) -MM $(INCLUDES) $(CXXFLAGS) $(DEFINES) $< > $@.$$$$; \
	sed 's,.*\.o[ :]*,$(<:.cpp=.o) $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

%.d : %.c
	@echo Resolving dependencies of $<
	@mkdir -p $(@D)
	@$(CC) -MM $(INCLUDES) $(CFLAGS) $(DEFINES) $< > $@.$$$$; \
	sed 's,.*\.o[ :]*,$(<:.c=.o) $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(IMAGE): $(OBJ)
	$(CXX) $(CFLAGS) $(DEFINES) $(OBJ) $(LIBS) -o $@

-include $(SRC:.cpp=.d) $(CSRC:.c=.d)

clean:
	rm -f $(OBJ) $(SRC:.cpp=.d) $(CSRC:.c=.d) $(IMAGE)

.PHONY: clean all
