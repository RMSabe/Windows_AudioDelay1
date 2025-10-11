/*
	Audio Real-Time Delay for Windows
	Version 1.2

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "AudioRTDSP_i24.hpp"
#include <combaseapi.h>

AudioRTDSP_i24::AudioRTDSP_i24(const audiortdsp_pb_params_t *p_params) : AudioRTDSP(p_params)
{
}

AudioRTDSP_i24::~AudioRTDSP_i24(VOID)
{
	this->stop_all_threads();
	this->status = this->STATUS_UNINITIALIZED;

	this->filein_close();
	this->audio_hw_deinit_all();
	this->buffer_free();
}

BOOL WINAPI AudioRTDSP_i24::audio_hw_init(VOID)
{
	SIZE_T n_channel = 0u;
	DWORD channel_mask = 0u;

	HRESULT n_ret;
	UINT32 u32;
	WAVEFORMATEXTENSIBLE wavfmt;

	if(this->p_audiodev == NULL)
	{
		this->err_msg = TEXT("AudioRTDSP_i24::audio_hw_init: Error: p_audiodev is NULL.");
		return FALSE;
	}

	n_ret = this->p_audiodev->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (VOID**) &(this->p_audiomgr));
	if(n_ret != S_OK)
	{
		this->audio_hw_deinit_device();
		this->err_msg = TEXT("AudioRTDSP_i24::audio_hw_init: Error: IMMDevice::Activate failed.");
		return FALSE;
	}

	channel_mask = 0u;
	for(n_channel = 0u; n_channel < this->N_CHANNELS; n_channel++) channel_mask |= (1 << n_channel);

	ZeroMemory(&wavfmt, sizeof(WAVEFORMATEXTENSIBLE));

	wavfmt.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	wavfmt.Format.nChannels = (WORD) this->N_CHANNELS;
	wavfmt.Format.wBitsPerSample = 32u;
	wavfmt.Format.nBlockAlign = (wavfmt.Format.nChannels)*4u;
	wavfmt.Format.nSamplesPerSec = (DWORD) this->SAMPLE_RATE;
	wavfmt.Format.nAvgBytesPerSec = ((DWORD) this->SAMPLE_RATE)*((DWORD) wavfmt.Format.nBlockAlign);
	wavfmt.Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
	wavfmt.Samples.wValidBitsPerSample = 24u;
	wavfmt.dwChannelMask = channel_mask;
	wavfmt.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;

	n_ret = this->p_audiomgr->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, (WAVEFORMATEX*) &wavfmt, NULL);
	if(n_ret != S_OK)
	{
		this->audio_hw_deinit_device();
		this->err_msg = TEXT("AudioRTDSP_i24::audio_hw_init: Error: audio format is not supported.");
		return FALSE;
	}

	n_ret = this->p_audiomgr->Initialize(AUDCLNT_SHAREMODE_EXCLUSIVE, 0, 10000000, 0, (WAVEFORMATEX*) &wavfmt, NULL);
	if(n_ret != S_OK)
	{
		this->audio_hw_deinit_device();
		this->err_msg = TEXT("AudioRTDSP_i24::audio_hw_init: Error: IAudioClient::Initialize failed.");
		return FALSE;
	}

	n_ret = this->p_audiomgr->GetBufferSize(&u32);
	if(n_ret != S_OK)
	{
		this->audio_hw_deinit_device();
		this->err_msg = TEXT("AudioRTDSP_i24::audio_hw_init: Error: IAudioClient::GetBufferSize failed.");
		return FALSE;
	}

	n_ret = this->p_audiomgr->GetService(__uuidof(IAudioRenderClient), (VOID**) &(this->p_audioout));
	if(n_ret != S_OK)
	{
		this->audio_hw_deinit_device();
		this->err_msg = TEXT("AudioRTDSP_i24::audio_hw_init: Error: IAudioClient::GetService failed.");
		return FALSE;
	}

	this->AUDIOBUFFER_SIZE_FRAMES = (SIZE_T) u32;
	this->AUDIOBUFFER_SIZE_SAMPLES = (this->AUDIOBUFFER_SIZE_FRAMES)*(this->N_CHANNELS);
	this->AUDIOBUFFER_SIZE_BYTES = this->AUDIOBUFFER_SIZE_SAMPLES*4u;

	this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES = _get_closest_power2_ceil(this->AUDIOBUFFER_SIZE_FRAMES/2u);
	this->AUDIOBUFFER_SEGMENT_SIZE_SAMPLES = (this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES)*(this->N_CHANNELS);
	this->AUDIOBUFFER_SEGMENT_SIZE_BYTES = this->AUDIOBUFFER_SEGMENT_SIZE_SAMPLES*4u;

	this->BUFFER_SEGMENT_SIZE_FRAMES = this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES;
	this->BUFFER_SEGMENT_SIZE_SAMPLES = (this->BUFFER_SEGMENT_SIZE_FRAMES)*(this->N_CHANNELS);
	this->BUFFER_SEGMENT_SIZE_BYTES = this->BUFFER_SEGMENT_SIZE_SAMPLES*4u;

	this->BUFFEROUT_SIZE_FRAMES = (this->BUFFER_SEGMENT_SIZE_FRAMES)*(this->BUFFEROUT_N_SEGMENTS);
	this->BUFFEROUT_SIZE_SAMPLES = (this->BUFFEROUT_SIZE_FRAMES)*(this->N_CHANNELS);
	this->BUFFEROUT_SIZE_BYTES = this->BUFFEROUT_SIZE_SAMPLES*4u;

	this->BUFFERIN_N_SEGMENTS = this->BUFFERIN_SIZE_FRAMES/this->BUFFER_SEGMENT_SIZE_FRAMES;

	this->BUFFERIN_SIZE_SAMPLES = (this->BUFFERIN_SIZE_FRAMES)*(this->N_CHANNELS);
	this->BUFFERIN_SIZE_BYTES = this->BUFFERIN_SIZE_SAMPLES*4u;

	this->BYTEBUF_SIZE = this->BUFFER_SEGMENT_SIZE_SAMPLES*3u;

	return TRUE;
}

BOOL WINAPI AudioRTDSP_i24::buffer_alloc(VOID)
{
	SIZE_T n_seg = 0u;

	this->buffer_free();

	this->p_bufferinput = HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, this->BUFFERIN_SIZE_BYTES);
	this->p_bufferoutput = HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, this->BUFFEROUT_SIZE_BYTES);

	this->pp_bufferin_segments = (VOID**) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, (this->BUFFERIN_N_SEGMENTS*sizeof(VOID*)));
	this->pp_bufferout_segments = (VOID**) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, (this->BUFFEROUT_N_SEGMENTS*sizeof(VOID*)));

	this->p_bytebuf = (UINT8*) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, this->BYTEBUF_SIZE);

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

	if(this->p_bytebuf == NULL)
	{
		this->buffer_free();
		return FALSE;
	}

	for(n_seg = 0u; n_seg < this->BUFFERIN_N_SEGMENTS; n_seg++) this->pp_bufferin_segments[n_seg] = (VOID*) (((SIZE_T) this->p_bufferinput) + n_seg*(this->BUFFER_SEGMENT_SIZE_BYTES));

	for(n_seg = 0u; n_seg < this->BUFFEROUT_N_SEGMENTS; n_seg++) this->pp_bufferout_segments[n_seg] = (VOID*) (((SIZE_T) this->p_bufferoutput) + n_seg*(this->BUFFER_SEGMENT_SIZE_BYTES));

	return TRUE;
}

VOID WINAPI AudioRTDSP_i24::buffer_free(VOID)
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

	if(this->p_bytebuf != NULL)
	{
		HeapFree(p_processheap, 0u, this->p_bytebuf);
		this->p_bytebuf = NULL;
	}

	return;
}

VOID WINAPI AudioRTDSP_i24::buffer_load(VOID)
{
	INT32 *p_currin_seg = NULL;

	SIZE_T n_sample = 0u;
	SIZE_T n_byte = 0u;
	INT32 sample = 0;

	DWORD dummy_32;

	if(*((ULONG64*) &(this->filein_pos_64)) >= this->AUDIO_DATA_END)
	{
		this->stop_playback = TRUE;
		return;
	}

	ZeroMemory(this->p_bytebuf, this->BYTEBUF_SIZE);

	SetFilePointer(this->h_filein, (LONG) this->filein_pos_64.l32, (LONG*) &(this->filein_pos_64.h32), FILE_BEGIN);
	ReadFile(this->h_filein, this->p_bytebuf, (DWORD) this->BYTEBUF_SIZE, &dummy_32, NULL);
	*((ULONG64*) &(this->filein_pos_64)) += (ULONG64) this->BYTEBUF_SIZE;

	p_currin_seg = (INT32*) this->pp_bufferin_segments[this->bufferin_nseg_curr];

	n_byte = 0u;
	for(n_sample = 0u; n_sample < this->BUFFER_SEGMENT_SIZE_SAMPLES; n_sample++)
	{
		sample = ((this->p_bytebuf[n_byte + 2u] << 16) | (this->p_bytebuf[n_byte + 1u] << 8) | (this->p_bytebuf[n_byte]));

		if(sample & 0x00800000) sample |= 0xff800000;
		else sample &= 0x007fffff; /*Not really necessary, but just to be safe.*/

		p_currin_seg[n_sample] = sample;

		n_byte += 3u;
	}

	return;
}

