CFLAGS = -g3 -O3
DEFINES =
INCLUDES = $(shell sdl-config --cflags)
LIBS = $(shell sdl-config --libs) -lGL

SRC = $(shell egrep 'ClCompile.*cpp"' build/SurfSplit.vcxproj | sed -e 's/.*\"\(.*\)\".*/\1/' | sed -e 's@\\@/@g' | sed -e 's@../@@')
CSRC = $(shell egrep 'ClCompile.*c"' build/SurfSplit.vcxproj | sed -e 's/.*\"\(.*\)\".*/\1/' | sed -e 's@\\@/@g' | sed -e 's@../@@')
OBJ = $(SRC:%.cpp=%.o) $(CSRC:%.c=%.o)

all: surfsplit

%.o: %.cpp
	$(CXX) -c $(INCLUDES) $(CFLAGS) $(DEFINES) $< -o $@

%.o: %.c
	$(CC) -c $(INCLUDES) $(CFLAGS) $(DEFINES) $< -o $@

%.d : %.cpp
	@echo Resolving dependencies of $<
	@mkdir -p $(@D)
	@$(CXX) -MM $(INCLUDES) $(CFLAGS) $(DEFINES) $< > $@.$$$$; \
	sed 's,.*\.o[ :]*,$(<:.cpp=.o) $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

%.d : %.c
	@echo Resolving dependencies of $<
	@mkdir -p $(@D)
	@$(CC) -MM $(INCLUDES) $(CFLAGS) $(DEFINES) $< > $@.$$$$; \
	sed 's,.*\.o[ :]*,$(<:.c=.o) $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

surfsplit: $(OBJ)
	$(CXX) $(CFLAGS) $(DEFINES) $(OBJ) $(LIBS) -o $@

-include $(SRC:.cpp=.d) $(CSRC:.c=.d)

clean:
	rm -f $(OBJ) $(SRC:.cpp=.d) $(CSRC:.c=.d) DarkRL

.PHONY: clean all
