/*
	Audio Real-Time Delay for Windows
	Version 1.2

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "AudioRTDSP.hpp"
#include "cstrdef.h"
#include "thread.h"
#include <combaseapi.h>

AudioRTDSP::AudioRTDSP(const audiortdsp_pb_params_t *p_params)
{
	this->setPlaybackParameters(p_params);
}

BOOL WINAPI AudioRTDSP::setPlaybackParameters(const audiortdsp_pb_params_t *p_params)
{
	if(this->status > 0) return FALSE;

	this->status = this->STATUS_UNINITIALIZED;

	if(p_params == NULL) return FALSE;
	if(p_params->file_dir == NULL) return FALSE;

	this->FILEIN_DIR = p_params->file_dir;
	this->AUDIO_DATA_BEGIN = p_params->audio_data_begin;
	this->AUDIO_DATA_END = p_params->audio_data_end;
	this->SAMPLE_RATE = p_params->sample_rate;
	this->N_CHANNELS = (SIZE_T) p_params->n_channels;

	return TRUE;
}

BOOL WINAPI AudioRTDSP::initialize(VOID)
{
	if(this->status > 0) return TRUE;

	this->status = this->STATUS_UNINITIALIZED;

	if(!this->filein_open())
	{
		this->status = this->STATUS_ERROR_NOFILE;
		this->err_msg = TEXT("AudioRTDSP::initialize: Error: could not open file.");
		return FALSE;
	}

	if(!this->audio_hw_init())
	{
		this->status = this->STATUS_ERROR_AUDIOHW;
		this->filein_close();
		return FALSE;
	}

	if(!this->buffer_alloc())
	{
		this->status = this->STATUS_ERROR_MEMALLOC;
		this->err_msg = TEXT("AudioRTDSP::initialize: Error: memory allocate failed.");
		this->filein_close();
		this->audio_hw_deinit_all();
		return FALSE;
	}

	this->status = this->STATUS_READY;
	return TRUE;
}

BOOL WINAPI AudioRTDSP::runPlayback(VOID)
{
	if(this->status < 1) return FALSE;

	this->status = this->STATUS_PLAYING;
	this->playback_proc();

	this->filein_close();
	this->audio_hw_deinit_device();
	this->buffer_free();

	this->status = this->STATUS_UNINITIALIZED;
	return TRUE;
}

VOID WINAPI AudioRTDSP::stopPlayback(VOID)
{
	this->stop_playback = TRUE;
	return;
}

BOOL WINAPI AudioRTDSP::loadAudioDeviceList(HWND p_listbox)
{
	const PROPERTYKEY* const P_PKEY = (const PROPERTYKEY*) P_PKEY_Device_FriendlyName;
	IMMDevice *p_dev = NULL;
	IPropertyStore *p_devprop = NULL;
	HRESULT n_ret = 0;
	UINT devcoll_count = 0u;
	UINT n_dev = 0u;
	PROPVARIANT propvar;

	if(this->status > 0) return FALSE;

	/*If p_listbox is NULL, all functions to listbox will fail, but the audio device initialization shall continue normally.*/

	listbox_clear(p_listbox);

	this->audio_hw_deinit_all();

	n_ret = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (VOID**) &(this->p_audiodevenum));
	if(n_ret != S_OK)
	{
		this->err_msg = TEXT("AudioRTDSP::loadAudioDeviceList: Error: CoCreateInstance (IMMDeviceEnumerator) failed.");
		return FALSE;
	}

	n_ret = this->p_audiodevenum->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &(this->p_audiodevcoll));
	if(n_ret != S_OK)
	{
		this->audio_hw_deinit_all();
		this->err_msg = TEXT("AudioRTDSP::loadAudioDeviceList: Error: IMMDeviceEnumerator::EnumAudioEndpoints failed.");
		return FALSE;
	}

	n_ret = this->p_audiodevcoll->GetCount(&devcoll_count);
	if(n_ret != S_OK)
	{
		this->audio_hw_deinit_all();
		this->err_msg = TEXT("AudioRTDSP::loadAudioDeviceList: Error: IMMDeviceCollection::GetCount failed.");
		return FALSE;
	}

	for(n_dev = 0u; n_dev < devcoll_count; n_dev++)
	{
		n_ret = this->p_audiodevcoll->Item(n_dev, &p_dev);
		if(n_ret != S_OK)
		{
			this->audio_hw_deinit_all();
			this->err_msg = TEXT("AudioRTDSP::loadAudioDeviceList: Error: IMMDeviceCollection::Item failed.");
			return FALSE;
		}

		n_ret = p_dev->OpenPropertyStore(STGM_READ, &p_devprop);
		if(n_ret != S_OK)
		{
			p_dev->Release();
			this->audio_hw_deinit_all();
			this->err_msg = TEXT("AudioRTDSP::loadAudioDeviceList: Error: IMMDevice::OpenPropertyStore failed.");
			return FALSE;
		}

		PropVariantInit(&propvar);

		n_ret = p_devprop->GetValue(*P_PKEY, &propvar);
		if(n_ret != S_OK)
		{
			p_devprop->Release();
			p_dev->Release();
			this->audio_hw_deinit_all();
			this->err_msg = TEXT("AudioRTDSP::loadAudioDeviceList: Error: IPropertyStore::GetValue failed.");
			return FALSE;
		}

		if(propvar.vt == VT_EMPTY) __SPRINTF(textbuf, TEXTBUF_SIZE_CHARS, TEXT("Unknown Audio Device"));
		else cstr_wchar_to_tchar(propvar.pwszVal, textbuf, TEXTBUF_SIZE_CHARS);

		listbox_add_item(p_listbox, textbuf);

		PropVariantClear(&propvar);

		p_devprop->Release();
		p_devprop = NULL;

		p_dev->Release();
		p_dev = NULL;
	}

	PropVariantClear(&propvar);

	if(p_devprop != NULL) p_devprop->Release();
	if(p_dev != NULL) p_dev->Release();

	return TRUE;
}

