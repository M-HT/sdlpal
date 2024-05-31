TARGET = sdlpal

INTER=.gp2x

HOST = arm-open2x-linux-

DEPFLAGS = -MT $@ -MMD -MP -MF $*$(INTER).Td

SOURCES = .. ../adplug ../timidity
CFILES = $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.c)) ../libmad/music_mad.c pal_utils.c
CPPFILES = $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.cpp))
OBJFILES = $(CFILES:.c=$(INTER).o) $(CPPFILES:.cpp=$(INTER).o)
DEPFILES = $(OBJFILES:.o=.d)
SDL_CONFIG = sdl-config

OPUS_CFLAGS = -I../../opus/include
OPUS_LDFLAGS = ../../opus/lib/armv4/libopusfile.a ../../opus/lib/armv4/libopus.a

CC = $(HOST)gcc
CXX = $(HOST)g++
CCFLAGS = `$(SDL_CONFIG) --cflags` -D_GNU_SOURCE -pipe -march=armv4t -mtune=arm920t -g -Wall -O2 -fno-strict-aliasing -I. -I../ -I/opt/open2x/gcc-4.1.1-glibc-2.3.6/include -I../timidity $(OPUS_CFLAGS) -DGP2X -DPAL_HAS_PLATFORM_SPECIFIC_UTILS
CXXFLAGS = $(CCFLAGS) -std=gnu++98 -DBINIO_WITH_MATH=1
CFLAGS = $(CCFLAGS) -std=gnu99
LDFLAGS = -s `$(SDL_CONFIG) --libs` $(OPUS_LDFLAGS) -lmad -logg -lvorbisidec -lstdc++ -lm -static

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
	-@rm -f $(TARGET) $(OBJFILES) $(DEPFILES)

%.d: ;
.PRECIOUS: %.d

-include $(DEPFILES)
