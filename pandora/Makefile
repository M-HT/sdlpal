TARGET = sdlpal

HOST = pandora-

DEPFLAGS = -MT $@ -MMD -MP -MF $*.Td

CFILES = $(wildcard ../adplug/*.c) ../libmad/music_mad.c $(wildcard ../native_midi/*.c) $(wildcard ../*.c) $(wildcard ./*.c)
CPPFILES = $(wildcard ../adplug/*.cpp) $(wildcard ../*.cpp)
OBJFILES = $(CFILES:.c=.o) $(CPPFILES:.cpp=.o)
DEPFILES = $(OBJFILES:.o=.d)

CCFLAGS = `sdl-config --cflags` -pipe -march=armv7-a -mcpu=cortex-a8 -mtune=cortex-a8 -mfpu=neon -mfloat-abi=softfp -g -Wall -O2 -fno-strict-aliasing -I. -I../ -I$(PNDSDK)/usr/include -I../../wildmidi/include -DPANDORA -DPAL_HAS_PLATFORM_SPECIFIC_UTILS
CXXFLAGS = $(CCFLAGS) -std=c++11
CFLAGS = $(CCFLAGS) -std=gnu99
LDFLAGS = `sdl-config --libs` ../../wildmidi/lib/arm/libWildMidi.a -lasound -lmad -lvorbisidec -logg -lstdc++ -lm -pthread

POSTCOMPILE = @mv -f $*.Td $*.d && touch $@

.PHONY : all clean

all: $(TARGET)

$(TARGET): $(OBJFILES)
	@echo [LD] $@
	@$(HOST)gcc $^ -o $@ $(LDFLAGS)

%.o: %.c %.d
	@echo [CC] $<
	@$(HOST)gcc $(DEPFLAGS) $(CFLAGS) -c $< -o $@
	$(POSTCOMPILE)

%.o: %.cpp %.d
	@echo [CC] $<
	@$(HOST)g++ $(DEPFLAGS) $(CXXFLAGS) -c $< -o $@
	$(POSTCOMPILE)

clean:
	-rm -f $(TARGET) $(OBJFILES) $(DEPFILES)

%.d: ;
.PRECIOUS: %.d

-include $(DEPFILES)