BOOL WINAPI AudioRTDSP::chooseDevice(SIZE_T index)
{
	HRESULT n_ret = 0;

	if(this->status > 0) return FALSE;

	if(this->p_audiodevcoll == NULL)
	{
		this->err_msg = TEXT("AudioRTDSP::chooseDevice: Error: p_audiodevcoll is NULL.");
		return FALSE;
	}

	this->audio_hw_deinit_device();

	n_ret = this->p_audiodevcoll->Item((UINT) index, &(this->p_audiodev));
	if(n_ret != S_OK)
	{
		this->err_msg = TEXT("AudioRTDSP::chooseDevice: Error: IMMDeviceCollection::Item failed.");
		return FALSE;
	}

	return TRUE;
}

BOOL WINAPI AudioRTDSP::chooseDefaultDevice(VOID)
{
	HRESULT n_ret = 0;

	if(this->status > 0) return FALSE;

	if(this->p_audiodevenum == NULL)
	{
		this->err_msg = TEXT("AudioRTDSP::chooseDefaultDevice: Error: p_audiodevnum is NULL.");
		return FALSE;
	}

	this->audio_hw_deinit_device();

	n_ret = this->p_audiodevenum->GetDefaultAudioEndpoint(eRender, eMultimedia, &(this->p_audiodev));
	if(n_ret != S_OK)
	{
		this->err_msg = TEXT("AudioRTDSP::chooseDefaultDevice: Error: IMMDeviceEnumerator::GetDefaultAudioEndpoint failed.");
		return FALSE;
	}

	return TRUE;
}

BOOL WINAPI AudioRTDSP::getFXParams(audiortdsp_fx_params_t *p_params)
{
	if(this->status < 1) return FALSE;

	if(p_params == NULL)
	{
		this->err_msg = TEXT("AudioRTDSP::getFXParams: Error: given params object pointer is null.");
		return FALSE;
	}

	CopyMemory(p_params, &(this->dsp_params), sizeof(audiortdsp_fx_params_t));

	return TRUE;
}

BOOL WINAPI AudioRTDSP::setFXDelay(SIZE_T n_delay)
{
	if(this->status < 1) return FALSE;

	if((((SIZE_T) this->dsp_params.n_feedback + 1u)*n_delay) >= this->BUFFERIN_SIZE_FRAMES)
	{
		this->err_msg = TEXT("AudioRTDSP::setFXDelay: Error: given delay time value is too big.");
		return FALSE;
	}

	this->dsp_params.n_delay = (INT32) n_delay;
	return TRUE;
}

BOOL WINAPI AudioRTDSP::setFXFeedback(SIZE_T n_feedback)
{
	if(this->status < 1) return FALSE;

	if((((SIZE_T) this->dsp_params.n_delay)*(n_feedback + 1u)) >= this->BUFFERIN_SIZE_FRAMES)
	{
		this->err_msg = TEXT("AudioRTDSP::setFXFeedback: Error: given feedback count is too big.");
		return FALSE;
	}

	this->dsp_params.n_feedback = (INT32) n_feedback;
	return TRUE;
}

BOOL WINAPI AudioRTDSP::enableFeedbackAltPol(BOOL enable)
{
	if(this->status < 1) return FALSE;

	this->dsp_params.feedback_alt_pol = enable;
	return TRUE;
}

BOOL WINAPI AudioRTDSP::enableCycleDivIncOne(BOOL enable)
{
	if(this->status < 1) return FALSE;

	this->dsp_params.cyclediv_inc_one = enable;
	return TRUE;
}

