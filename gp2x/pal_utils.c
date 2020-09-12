#include "main.h"

#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/soundcard.h>
#include <fcntl.h>
#if SDL_VERSION_ATLEAST(1,2,9)
    #include <SDL_gp2x.h>
#endif

// GP2X buttons
#define GP2X_BUTTON_UP              (0)
#define GP2X_BUTTON_DOWN            (4)
#define GP2X_BUTTON_LEFT            (2)
#define GP2X_BUTTON_RIGHT           (6)
#define GP2X_BUTTON_UPLEFT          (1)
#define GP2X_BUTTON_UPRIGHT         (7)
#define GP2X_BUTTON_DOWNLEFT        (3)
#define GP2X_BUTTON_DOWNRIGHT       (5)
#define GP2X_BUTTON_CLICK           (18)
#define GP2X_BUTTON_A               (12)
#define GP2X_BUTTON_B               (13)
#define GP2X_BUTTON_X               (14)
#define GP2X_BUTTON_Y               (15)
#define GP2X_BUTTON_L               (10)
#define GP2X_BUTTON_R               (11)
#define GP2X_BUTTON_START           (8)
#define GP2X_BUTTON_SELECT          (9)
#define GP2X_BUTTON_VOLUP           (16)
#define GP2X_BUTTON_VOLDOWN         (17)

#define BUTTON_FLAG(x) (1 << (x))
#define QUIT_COMBO (BUTTON_FLAG(GP2X_BUTTON_L) | BUTTON_FLAG(GP2X_BUTTON_R) | BUTTON_FLAG(GP2X_BUTTON_START))


const char *gSdlpalCfgFilename;

static int InitialVolume;
static int ButtonState;
static SDL_Joystick *GP2XJoystick;

// Set new GP2X mixer level, 0-100
static void Set_GP2X_Volume (int newvol)
{
    int soundDev, vol;

    if ((newvol >= 0) && (newvol <= 100))
    {
        soundDev = open("/dev/mixer", O_RDWR);
        if (soundDev != -1)
        {
            vol = ((newvol << 8) | newvol);
            ioctl(soundDev, SOUND_MIXER_WRITE_PCM, &vol);
            close(soundDev);
        }
    }
}

// Returns 0-100, current mixer volume, -1 on error.
static int Get_GP2X_Volume (void)
{
    int soundDev, vol;

    vol = -1;
    soundDev = open("/dev/mixer", O_RDONLY);
    if (soundDev != -1)
    {
        ioctl(soundDev, SOUND_MIXER_READ_PCM, &vol);
        close(soundDev);
        if (vol != -1)
        {
            //just return one channel , not both channels, they're hopefully the same anyways
            return (vol & 0xFF);
        }
    }

    return vol;
}

static void Set_Initial_GP2X_Volume (void)
{
    Set_GP2X_Volume(InitialVolume);
}

static void Change_HW_Audio_Volume (int amount)
{
    int current_volume;

    current_volume = Get_GP2X_Volume();

    if (current_volume == -1) current_volume = 68;

    if ((amount > 1) && current_volume < 12)
    {
        amount = 1;
    }
    else if ((amount < -1) && current_volume <= 12)
    {
        amount = -1;
    }

    current_volume += amount;

    if (current_volume > 100)
    {
        current_volume = 100;
    }
    else if (current_volume < 0)
    {
        current_volume = 0;
    }
    Set_GP2X_Volume(current_volume);
}


