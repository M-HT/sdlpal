#include <gtest/gtest.h>
#include <arm_neon.h>
extern "C"{
    #include "battle.h"
}

BATTLE          g_Battle;


VOID
PAL_BattleMakeScene_blit_c(
    VOID
)
{
   int          i;
   LPBYTE       pSrc, pDst;
   BYTE         b;

   //
   // Draw the background
   //
   pSrc = (LPBYTE) g_Battle.lpBackground->pixels;
   pDst = (LPBYTE) g_Battle.lpSceneBuf->pixels;

   for (i = 0; i < g_Battle.lpSceneBuf->pitch * g_Battle.lpSceneBuf->h; i++)
   {
      b = (*pSrc & 0x0F);
      b += g_Battle.sBackgroundColorShift;

      if (b & 0x80)
      {
         b = 0;
      }
      else if (b & 0x70)
      {
         b = 0x0F;
      }

      *pDst = (b | (*pSrc & 0xF0));

      ++pSrc;
      ++pDst;
   }
}

VOID
PAL_BattleMakeScene_blit_neon(
    VOID
)
{
   int          i;
   LPBYTE       pSrc, pDst;
   BYTE         b;

   //
   // Draw the background
   //
   pSrc = (LPBYTE) g_Battle.lpBackground->pixels;
   pDst = (LPBYTE) g_Battle.lpSceneBuf->pixels;

   int8x16_t iColorShiftx16 = vdupq_n_s8(g_Battle.sBackgroundColorShift);
   for (i = g_Battle.lpSceneBuf->pitch * g_Battle.lpSceneBuf->h; i != 0; i -= 16)
   {
      uint8x16_t bx16 = vld1q_u8(pSrc);
      vst1q_u8(pDst, vshrq_n_u8(vqshluq_n_s8((int8x16_t)((bx16 & 0x0F) + iColorShiftx16), 4), 4) | (bx16 & 0xF0));
      pSrc += 16;
      pDst += 16;
   }
}


TEST(sdlpal, PAL_BattleMakeScene_blit) {
    SDL_Surface *surface_src = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 200, 8, 0, 0, 0, 0);
    ASSERT_NE(surface_src, nullptr) << "Unable to allocate surface_src";
    SDL_Surface *surface_dst1 = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 200, 8, 0, 0, 0, 0);
    ASSERT_NE(surface_dst1, nullptr) << "Unable to allocate surface_dst1";
    SDL_Surface *surface_dst2 = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 200, 8, 0, 0, 0, 0);
    ASSERT_NE(surface_dst2, nullptr) << "Unable to allocate surface_dst2";

    for (int y = 0; y < surface_src->h; y++)
    {
        uint8_t *psrc = (uint8_t *) (y * surface_src->pitch + (uintptr_t)surface_src->pixels);
        for (int x = 0; x < surface_src->pitch; x++)
        {
            uint8_t pix = (x & 0x0f) | (y << 4);
            psrc[x] = pix;
        }
    }

    g_Battle.lpBackground = surface_src;

    for (g_Battle.sBackgroundColorShift = -16; g_Battle.sBackgroundColorShift <= 16; g_Battle.sBackgroundColorShift++)
    {
        SDL_FillRect(surface_dst1, nullptr, 0);
        SDL_FillRect(surface_dst2, nullptr, 0);

        g_Battle.lpSceneBuf = surface_dst1;
        PAL_BattleMakeScene_blit_c();

        g_Battle.lpSceneBuf = surface_dst2;
        PAL_BattleMakeScene_blit_neon();

        EXPECT_EQ(0, memcmp(surface_dst1->pixels, surface_dst2->pixels, surface_dst1->h * surface_dst1->pitch)) << "Failed at sBackgroundColorShift " << g_Battle.sBackgroundColorShift;
    }

    SDL_FreeSurface(surface_dst2);
    SDL_FreeSurface(surface_dst1);
    SDL_FreeSurface(surface_src);
}

