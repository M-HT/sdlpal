/* -*- mode: c; tab-width: 4; c-basic-offset: 4; c-file-style: "linux" -*- */
//
// Copyright (c) 2009-2011, Wei Mingzhi <whistler_wmz@users.sf.net>.
// Copyright (c) 2011-2024, SDLPAL development team.
// All rights reserved.
//
// This file is part of SDLPAL.
//
// SDLPAL is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License, version 3
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include "palcommon.h"
#include "global.h"
#include "palcfg.h"

PAL_FORCE_INLINE
BYTE
PAL_CalcShadowColor(
   BYTE bSourceColor
)
{
    return ((bSourceColor&0xF0)|((bSourceColor&0x0F)>>1));
}

INT
PAL_RLEBlitToSurface(
   LPCBITMAPRLE      lpBitmapRLE,
   SDL_Surface      *lpDstSurface,
   PAL_POS           pos
)
{
    return PAL_RLEBlitToSurfaceWithShadow ( lpBitmapRLE, lpDstSurface, pos, FALSE );
}

INT
PAL_RLEBlitToSurfaceWithShadow(
   LPCBITMAPRLE      lpBitmapRLE,
   SDL_Surface      *lpDstSurface,
   PAL_POS           pos,
   BOOL              bShadow
)
/*++
  Purpose:

    Blit an RLE-compressed bitmap to an SDL surface.
    NOTE: Assume the surface is already locked, and the surface is a 8-bit one.

  Parameters:

    [IN]  lpBitmapRLE - pointer to the RLE-compressed bitmap to be decoded.

    [OUT] lpDstSurface - pointer to the destination SDL surface.

    [IN]  pos - position of the destination area.

    [IN]  bShadow - flag to mention whether blit source color or just shadow.

  Return value:

    0 = success, -1 = error.

--*/
{
   UINT          i, j, k, sx;
   INT           x, y;
   UINT          uiLen       = 0;
   UINT          uiWidth     = 0;
   UINT          uiHeight    = 0;
   UINT          uiSrcX      = 0;
   BYTE          T;
   INT           dx          = PAL_X(pos);
   INT           dy          = PAL_Y(pos);
   LPBYTE        p;

   //
   // Check for NULL pointer.
   //
   if (lpBitmapRLE == NULL || lpDstSurface == NULL)
   {
      return -1;
   }

   //
   // Skip the 0x00000002 in the file header.
   //
   if (lpBitmapRLE[0] == 0x02 && lpBitmapRLE[1] == 0x00 &&
      lpBitmapRLE[2] == 0x00 && lpBitmapRLE[3] == 0x00)
   {
      lpBitmapRLE += 4;
   }

   //
   // Get the width and height of the bitmap.
   //
   uiWidth = lpBitmapRLE[0] | (lpBitmapRLE[1] << 8);
   uiHeight = lpBitmapRLE[2] | (lpBitmapRLE[3] << 8);

   //
   // Check whether bitmap intersects the surface.
   //
   if (uiWidth + dx <= 0 || dx >= lpDstSurface->w ||
       uiHeight + dy <= 0 || dy >= lpDstSurface->h)
   {
      goto end;
   }

   //
   // Calculate the total length of the bitmap.
   // The bitmap is 8-bpp, each pixel will use 1 byte.
   //
   uiLen = uiWidth * uiHeight;

   //
   // Start decoding and blitting the bitmap.
   //
   lpBitmapRLE += 4;
   for (i = 0; i < uiLen;)
   {
      T = *lpBitmapRLE++;
      if ((T & 0x80) && T <= 0x80 + uiWidth)
      {
         i += T - 0x80;
         uiSrcX += T - 0x80;
         if (uiSrcX >= uiWidth)
         {
            uiSrcX -= uiWidth;
            dy++;
         }
      }
      else
      {
         //
         // Prepare coordinates.
         //
         j = 0;
         sx = uiSrcX;
         x = dx + uiSrcX;
         y = dy;

         //
         // Skip the points which are out of the surface.
         //
         if (y < 0)
         {
            j += -y * uiWidth;
            y = 0;
         }
         else if (y >= lpDstSurface->h)
         {
            goto end; // No more pixels needed, break out
         }

         while (j < T)
         {
            //
            // Skip the points which are out of the surface.
            //
            if (x < 0)
            {
               j += -x;
               if (j >= T) break;
               sx += -x;
               x = 0;
            }
            else if (x >= lpDstSurface->w)
            {
               j += uiWidth - sx;
               x -= sx;
               sx = 0;
               y++;
               if (y >= lpDstSurface->h)
               {
                  goto end; // No more pixels needed, break out
               }
               continue;
            }

            //
            // Put the pixels in row onto the surface
            //
            k = T - j;
            if (lpDstSurface->w - x < k) k = lpDstSurface->w - x;
            if (uiWidth - sx < k) k = uiWidth - sx;
            sx += k;
            p = ((LPBYTE)lpDstSurface->pixels) + y * lpDstSurface->pitch;
            if(bShadow)
            {
               j += k;
               for (; k != 0; k--)
               {
                  p[x] = PAL_CalcShadowColor(p[x]);
                  x++;
               }
            }
            else
            {
               for (; k != 0; k--)
               {
                  p[x] = lpBitmapRLE[j];
                  j++;
                  x++;
               }
            }

            if (sx >= uiWidth)
            {
               sx -= uiWidth;
               x -= uiWidth;
               y++;
               if (y >= lpDstSurface->h)
               {
                  goto end; // No more pixels needed, break out
               }
            }
         }
         lpBitmapRLE += T;
         i += T;
         uiSrcX += T;
         while (uiSrcX >= uiWidth)
         {
            uiSrcX -= uiWidth;
            dy++;
         }
      }
   }

end:
   //
   // Success
   //
   return 0;
}