VOID WINAPI AudioRTDSP_i24::dsp_proc(VOID)
{
	INT32 *p_currin_seg = NULL;
	INT32 *p_loadout_seg = NULL;

	INT32 *p_bufferin = NULL;

	SIZE_T currin_seg_nframe = 0u;
	SIZE_T previn_buf_nframe = 0u;

	SIZE_T n_currsample = 0u;
	SIZE_T n_prevsample = 0u;
	SIZE_T n_channel = 0u;

	INT32 n_cycle = 0;
	INT32 n_delay = 0;
	INT32 cycle_div = 0;
	INT32 pol = 0;

	audiortdsp_fx_params_t fx_params;

	p_currin_seg = (INT32*) this->pp_bufferin_segments[this->bufferin_nseg_curr];
	p_loadout_seg = (INT32*) this->pp_bufferout_segments[this->bufferout_nseg_load];
	p_bufferin = (INT32*) this->p_bufferinput;

	CopyMemory(&fx_params, &(this->dsp_params), sizeof(audiortdsp_fx_params_t));

	fx_params.n_feedback++;

	for(currin_seg_nframe = 0u; currin_seg_nframe < this->BUFFER_SEGMENT_SIZE_FRAMES; currin_seg_nframe++)
	{
		n_currsample = currin_seg_nframe*(this->N_CHANNELS);
		for(n_channel = 0u; n_channel < this->N_CHANNELS; n_channel++)
		{
			p_loadout_seg[n_currsample] = p_currin_seg[n_currsample];
			n_currsample++;
		}

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

			n_currsample = currin_seg_nframe*(this->N_CHANNELS);
			n_prevsample = previn_buf_nframe*(this->N_CHANNELS);
			for(n_channel = 0u; n_channel < this->N_CHANNELS; n_channel++)
			{
				p_loadout_seg[n_currsample] += pol*(p_bufferin[n_prevsample])/cycle_div;

				n_currsample++;
				n_prevsample++;
			}

			n_cycle++;
		}

		n_currsample = currin_seg_nframe*(this->N_CHANNELS);
		for(n_channel = 0u; n_channel < this->N_CHANNELS; n_channel++)
		{
			p_loadout_seg[n_currsample] /= 2;

			if(p_loadout_seg[n_currsample] > this->SAMPLE_MAX_VALUE) p_loadout_seg[n_currsample] = this->SAMPLE_MAX_VALUE;
			else if(p_loadout_seg[n_currsample] < this->SAMPLE_MIN_VALUE) p_loadout_seg[n_currsample] = this->SAMPLE_MIN_VALUE;

			p_loadout_seg[n_currsample] = ((p_loadout_seg[n_currsample]) << 8);

			n_currsample++;
		}
	}

	return;
}
