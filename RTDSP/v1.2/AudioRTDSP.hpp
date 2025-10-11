/*
	Audio Real-Time Delay for Windows
	Version 1.2

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#ifndef AUDIORTDSP_HPP
#define AUDIORTDSP_HPP

#include "globldef.h"

#include "strdef.hpp"
#include "shared.hpp"

#include <mmdeviceapi.h>
#include <audioclient.h>

struct _audiortdsp_pb_params {
	const TCHAR *file_dir;
	ULONG64 audio_data_begin;
	ULONG64 audio_data_end;
	UINT32 sample_rate;
	UINT16 n_channels;
};

struct _audiortdsp_fx_params {
	INT32 n_delay;
	INT32 n_feedback;
	BOOL feedback_alt_pol;
	BOOL cyclediv_inc_one;
};

typedef struct _audiortdsp_pb_params audiortdsp_pb_params_t;
typedef struct _audiortdsp_fx_params audiortdsp_fx_params_t;

class AudioRTDSP {
	public:
		AudioRTDSP(const audiortdsp_pb_params_t *p_params);

		BOOL WINAPI setPlaybackParameters(const audiortdsp_pb_params_t *p_params);
		BOOL WINAPI initialize(VOID);
		BOOL WINAPI runPlayback(VOID);
		VOID WINAPI stopPlayback(VOID);

		BOOL WINAPI loadAudioDeviceList(HWND p_listbox);
		BOOL WINAPI chooseDevice(SIZE_T index);
		BOOL WINAPI chooseDefaultDevice(VOID);

		BOOL WINAPI getFXParams(audiortdsp_fx_params_t *p_params);

		BOOL WINAPI setFXDelay(SIZE_T n_delay);
		BOOL WINAPI setFXFeedback(SIZE_T n_feedback);
		BOOL WINAPI enableFeedbackAltPol(BOOL enable);
		BOOL WINAPI enableCycleDivIncOne(BOOL enable);

		__string WINAPI getLastErrorMessage(VOID);

		enum Status {
			STATUS_ERROR_MEMALLOC = -4,
			STATUS_ERROR_NOFILE = -3,
			STATUS_ERROR_AUDIOHW = -2,
			STATUS_ERROR_GENERIC = -1,
			STATUS_UNINITIALIZED = 0,
			STATUS_READY = 1,
			STATUS_PLAYING = 2
		};

	protected:
		HANDLE h_filein = INVALID_HANDLE_VALUE;

		fileptr64_t filein_size_64 = {
			.l32 = 0u,
			.h32 = 0u
		};

		fileptr64_t filein_pos_64 = {
			.l32 = 0u,
			.h32 = 0u
		};

		ULONG64 AUDIO_DATA_BEGIN = 0u;
		ULONG64 AUDIO_DATA_END = 0u;

		IMMDeviceEnumerator *p_audiodevenum = NULL;
		IMMDeviceCollection *p_audiodevcoll = NULL;

		IMMDevice *p_audiodev = NULL;
		IAudioClient *p_audiomgr = NULL;
		IAudioRenderClient *p_audioout = NULL;

		/*
			BUFFERS:
			Buffers will be split into segments:

			Input buffer is the source to the DSP. It should be split into multiple segments.
			Output buffer is just 2 segments: currently loading and currently playing.

			1 buffer segment size should be close (maybe a little above) half the audio buffer size. It MUST be a power of 2.
		*/

		SIZE_T AUDIOBUFFER_SIZE_FRAMES = 0u;
		SIZE_T AUDIOBUFFER_SIZE_SAMPLES = 0u;
		SIZE_T AUDIOBUFFER_SIZE_BYTES = 0u;

		/*
			AUDIOBUFFER_SEGMENT_SIZE and BUFFER_SEGMENT_SIZE theoretically should be the same,
			however, due to possible audio hardware limitations (bit depth, number of channels, etc...), 
			it might be needed to make format conversions from p_bufferoutput to p_audiobuffer.

			BUFFER_SEGMENT_SIZE_... refers to p_bufferinput and p_bufferoutput.
			AUDIOBUFFER_SEGMENT_SIZE_... refers to p_audiobuffer.
		*/

		SIZE_T AUDIOBUFFER_SEGMENT_SIZE_FRAMES = 0u;
		SIZE_T AUDIOBUFFER_SEGMENT_SIZE_SAMPLES = 0u;
		SIZE_T AUDIOBUFFER_SEGMENT_SIZE_BYTES = 0u;

		SIZE_T BUFFER_SEGMENT_SIZE_FRAMES = 0u;
		SIZE_T BUFFER_SEGMENT_SIZE_SAMPLES = 0u;
		SIZE_T BUFFER_SEGMENT_SIZE_BYTES = 0u;

		static constexpr SIZE_T BUFFERIN_SIZE_FRAMES = 65536u;
		SIZE_T BUFFERIN_SIZE_SAMPLES = 0u;
		SIZE_T BUFFERIN_SIZE_BYTES = 0u;

		SIZE_T BUFFEROUT_SIZE_FRAMES = 0u;
		SIZE_T BUFFEROUT_SIZE_SAMPLES = 0u;
		SIZE_T BUFFEROUT_SIZE_BYTES = 0u;

		SIZE_T BUFFERIN_N_SEGMENTS = 0u;

		static constexpr SIZE_T BUFFEROUT_N_SEGMENTS = 2u;

		/*
			Segment Indexes:

			bufferin_nseg_curr = index for the input buffer segment currently being loaded.
			bufferout_nseg_load = index for the output buffer segment currently being loaded.
			bufferout_nseg_play = index for the output buffer segment currently being played.
		*/

		SIZE_T bufferin_nseg_curr = 0u;
		SIZE_T bufferout_nseg_load = 0u;
		SIZE_T bufferout_nseg_play = 0u;

		VOID *p_audiobuffer = NULL;

		VOID *p_bufferinput = NULL;
		VOID *p_bufferoutput = NULL;

		/*
			pp_bufferin_segments & pp_bufferout_segments:

			An array of pointers, each pointer references a different section (segment) of p_bufferinput and p_bufferoutput.
			Each pointer points to the first sample of each buffer segment.
		*/

		VOID **pp_bufferin_segments = NULL;
		VOID **pp_bufferout_segments = NULL;

		HANDLE p_loadthread = NULL;
		HANDLE p_playthread = NULL;

		audiortdsp_fx_params_t dsp_params = {
			.n_delay = 240,
			.n_feedback = 20,
			.feedback_alt_pol = TRUE,
			.cyclediv_inc_one = TRUE
		};

		__string FILEIN_DIR = TEXT("");
		__string err_msg = TEXT("");

		SIZE_T N_CHANNELS = 0u;

		UINT32 SAMPLE_RATE = 0u;

		INT status = this->STATUS_UNINITIALIZED;

		BOOL stop_playback = FALSE;

		BOOL WINAPI filein_open(VOID);
		VOID WINAPI filein_close(VOID);

		virtual BOOL WINAPI audio_hw_init(VOID) = 0;

		VOID WINAPI audio_hw_deinit_device(VOID);
		VOID WINAPI audio_hw_deinit_all(VOID);

		VOID WINAPI stop_all_threads(VOID);

		virtual BOOL WINAPI buffer_alloc(VOID) = 0;
		virtual VOID WINAPI buffer_free(VOID) = 0;

		VOID WINAPI playback_proc(VOID);
		VOID WINAPI playback_init(VOID);
		VOID WINAPI playback_loop(VOID);

		VOID WINAPI buffer_segment_update(VOID);

		virtual VOID WINAPI buffer_load(VOID) = 0;
		virtual VOID WINAPI dsp_proc(VOID) = 0;

		VOID WINAPI buffer_play(VOID);
		VOID WINAPI audio_hw_wait(VOID);

		/*
			retrieve_previn_nframe() : Retrieve (calculates) the index for a previous frame based on the current frame index and the delay time.

			Naming:

			..._nframe: frame index.

			..._buf_nframe: frame index within the whole input buffer.
			..._seg_nframe: frame index within a specific input buffer segment.
			..._nseg: segment index within the input buffer.

			currin_... current context.
			previn_... previous (delayed) context.

			Inputs:

			currin_buf_nframe: the current frame index within the whole input buffer.
			currin_nseg: the current input buffer segment index in context.
			currin_seg_nframe: the current frame index within the segment in context.

			n_delay: delay time (number of frames).

			Outputs:

			p_previn_buf_nframe: pointer to variable that receives the previous frame index within the whole input buffer. Set to NULL if unused.
			p_previn_nseg: pointer to variable that receives the previous frame segment index. Set to NULL if unused.
			p_previn_seg_nframe: pointer to variable that receives the previous frame index within the previous segment. Set to NULL if unused.

			returns true if successful, false otherwise.
		*/

		BOOL WINAPI retrieve_previn_nframe(SIZE_T currin_buf_nframe, SIZE_T n_delay, SIZE_T *p_previn_buf_nframe, SIZE_T *p_previn_nseg, SIZE_T *p_previn_seg_nframe);
		BOOL WINAPI retrieve_previn_nframe(SIZE_T currin_nseg, SIZE_T currin_seg_nframe, SIZE_T n_delay, SIZE_T *p_previn_buf_nframe, SIZE_T *p_previn_nseg, SIZE_T *p_previn_seg_nframe);

		DWORD WINAPI loadthread_proc(VOID *p_args);
		DWORD WINAPI playthread_proc(VOID *p_args);
};

#endif /*AUDIORTDSP_HPP*/
