/* -*- mode: c; tab-width: 4; c-basic-offset: 4; c-file-style: "linux" -*- */
//
// Copyright (c) 2009-2011, Wei Mingzhi <whistler_wmz@users.sf.net>.
// Copyright (c) 2011-2017, SDLPAL development team.
// All rights reserved.
//
// This file is part of SDLPAL.
//
// SDLPAL is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include "main.h"

#if PAL_HAS_NATIVEMIDI
static int  g_iMidiCurrent = -1;
static NativeMidiSong *g_pMidi = NULL;
static int  g_iFirstRun = 1;
static const char *g_pMusicsDir = NULL;

#if !defined( _WIN32)
#include <sys/stat.h>
#endif

static char const *check_directory(char const *dirname)
{
	if (dirname == NULL) return NULL;
	if (*dirname == 0) return NULL;

#ifdef _WIN32
	DWORD dwAttrib;

	dwAttrib = GetFileAttributesA(dirname);
	if ((dwAttrib == INVALID_FILE_ATTRIBUTES) || !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
	{
		return NULL;
	}
#else
	struct stat st;
	if (0 != stat(dirname, &st)) return NULL;
	if (!S_ISDIR(st.st_mode)) return NULL;
#endif

	return dirname;
}
#endif

void
MIDI_Play(
	int       iNumRIX,
	BOOL      fLoop
)
{
#if PAL_HAS_NATIVEMIDI
	if (!native_midi_detect())
		return;

	if (native_midi_active(g_pMidi) && iNumRIX == g_iMidiCurrent)
	{
		return;
	}

	AUDIO_PlayCDTrack(-1);
	native_midi_stop(g_pMidi);
	native_midi_freesong(g_pMidi);
	g_pMidi = NULL;
	g_iMidiCurrent = -1;

	if (!AUDIO_MusicEnabled() || iNumRIX <= 0)
	{
		return;
	}

	if (g_iFirstRun)
	{
		g_iFirstRun = 0;
		if (check_directory(va("%s%s", gConfig.pszGamePath, "Musics"))) g_pMusicsDir = "Musics";
#if !defined(PAL_FILESYSTEM_IGNORE_CASE) || !PAL_FILESYSTEM_IGNORE_CASE
		else if (check_directory(va("%s%s", gConfig.pszGamePath, "musics"))) g_pMusicsDir = "musics";
		else if (check_directory(va("%s%s", gConfig.pszGamePath, "MUSICS"))) g_pMusicsDir = "MUSICS";
#endif
	}

	if (gConfig.fIsWIN95 && g_pMusicsDir)
	{
		g_pMidi = native_midi_loadsong(va("%s%s/%.3d.mid", gConfig.pszGamePath, g_pMusicsDir, iNumRIX));
#if !defined(PAL_FILESYSTEM_IGNORE_CASE) || !PAL_FILESYSTEM_IGNORE_CASE
		if (!g_pMidi)
		{
			g_pMidi = native_midi_loadsong(va("%s%s/%.3d.MID", gConfig.pszGamePath, g_pMusicsDir, iNumRIX));
		}
#endif
	}

	if (!g_pMidi)
	{
		FILE    *fp  = NULL;
		uint8_t *buf = NULL;
		int      size;

		if ((fp = UTIL_OpenFile("midi.mkf")) != NULL)
		{

			if ((size = PAL_MKFGetChunkSize(iNumRIX, fp)) > 0 &&
				(buf = (uint8_t*)UTIL_malloc(size)))
			{
				PAL_MKFReadChunk(buf, size, iNumRIX, fp);
			}
			fclose(fp);
		}

		if (buf)
		{
			SDL_RWops *rw = SDL_RWFromConstMem(buf, size);
			g_pMidi = native_midi_loadsong_RW(rw);
			SDL_RWclose(rw);
			free(buf);
		}
	}

	if (g_pMidi)
	{
		native_midi_start(g_pMidi, fLoop);
		g_iMidiCurrent = iNumRIX;
	}
#endif
}