INT
PAL_RLEBlitWithColorShift(
   LPCBITMAPRLE      lpBitmapRLE,
   SDL_Surface      *lpDstSurface,
   PAL_POS           pos,
   INT               iColorShift
)
/*++
  Purpose:

    Blit an RLE-compressed bitmap to an SDL surface.
    NOTE: Assume the surface is already locked, and the surface is a 8-bit one.

  Parameters:

    [IN]  lpBitmapRLE - pointer to the RLE-compressed bitmap to be decoded.

    [OUT] lpDstSurface - pointer to the destination SDL surface.

    [IN]  pos - position of the destination area.

    [IN]  iColorShift - shift the color by this value.

  Return value:

    0 = success, -1 = error.

--*/
{
   UINT          i, j, k, sx;
   INT           x, y;
   UINT          uiLen       = 0;
   UINT          uiWidth     = 0;
   UINT          uiHeight    = 0;
   UINT          uiSrcX      = 0;
   BYTE          T, b;
   INT           dx          = PAL_X(pos);
   INT           dy          = PAL_Y(pos);
   LPBYTE        p;

   //
   // Check for NULL pointer.
   //
   if (lpBitmapRLE == NULL || lpDstSurface == NULL)
   {
      return -1;
   }

   //
   // Skip the 0x00000002 in the file header.
   //
   if (lpBitmapRLE[0] == 0x02 && lpBitmapRLE[1] == 0x00 &&
      lpBitmapRLE[2] == 0x00 && lpBitmapRLE[3] == 0x00)
   {
      lpBitmapRLE += 4;
   }

   //
   // Get the width and height of the bitmap.
   //
   uiWidth = lpBitmapRLE[0] | (lpBitmapRLE[1] << 8);
   uiHeight = lpBitmapRLE[2] | (lpBitmapRLE[3] << 8);

   //
   // Check whether bitmap intersects the surface.
   //
   if (uiWidth + dx <= 0 || dx >= lpDstSurface->w ||
       uiHeight + dy <= 0 || dy >= lpDstSurface->h)
   {
      goto end;
   }

   //
   // Calculate the total length of the bitmap.
   // The bitmap is 8-bpp, each pixel will use 1 byte.
   //
   uiLen = uiWidth * uiHeight;

   //
   // Start decoding and blitting the bitmap.
   //
   lpBitmapRLE += 4;
   for (i = 0; i < uiLen;)
   {
      T = *lpBitmapRLE++;
      if ((T & 0x80) && T <= 0x80 + uiWidth)
      {
         i += T - 0x80;
         uiSrcX += T - 0x80;
         if (uiSrcX >= uiWidth)
         {
            uiSrcX -= uiWidth;
            dy++;
         }
      }
      else
      {
         //
         // Prepare coordinates.
         //
         j = 0;
         sx = uiSrcX;
         x = dx + uiSrcX;
         y = dy;

         //
         // Skip the points which are out of the surface.
         //
         if (y < 0)
         {
            j += -y * uiWidth;
            y = 0;
         }
         else if (y >= lpDstSurface->h)
         {
            goto end; // No more pixels needed, break out
         }

         while (j < T)
         {
            //
            // Skip the points which are out of the surface.
            //
            if (x < 0)
            {
               j += -x;
               if (j >= T) break;
               sx += -x;
               x = 0;
            }
            else if (x >= lpDstSurface->w)
            {
               j += uiWidth - sx;
               x -= sx;
               sx = 0;
               y++;
               if (y >= lpDstSurface->h)
               {
                  goto end; // No more pixels needed, break out
               }
               continue;
            }

            //
            // Put the pixels in row onto the surface
            //
            k = T - j;
            if (lpDstSurface->w - x < k) k = lpDstSurface->w - x;
            if (uiWidth - sx < k) k = uiWidth - sx;
            sx += k;
            p = ((LPBYTE)lpDstSurface->pixels) + y * lpDstSurface->pitch;
            for (; k != 0; k--)
            {
               b = (lpBitmapRLE[j] & 0x0F);
               if ((INT)b + iColorShift > 0x0F)
               {
                  b = 0x0F;
               }
               else if ((INT)b + iColorShift < 0)
               {
                  b = 0;
               }
               else
               {
                  b += iColorShift;
               }

               p[x] = (b | (lpBitmapRLE[j] & 0xF0));
               j++;
               x++;
            }

            if (sx >= uiWidth)
            {
               sx -= uiWidth;
               x -= uiWidth;
               y++;
               if (y >= lpDstSurface->h)
               {
                  goto end; // No more pixels needed, break out
               }
            }
         }
         lpBitmapRLE += T;
         i += T;
         uiSrcX += T;
         while (uiSrcX >= uiWidth)
         {
            uiSrcX -= uiWidth;
            dy++;
         }
      }
   }

end:
   //
   // Success
   //
   return 0;
}