static int input_event_filter(const SDL_Event *lpEvent, volatile PALINPUTSTATE *state)
{
    switch (lpEvent->type)
    {
    case SDL_JOYBUTTONDOWN:
        ButtonState |= BUTTON_FLAG(lpEvent->jbutton.button);

        if ((ButtonState & QUIT_COMBO) == QUIT_COMBO)
        {
            PAL_Shutdown(0);
        }

        switch (lpEvent->jbutton.button)
        {
        case GP2X_BUTTON_UP:
        case GP2X_BUTTON_UPRIGHT:
            if (gpGlobals->fInBattle || g_InputState.dir != kDirNorth)
            {
                g_InputState.prevdir = (gpGlobals->fInBattle ? kDirUnknown : g_InputState.dir);
                g_InputState.dir = kDirNorth;
                g_InputState.dwKeyPress |= kKeyUp;
            }
            break;

        case GP2X_BUTTON_DOWN:
        case GP2X_BUTTON_DOWNLEFT:
            if (gpGlobals->fInBattle || g_InputState.dir != kDirSouth)
            {
                g_InputState.prevdir = (gpGlobals->fInBattle ? kDirUnknown : g_InputState.dir);
                g_InputState.dir = kDirSouth;
                g_InputState.dwKeyPress |= kKeyDown;
            }
            break;

        case GP2X_BUTTON_LEFT:
        case GP2X_BUTTON_UPLEFT:
            if (gpGlobals->fInBattle || g_InputState.dir != kDirWest)
            {
                g_InputState.prevdir = (gpGlobals->fInBattle ? kDirUnknown : g_InputState.dir);
                g_InputState.dir = kDirWest;
                g_InputState.dwKeyPress |= kKeyLeft;
            }
            break;

        case GP2X_BUTTON_RIGHT:
        case GP2X_BUTTON_DOWNRIGHT:
            if (gpGlobals->fInBattle || g_InputState.dir != kDirEast)
            {
                g_InputState.prevdir = (gpGlobals->fInBattle ? kDirUnknown : g_InputState.dir);
                g_InputState.dir = kDirEast;
                g_InputState.dwKeyPress |= kKeyRight;
            }
            break;

        case GP2X_BUTTON_A:
        case GP2X_BUTTON_Y:
            g_InputState.dwKeyPress |= kKeyMenu;
            break;

        case GP2X_BUTTON_B:
        case GP2X_BUTTON_X:
            g_InputState.dwKeyPress |= kKeySearch;
            break;

        case GP2X_BUTTON_L:
            g_InputState.dwKeyPress |= kKeyPgUp;
            break;

        case GP2X_BUTTON_R:
            g_InputState.dwKeyPress |= kKeyPgDn;
            break;

        case GP2X_BUTTON_SELECT:
            g_InputState.dwKeyPress |= kKeyRepeat;
            break;

        case GP2X_BUTTON_START:
            g_InputState.dwKeyPress |= kKeyAuto;
            break;

        case GP2X_BUTTON_VOLUP:
            Change_HW_Audio_Volume(4);
            break;

        case GP2X_BUTTON_VOLDOWN:
            Change_HW_Audio_Volume(-4);
            break;

        default:
            break;
        }
        break;

    case SDL_JOYBUTTONUP:
        ButtonState &= ~BUTTON_FLAG(lpEvent->jbutton.button);
        switch (lpEvent->jbutton.button)
        {
        case GP2X_BUTTON_UP:
        case GP2X_BUTTON_UPRIGHT:
            if (g_InputState.dir == kDirNorth)
            {
                g_InputState.dir = g_InputState.prevdir;
            }
            g_InputState.prevdir = kDirUnknown;
            break;

        case GP2X_BUTTON_DOWN:
        case GP2X_BUTTON_DOWNLEFT:
            if (g_InputState.dir == kDirSouth)
            {
                g_InputState.dir = g_InputState.prevdir;
            }
            g_InputState.prevdir = kDirUnknown;
            break;

        case GP2X_BUTTON_LEFT:
        case GP2X_BUTTON_UPLEFT:
            if (g_InputState.dir == kDirWest)
            {
                g_InputState.dir = g_InputState.prevdir;
            }
            g_InputState.prevdir = kDirUnknown;
            break;

        case GP2X_BUTTON_RIGHT:
        case GP2X_BUTTON_DOWNRIGHT:
            if (g_InputState.dir == kDirEast)
            {
                g_InputState.dir = g_InputState.prevdir;
            }
            g_InputState.prevdir = kDirUnknown;
            break;

        default:
            break;
        }
        break;
    }

    return 0;
}

static void input_init_filter()
{
    GP2XJoystick = SDL_JoystickOpen(0);
#if defined(SDL_GP2X__H)
    if (SDL_GP2X_MouseType() == GP2X_MOUSE_TOUCHSCREEN)
    {
        SDL_GP2X_TouchpadMouseMotionEvents(0);
        SDL_GP2X_TouchpadMouseButtonEvents(0);
    }
#endif
}

static void input_shutdown_filter()
{
    if (GP2XJoystick != NULL)
    {
        SDL_JoystickClose(GP2XJoystick);
        GP2XJoystick = NULL;
    }
}

BOOL
UTIL_GetScreenSize(
   DWORD *pdwScreenWidth,
   DWORD *pdwScreenHeight
)
{
   return FALSE;
}

BOOL
UTIL_IsAbsolutePath(
    LPCSTR  lpszFileName
)
{
    return FALSE;
}

int
UTIL_Platform_Startup(
    int   argc,
    char *argv[]
)
{
    gSdlpalCfgFilename = NULL;

    for (int index = 1; index < argc; index++)
    {
        if ( strncmp(argv[index], "--config-file=", 14) == 0)
        {
            const char *param = argv[index] + 14;
            if (param[0] != 0)
            {
                gSdlpalCfgFilename = strdup(param);
            }
        }
    }

    if (gSdlpalCfgFilename == NULL)
    {
        gSdlpalCfgFilename = "sdlpal.cfg";
    }
    return 0;
}

INT
UTIL_Platform_Init(
   int argc,
   char* argv[]
)
{
    InitialVolume = Get_GP2X_Volume();
    atexit(Set_Initial_GP2X_Volume);

    ButtonState = 0;
    GP2XJoystick = NULL;
    PAL_RegisterInputFilter(input_init_filter, input_event_filter, input_shutdown_filter);
    gConfig.fLaunchSetting = FALSE;
    return 0;
}