__string WINAPI AudioRTDSP::getLastErrorMessage(VOID)
{
	if(this->status == this->STATUS_UNINITIALIZED)
		return TEXT("Error: Audio Object has not been initialized.\r\nExtended error message: ") + this->err_msg;

	return this->err_msg;
}

BOOL WINAPI AudioRTDSP::filein_open(VOID)
{
	this->filein_close();

	this->h_filein = CreateFile(this->FILEIN_DIR.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0u, NULL);
	if(this->h_filein == INVALID_HANDLE_VALUE) return FALSE;

	this->filein_size_64.l32 = (ULONG32) GetFileSize(this->h_filein, (DWORD*) &(this->filein_size_64.h32));
	return TRUE;
}

VOID WINAPI AudioRTDSP::filein_close(VOID)
{
	if(this->h_filein == INVALID_HANDLE_VALUE) return;

	CloseHandle(this->h_filein);
	this->h_filein = INVALID_HANDLE_VALUE;
	*((ULONG64*) &(this->filein_size_64)) = 0u;

	return;
}

VOID WINAPI AudioRTDSP::audio_hw_deinit_device(VOID)
{
	if(this->p_audiomgr != NULL) this->p_audiomgr->Stop();

	if(this->p_audioout != NULL)
	{
		this->p_audioout->Release();
		this->p_audioout = NULL;
	}

	if(this->p_audiomgr != NULL)
	{
		this->p_audiomgr->Release();
		this->p_audiomgr = NULL;
	}

	if(this->p_audiodev != NULL)
	{
		this->p_audiodev->Release();
		this->p_audiodev = NULL;
	}

	return;
}

VOID WINAPI AudioRTDSP::audio_hw_deinit_all(VOID)
{
	this->audio_hw_deinit_device();

	if(this->p_audiodevcoll != NULL)
	{
		this->p_audiodevcoll->Release();
		this->p_audiodevcoll = NULL;
	}

	if(this->p_audiodevenum != NULL)
	{
		this->p_audiodevenum->Release();
		this->p_audiodevenum = NULL;
	}

	return;
}

VOID WINAPI AudioRTDSP::stop_all_threads(VOID)
{
	thread_stop(&(this->p_loadthread), 0u);
	thread_stop(&(this->p_playthread), 0u);

	return;
}

VOID WINAPI AudioRTDSP::playback_proc(VOID)
{
	this->playback_init();
	this->playback_loop();

	return;
}

VOID WINAPI AudioRTDSP::playback_init(VOID)
{
	HRESULT n_ret = 0;

	*((ULONG64*) &(this->filein_pos_64)) = this->AUDIO_DATA_BEGIN;
	this->stop_playback = FALSE;

	this->bufferout_nseg_load = 0u;
	this->bufferout_nseg_play = 1u;

	this->bufferin_nseg_curr = 0u;

	/*Default FX Initialization*/

	this->setFXDelay(240u);
	this->setFXFeedback(20u);
	this->enableFeedbackAltPol(TRUE);
	this->enableCycleDivIncOne(TRUE);

	n_ret = this->p_audioout->GetBuffer((UINT32) this->AUDIOBUFFER_SIZE_FRAMES, (BYTE**) &(this->p_audiobuffer));
	if(n_ret != S_OK) app_exit(1u, TEXT("AudioRTDSP::playback_init: Error: IAudioRenderClient::GetBuffer failed."));

	ZeroMemory(this->p_audiobuffer, this->AUDIOBUFFER_SIZE_BYTES);

	n_ret = this->p_audioout->ReleaseBuffer((UINT32) this->AUDIOBUFFER_SIZE_FRAMES, 0u);
	if(n_ret != S_OK) app_exit(1u, TEXT("AudioRTDSP::playback_init: Error: IAudioRenderClient::ReleaseBuffer failed."));

	n_ret = this->p_audiomgr->Start();
	if(n_ret != S_OK) app_exit(1u, TEXT("AudioRTDSP::playback_init: Error: IAudioClient::Start failed."));

	this->buffer_load();
	this->dsp_proc();
	this->audio_hw_wait();

	this->buffer_segment_update();
	return;
}

VOID WINAPI AudioRTDSP::playback_loop(VOID)
{
	while(!this->stop_playback)
	{
		this->p_playthread = thread_create_default(((DWORD (WINAPI *)(VOID*)) &AudioRTDSP::playthread_proc), this, NULL);
		this->p_loadthread = thread_create_default(((DWORD (WINAPI *)(VOID*)) &AudioRTDSP::loadthread_proc), this, NULL);
		thread_wait(&(this->p_loadthread));
		thread_wait(&(this->p_playthread));

		this->buffer_segment_update();
	}

	return;
}

