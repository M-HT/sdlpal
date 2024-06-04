TARGET = sdlpal
TEST_TARGET = ./sdlpal-tests

INTER=.pyra

HOST =
TEST_CCFLAGS =

DEPFLAGS = -MT $@ -MMD -MP -MF $*$(INTER).Td

GTEST_DIR = ../3rd/googletest/googletest

SOURCES = .. ../adplug ../timidity
CFILES = $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.c)) ../libmad/music_mad.c pal_utils.c $(wildcard ../native_midi/*.c) ../unix/native_midi_alsa.c
CPPFILES = $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.cpp))
OBJFILES = $(CFILES:.c=$(INTER).o) $(CPPFILES:.cpp=$(INTER).o)
DEPFILES = $(OBJFILES:.o=.d)
TEST_CPPFILES = $(wildcard ../tests/*.cpp) $(wildcard ../tests/pandora/*.cpp)
TEST_OBJFILES = $(TEST_CPPFILES:.cpp=.o)
SDL_CONFIG = sdl2-config

WILDMIDI_CFLAGS = -I../../wildmidi/include
WILDMIDI_LDFLAGS = -lWildMidi -L../../wildmidi/lib/armhf

OPUS_CFLAGS = -I/usr/include/opus
OPUS_LDFLAGS = -lopusfile

CC = $(HOST)gcc
CXX = $(HOST)g++
CCFLAGS = `$(SDL_CONFIG) --cflags` -D_GNU_SOURCE -pipe -march=armv7ve+simd -mcpu=cortex-a15 -mtune=cortex-a15 -mfpu=neon-vfpv4 -mfloat-abi=hard -mthumb -g -Wall -O2 -fno-strict-aliasing -I. -I../ -I../timidity $(WILDMIDI_CFLAGS) $(OPUS_CFLAGS) -DPYRA -DPAL_HAS_PLATFORM_SPECIFIC_UTILS $(TEST_CCFLAGS)
CXXFLAGS = $(CCFLAGS) -std=c++11
CFLAGS = $(CCFLAGS) -std=gnu99
LDFLAGS = -s `$(SDL_CONFIG) --libs` $(WILDMIDI_LDFLAGS) $(OPUS_LDFLAGS) -lasound -lmad -lvorbis -logg -lstdc++ -lm
TEST_CXXFLAGS += -isystem $(GTEST_DIR)/include -I $(GTEST_DIR) -g -Wall -Wextra -pthread

POSTCOMPILE = @mv -f $*$(INTER).Td $*$(INTER).d && touch $@

.PHONY : all clean check

all: $(TARGET)

$(TARGET): $(OBJFILES)
	@echo [LD] $@
	@$(CXX) $^ -o $@ $(LDFLAGS)

gtest-all.o : $(GTEST_DIR)/src/gtest-all.cc %.d
	@echo [CC] $<
	@$(CXX) $(DEPFLAGS) $(TEST_CXXFLAGS) -c $< -o $@
	$(POSTCOMPILE)

%$(INTER).o: %.c %$(INTER).d
	@echo [CC] $<
	@$(CC) $(DEPFLAGS) $(CFLAGS) -c $< -o $@
	$(POSTCOMPILE)

%$(INTER).o: %.cpp %$(INTER).d
	@echo [CC] $<
	@$(CXX) $(DEPFLAGS) $(CXXFLAGS) -c $< -o $@
	$(POSTCOMPILE)

$(TEST_TARGET): $(OBJFILES) $(TEST_OBJFILES) gtest-all.o
	@echo [LD] $@
	@$(CXX) $^ -o $@ $(LDFLAGS) -lpthread

clean:
	-@rm -f $(TARGET) $(TEST_TARGET) $(OBJFILES) $(TEST_OBJFILES) gtest-all.o $(DEPFILES) gtest-all$(INTER).d

%.d: ;
.PRECIOUS: %.d

-include $(DEPFILES)

check: TEST_CCFLAGS = -I. -I.. -DUNIT_TEST=1 -isystem $(GTEST_DIR)/include
check: $(TEST_TARGET)
	@chmod +x $(TEST_TARGET)
	@echo Run $(TEST_TARGET) on Pyra to execute tests
