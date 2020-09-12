#include <gtest/gtest.h>
#include <arm_neon.h>
extern "C"{
    #include "audio.h"
}


void
AUDIO_AdjustVolume_c(
	short     *srcdst,
	int        iVolume,
	int        samples
)
{
	if (iVolume == SDL_MIX_MAXVOLUME) return;
	if (iVolume == 0) { memset(srcdst, 0, samples << 1); return; }
	while (samples > 0)
	{
		*srcdst = *srcdst * iVolume / SDL_MIX_MAXVOLUME;
		samples--; srcdst++;
	}
}

void
AUDIO_AdjustVolume_neon(
	short     *srcdst,
	int        iVolume,
	int        samples
)
{
	if (iVolume == SDL_MIX_MAXVOLUME) return;
	if (iVolume == 0) { memset(srcdst, 0, samples << 1); return; }
	const int16x4_t iVolumex4 = vdup_n_s16(iVolume);
	const int32x4_t iZerox4 = vmovq_n_s32(0);
	for (; samples >= 4; samples -= 4)
	{
		int32x4_t valx4 = vmull_s16(vld1_s16(srcdst), iVolumex4);
		valx4 += (int32x4_t)(vcltq_s32(valx4, iZerox4) & 127);
		vst1_s16(srcdst, vshrn_n_s32(valx4, 7));
		srcdst += 4;
	}
	while (samples > 0)
	{
		*srcdst = *srcdst * iVolume / SDL_MIX_MAXVOLUME;
		samples--; srcdst++;
	}
}


void
AUDIO_MixNative_c(
	short     *dst,
	short     *src,
	int        samples
)
{
	while (samples > 0)
	{
		int val = *src++ + *dst;
		if (val > SHRT_MAX)
			*dst++ = SHRT_MAX;
		else if (val < SHRT_MIN)
			*dst++ = SHRT_MIN;
		else
			*dst++ = (short)val;
		samples--;
	}
}

void
AUDIO_MixNative_neon(
	short     *dst,
	short     *src,
	int        samples
)
{
	for (; samples >= 8; samples -= 8)
	{
		vst1q_s16(dst, vqaddq_s16(vld1q_s16(src), vld1q_s16(dst)));
		src += 8;
		dst += 8;
	}
	Uint32 val1, val2;
	asm volatile (
		"cmp %[samples], #1\n"
		"blt 3f\n"
		"beq 2f\n"
		"1:\n"
		"ldr %[val1], [%[src]], #4\n"
		"sub %[samples], %[samples], #2\n"
		"ldr %[val2], [%[dst]]\n"
		"cmp %[samples], #1\n"
		"qadd16 %[val2], %[val1], %[val2]\n"
		"str %[val2], [%[dst]], #4\n"
		"bgt 1b\n"
		"blt 3f\n"
		"2:\n"
		"ldrh %[val1], [%[src]]\n"
		"ldrh %[val2], [%[dst]]\n"
		"qadd16 %[val2], %[val1], %[val2]\n"
		"strh %[val2], [%[dst]]\n"
		"3:\n"
		: [src] "+r" (src), [dst] "+r" (dst), [samples] "+r" (samples), [val1] "=&r" (val1), [val2] "=&r" (val2)
		:
		: "cc", "memory"
	);
}


TEST(sdlpal, AUDIO_AdjustVolume) {
    ASSERT_EQ(SDL_MIX_MAXVOLUME, 128) << "Unexpected SDL_MIX_MAXVOLUME value";

    short *values = (short *)malloc((65536 + 4) * 2);
    ASSERT_NE(values, nullptr) << "Unable to allocate values";
    short *dst1 = (short *)malloc((65536 + 8) * 2);
    ASSERT_NE(dst1, nullptr) << "Unable to allocate dst1";
    short *dst2 = (short *)malloc((65536 + 4) * 2);
    ASSERT_NE(dst2, nullptr) << "Unable to allocate dst2";

    short *dst_aligned = (short *) ((7 + (uintptr_t)dst1) & ~(uintptr_t)7);
    short *dst_unaligned = (short *) (((7 + (uintptr_t)dst1) & ~(uintptr_t)7) + 2);

    for (int iValue = 0; iValue < 65536 + 4; iValue++)
    {
        values[iValue] = (short)((16384 + iValue) & 0xffff);
    }

    for (int iVolume = 1; iVolume <= 127; iVolume++)
    {
        memcpy(dst2, values, (65536 + 3) * 2);
        AUDIO_AdjustVolume_c(dst2, iVolume, 65536 + 3);

        memcpy(dst_aligned, values, (65536 + 3) * 2);
        AUDIO_AdjustVolume_neon(dst_aligned, iVolume, 65536 + 3);

        EXPECT_EQ(0, memcmp(dst2, dst_aligned, (65536 + 3) * 2)) << "Failed aligned at iVolume " << iVolume;


        memcpy(dst_unaligned, values, (65536 + 3) * 2);
        AUDIO_AdjustVolume_neon(dst_unaligned, iVolume, 65536 + 3);

        EXPECT_EQ(0, memcmp(dst2, dst_unaligned, (65536 + 3) * 2)) << "Failed unaligned at iVolume " << iVolume;
    }


    free(dst2);
    free(dst1);
    free(values);
}


TEST(sdlpal, AUDIO_MixNative) {
    short *src = (short *)malloc((65536 + 8) * 2);
    ASSERT_NE(src, nullptr) << "Unable to allocate src";
    short *dst1 = (short *)malloc((65536 + 8) * 2);
    ASSERT_NE(dst1, nullptr) << "Unable to allocate dst1";
    short *dst2 = (short *)malloc((65536 + 8) * 2);
    ASSERT_NE(dst2, nullptr) << "Unable to allocate dst2";

    for (int iValue = 0; iValue < 65536 + 8; iValue++)
    {
        src[iValue] = (short)((16384 + iValue) & 0xffff);
        dst1[iValue] = dst2[iValue] = (short)((iValue) & 0xffff);
    }

    AUDIO_MixNative_c(dst1, src, 65536 + 7);
    AUDIO_MixNative_neon(dst2, src, 65536 + 7);

    EXPECT_EQ(0, memcmp(dst1, dst2, (65536 + 7) * 2));

    free(dst2);
    free(dst1);
    free(src);
}