VOID WINAPI AudioRTDSP::buffer_segment_update(VOID)
{
	this->bufferin_nseg_curr++;
	this->bufferin_nseg_curr %= this->BUFFERIN_N_SEGMENTS;

	this->bufferout_nseg_load++;
	this->bufferout_nseg_load %= this->BUFFEROUT_N_SEGMENTS;

	this->bufferout_nseg_play++;
	this->bufferout_nseg_play %= this->BUFFEROUT_N_SEGMENTS;

	return;
}

VOID WINAPI AudioRTDSP::buffer_play(VOID)
{
	HRESULT n_ret = 0;

	n_ret = this->p_audioout->GetBuffer((UINT32) this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES, (BYTE**) &(this->p_audiobuffer));
	if(n_ret != S_OK) app_exit(1u, TEXT("AudioRTDSP::buffer_play: Error: IAudioRenderClient::GetBuffer failed."));

	CopyMemory(this->p_audiobuffer, this->pp_bufferout_segments[this->bufferout_nseg_play], this->AUDIOBUFFER_SEGMENT_SIZE_BYTES);

	n_ret = this->p_audioout->ReleaseBuffer((UINT32) this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES, 0u);
	if(n_ret != S_OK) app_exit(1u, TEXT("AudioRTDSP::buffer_play: Error: IAudioRenderClient::ReleaseBuffer failed."));

	return;
}

VOID WINAPI AudioRTDSP::audio_hw_wait(VOID)
{
	SIZE_T n_frames_free = 0u;
	HRESULT n_ret = 0;
	UINT32 u32 = 0u;

	do{
		n_ret = this->p_audiomgr->GetCurrentPadding(&u32);
		if(n_ret != S_OK) app_exit(1u, TEXT("AudioRTDSP::audio_hw_wait: Error: IAudioClient::GetCurrentPadding failed."));

		n_frames_free = this->AUDIOBUFFER_SIZE_FRAMES - ((SIZE_T) u32);

		Sleep(1u);
	}while(n_frames_free < this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES);

	return;
}

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

BOOL WINAPI AudioRTDSP::retrieve_previn_nframe(SIZE_T currin_buf_nframe, SIZE_T n_delay, SIZE_T *p_previn_buf_nframe, SIZE_T *p_previn_nseg, SIZE_T *p_previn_seg_nframe)
{
	SIZE_T previn_buf_nframe = 0u;
	SIZE_T previn_nseg = 0u;
	SIZE_T previn_seg_nframe = 0u;

	if(currin_buf_nframe >= this->BUFFERIN_SIZE_FRAMES) return FALSE;
	if(n_delay >= this->BUFFERIN_SIZE_FRAMES) return FALSE;

	if(n_delay > currin_buf_nframe) previn_buf_nframe = this->BUFFERIN_SIZE_FRAMES - (n_delay - currin_buf_nframe);
	else previn_buf_nframe = currin_buf_nframe - n_delay;

	previn_nseg = previn_buf_nframe/this->BUFFER_SEGMENT_SIZE_FRAMES;
	previn_seg_nframe = previn_buf_nframe%this->BUFFER_SEGMENT_SIZE_FRAMES;

	if(p_previn_buf_nframe != NULL) *p_previn_buf_nframe = previn_buf_nframe;
	if(p_previn_nseg != NULL) *p_previn_nseg = previn_nseg;
	if(p_previn_seg_nframe != NULL) *p_previn_seg_nframe = previn_seg_nframe;

	return TRUE;
}

BOOL WINAPI AudioRTDSP::retrieve_previn_nframe(SIZE_T currin_nseg, SIZE_T currin_seg_nframe, SIZE_T n_delay, SIZE_T *p_previn_buf_nframe, SIZE_T *p_previn_nseg, SIZE_T *p_previn_seg_nframe)
{
	SIZE_T currin_buf_nframe = 0u;

	if(currin_nseg >= this->BUFFERIN_N_SEGMENTS) return FALSE;
	if(currin_seg_nframe >= this->BUFFER_SEGMENT_SIZE_FRAMES) return FALSE;

	currin_buf_nframe = currin_nseg*(this->BUFFER_SEGMENT_SIZE_FRAMES) + currin_seg_nframe;

	return this->retrieve_previn_nframe(currin_buf_nframe, n_delay, p_previn_buf_nframe, p_previn_nseg, p_previn_seg_nframe);
}

DWORD WINAPI AudioRTDSP::loadthread_proc(VOID *p_args)
{
	this->buffer_load();
	if(this->stop_playback) return 0u;

	this->dsp_proc();
	return 0u;
}

DWORD WINAPI AudioRTDSP::playthread_proc(VOID *p_args)
{
	this->buffer_play();
	this->audio_hw_wait();

	return 0u;
}
