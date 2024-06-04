#include "main.h"
#include <arm_neon.h>

const char *gSdlpalCfgFilename;

static int input_event_filter(const SDL_Event *lpEvent, volatile PALINPUTSTATE *state)
{
    if (lpEvent->type == SDL_KEYDOWN)
    {
        if ((lpEvent->key.keysym.mod & KMOD_ALT) && (lpEvent->key.keysym.sym == SDLK_x))
        {
            PAL_Shutdown(0);
        }
        switch (lpEvent->key.keysym.sym)
        {
        case SDLK_INSERT:
            if (gpGlobals->fInBattle)
            {
                g_InputState.dwKeyPress |= kKeyRepeat;
            }
            else
            {
                g_InputState.dwKeyPress |= kKeyPgUp;
            }
            break;
        case SDLK_DELETE:
            if (gpGlobals->fInBattle)
            {
                g_InputState.dwKeyPress |= kKeyAuto;
            }
            else
            {
                g_InputState.dwKeyPress |= kKeyPgDn;
            }
            break;
        default:
            break;
        }
    }

    return 0;
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
    PAL_RegisterInputFilter(NULL, input_event_filter, NULL);
    gConfig.fLaunchSetting = FALSE;
    return 0;
}

VOID
VIDEO_Platform_Blit_15_32(
    SDL_Surface    *pSrc,
    SDL_Surface    *pDst
)
{
    if ((pSrc->format->BitsPerPixel == 15 || pSrc->format->BitsPerPixel == 16) && (pDst->format->BitsPerPixel == 32) &&
        (pSrc->format->Rmask == 0x7c00) && (pDst->format->Rmask == 0x00ff0000) &&
        (pSrc->format->Gmask == 0x03e0) && (pDst->format->Gmask == 0x0000ff00) &&
        (pSrc->format->Bmask == 0x001f) && (pDst->format->Bmask == 0x000000ff)
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

        uint16_t *src = (uint16_t *)pSrc->pixels;
        uint32_t *dst = (uint32_t *)pDst->pixels;

        if (width & 3)
        {
            for (; height != 0; height--)
            {
                for (int count = width; count != 0; count--)
                {
                    uint32_t pix = *src++;
                    *dst++ = ((pix & 0x001f) << 3) | ((pix & 0x001f) >> 2)
                           | ((pix & 0x03e0) << 6) | ((pix & 0x0380) << 1)
                           | ((pix & 0x7c00) << 9) | ((pix & 0x7000) << 4)
                           ;
                }

                src = (uint16_t *)((pSrc->pitch - width * 2) + (uintptr_t)src);
                dst = (uint32_t *)((pDst->pitch - width * 4) + (uintptr_t)dst);
            }
        }
        else
        {
            uint32_t count;
            asm volatile (
                "MOVW       %[count], #0x001f\n"
                "VDUP.16    q0, %[count]\n"         // q0 = 0x001f
                "MOVW       %[count], #0x03e0\n"
                "VDUP.16    q1, %[count]\n"         // q1 = 0x03e0
                "MOVW       %[count], #0x0380\n"
                "VDUP.16    q2, %[count]\n"         // q2 = 0x0380
                "MOVW       %[count], #0x7c00\n"
                "VDUP.16    q3, %[count]\n"         // q3 = 0x7c00

                "1:\n"
                "MOV        %[count], %[width]\n"

                "2:\n"
                "VLD1.16    {q13}, [%[src]]!\n"     // q13 = src[0-7]
                "SUB        %[count], %[count], #8\n"
                "VAND       q10, q13, q0\n"         // q10 = pix & 0x001f
                "VAND       q11, q13, q1\n"         // q11 = pix & 0x03e0
                "VSHL.i16   q8, q10, #3\n"          // q8 = (pix & 0x001f) << 3
                "VSHR.u16   q10, q10, #2\n"         // q10 = (pix & 0x001f) >> 2
                "VAND       q12, q13, q2\n"         // q12 = pix & 0x0380
                "VORR       q8, q8, q10\n"          // q8 = ((pix & 0x001f) << 3) | ((pix & 0x001f) >> 2)
                "VSHL.i16   q11, q11, #6\n"         // q11 = (pix & 0x03e0) << 6
                "VAND       q13, q13, q3\n"         // q13 = pix & 0x7c00
                "VORR       q8, q8, q11\n"          // q8 = ((pix & 0x001f) << 3) | ((pix & 0x001f) >> 2) | ((pix & 0x03e0) << 6)
                "VSHL.i16   q12, q12, #1\n"         // q12 = (pix & 0x0380) << 1
                "VSHR.u16   q9, q13, #7\n"          // q9 = (pix & 0x7c00) >> 7
                "VSHR.u16   q13, q13, #12\n"        // q13 = (pix & 0x7c00) >> 12
                "VORR       q8, q8, q12\n"          // q8 = ((pix & 0x001f) << 3) | ((pix & 0x001f) >> 2) | ((pix & 0x03e0) << 6) | ((pix & 0x0380) << 1)
                "VORR       q9, q9, q13\n"          // q9 = ((pix & 0x7c00) >> 7 | (pix & 0x7c00) >> 12)
                "CMP        %[count], #8\n"
                "VST2.16    {q8,q9}, [%[dst]]!\n"   // dst[0-7] = q8,q9
                "BHS        2b\n"

                "CMP        %[count], #0\n"
                "BEQ        3f\n"

                "VLD1.16    {d26}, [%[src]]!\n"     // d26 = src[0-3]
                "VAND       d20, d26, d0\n"         // d20 = pix & 0x001f
                "VAND       d22, d26, d2\n"         // d22 = pix & 0x03e0
                "VSHL.i16   d16, d20, #3\n"         // d16 = (pix & 0x001f) << 3
                "VSHR.u16   d20, d20, #2\n"         // d20 = (pix & 0x001f) >> 2
                "VAND       d24, d26, d4\n"         // d24 = pix & 0x0380
                "VORR       d16, d16, d20\n"        // d16 = ((pix & 0x001f) << 3) | ((pix & 0x001f) >> 2)
                "VSHL.i16   d22, d22, #6\n"         // d22 = (pix & 0x03e0) << 6
                "VAND       d26, d26, d6\n"         // d26 = pix & 0x7c00
                "VORR       d16, d16, d22\n"        // d16 = ((pix & 0x001f) << 3) | ((pix & 0x001f) >> 2) | ((pix & 0x03e0) << 6)
                "VSHL.i16   d24, d24, #1\n"         // d24 = (pix & 0x0380) << 1
                "VSHR.u16   d18, d26, #7\n"         // d18 = (pix & 0x7c00) >> 7
                "VSHR.u16   d26, d26, #12\n"        // d26 = (pix & 0x7c00) >> 12
                "VORR       d16, d16, d24\n"        // d16 = ((pix & 0x001f) << 3) | ((pix & 0x001f) >> 2) | ((pix & 0x03e0) << 6) | ((pix & 0x0380) << 1)
                "VORR       d18, d18, d26\n"        // d18 = ((pix & 0x7c00) >> 7 | (pix & 0x7c00) >> 12)
                "VST2.16    {d16,d18}, [%[dst]]!\n" // dst[0-3] = d16,d18

                "3:\n"
                "SUBS    %[height], %[height], #1\n"
                "ADD     %[src], %[src], %[srcdiff]\n"
                "ADD     %[dst], %[dst], %[dstdiff]\n"
                "BNE     1b\n"
                : [src] "+r" (src), [dst] "+r" (dst), [height] "+r" (height), [count] "=&r" (count)
                : [srcdiff] "r" (pSrc->pitch - width * 2), [dstdiff] "r" (pDst->pitch - width * 4), [width] "r" (width)
                : "cc", "memory", "q0", "q1", "q2", "q3", "q8", "q9", "q10", "q11", "q12", "q13");
        }

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

