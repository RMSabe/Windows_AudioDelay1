/*
	Audio Real-Time Delay for Windows
	Version 1.1

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#ifndef AUDIORTDSP_I16_2CH_HPP
#define AUDIORTDSP_I16_2CH_HPP

#include "AudioRTDSP.hpp"

class AudioRTDSP_i16_2ch : public AudioRTDSP {
	public:
		AudioRTDSP_i16_2ch(const audiortdsp_pb_params_t *p_params);
		~AudioRTDSP_i16_2ch(VOID);

	protected:
		static constexpr INT32 SAMPLE_MAX_VALUE = 0x7fff;
		static constexpr INT32 SAMPLE_MIN_VALUE = -0x8000;

		BOOL WINAPI audio_hw_init(VOID) override;
		BOOL WINAPI buffer_alloc(VOID) override;
		VOID WINAPI buffer_free(VOID) override;
		VOID WINAPI buffer_load(VOID) override;
		VOID WINAPI buffer_play(VOID) override;
		VOID WINAPI dsp_proc(VOID) override;
};

#endif /*AUDIORTDSP_I16_2CH_HPP*/
