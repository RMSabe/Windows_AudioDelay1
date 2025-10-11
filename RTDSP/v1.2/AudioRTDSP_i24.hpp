/*
	Audio Real-Time Delay for Windows
	Version 1.2

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#ifndef AUDIORTDSP_I24_HPP
#define AUDIORTDSP_I24_HPP

#include "AudioRTDSP.hpp"

class AudioRTDSP_i24 : public AudioRTDSP {
	public:
		AudioRTDSP_i24(const audiortdsp_pb_params_t *p_params);
		~AudioRTDSP_i24(VOID);

	protected:
		static constexpr INT32 SAMPLE_MAX_VALUE = 0x7fffff;
		static constexpr INT32 SAMPLE_MIN_VALUE = -0x800000;

		SIZE_T BYTEBUF_SIZE = 0u;

		UINT8 *p_bytebuf = NULL;

		BOOL WINAPI audio_hw_init(VOID) override;
		BOOL WINAPI buffer_alloc(VOID) override;
		VOID WINAPI buffer_free(VOID) override;
		VOID WINAPI buffer_load(VOID) override;
		VOID WINAPI dsp_proc(VOID) override;
};

#endif /*AUDIORTDSP_I24_HPP*/