INT
PAL_RLEBlitMonoColor(
   LPCBITMAPRLE      lpBitmapRLE,
   SDL_Surface      *lpDstSurface,
   PAL_POS           pos,
   BYTE              bColor,
   INT               iColorShift
)
/*++
  Purpose:

    Blit an RLE-compressed bitmap to an SDL surface in mono-color form.
    NOTE: Assume the surface is already locked, and the surface is a 8-bit one.

  Parameters:

    [IN]  lpBitmapRLE - pointer to the RLE-compressed bitmap to be decoded.

    [OUT] lpDstSurface - pointer to the destination SDL surface.

    [IN]  pos - position of the destination area.

    [IN]  bColor - the color to be used while drawing.

    [IN]  iColorShift - shift the color by this value.

  Return value:

    0 = success, -1 = error.

--*/
{
   UINT          i, j, k, sx;
   INT           x, y;
   UINT          uiLen       = 0;
   UINT          uiWidth     = 0;
   UINT          uiHeight    = 0;
   UINT          uiSrcX      = 0;
   BYTE          T, b;
   INT           dx          = PAL_X(pos);
   INT           dy          = PAL_Y(pos);
   LPBYTE        p;

   //
   // Check for NULL pointer.
   //
   if (lpBitmapRLE == NULL || lpDstSurface == NULL)
   {
      return -1;
   }

   //
   // Skip the 0x00000002 in the file header.
   //
   if (lpBitmapRLE[0] == 0x02 && lpBitmapRLE[1] == 0x00 &&
      lpBitmapRLE[2] == 0x00 && lpBitmapRLE[3] == 0x00)
   {
      lpBitmapRLE += 4;
   }

   //
   // Get the width and height of the bitmap.
   //
   uiWidth = lpBitmapRLE[0] | (lpBitmapRLE[1] << 8);
   uiHeight = lpBitmapRLE[2] | (lpBitmapRLE[3] << 8);

   //
   // Check whether bitmap intersects the surface.
   //
   if (uiWidth + dx <= 0 || dx >= lpDstSurface->w ||
       uiHeight + dy <= 0 || dy >= lpDstSurface->h)
   {
      goto end;
   }

   //
   // Calculate the total length of the bitmap.
   // The bitmap is 8-bpp, each pixel will use 1 byte.
   //
   uiLen = uiWidth * uiHeight;

   //
   // Start decoding and blitting the bitmap.
   //
   lpBitmapRLE += 4;
   bColor &= 0xF0;
   for (i = 0; i < uiLen;)
   {
      T = *lpBitmapRLE++;
      if ((T & 0x80) && T <= 0x80 + uiWidth)
      {
         i += T - 0x80;
         uiSrcX += T - 0x80;
         if (uiSrcX >= uiWidth)
         {
            uiSrcX -= uiWidth;
            dy++;
         }
      }
      else
      {
         //
         // Prepare coordinates.
         //
         j = 0;
         sx = uiSrcX;
         x = dx + uiSrcX;
         y = dy;

         //
         // Skip the points which are out of the surface.
         //
         if (y < 0)
         {
            j += -y * uiWidth;
            y = 0;
         }
         else if (y >= lpDstSurface->h)
         {
            goto end; // No more pixels needed, break out
         }

         while (j < T)
         {
            //
            // Skip the points which are out of the surface.
            //
            if (x < 0)
            {
               j += -x;
               if (j >= T) break;
               sx += -x;
               x = 0;
            }
            else if (x >= lpDstSurface->w)
            {
               j += uiWidth - sx;
               x -= sx;
               sx = 0;
               y++;
               if (y >= lpDstSurface->h)
               {
                  goto end; // No more pixels needed, break out
               }
               continue;
            }

            //
            // Put the pixels in row onto the surface
            //
            k = T - j;
            if (lpDstSurface->w - x < k) k = lpDstSurface->w - x;
            if (uiWidth - sx < k) k = uiWidth - sx;
            sx += k;
            p = ((LPBYTE)lpDstSurface->pixels) + y * lpDstSurface->pitch;
            for (; k != 0; k--)
            {
               b = lpBitmapRLE[j] & 0x0F;
               if ((INT)b + iColorShift > 0x0F)
               {
                  b = 0x0F;
               }
               else if ((INT)b + iColorShift < 0)
               {
                  b = 0;
               }
               else
               {
                  b += iColorShift;
               }

               p[x] = (b | bColor);
               j++;
               x++;
            }

            if (sx >= uiWidth)
            {
               sx -= uiWidth;
               x -= uiWidth;
               y++;
               if (y >= lpDstSurface->h)
               {
                  goto end; // No more pixels needed, break out
               }
            }
         }
         lpBitmapRLE += T;
         i += T;
         uiSrcX += T;
         while (uiSrcX >= uiWidth)
         {
            uiSrcX -= uiWidth;
            dy++;
         }
      }
   }

end:
   //
   // Success
   //
   return 0;
}