VOID
AUDIO_Platform_AfterOpenDevice(
   VOID
)
{
    Set_Initial_GP2X_Volume();
}

VOID
VIDEO_Platform_AfterSetVideoMode(
   VOID
)
{
    SDL_ShowCursor(SDL_DISABLE);
#if defined(SDL_GP2X__H)
    SDL_Rect size;
    SDL_GP2X_GetPhysicalScreenSize(&size);
    if (size.h == 240)
    {
        SDL_GP2X_MiniDisplay(0, 20);
    }
#endif
}

VOID
VIDEO_Platform_StretchSurface(
    SDL_Surface    *pSurface
)
{
#if defined(SDL_GP2X__H)
    SDL_Rect size;
    SDL_GP2X_GetPhysicalScreenSize(&size);
    if (size.h == 240)
    {
        size.x = 0;
        size.y = 0;
        size.w = 320;
        size.h = 240;
        SDL_GP2X_DefineRegion(1, &size);

        size.w = pSurface->w;
        size.h = pSurface->h;
        SDL_GP2X_Display(&size);
    }
#endif
}

VOID
VIDEO_Platform_Blit_15_16(
    SDL_Surface    *pSrc,
    SDL_Surface    *pDst
)
{
    if ((pSrc->format->BitsPerPixel == 16) && (pDst->format->BitsPerPixel == 16) &&
        (pSrc->format->Rmask == 0x7c00) && (pDst->format->Rmask == 0xf800) &&
        (pSrc->format->Gmask == 0x03e0) && (pDst->format->Gmask == 0x07e0) &&
        (pSrc->format->Bmask == 0x001f) && (pDst->format->Bmask == 0x001f)
       )
    {
        int srclocked = 0;
        if (SDL_MUSTLOCK(pSrc))
        {
            if (SDL_LockSurface(pSrc))
            {
                goto sdl_blit;
            }
            else
            {
                srclocked = 1;
            }
        }

        int dstlocked = 0;
        if (SDL_MUSTLOCK(pDst))
        {
            if (SDL_LockSurface(pDst))
            {
                if (srclocked) SDL_UnlockSurface(pSrc);
                goto sdl_blit;
            }
            else
            {
                dstlocked = 1;
            }
        }

        int width = pSrc->w;
        if (pDst->w < width) width = pDst->w;

        int height = pSrc->h;
        if (pDst->h < height) height = pDst->h;

        uint32_t *src = (uint32_t *)pSrc->pixels;
        uint32_t *dst = (uint32_t *)pDst->pixels;

#if 0
        for (; height != 0; height--)
        {
            for (int count = width; count != 0; count -= 2)
            {
                uint32_t pix = *src++;
                *dst++ = (pix & 0x001f001f) | ((pix & 0x7fe07fe0) << 1) | ((pix & 0x02000200) >> 4);
            }

            src = (uint32_t *)((pSrc->pitch - width * 2) + (uintptr_t)src);
            dst = (uint32_t *)((pDst->pitch - width * 2) + (uintptr_t)dst);
        }
#else
        uint32_t pix, tmp1, tmp2, count;
        asm volatile (
            "1:\n"
            "MOV     %[count], %[width]\n"
            "2:\n"
            "LDR     %[pix], [%[src]], #4\n"
            "SUBS    %[count], %[count], #2\n"
            "AND     %[tmp1], %[pix], %[blue]\n"
            "AND     %[tmp2], %[pix], %[redgreen]\n"
            "AND     %[pix], %[pix], %[green2]\n"
            "ORR     %[tmp1], %[tmp1], %[tmp2], LSL #1\n"
            "ORR     %[tmp1], %[tmp1], %[pix], LSR #4\n"
            "STR     %[tmp1], [%[dst]], #4\n"
            "BNE     2b\n"
            "SUBS    %[height], %[height], #1\n"
            "ADD     %[src], %[src], %[srcdiff]\n"
            "ADD     %[dst], %[dst], %[dstdiff]\n"
            "BNE     1b\n"
            : [src] "+r" (src), [dst] "+r" (dst), [height] "+r" (height), [pix] "=&r" (pix), [tmp1] "=&r" (tmp1), [tmp2] "=&r" (tmp2), [count] "=&r" (count)
            : [srcdiff] "r" (pSrc->pitch - width * 2), [dstdiff] "r" (pDst->pitch - width * 2), [width] "r" (width), [blue] "r" (0x001f001f), [redgreen] "r" (0x7fe07fe0), [green2] "r" (0x02000200)
            : "cc", "memory");
#endif

        if (dstlocked) SDL_UnlockSurface(pDst);
        if (srclocked) SDL_UnlockSurface(pSrc);

        return;
    }

sdl_blit:
    SDL_BlitSurface(pSrc, NULL, pDst, NULL);
}

VOID
UTIL_Platform_Quit(
   VOID
)
{
}

