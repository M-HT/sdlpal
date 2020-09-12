#include "util.h"
#include "global.h"
#include "palcfg.h"
#include "audio.h"
#include "players.h"

#if PAL_HAS_SOFTMIDI
#if !(defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__))
#include <sys/stat.h>
#endif
#include <stdlib.h>
#include "wildmidi_lib.h"
#include "resampler.h"

typedef struct tagMIDPLAYER
{
	AUDIOPLAYER_COMMONS;

	midi               *pMID;
	INT                 iMusic;
	BOOL                fLoop;
} MIDPLAYER, *LPMIDPLAYER;


static char const *check_file(char const *filename)
{
	if (filename == NULL) return NULL;
	if (*filename == 0) return NULL;

#if (defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__))
	DWORD dwAttrib;

	dwAttrib = GetFileAttributesA(filename);
	if ((dwAttrib == INVALID_FILE_ATTRIBUTES) || (dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
#else
	if (access(filename, F_OK))
#endif
	{
		return NULL;
	}
	else
	{
		return filename;
	}
}



static VOID MID_Close(
	LPMIDPLAYER player
	)
{
	if (player->pMID)
	{
		WildMidi_Close(player->pMID);

		player->pMID = NULL;
		player->iMusic = -1;
	}
}

static VOID
MID_FillBuffer(
	VOID       *object,
	LPBYTE      stream,
	INT         len
	)
{
	LPMIDPLAYER player = (LPMIDPLAYER)object;
	if (player->pMID) {
		int readsize = WildMidi_GetOutput(player->pMID, (int8_t *)stream, len);
		if (readsize < len)
		{
			if (player->fLoop)
			{
				unsigned long int sample_pos = 0;
				WildMidi_FastSeek(player->pMID, &sample_pos);
				WildMidi_GetOutput(player->pMID, readsize + (int8_t *)stream, len - readsize);
			}
			else
			{
				memset(stream + readsize, 0, len - readsize);
			}
		}
	}
}

static VOID
MID_Shutdown(
	VOID       *object
	)
{
	if (object != NULL)
	{
		MID_Close((LPMIDPLAYER)object);
		free(object);
		WildMidi_Shutdown();
	}
}

static BOOL
MID_Play(
	VOID       *object,
	INT         iNum,
	BOOL        fLoop,
	FLOAT       flFadeTime
	)
{
	LPMIDPLAYER player = (LPMIDPLAYER)object;

	//
	// Check for NULL pointer.
	//
	if (player == NULL)
	{
		return FALSE;
	}

	//
	// Stop the current CD music.
	//
	AUDIO_PlayCDTrack(-1);

	player->fLoop = fLoop;

	if (iNum == player->iMusic)
	{
		return TRUE;
	}

	MID_Close(player);

	if (iNum == -1)
	{
		return TRUE;
	}

	if (gConfig.fIsWIN95)
	{
		player->pMID = WildMidi_Open(UTIL_GetFullPathName(PAL_BUFFER_SIZE_ARGS(0), gConfig.pszGamePath, PAL_va(1, "Musics/%.3d.mid", iNum)));
	}

	if (player->pMID == NULL)
	{
		FILE    *fp  = NULL;
		uint8_t *buf = NULL;
		int      size;

		if ((fp = UTIL_OpenFile("midi.mkf")) != NULL)
		{

			if ((size = PAL_MKFGetChunkSize(iNum, fp)) > 0 &&
				(buf = (uint8_t*)UTIL_malloc(size)))
			{
				PAL_MKFReadChunk(buf, size, iNum, fp);
			}
			fclose(fp);
		}

		if (buf)
		{
			player->pMID = WildMidi_OpenBuffer(buf, size);
			free(buf);
		}
	}

	if (player->pMID)
	{
		player->iMusic = iNum;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

LPAUDIOPLAYER
SOFTMIDI_Init(
	VOID
)
{
	char const *timidity_cfg;

	timidity_cfg = check_file(getenv("TIMIDITY_CFG"));

	if (timidity_cfg == NULL) timidity_cfg = check_file("wildmidi.cfg");
	if (timidity_cfg == NULL) timidity_cfg = check_file("timidity.cfg");

#if (defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__))
	if (timidity_cfg == NULL) timidity_cfg = check_file("C:\\TIMIDITY\\timidity.cfg");
#else
	if (timidity_cfg == NULL) timidity_cfg = check_file("/etc/wildmidi/wildmidi.cfg");
	if (timidity_cfg == NULL) timidity_cfg = check_file("/etc/wildmidi.cfg");
	if (timidity_cfg == NULL) timidity_cfg = check_file("/usr/share/wildmidi/wildmidi.cfg");
	if (timidity_cfg == NULL) timidity_cfg = check_file("/usr/local/lib/wildmidi/wildmidi.cfg");

	if (timidity_cfg == NULL) timidity_cfg = check_file("/etc/timidity/timidity.cfg");
	if (timidity_cfg == NULL) timidity_cfg = check_file("/etc/timidity.cfg");
	if (timidity_cfg == NULL) timidity_cfg = check_file("/usr/share/timidity/timidity.cfg");
	if (timidity_cfg == NULL) timidity_cfg = check_file("/usr/local/lib/timidity/timidity.cfg");
#endif

	if (timidity_cfg == NULL) return NULL;

	LPMIDPLAYER player;
	player = (LPMIDPLAYER) malloc(sizeof(MIDPLAYER));
	if (player == NULL) return NULL;

	player->FillBuffer = MID_FillBuffer;
	player->Play = MID_Play;
	player->Shutdown = MID_Shutdown;

	player->pMID = NULL;
	player->iMusic = -1;
	player->fLoop = FALSE;

	if (WildMidi_Init(timidity_cfg, gConfig.iSampleRate, (gConfig.iResampleQuality >= RESAMPLER_QUALITY_SINC)?WM_MO_ENHANCED_RESAMPLING:0))
	{
		free(player);
		return NULL;
	}

	WildMidi_MasterVolume(127);

	return (LPAUDIOPLAYER)player;
}

#else

LPAUDIOPLAYER
SOFTMIDI_Init(
	VOID
)
{
	return NULL;
}

#endif
