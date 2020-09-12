TARGET = sdlpal

INTER=.gp2x

HOST = arm-open2x-linux-

DEPFLAGS = -MT $@ -MMD -MP -MF $*$(INTER).Td

CFILES = $(wildcard ../adplug/*.c) ../libmad/music_mad.c $(wildcard ../*.c) $(wildcard ./*.c)
CPPFILES = $(wildcard ../adplug/*.cpp) $(wildcard ../*.cpp)
OBJFILES = $(CFILES:.c=$(INTER).o) $(CPPFILES:.cpp=$(INTER).o)
DEPFILES = $(OBJFILES:.o=.d)
SDL_CONFIG = sdl-config

CC = $(HOST)gcc
CXX = $(HOST)g++
CCFLAGS = `$(SDL_CONFIG) --cflags` -D_GNU_SOURCE -pipe -march=armv4t -mtune=arm920t -g -Wall -O2 -fno-strict-aliasing -I. -I../ -I/opt/open2x/gcc-4.1.1-glibc-2.3.6/include -DGP2X -DPAL_HAS_PLATFORM_SPECIFIC_UTILS
CXXFLAGS = $(CCFLAGS) -std=gnu++98 -DBINIO_WITH_MATH=1
CFLAGS = $(CCFLAGS) -std=gnu99
LDFLAGS = -s `$(SDL_CONFIG) --libs` -lmad -logg -lvorbisidec -lstdc++ -lm -pthread -static

POSTCOMPILE = @mv -f $*$(INTER).Td $*$(INTER).d && touch $@

.PHONY : all clean

all: $(TARGET)

$(TARGET): $(OBJFILES)
	@echo [LD] $@
	@$(CXX) $^ -o $@ $(LDFLAGS)

%$(INTER).o: %.c %$(INTER).d
	@echo [CC] $<
	@$(CC) $(DEPFLAGS) $(CFLAGS) -c $< -o $@
	$(POSTCOMPILE)

%$(INTER).o: %.cpp %$(INTER).d
	@echo [CC] $<
	@$(CXX) $(DEPFLAGS) $(CXXFLAGS) -c $< -o $@
	$(POSTCOMPILE)

clean:
	-rm -f $(TARGET) $(OBJFILES) $(DEPFILES)

%.d: ;
.PRECIOUS: %.d

-include $(DEPFILES)
