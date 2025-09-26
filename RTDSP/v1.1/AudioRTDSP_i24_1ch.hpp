/*
	Audio Real-Time Delay for Windows
	Version 1.1

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#ifndef AUDIORTDSP_I24_1CH_HPP
#define AUDIORTDSP_I24_1CH_HPP

#include "AudioRTDSP.hpp"

class AudioRTDSP_i24_1ch : public AudioRTDSP {
	public:
		AudioRTDSP_i24_1ch(const audiortdsp_pb_params_t *p_params);
		~AudioRTDSP_i24_1ch(VOID);

	protected:
		static constexpr INT32 SAMPLE_MAX_VALUE_I24 = 0x7fffff;
		static constexpr INT32 SAMPLE_MIN_VALUE_I24 = -0x800000;

		static constexpr INT32 SAMPLE_MAX_VALUE_I16 = 0x7fff;
		static constexpr INT32 SAMPLE_MIN_VALUE_I16 = -0x8000;

		SIZE_T BYTEBUF_SIZE = 0u;

		UINT8 *p_bytebuf = NULL;

		BOOL WINAPI audio_hw_init(VOID) override;
		BOOL WINAPI buffer_alloc(VOID) override;
		VOID WINAPI buffer_free(VOID) override;
		VOID WINAPI buffer_load(VOID) override;
		VOID WINAPI buffer_play(VOID) override;
		VOID WINAPI dsp_proc(VOID) override;
};

#endif /*AUDIORTDSP_I24_1CH_HPP*/