INT
PAL_FBPBlitToSurface(
   LPBYTE            lpBitmapFBP,
   SDL_Surface      *lpDstSurface
)
/*++
  Purpose:

    Blit an uncompressed bitmap in FBP.MKF to an SDL surface.
    NOTE: Assume the surface is already locked, and the surface is a 8-bit 320x200 one.

  Parameters:

    [IN]  lpBitmapFBP - pointer to the RLE-compressed bitmap to be decoded.

    [OUT] lpDstSurface - pointer to the destination SDL surface.

  Return value:

    0 = success, -1 = error.

--*/
{
   int       x, y;
   LPBYTE    p;

   if (lpBitmapFBP == NULL || lpDstSurface == NULL ||
      lpDstSurface->w != 320 || lpDstSurface->h != 200)
   {
      return -1;
   }

   //
   // simply copy everything to the surface
   //
   for (y = 0; y < 200; y++)
   {
      p = (LPBYTE)(lpDstSurface->pixels) + y * lpDstSurface->pitch;
      for (x = 0; x < 320; x++)
      {
         *(p++) = *(lpBitmapFBP++);
      }
   }

   return 0;
}

INT
PAL_RLEGetWidth(
   LPCBITMAPRLE    lpBitmapRLE
)
/*++
  Purpose:

    Get the width of an RLE-compressed bitmap.

  Parameters:

    [IN]  lpBitmapRLE - pointer to an RLE-compressed bitmap.

  Return value:

    Integer value which indicates the height of the bitmap.

--*/
{
   if (lpBitmapRLE == NULL)
   {
      return 0;
   }

   //
   // Skip the 0x00000002 in the file header.
   //
   if (lpBitmapRLE[0] == 0x02 && lpBitmapRLE[1] == 0x00 &&
      lpBitmapRLE[2] == 0x00 && lpBitmapRLE[3] == 0x00)
   {
      lpBitmapRLE += 4;
   }

   //
   // Return the width of the bitmap.
   //
   return lpBitmapRLE[0] | (lpBitmapRLE[1] << 8);
}

