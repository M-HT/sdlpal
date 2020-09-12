#ifndef PAL_CONFIG_H
# define PAL_CONFIG_H

# define PAL_HAS_JOYSTICKS     0
# define PAL_HAS_MOUSE         0
# define PAL_HAS_TOUCH         0
# define PAL_PREFIX            "./"
# define PAL_SAVE_PREFIX       "./"

# define PAL_DEFAULT_WINDOW_WIDTH   320
# define PAL_DEFAULT_WINDOW_HEIGHT  200

# if SDL_VERSION_ATLEAST(2,0,0)
#  define PAL_VIDEO_INIT_FLAGS  (SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP)
# else
#  define PAL_VIDEO_INIT_FLAGS  (SDL_HWSURFACE | SDL_FULLSCREEN | SDL_NOFRAME)
# endif

# define PAL_SDL_INIT_FLAGS	(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_NOPARACHUTE)

# define PAL_PLATFORM         "GP2X"
# define PAL_CREDIT           "M-HT"
# define PAL_PORTYEAR         "2020"

# define PAL_HAS_SOFTMIDI  0

# define PAL_HAS_PLATFORM_STARTUP 1

# define PAL_CONFIG_FILENAME gSdlpalCfgFilename

extern const char *gSdlpalCfgFilename;

#include <sys/time.h>

#endif
