#ifndef PAL_CONFIG_H
# define PAL_CONFIG_H

# define PAL_PREFIX            "./"
# define PAL_SAVE_PREFIX       "./"

# define PAL_DEFAULT_WINDOW_WIDTH   320
# define PAL_DEFAULT_WINDOW_HEIGHT  200

# if SDL_VERSION_ATLEAST(2,0,0)
#  define PAL_VIDEO_INIT_FLAGS  (SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP)
# else
#  define PAL_VIDEO_INIT_FLAGS  (SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN | SDL_NOFRAME)
#  define PAL_FATAL_OUTPUT(s)   system(PAL_va(0, "zenity --error --text=\"FATAL ERROR:\\n%s\"", (s)))
# endif

# define PAL_SDL_INIT_FLAGS	(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE)

# define PAL_PLATFORM         "Pandora"
# define PAL_CREDIT           "M-HT"
# define PAL_PORTYEAR         "2017"

# define PAL_HAS_CONFIG_PAGE  1
# define PAL_HAS_NATIVEMIDI 1

# define PAL_HAS_PLATFORM_STARTUP 1

# define PAL_CONFIG_FILENAME gSdlpalCfgFilename

extern const char *gSdlpalCfgFilename;

#include <sys/time.h>

#endif