INT
PAL_RLEGetHeight(
   LPCBITMAPRLE       lpBitmapRLE
)
/*++
  Purpose:

    Get the height of an RLE-compressed bitmap.

  Parameters:

    [IN]  lpBitmapRLE - pointer of an RLE-compressed bitmap.

  Return value:

    Integer value which indicates the height of the bitmap.

--*/
{
   if (lpBitmapRLE == NULL)
   {
      return 0;
   }

   //
   // Skip the 0x00000002 in the file header.
   //
   if (lpBitmapRLE[0] == 0x02 && lpBitmapRLE[1] == 0x00 &&
      lpBitmapRLE[2] == 0x00 && lpBitmapRLE[3] == 0x00)
   {
      lpBitmapRLE += 4;
   }

   //
   // Return the height of the bitmap.
   //
   return lpBitmapRLE[2] | (lpBitmapRLE[3] << 8);
}

WORD
PAL_SpriteGetNumFrames(
   LPCSPRITE       lpSprite
)
/*++
  Purpose:

    Get the total number of frames of a sprite.

  Parameters:

    [IN]  lpSprite - pointer to the sprite.

  Return value:

    Number of frames of the sprite.

--*/
{
   if (lpSprite == NULL)
   {
      return 0;
   }

   return (lpSprite[0] | (lpSprite[1] << 8)) - 1;
}

