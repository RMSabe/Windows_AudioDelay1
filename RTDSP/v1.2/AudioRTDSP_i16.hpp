/*
	Audio Real-Time Delay for Windows
	Version 1.2

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#ifndef AUDIORTDSP_I16_HPP
#define AUDIORTDSP_I16_HPP

#include "AudioRTDSP.hpp"

class AudioRTDSP_i16 : public AudioRTDSP {
	public:
		AudioRTDSP_i16(const audiortdsp_pb_params_t *p_params);
		~AudioRTDSP_i16(VOID);

	protected:
		static constexpr INT32 SAMPLE_MAX_VALUE = 0x7fff;
		static constexpr INT32 SAMPLE_MIN_VALUE = -0x8000;

		/*
			DSPFRAME is a buffer that stores a single frame of audio.
			Sample size on DSPFRAME is bigger than the audio sample size.
			DSPFRAME is where most DSP math operations will happen, it's used
			to prevent integer overflow.
		*/

		static constexpr SIZE_T DSPFRAME_SAMPLE_SIZE_BYTES = 4u;
		SIZE_T DSPFRAME_SIZE_BYTES = 0u;

		INT32 *p_dspframe = NULL;

		BOOL WINAPI audio_hw_init(VOID) override;
		BOOL WINAPI buffer_alloc(VOID) override;
		VOID WINAPI buffer_free(VOID) override;
		VOID WINAPI buffer_load(VOID) override;
		VOID WINAPI dsp_proc(VOID) override;
};

#endif /*AUDIORTDSP_I16_HPP*/
