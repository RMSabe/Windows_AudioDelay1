/*
	Audio Real-Time Delay for Windows
	Version 1.0

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "AudioRTDSP_i16_1ch.hpp"
#include <combaseapi.h>

AudioRTDSP_i16_1ch::AudioRTDSP_i16_1ch(const audiortdsp_pb_params_t *p_params) : AudioRTDSP(p_params)
{
}

AudioRTDSP_i16_1ch::~AudioRTDSP_i16_1ch(VOID)
{
	this->stop_all_threads();
	this->status = this->STATUS_UNINITIALIZED;

	this->filein_close();
	this->audio_hw_deinit_all();
	this->buffer_free();
}

BOOL WINAPI AudioRTDSP_i16_1ch::audio_hw_init(VOID)
{
	HRESULT n_ret;
	UINT32 u32;
	WAVEFORMATEX wavfmt;

	if(this->p_audiodev == NULL)
	{
		this->err_msg = TEXT("AudioRTDSP_i16_1ch::audio_hw_init: Error: p_audiodev is NULL.");
		return FALSE;
	}

	n_ret = this->p_audiodev->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (VOID**) &(this->p_audiomgr));
	if(n_ret != S_OK)
	{
		this->audio_hw_deinit_device();
		this->err_msg = TEXT("AudioRTDSP_i16_1ch::audio_hw_init: Error: IMMDevice::Activate failed.");
		return FALSE;
	}

	wavfmt.wFormatTag = WAVE_FORMAT_PCM;
	wavfmt.nChannels = 2u;
	wavfmt.nSamplesPerSec = this->SAMPLE_RATE;
	wavfmt.nAvgBytesPerSec = this->SAMPLE_RATE*4u;
	wavfmt.nBlockAlign = 4u;
	wavfmt.wBitsPerSample = 16u;
	wavfmt.cbSize = 0u;

	n_ret = this->p_audiomgr->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, &wavfmt, NULL);
	if(n_ret != S_OK)
	{
		this->audio_hw_deinit_device();
		this->err_msg = TEXT("AudioRTDSP_i16_1ch::audio_hw_init: Error: audio format is not supported.");
		return FALSE;
	}

	n_ret = this->p_audiomgr->Initialize(AUDCLNT_SHAREMODE_EXCLUSIVE, 0, 10000000, 0, &wavfmt, NULL);
	if(n_ret != S_OK)
	{
		this->audio_hw_deinit_device();
		this->err_msg = TEXT("AudioRTDSP_i16_1ch::audio_hw_init: Error: IAudioClient::Initialize failed.");
		return FALSE;
	}

	n_ret = this->p_audiomgr->GetBufferSize(&u32);
	if(n_ret != S_OK)
	{
		this->audio_hw_deinit_device();
		this->err_msg = TEXT("AudioRTDSP_i16_1ch::audio_hw_init: Error: IAudioClient::GetBufferSize failed.");
		return FALSE;
	}

	n_ret = this->p_audiomgr->GetService(__uuidof(IAudioRenderClient), (VOID**) &(this->p_audioout));
	if(n_ret != S_OK)
	{
		this->audio_hw_deinit_device();
		this->err_msg = TEXT("AudioRTDSP_i16_1ch::audio_hw_init: Error: IAudioClient::GetService failed.");
		return FALSE;
	}

	this->AUDIOBUFFER_SIZE_FRAMES = (SIZE_T) u32;
	this->AUDIOBUFFER_SIZE_SAMPLES = this->AUDIOBUFFER_SIZE_FRAMES*2u;
	this->AUDIOBUFFER_SIZE_BYTES = this->AUDIOBUFFER_SIZE_SAMPLES*2u;

	this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES = _get_closest_power2_ceil(this->AUDIOBUFFER_SIZE_FRAMES/2u);
	this->AUDIOBUFFER_SEGMENT_SIZE_SAMPLES = this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES*2u;
	this->AUDIOBUFFER_SEGMENT_SIZE_BYTES = this->AUDIOBUFFER_SEGMENT_SIZE_SAMPLES*2u;

	this->BUFFER_SEGMENT_SIZE_FRAMES = this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES;
	this->BUFFER_SEGMENT_SIZE_SAMPLES = this->BUFFER_SEGMENT_SIZE_FRAMES;
	this->BUFFER_SEGMENT_SIZE_BYTES = this->BUFFER_SEGMENT_SIZE_SAMPLES*2u;

	this->BUFFEROUT_SIZE_FRAMES = (this->BUFFER_SEGMENT_SIZE_FRAMES)*(this->BUFFEROUT_N_SEGMENTS);
	this->BUFFEROUT_SIZE_SAMPLES = this->BUFFEROUT_SIZE_FRAMES;
	this->BUFFEROUT_SIZE_BYTES = this->BUFFEROUT_SIZE_SAMPLES*2u;

	this->BUFFERIN_N_SEGMENTS = this->BUFFERIN_SIZE_FRAMES/this->BUFFER_SEGMENT_SIZE_FRAMES;

	this->BUFFERIN_SIZE_SAMPLES = this->BUFFERIN_SIZE_FRAMES;
	this->BUFFERIN_SIZE_BYTES = this->BUFFERIN_SIZE_SAMPLES*2u;

	return TRUE;
}

BOOL WINAPI AudioRTDSP_i16_1ch::buffer_alloc(VOID)
{
	SIZE_T n_seg = 0u;

	this->buffer_free();

	this->p_bufferinput = HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, this->BUFFERIN_SIZE_BYTES);
	this->p_bufferoutput = HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, this->BUFFEROUT_SIZE_BYTES);

	this->pp_bufferin_segments = (VOID**) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, (this->BUFFERIN_N_SEGMENTS*sizeof(VOID*)));
	this->pp_bufferout_segments = (VOID**) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, (this->BUFFEROUT_N_SEGMENTS*sizeof(VOID*)));

	if(this->p_bufferinput == NULL)
	{
		this->buffer_free();
		return FALSE;
	}

	if(this->p_bufferoutput == NULL)
	{
		this->buffer_free();
		return FALSE;
	}

	if(this->pp_bufferin_segments == NULL)
	{
		this->buffer_free();
		return FALSE;
	}

	if(this->pp_bufferout_segments == NULL)
	{
		this->buffer_free();
		return FALSE;
	}

	for(n_seg = 0u; n_seg < this->BUFFERIN_N_SEGMENTS; n_seg++) this->pp_bufferin_segments[n_seg] = &((UINT8*) (this->p_bufferinput))[n_seg*(this->BUFFER_SEGMENT_SIZE_BYTES)];

	for(n_seg = 0u; n_seg < this->BUFFEROUT_N_SEGMENTS; n_seg++) this->pp_bufferout_segments[n_seg] = &((UINT8*) (this->p_bufferoutput))[n_seg*(this->BUFFER_SEGMENT_SIZE_BYTES)];

	return TRUE;
}

VOID WINAPI AudioRTDSP_i16_1ch::buffer_free(VOID)
{
	if(this->p_bufferinput != NULL)
	{
		HeapFree(p_processheap, 0u, this->p_bufferinput);
		this->p_bufferinput = NULL;
	}

	if(this->p_bufferoutput != NULL)
	{
		HeapFree(p_processheap, 0u, this->p_bufferoutput);
		this->p_bufferoutput = NULL;
	}

	if(this->pp_bufferin_segments != NULL)
	{
		HeapFree(p_processheap, 0u, this->pp_bufferin_segments);
		this->pp_bufferin_segments = NULL;
	}

	if(this->pp_bufferout_segments != NULL)
	{
		HeapFree(p_processheap, 0u, this->pp_bufferout_segments);
		this->pp_bufferout_segments = NULL;
	}

	return;
}

VOID WINAPI AudioRTDSP_i16_1ch::buffer_load(VOID)
{
	DWORD dummy_32;

	if(*((ULONG64*) &(this->filein_pos_64)) >= this->AUDIO_DATA_END)
	{
		this->stop_playback = TRUE;
		return;
	}

	ZeroMemory(this->pp_bufferin_segments[this->bufferin_nseg_curr], this->BUFFER_SEGMENT_SIZE_BYTES);

	SetFilePointer(this->h_filein, (LONG) this->filein_pos_64.l32, (LONG*) &(this->filein_pos_64.h32), FILE_BEGIN);
	ReadFile(this->h_filein, this->pp_bufferin_segments[this->bufferin_nseg_curr], (DWORD) this->BUFFER_SEGMENT_SIZE_BYTES, &dummy_32, NULL);
	*((ULONG64*) &(this->filein_pos_64)) += (ULONG64) this->BUFFER_SEGMENT_SIZE_BYTES;

	return;
}

VOID WINAPI AudioRTDSP_i16_1ch::buffer_play(VOID)
{
	SIZE_T n_frame = 0u;
	HRESULT n_ret = 0;

	n_ret = this->p_audioout->GetBuffer((UINT32) this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES, (BYTE**) &(this->p_audiobuffer));
	if(n_ret != S_OK) app_exit(1u, TEXT("AudioRTDSP_i16_1ch::buffer_play: Error: IAudioRenderClient::GetBuffer failed."));

	for(n_frame = 0u; n_frame < this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES; n_frame++)
	{
		((INT16*) (this->p_audiobuffer))[2u*n_frame] = ((INT16*) (this->pp_bufferout_segments[this->bufferout_nseg_play]))[n_frame];
		((INT16*) (this->p_audiobuffer))[2u*n_frame + 1u] = ((INT16*) (this->p_audiobuffer))[2u*n_frame];
	}

	n_ret = this->p_audioout->ReleaseBuffer((UINT32) this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES, 0u);
	if(n_ret != S_OK) app_exit(1u, TEXT("AudioRTDSP_i16_1ch::buffer_play: Error: IAudioRenderClient::ReleaseBuffer failed."));

	return;
}

VOID WINAPI AudioRTDSP_i16_1ch::dsp_proc(VOID)
{
	INT16 *p_currin_seg = NULL;
	INT16 *p_loadout_seg = NULL;

	INT16 *p_bufferin = NULL;

	SIZE_T currin_seg_nframe = 0u;
	SIZE_T previn_buf_nframe = 0u;

	INT32 sample_val = 0;

	INT32 n_cycle = 0;
	INT32 n_delay = 0;
	INT32 cycle_div = 0;
	INT32 pol = 0;

	audiortdsp_fx_params_t fx_params;

	p_currin_seg = (INT16*) this->pp_bufferin_segments[this->bufferin_nseg_curr];
	p_loadout_seg = (INT16*) this->pp_bufferout_segments[this->bufferout_nseg_load];
	p_bufferin = (INT16*) this->p_bufferinput;

	CopyMemory(&fx_params, &(this->dsp_params), sizeof(audiortdsp_fx_params_t));

	fx_params.n_feedback++;

	for(currin_seg_nframe = 0u; currin_seg_nframe < this->BUFFER_SEGMENT_SIZE_FRAMES; currin_seg_nframe++)
	{
		sample_val = (INT32) p_currin_seg[currin_seg_nframe];

		pol = 1;
		n_cycle = 1;

		while(n_cycle <= fx_params.n_feedback)
		{
			if(fx_params.feedback_alt_pol) pol ^= 0xfffffffe; /*Toggle between -1 and 1*/

			if(fx_params.cyclediv_inc_one) cycle_div = n_cycle + 1;
			else cycle_div = (1 << n_cycle);

			if(!cycle_div) break; /*Stop if cycle_div == 0*/

			n_delay = n_cycle*(fx_params.n_delay);

			this->retrieve_previn_nframe(this->bufferin_nseg_curr, currin_seg_nframe, (SIZE_T) n_delay, &previn_buf_nframe, NULL, NULL);

			sample_val += pol*((INT32) p_bufferin[previn_buf_nframe])/cycle_div;

			n_cycle++;
		}

		sample_val /= 2;

		if(sample_val > this->SAMPLE_MAX_VALUE) p_loadout_seg[currin_seg_nframe] = (INT16) this->SAMPLE_MAX_VALUE;
		else if(sample_val < this->SAMPLE_MIN_VALUE) p_loadout_seg[currin_seg_nframe] = (INT16) this->SAMPLE_MIN_VALUE;
		else p_loadout_seg[currin_seg_nframe] = (INT16) sample_val;
	}

	return;
}