LPCBITMAPRLE
PAL_SpriteGetFrame(
   LPCSPRITE       lpSprite,
   INT             iFrameNum
)
/*++
  Purpose:

    Get the pointer to the specified frame from a sprite.

  Parameters:

    [IN]  lpSprite - pointer to the sprite.

    [IN]  iFrameNum - number of the frame.

  Return value:

    Pointer to the specified frame. NULL if the frame does not exist.

--*/
{
   int imagecount, offset;

   if (lpSprite == NULL)
   {
      return NULL;
   }

   //
   // Hack for broken sprites like the Bloody-Mouth Bug
   //
//   imagecount = (lpSprite[0] | (lpSprite[1] << 8)) - 1;
   imagecount = (lpSprite[0] | (lpSprite[1] << 8));

   if (iFrameNum < 0 || iFrameNum >= imagecount)
   {
      //
      // The frame does not exist
      //
      return NULL;
   }

   //
   // Get the offset of the frame
   //
   iFrameNum <<= 1;
   offset = ((lpSprite[iFrameNum] | (lpSprite[iFrameNum + 1] << 8)) << 1);
   if (offset == 0x18444) offset = (WORD)offset;
   return &lpSprite[offset];
}

INT
PAL_MKFGetChunkCount(
   FILE *fp
)
/*++
  Purpose:

    Get the number of chunks in an MKF archive.

  Parameters:

    [IN]  fp - pointer to an fopen'ed MKF file.

  Return value:

    Integer value which indicates the number of chunks in the specified MKF file.

--*/
{
   INT iNumChunk;
   if (fp == NULL)
   {
      return 0;
   }

   fseek(fp, 0, SEEK_SET);
   if (fread(&iNumChunk, sizeof(INT), 1, fp) == 1)
      return (SDL_SwapLE32(iNumChunk) - 4) >> 2;
   else
      return 0;
}

INT
PAL_MKFGetChunkSize(
   UINT    uiChunkNum,
   FILE   *fp
)
/*++
  Purpose:

    Get the size of a chunk in an MKF archive.

  Parameters:

    [IN]  uiChunkNum - the number of the chunk in the MKF archive.

    [IN]  fp - pointer to the fopen'ed MKF file.

  Return value:

    Integer value which indicates the size of the chunk.
    -1 if the chunk does not exist.

--*/
{
   UINT    uiOffset       = 0;
   UINT    uiNextOffset   = 0;
   UINT    uiChunkCount   = 0;

   //
   // Get the total number of chunks.
   //
   uiChunkCount = PAL_MKFGetChunkCount(fp);
   if (uiChunkNum >= uiChunkCount)
   {
      return -1;
   }

   //
   // Get the offset of the specified chunk and the next chunk.
   //
   fseek(fp, 4 * uiChunkNum, SEEK_SET);
   PAL_fread(&uiOffset, sizeof(UINT), 1, fp);
   PAL_fread(&uiNextOffset, sizeof(UINT), 1, fp);

   uiOffset = SDL_SwapLE32(uiOffset);
   uiNextOffset = SDL_SwapLE32(uiNextOffset);

   //
   // Return the length of the chunk.
   //
   return uiNextOffset - uiOffset;
}

INT
PAL_MKFReadChunk(
   LPBYTE          lpBuffer,
   UINT            uiBufferSize,
   UINT            uiChunkNum,
   FILE           *fp
)
/*++
  Purpose:

    Read a chunk from an MKF archive into lpBuffer.

  Parameters:

    [OUT] lpBuffer - pointer to the destination buffer.

    [IN]  uiBufferSize - size of the destination buffer.

    [IN]  uiChunkNum - the number of the chunk in the MKF archive to read.

    [IN]  fp - pointer to the fopen'ed MKF file.

  Return value:

    Integer value which indicates the size of the chunk.
    -1 if there are error in parameters.
    -2 if buffer size is not enough.

--*/
{
   UINT     uiOffset       = 0;
   UINT     uiNextOffset   = 0;
   UINT     uiChunkCount;
   UINT     uiChunkLen;

   if (lpBuffer == NULL || fp == NULL || uiBufferSize == 0)
   {
      return -1;
   }

   //
   // Get the total number of chunks.
   //
   uiChunkCount = PAL_MKFGetChunkCount(fp);
   if (uiChunkNum >= uiChunkCount)
   {
      return -1;
   }

   //
   // Get the offset of the chunk.
   //
   fseek(fp, 4 * uiChunkNum, SEEK_SET);
   PAL_fread(&uiOffset, 4, 1, fp);
   PAL_fread(&uiNextOffset, 4, 1, fp);
   uiOffset = SDL_SwapLE32(uiOffset);
   uiNextOffset = SDL_SwapLE32(uiNextOffset);

   //
   // Get the length of the chunk.
   //
   uiChunkLen = uiNextOffset - uiOffset;

   if (uiChunkLen > uiBufferSize)
   {
      return -2;
   }

   if (uiChunkLen != 0)
   {
      fseek(fp, uiOffset, SEEK_SET);
      return (int)fread(lpBuffer, 1, uiChunkLen, fp);
   }

   return -1;
}

INT
PAL_MKFGetDecompressedSize(
   UINT    uiChunkNum,
   FILE   *fp
)
/*++
  Purpose:

    Get the decompressed size of a compressed chunk in an MKF archive.

  Parameters:

    [IN]  uiChunkNum - the number of the chunk in the MKF archive.

    [IN]  fp - pointer to the fopen'ed MKF file.

  Return value:

    Integer value which indicates the size of the chunk.
    -1 if the chunk does not exist.

--*/
{
   DWORD         buf[2];
   UINT          uiOffset;
   UINT          uiChunkCount;

   if (fp == NULL)
   {
      return -1;
   }

   //
   // Get the total number of chunks.
   //
   uiChunkCount = PAL_MKFGetChunkCount(fp);
   if (uiChunkNum >= uiChunkCount)
   {
      return -1;
   }

   //
   // Get the offset of the chunk.
   //
   fseek(fp, 4 * uiChunkNum, SEEK_SET);
   PAL_fread(&uiOffset, 4, 1, fp);
   uiOffset = SDL_SwapLE32(uiOffset);

   //
   // Read the header.
   //
   fseek(fp, uiOffset, SEEK_SET);
   if (gConfig.fIsWIN95)
   {
      PAL_fread(buf, sizeof(DWORD), 1, fp);
      buf[0] = SDL_SwapLE32(buf[0]);

      return (INT)buf[0];
   }
   else
   {
      PAL_fread(buf, sizeof(DWORD), 2, fp);
      buf[0] = SDL_SwapLE32(buf[0]);
      buf[1] = SDL_SwapLE32(buf[1]);

      return (buf[0] != 0x315f4a59) ? -1 : (INT)buf[1];
   }
}

INT
PAL_MKFDecompressChunk(
   LPBYTE          lpBuffer,
   UINT            uiBufferSize,
   UINT            uiChunkNum,
   FILE           *fp
)
/*++
  Purpose:

    Decompress a compressed chunk from an MKF archive into lpBuffer.

  Parameters:

    [OUT] lpBuffer - pointer to the destination buffer.

    [IN]  uiBufferSize - size of the destination buffer.

    [IN]  uiChunkNum - the number of the chunk in the MKF archive to read.

    [IN]  fp - pointer to the fopen'ed MKF file.

  Return value:

    Integer value which indicates the size of the chunk.
    -1 if there are error in parameters, or buffer size is not enough.
    -3 if cannot allocate memory for decompression.

--*/
{
   LPBYTE          buf;
   int             len;

   len = PAL_MKFGetChunkSize(uiChunkNum, fp);

   if (len <= 0)
   {
      return len;
   }

   buf = (LPBYTE)malloc(len);
   if (buf == NULL)
   {
      return -3;
   }

   PAL_MKFReadChunk(buf, len, uiChunkNum, fp);

   len = Decompress(buf, lpBuffer, uiBufferSize);
   free(buf);

   return len;
}
