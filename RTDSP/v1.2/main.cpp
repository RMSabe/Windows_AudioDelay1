/*
	Audio Real-Time Delay for Windows
	Version 1.2

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "globldef.h"
#include "cstrdef.h"
#include "thread.h"
#include "strdef.hpp"

#include "shared.hpp"

#include <combaseapi.h>

#include "AudioRTDSP.hpp"
#include "AudioRTDSP_i16.hpp"
#include "AudioRTDSP_i24.hpp"

#define CUSTOM_GENERIC_WNDCLASS_NAME TEXT("__CUSTOMGENERICWNDCLASS__")

#define CUSTOM_WM_PLAYBACK_FINISHED (WM_USER | 1U)

#define RUNTIME_STATUS_INIT 0
#define RUNTIME_STATUS_IDLE 1
#define RUNTIME_STATUS_CHOOSEFILE 2
#define RUNTIME_STATUS_CHOOSEAUDIODEV 3
#define RUNTIME_STATUS_PLAYBACK_RUNNING 4
#define RUNTIME_STATUS_PLAYBACK_FINISHED 5

#define CUSTOMCOLOR_BLACK 0x00000000
#define CUSTOMCOLOR_WHITE 0x00ffffff
#define CUSTOMCOLOR_LTGRAY 0x00c0c0c0

#define BRUSHINDEX_TRANSPARENT 0U
#define BRUSHINDEX_CUSTOM_SOLID_BLACK 1U
#define BRUSHINDEX_CUSTOM_SOLID_WHITE 2U
#define BRUSHINDEX_CUSTOM_SOLID_LTGRAY 3U

#define PP_BRUSH_LENGTH 4U
#define PP_BRUSH_SIZE (PP_BRUSH_LENGTH*sizeof(HBRUSH))

#define CUSTOMFONT_NORMAL_CHARSET DEFAULT_CHARSET
#define CUSTOMFONT_NORMAL_WIDTH 8U
#define CUSTOMFONT_NORMAL_HEIGHT 16U
#define CUSTOMFONT_NORMAL_WEIGHT FW_NORMAL

#define CUSTOMFONT_PARAMS_CHARSET DEFAULT_CHARSET
#define CUSTOMFONT_PARAMS_WIDTH 16U
#define CUSTOMFONT_PARAMS_HEIGHT 24U
#define CUSTOMFONT_PARAMS_WEIGHT FW_NORMAL

#define CUSTOMFONT_LARGE_CHARSET DEFAULT_CHARSET
#define CUSTOMFONT_LARGE_WIDTH 20U
#define CUSTOMFONT_LARGE_HEIGHT 35U
#define CUSTOMFONT_LARGE_WEIGHT FW_NORMAL

#define FONTINDEX_CUSTOM_NORMAL 0U
#define FONTINDEX_CUSTOM_PARAMS 1U
#define FONTINDEX_CUSTOM_LARGE 2U

#define PP_FONT_LENGTH 3U
#define PP_FONT_SIZE (PP_FONT_LENGTH*sizeof(HFONT))

/*
	During Playback, children window (except text1 and button1) are organized in containers:

	text1 = header, button1 = foot.

	containers:

	container1 {
		(set delay time)
		btngroupbox1
		textbox1
		button2
	};

	container2 {
		(set feedback count)
		btngroupbox2
		textbox2
		button3
	};

	container3 {
		(alternate feedback polarity)
		btngroupbox3
		checkbox1
	};

	container4 {
		(set cycle divider increment)
		btngroupbox4
		radiobutton1
		radiobutton2
	};

	container5 {
		(current fx parameters)
		text2
	};
*/

#define CHILDWNDINDEX_TEXT1 0U
#define CHILDWNDINDEX_TEXT2 1U
#define CHILDWNDINDEX_BUTTON1 2U
#define CHILDWNDINDEX_BUTTON2 3U
#define CHILDWNDINDEX_BUTTON3 4U
#define CHILDWNDINDEX_TEXTBOX1 5U
#define CHILDWNDINDEX_TEXTBOX2 6U
#define CHILDWNDINDEX_CHECKBOX1 7U
#define CHILDWNDINDEX_RADIOBUTTON1 8U
#define CHILDWNDINDEX_RADIOBUTTON2 9U
#define CHILDWNDINDEX_LISTBOX1 10U
#define CHILDWNDINDEX_BTNGROUPBOX1 11U
#define CHILDWNDINDEX_BTNGROUPBOX2 12U
#define CHILDWNDINDEX_BTNGROUPBOX3 13U
#define CHILDWNDINDEX_BTNGROUPBOX4 14U
#define CHILDWNDINDEX_CONTAINER1 15U
#define CHILDWNDINDEX_CONTAINER2 16U
#define CHILDWNDINDEX_CONTAINER3 17U
#define CHILDWNDINDEX_CONTAINER4 18U
#define CHILDWNDINDEX_CONTAINER5 19U

#define PP_CHILDWND_LENGTH 20U
#define PP_CHILDWND_SIZE (PP_CHILDWND_LENGTH*sizeof(HWND))

#define MAINWND_CAPTION TEXT("Audio Real-Time Delay")

#define MAINWND_BKCOLOR CUSTOMCOLOR_LTGRAY
#define MAINWND_BRUSHINDEX BRUSHINDEX_CUSTOM_SOLID_LTGRAY

#define CONTAINERWND_BKCOLOR MAINWND_BKCOLOR
#define CONTAINERWND_BRUSHINDEX BRUSHINDEX_TRANSPARENT

#define TEXTWND_TEXTCOLOR CUSTOMCOLOR_BLACK
#define TEXTWND_BKCOLOR MAINWND_BKCOLOR
#define TEXTWND_BRUSHINDEX BRUSHINDEX_TRANSPARENT

#define PB_I16 1
#define PB_I24 2

HANDLE p_audiothread = NULL;
HANDLE h_filein = INVALID_HANDLE_VALUE;

HBRUSH pp_brush[PP_BRUSH_LENGTH] = {NULL};
HFONT pp_font[PP_FONT_LENGTH] = {NULL};
HWND pp_childwnd[PP_CHILDWND_LENGTH] = {NULL};

HWND p_mainwnd = NULL;

AudioRTDSP *p_audio = NULL;
audiortdsp_pb_params_t pb_params;

__string tstr = TEXT("");

INT runtime_status = -1;
INT prev_status = -1;

WORD custom_generic_wndclass_id = 0u;

extern BOOL WINAPI app_init(VOID);
extern VOID WINAPI app_deinit(VOID);

extern BOOL WINAPI gdiobj_init(VOID);
extern VOID WINAPI gdiobj_deinit(VOID);

extern BOOL WINAPI register_wndclass(VOID);
extern BOOL WINAPI create_mainwnd(VOID);
extern BOOL WINAPI create_childwnd(VOID);

extern INT WINAPI app_get_ref_status(VOID);

extern VOID WINAPI runtime_loop(VOID);

extern VOID WINAPI paintscreen_choosefile(VOID);
extern VOID WINAPI paintscreen_chooseaudiodev(VOID);
extern VOID WINAPI paintscreen_playback_running(VOID);
extern VOID WINAPI paintscreen_playback_finished(VOID);

extern VOID WINAPI text_choose_font(VOID);
extern VOID WINAPI text_align(VOID);
extern VOID WINAPI button_align(VOID);
extern VOID WINAPI listbox_align(VOID);
extern VOID WINAPI container_align(VOID);
extern VOID WINAPI containerchildren_align(VOID);
extern VOID WINAPI ctrls_setup(BOOL redraw_mainwnd);

extern BOOL WINAPI window_get_dimensions(HWND p_wnd, INT *p_xpos, INT *p_ypos, INT *p_width, INT *p_height, INT *p_centerx, INT *p_centery);
extern BOOL WINAPI window_enable_vscroll(HWND p_wnd, BOOL enable);
extern BOOL WINAPI window_enable_hscroll(HWND p_wnd, BOOL enable);
extern BOOL WINAPI window_set_parent(HWND p_wnd, HWND p_parent);

extern BOOL WINAPI mainwnd_redraw(VOID);

extern VOID WINAPI childwnd_hide_all(VOID);

extern BOOL WINAPI catch_messages(VOID);

extern LRESULT CALLBACK mainwnd_wndproc(HWND p_wnd, UINT msg, WPARAM wparam, LPARAM lparam);

extern LRESULT CALLBACK mainwnd_event_wmcommand(HWND p_wnd, WPARAM wparam, LPARAM lparam);
extern LRESULT CALLBACK mainwnd_event_wmdestroy(HWND p_wnd, WPARAM wparam, LPARAM lparam);
extern LRESULT CALLBACK mainwnd_event_wmsize(HWND p_wnd, WPARAM wparam, LPARAM lparam);
extern LRESULT CALLBACK mainwnd_event_wmpaint(HWND p_wnd, WPARAM wparam, LPARAM lparam);

extern LRESULT CALLBACK container_wndproc(HWND p_wnd, UINT msg, WPARAM wparam, LPARAM lparam);
extern LRESULT CALLBACK container_wndproc_fwrdtoparent(HWND p_wnd, UINT msg, WPARAM wparam, LPARAM lparam);

extern LRESULT CALLBACK container_event_wmpaint(HWND p_wnd, WPARAM wparam, LPARAM lparam);

extern LRESULT CALLBACK window_event_wmctlcolorstatic(HWND p_wnd, WPARAM wparam, LPARAM lparam);

extern VOID WINAPI fxtext_update(VOID);

extern BOOL WINAPI attempt_update_ndelay(VOID);
extern BOOL WINAPI attempt_update_nfeedback(VOID);
extern VOID WINAPI update_feedbackaltpol(VOID);
extern VOID WINAPI update_cycledivincone(VOID);

extern VOID WINAPI preload_ui_state_from_dsp(VOID);

extern BOOL WINAPI choosefile_proc(VOID);
extern BOOL WINAPI chooseaudiodev_proc(SIZE_T index_sel, BOOL dev_default);
extern BOOL WINAPI initaudioobj_proc(VOID);

extern BOOL WINAPI filein_open(const TCHAR *filein_dir);
extern VOID WINAPI filein_close(VOID);

extern INT WINAPI filein_get_params(VOID);
extern BOOL WINAPI compare_signature(const CHAR *auth, const CHAR *buf);

extern DWORD WINAPI audiothread_proc(VOID *p_args);

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{
	p_instance = hInstance;

	if(!app_init()) return 1;

	runtime_loop();

	app_deinit();
	return 0;
}

BOOL WINAPI app_init(VOID)
{
	HRESULT n_ret = 0;

	if(p_instance == NULL)
	{
		__SPRINTF(textbuf, TEXTBUF_SIZE_CHARS, TEXT("Error: Invalid Instance."));
		goto _l_app_init_error;
	}

	p_processheap = GetProcessHeap();
	if(p_processheap == NULL)
	{
		__SPRINTF(textbuf, TEXTBUF_SIZE_CHARS, TEXT("Error: Invalid Process Heap."));
		goto _l_app_init_error;
	}

	if(!gdiobj_init())
	{
		__SPRINTF(textbuf, TEXTBUF_SIZE_CHARS, TEXT("Error: GDIOBJ Init Failed."));
		goto _l_app_init_error;
	}

	if(!register_wndclass())
	{
		__SPRINTF(textbuf, TEXTBUF_SIZE_CHARS, TEXT("Error: Register WNDCLASS Failed."));
		goto _l_app_init_error;
	}

	if(!create_mainwnd())
	{
		__SPRINTF(textbuf, TEXTBUF_SIZE_CHARS, TEXT("Error: Create MAINWND Failed."));
		goto _l_app_init_error;
	}

	if(!create_childwnd())
	{
		__SPRINTF(textbuf, TEXTBUF_SIZE_CHARS, TEXT("Error: Create CHILDWND Failed."));
		goto _l_app_init_error;
	}

	n_ret = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if(n_ret != S_OK)
	{
		__SPRINTF(textbuf, TEXTBUF_SIZE_CHARS, TEXT("Error: COMBASEAPI Init Failed."));
		goto _l_app_init_error;
	}

	runtime_status = RUNTIME_STATUS_INIT;
	return TRUE;

_l_app_init_error:
	MessageBox(NULL, textbuf, TEXT("INIT ERROR"), (MB_ICONSTOP | MB_OK));
	app_deinit();
	return FALSE;
}

VOID WINAPI app_deinit(VOID)
{
	if(p_audiothread != NULL) thread_stop(&p_audiothread, 0u);

	if(p_audio != NULL)
	{
		delete p_audio;
		p_audio = NULL;
	}

	filein_close();

	if(p_mainwnd != NULL) DestroyWindow(p_mainwnd);

	if(custom_generic_wndclass_id)
	{
		UnregisterClass(CUSTOM_GENERIC_WNDCLASS_NAME, p_instance);
		custom_generic_wndclass_id = 0u;
	}

	gdiobj_deinit();
	CoUninitialize();

	return;
}

__declspec(noreturn) VOID WINAPI app_exit(UINT exit_code, const TCHAR *exit_msg)
{
	app_deinit();

	if(exit_msg != NULL) MessageBox(NULL, exit_msg, TEXT("PROCESS EXIT CALLED"), (MB_ICONSTOP | MB_OK));

	ExitProcess(exit_code);

	while(TRUE) Sleep(10u);
}

BOOL WINAPI gdiobj_init(VOID)
{
	SIZE_T n_obj = 0u;
	LOGFONT logfont;

	pp_brush[BRUSHINDEX_TRANSPARENT] = (HBRUSH) GetStockObject(HOLLOW_BRUSH);
	pp_brush[BRUSHINDEX_CUSTOM_SOLID_BLACK] = CreateSolidBrush(CUSTOMCOLOR_BLACK);
	pp_brush[BRUSHINDEX_CUSTOM_SOLID_WHITE] = CreateSolidBrush(CUSTOMCOLOR_WHITE);
	pp_brush[BRUSHINDEX_CUSTOM_SOLID_LTGRAY] = CreateSolidBrush(CUSTOMCOLOR_LTGRAY);

	for(n_obj = 0u; n_obj < PP_BRUSH_LENGTH; n_obj++) if(pp_brush[n_obj] == NULL) return FALSE;

	ZeroMemory(&logfont, sizeof(LOGFONT));

	logfont.lfCharSet = CUSTOMFONT_NORMAL_CHARSET;
	logfont.lfWidth = CUSTOMFONT_NORMAL_WIDTH;
	logfont.lfHeight = CUSTOMFONT_NORMAL_HEIGHT;
	logfont.lfWeight = CUSTOMFONT_NORMAL_WEIGHT;

	pp_font[FONTINDEX_CUSTOM_NORMAL] = CreateFontIndirect(&logfont);

	logfont.lfCharSet = CUSTOMFONT_PARAMS_CHARSET;
	logfont.lfWidth = CUSTOMFONT_PARAMS_WIDTH;
	logfont.lfHeight = CUSTOMFONT_PARAMS_HEIGHT;
	logfont.lfWeight = CUSTOMFONT_PARAMS_WEIGHT;

	pp_font[FONTINDEX_CUSTOM_PARAMS] = CreateFontIndirect(&logfont);

	logfont.lfCharSet = CUSTOMFONT_LARGE_CHARSET;
	logfont.lfWidth = CUSTOMFONT_LARGE_WIDTH;
	logfont.lfHeight = CUSTOMFONT_LARGE_HEIGHT;
	logfont.lfWeight = CUSTOMFONT_LARGE_WEIGHT;

	pp_font[FONTINDEX_CUSTOM_LARGE] = CreateFontIndirect(&logfont);

	for(n_obj = 0u; n_obj < PP_FONT_LENGTH; n_obj++) if(pp_font[n_obj] == NULL) return FALSE;

	return TRUE;
}

VOID WINAPI gdiobj_deinit(VOID)
{
	SIZE_T n_obj = 0u;

	for(n_obj = 0u; n_obj < PP_BRUSH_LENGTH; n_obj++)
	{
		if(pp_brush[n_obj] != NULL)
		{
			DeleteObject(pp_brush[n_obj]);
			pp_brush[n_obj] = NULL;
		}
	}

	for(n_obj = 0u; n_obj < PP_FONT_LENGTH; n_obj++)
	{
		if(pp_font[n_obj] != NULL)
		{
			DeleteObject(pp_font[n_obj]);
			pp_font[n_obj] = NULL;
		}
	}

	return;
}

BOOL WINAPI register_wndclass(VOID)
{
	WNDCLASS wndclass;

	ZeroMemory(&wndclass, sizeof(WNDCLASS));

	wndclass.style = CS_OWNDC;
	wndclass.lpfnWndProc = &DefWindowProc;
	wndclass.hInstance = p_instance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = pp_brush[BRUSHINDEX_TRANSPARENT];
	wndclass.lpszClassName = CUSTOM_GENERIC_WNDCLASS_NAME;

	custom_generic_wndclass_id = RegisterClass(&wndclass);

	return (BOOL) custom_generic_wndclass_id;
}

BOOL WINAPI create_mainwnd(VOID)
{
	DWORD style = (WS_CAPTION | WS_VISIBLE | WS_SYSMENU | WS_OVERLAPPED | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX | WS_MAXIMIZE);

	p_mainwnd = CreateWindow(CUSTOM_GENERIC_WNDCLASS_NAME, MAINWND_CAPTION, style, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, p_instance, NULL);
	if(p_mainwnd == NULL) return FALSE;

	if(SetWindowLongPtr(p_mainwnd, GWLP_WNDPROC, (LONG_PTR) &mainwnd_wndproc)) return TRUE;

	return FALSE;

	/*
		Avoid using "return (BOOL) SetWindowLongPtr(...);"

		SetWindowLongPtr() return type is LONG_PTR, casting it to BOOL could cause runtime errors on 64bit Windows.
	*/
}

BOOL WINAPI create_childwnd(VOID)
{
	SIZE_T n_wnd = 0u;
	DWORD style = 0u;

	style = WS_CHILD;

	pp_childwnd[CHILDWNDINDEX_CONTAINER1] = CreateWindow(CUSTOM_GENERIC_WNDCLASS_NAME, NULL, style, 0, 0, 0, 0, p_mainwnd, NULL, p_instance, NULL);
	pp_childwnd[CHILDWNDINDEX_CONTAINER2] = CreateWindow(CUSTOM_GENERIC_WNDCLASS_NAME, NULL, style, 0, 0, 0, 0, p_mainwnd, NULL, p_instance, NULL);
	pp_childwnd[CHILDWNDINDEX_CONTAINER3] = CreateWindow(CUSTOM_GENERIC_WNDCLASS_NAME, NULL, style, 0, 0, 0, 0, p_mainwnd, NULL, p_instance, NULL);
	pp_childwnd[CHILDWNDINDEX_CONTAINER4] = CreateWindow(CUSTOM_GENERIC_WNDCLASS_NAME, NULL, style, 0, 0, 0, 0, p_mainwnd, NULL, p_instance, NULL);
	pp_childwnd[CHILDWNDINDEX_CONTAINER5] = CreateWindow(CUSTOM_GENERIC_WNDCLASS_NAME, NULL, style, 0, 0, 0, 0, p_mainwnd, NULL, p_instance, NULL);

	if(pp_childwnd[CHILDWNDINDEX_CONTAINER1] == NULL) return FALSE;
	if(pp_childwnd[CHILDWNDINDEX_CONTAINER2] == NULL) return FALSE;
	if(pp_childwnd[CHILDWNDINDEX_CONTAINER3] == NULL) return FALSE;
	if(pp_childwnd[CHILDWNDINDEX_CONTAINER4] == NULL) return FALSE;
	if(pp_childwnd[CHILDWNDINDEX_CONTAINER5] == NULL) return FALSE;

	if(!SetWindowLongPtr(pp_childwnd[CHILDWNDINDEX_CONTAINER1], GWLP_WNDPROC, (LONG_PTR) &container_wndproc)) return FALSE;
	if(!SetWindowLongPtr(pp_childwnd[CHILDWNDINDEX_CONTAINER2], GWLP_WNDPROC, (LONG_PTR) &container_wndproc)) return FALSE;
	if(!SetWindowLongPtr(pp_childwnd[CHILDWNDINDEX_CONTAINER3], GWLP_WNDPROC, (LONG_PTR) &container_wndproc)) return FALSE;
	if(!SetWindowLongPtr(pp_childwnd[CHILDWNDINDEX_CONTAINER4], GWLP_WNDPROC, (LONG_PTR) &container_wndproc)) return FALSE;
	if(!SetWindowLongPtr(pp_childwnd[CHILDWNDINDEX_CONTAINER5], GWLP_WNDPROC, (LONG_PTR) &container_wndproc)) return FALSE;

	style = (WS_CHILD | BS_GROUPBOX);

	pp_childwnd[CHILDWNDINDEX_BTNGROUPBOX1] = CreateWindow(TEXT("BUTTON"), TEXT("Set Delay Time (# Of Samples)"), style, 0, 0, 0, 0, pp_childwnd[CHILDWNDINDEX_CONTAINER1], NULL, p_instance, NULL);
	pp_childwnd[CHILDWNDINDEX_BTNGROUPBOX2] = CreateWindow(TEXT("BUTTON"), TEXT("Set Delay FB Loop Count"), style, 0, 0, 0, 0, pp_childwnd[CHILDWNDINDEX_CONTAINER2], NULL, p_instance, NULL);
	pp_childwnd[CHILDWNDINDEX_BTNGROUPBOX3] = CreateWindow(TEXT("BUTTON"), TEXT("FB Polarity Mode"), style, 0, 0, 0, 0, pp_childwnd[CHILDWNDINDEX_CONTAINER3], NULL, p_instance, NULL);
	pp_childwnd[CHILDWNDINDEX_BTNGROUPBOX4] = CreateWindow(TEXT("BUTTON"), TEXT("FB Cycle Div. Inc. Mode"), style, 0, 0, 0, 0, pp_childwnd[CHILDWNDINDEX_CONTAINER4], NULL, p_instance, NULL);

	style = (WS_CHILD | SS_CENTER);
	pp_childwnd[CHILDWNDINDEX_TEXT1] = CreateWindow(TEXT("STATIC"), NULL, style, 0, 0, 0, 0, p_mainwnd, NULL, p_instance, NULL);

	style = (WS_CHILD | SS_LEFT);
	pp_childwnd[CHILDWNDINDEX_TEXT2] = CreateWindow(TEXT("STATIC"), NULL, style, 0, 0, 0, 0, pp_childwnd[CHILDWNDINDEX_CONTAINER5], NULL, p_instance, NULL);

	style = (WS_CHILD | WS_TABSTOP | ES_CENTER | ES_NUMBER);

	pp_childwnd[CHILDWNDINDEX_TEXTBOX1] = CreateWindow(TEXT("EDIT"), NULL, style, 0, 0, 0, 0, pp_childwnd[CHILDWNDINDEX_CONTAINER1], NULL, p_instance, NULL);
	pp_childwnd[CHILDWNDINDEX_TEXTBOX2] = CreateWindow(TEXT("EDIT"), NULL, style, 0, 0, 0, 0, pp_childwnd[CHILDWNDINDEX_CONTAINER2], NULL, p_instance, NULL);

	style = (WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON | BS_TEXT | BS_CENTER | BS_VCENTER);

	pp_childwnd[CHILDWNDINDEX_BUTTON1] = CreateWindow(TEXT("BUTTON"), NULL, style, 0, 0, 0, 0, p_mainwnd, NULL, p_instance, NULL);
	pp_childwnd[CHILDWNDINDEX_BUTTON2] = CreateWindow(TEXT("BUTTON"), NULL, style, 0, 0, 0, 0, p_mainwnd, NULL, p_instance, NULL);
	pp_childwnd[CHILDWNDINDEX_BUTTON3] = CreateWindow(TEXT("BUTTON"), NULL, style, 0, 0, 0, 0, p_mainwnd, NULL, p_instance, NULL);

	style = (WS_CHILD | WS_TABSTOP | WS_VSCROLL | LBS_HASSTRINGS);
	pp_childwnd[CHILDWNDINDEX_LISTBOX1] = CreateWindow(TEXT("LISTBOX"), NULL, style, 0, 0, 0, 0, p_mainwnd, NULL, p_instance, NULL);

	style = (WS_CHILD | WS_TABSTOP | BS_LEFT | BS_AUTOCHECKBOX);
	pp_childwnd[CHILDWNDINDEX_CHECKBOX1] = CreateWindow(TEXT("BUTTON"), TEXT("Alternate FB Polarity"), style, 0, 0, 0, 0, pp_childwnd[CHILDWNDINDEX_CONTAINER3], NULL, p_instance, NULL);

	style = (WS_CHILD | WS_TABSTOP | BS_LEFT | BS_AUTORADIOBUTTON);

	pp_childwnd[CHILDWNDINDEX_RADIOBUTTON1] = CreateWindow(TEXT("BUTTON"), TEXT("Inc. Div. By 1"), style, 0, 0, 0, 0, pp_childwnd[CHILDWNDINDEX_CONTAINER4], NULL, p_instance, NULL);
	pp_childwnd[CHILDWNDINDEX_RADIOBUTTON2] = CreateWindow(TEXT("BUTTON"), TEXT("Inc. Div. Exponentially"), style, 0, 0, 0, 0, pp_childwnd[CHILDWNDINDEX_CONTAINER4], NULL, p_instance, NULL);

	for(n_wnd = 0u; n_wnd < PP_CHILDWND_LENGTH; n_wnd++) if(pp_childwnd[n_wnd] == NULL) return FALSE;

	return TRUE;
}

INT WINAPI app_get_ref_status(VOID)
{
	if(runtime_status == RUNTIME_STATUS_IDLE) return prev_status;

	return runtime_status;
}

VOID WINAPI runtime_loop(VOID)
{
	while(catch_messages())
	{
		switch(runtime_status)
		{
			case RUNTIME_STATUS_IDLE:
				Sleep(10u);
				break;

			case RUNTIME_STATUS_INIT:
				ctrls_setup(TRUE);
				runtime_status = RUNTIME_STATUS_CHOOSEFILE;

			case RUNTIME_STATUS_CHOOSEFILE:
				paintscreen_choosefile();
				goto _l_runtime_loop_runtimestatus_notidle;
				break;

			case RUNTIME_STATUS_CHOOSEAUDIODEV:
				paintscreen_chooseaudiodev();
				goto _l_runtime_loop_runtimestatus_notidle;
				break;

			case RUNTIME_STATUS_PLAYBACK_RUNNING:
				paintscreen_playback_running();
				goto _l_runtime_loop_runtimestatus_notidle;
				break;

			case RUNTIME_STATUS_PLAYBACK_FINISHED:
				paintscreen_playback_finished();
				goto _l_runtime_loop_runtimestatus_notidle;
				break;
		}

_l_runtime_loop_runtimestatus_idle:
		continue;

_l_runtime_loop_runtimestatus_notidle:
		prev_status = runtime_status;
		runtime_status = RUNTIME_STATUS_IDLE;
	}

	return;
}

VOID WINAPI paintscreen_choosefile(VOID)
{
	childwnd_hide_all();

	button_align();

	SendMessage(pp_childwnd[CHILDWNDINDEX_TEXT1], WM_SETTEXT, 0, (LPARAM) TEXT("Choose Audio File"));
	SendMessage(pp_childwnd[CHILDWNDINDEX_BUTTON1], WM_SETTEXT, 0, (LPARAM) TEXT("Browse"));

	ShowWindow(pp_childwnd[CHILDWNDINDEX_TEXT1], SW_SHOW);
	ShowWindow(pp_childwnd[CHILDWNDINDEX_BUTTON1], SW_SHOW);
	return;
}

VOID WINAPI paintscreen_chooseaudiodev(VOID)
{
	childwnd_hide_all();

	window_set_parent(pp_childwnd[CHILDWNDINDEX_BUTTON2], p_mainwnd);
	window_set_parent(pp_childwnd[CHILDWNDINDEX_BUTTON3], p_mainwnd);

	button_align();

	SendMessage(pp_childwnd[CHILDWNDINDEX_TEXT1], WM_SETTEXT, 0, (LPARAM) TEXT("Choose Playback Device"));
	SendMessage(pp_childwnd[CHILDWNDINDEX_BUTTON1], WM_SETTEXT, 0, (LPARAM) TEXT("Return"));
	SendMessage(pp_childwnd[CHILDWNDINDEX_BUTTON2], WM_SETTEXT, 0, (LPARAM) TEXT("Choose Selected Device"));
	SendMessage(pp_childwnd[CHILDWNDINDEX_BUTTON3], WM_SETTEXT, 0, (LPARAM) TEXT("Choose Default Device"));

	if(p_audio->loadAudioDeviceList(pp_childwnd[CHILDWNDINDEX_LISTBOX1]))
	{
		ShowWindow(pp_childwnd[CHILDWNDINDEX_BUTTON2], SW_SHOW);
		ShowWindow(pp_childwnd[CHILDWNDINDEX_LISTBOX1], SW_SHOW);
	}
	else
	{
		tstr = TEXT("Error: could not load device list\r\nExtended Error Info: ");
		tstr += p_audio->getLastErrorMessage();
		MessageBox(NULL, tstr.c_str(), TEXT("ERROR"), (MB_ICONEXCLAMATION | MB_OK));
	}

	ShowWindow(pp_childwnd[CHILDWNDINDEX_TEXT1], SW_SHOW);
	ShowWindow(pp_childwnd[CHILDWNDINDEX_BUTTON1], SW_SHOW);
	ShowWindow(pp_childwnd[CHILDWNDINDEX_BUTTON3], SW_SHOW);

	return;
}

VOID WINAPI paintscreen_playback_running(VOID)
{
	childwnd_hide_all();

	window_set_parent(pp_childwnd[CHILDWNDINDEX_BUTTON2], pp_childwnd[CHILDWNDINDEX_CONTAINER1]);
	window_set_parent(pp_childwnd[CHILDWNDINDEX_BUTTON3], pp_childwnd[CHILDWNDINDEX_CONTAINER2]);

	button_align();
	containerchildren_align();

	SendMessage(pp_childwnd[CHILDWNDINDEX_TEXT1], WM_SETTEXT, 0, (LPARAM) TEXT("Playback Running"));
	SendMessage(pp_childwnd[CHILDWNDINDEX_BUTTON1], WM_SETTEXT, 0, (LPARAM) TEXT("Stop Playback"));
	SendMessage(pp_childwnd[CHILDWNDINDEX_BUTTON2], WM_SETTEXT, 0, (LPARAM) TEXT("Update Delay Time"));
	SendMessage(pp_childwnd[CHILDWNDINDEX_BUTTON3], WM_SETTEXT, 0, (LPARAM) TEXT("Update FB Loop Count"));

	SendMessage(pp_childwnd[CHILDWNDINDEX_TEXTBOX1], WM_SETTEXT, 0, (LPARAM) TEXT(""));
	SendMessage(pp_childwnd[CHILDWNDINDEX_TEXTBOX2], WM_SETTEXT, 0, (LPARAM) TEXT(""));

	preload_ui_state_from_dsp();

	ShowWindow(pp_childwnd[CHILDWNDINDEX_CONTAINER1], SW_SHOW);
	ShowWindow(pp_childwnd[CHILDWNDINDEX_CONTAINER2], SW_SHOW);
	ShowWindow(pp_childwnd[CHILDWNDINDEX_CONTAINER3], SW_SHOW);
	ShowWindow(pp_childwnd[CHILDWNDINDEX_CONTAINER4], SW_SHOW);
	ShowWindow(pp_childwnd[CHILDWNDINDEX_CONTAINER5], SW_SHOW);
	ShowWindow(pp_childwnd[CHILDWNDINDEX_BTNGROUPBOX1], SW_SHOW);
	ShowWindow(pp_childwnd[CHILDWNDINDEX_BTNGROUPBOX2], SW_SHOW);
	ShowWindow(pp_childwnd[CHILDWNDINDEX_BTNGROUPBOX3], SW_SHOW);
	ShowWindow(pp_childwnd[CHILDWNDINDEX_BTNGROUPBOX4], SW_SHOW);
	ShowWindow(pp_childwnd[CHILDWNDINDEX_TEXT1], SW_SHOW);
	ShowWindow(pp_childwnd[CHILDWNDINDEX_TEXT2], SW_SHOW);
	ShowWindow(pp_childwnd[CHILDWNDINDEX_BUTTON1], SW_SHOW);
	ShowWindow(pp_childwnd[CHILDWNDINDEX_BUTTON2], SW_SHOW);
	ShowWindow(pp_childwnd[CHILDWNDINDEX_BUTTON3], SW_SHOW);
	ShowWindow(pp_childwnd[CHILDWNDINDEX_TEXTBOX1], SW_SHOW);
	ShowWindow(pp_childwnd[CHILDWNDINDEX_TEXTBOX2], SW_SHOW);
	ShowWindow(pp_childwnd[CHILDWNDINDEX_CHECKBOX1], SW_SHOW);
	ShowWindow(pp_childwnd[CHILDWNDINDEX_RADIOBUTTON1], SW_SHOW);
	ShowWindow(pp_childwnd[CHILDWNDINDEX_RADIOBUTTON2], SW_SHOW);

	return;
}

VOID WINAPI paintscreen_playback_finished(VOID)
{
	childwnd_hide_all();

	button_align();

	SendMessage(pp_childwnd[CHILDWNDINDEX_TEXT1], WM_SETTEXT, 0, (LPARAM) TEXT("Playback Finished"));
	SendMessage(pp_childwnd[CHILDWNDINDEX_BUTTON1], WM_SETTEXT, 0, (LPARAM) TEXT("Start Over"));

	ShowWindow(pp_childwnd[CHILDWNDINDEX_TEXT1], SW_SHOW);
	ShowWindow(pp_childwnd[CHILDWNDINDEX_BUTTON1], SW_SHOW);

	return;
}

VOID WINAPI text_choose_font(VOID)
{
	INT mainwnd_width = 0;
	INT mainwnd_height = 0;

	window_get_dimensions(p_mainwnd, NULL, NULL, &mainwnd_width, &mainwnd_height, NULL, NULL);

	if((mainwnd_width < 800) || (mainwnd_height < 600))
	{
		SendMessage(pp_childwnd[CHILDWNDINDEX_TEXT1], WM_SETFONT, (WPARAM) pp_font[FONTINDEX_CUSTOM_NORMAL], (LPARAM) TRUE);
		SendMessage(pp_childwnd[CHILDWNDINDEX_TEXT2], WM_SETFONT, (WPARAM) pp_font[FONTINDEX_CUSTOM_NORMAL], (LPARAM) TRUE);
	}
	else
	{
		SendMessage(pp_childwnd[CHILDWNDINDEX_TEXT1], WM_SETFONT, (WPARAM) pp_font[FONTINDEX_CUSTOM_LARGE], (LPARAM) TRUE);
		SendMessage(pp_childwnd[CHILDWNDINDEX_TEXT2], WM_SETFONT, (WPARAM) pp_font[FONTINDEX_CUSTOM_PARAMS], (LPARAM) TRUE);
	}

	return;
}

VOID WINAPI text_align(VOID)
{
	INT mainwnd_width = 0;
	INT mainwnd_height = 0;

	INT text1_dim[4] = {0};

	BOOL small_wnd = FALSE;

	window_get_dimensions(p_mainwnd, NULL, NULL, &mainwnd_width, &mainwnd_height, NULL, NULL);

	small_wnd = ((mainwnd_width < 800) || (mainwnd_height < 600));

	text1_dim[1] = 20;

	if(small_wnd)
	{
		text1_dim[0] = 20;
		text1_dim[3] = CUSTOMFONT_NORMAL_HEIGHT;
	}
	else
	{
		text1_dim[0] = 40;
		text1_dim[3] = CUSTOMFONT_LARGE_HEIGHT;
	}

	text1_dim[2] = mainwnd_width - 2*text1_dim[0];

	SetWindowPos(pp_childwnd[CHILDWNDINDEX_TEXT1], NULL, text1_dim[0], text1_dim[1], text1_dim[2], text1_dim[3], 0u);
	return;
}

VOID WINAPI button_align(VOID)
{
	constexpr INT BUTTON_HEIGHT = 20;

	INT mainwnd_width = 0;
	INT mainwnd_height = 0;
	INT mainwnd_centerx = 0;

	INT button1_dim[4] = {0};
	INT button2_dim[4] = {0};
	INT button3_dim[4] = {0};

	INT ref_status = 0;

	BOOL small_wnd = FALSE;

	window_get_dimensions(p_mainwnd, NULL, NULL, &mainwnd_width, &mainwnd_height, &mainwnd_centerx, NULL);

	small_wnd = ((mainwnd_width < 800) || (mainwnd_height < 600));

	ref_status = app_get_ref_status();

	button1_dim[3] = BUTTON_HEIGHT;
	button2_dim[3] = button1_dim[3];
	button3_dim[3] = button1_dim[3];

	if(ref_status == RUNTIME_STATUS_CHOOSEAUDIODEV)
	{
		button1_dim[2] = 200;
		button2_dim[2] = button1_dim[2];
		button2_dim[3] = button1_dim[3];
		button3_dim[2] = button1_dim[2];
		button3_dim[3] = button1_dim[3];

		button2_dim[0] = mainwnd_centerx - button2_dim[2]/2;

		if(small_wnd)
		{
			button3_dim[1] = mainwnd_height - button3_dim[3] - 40;

			button1_dim[0] = button2_dim[0];
			button3_dim[0] = button2_dim[0];

			button2_dim[1] = button3_dim[1] - button2_dim[3] - 10;
			button1_dim[1] = button2_dim[1] - button1_dim[3] - 10;
		}
		else
		{
			button3_dim[1] = mainwnd_height - button3_dim[3] - 60;

			button1_dim[1] = button3_dim[1];
			button2_dim[1] = button3_dim[1];

			button1_dim[0] = button2_dim[0] - button1_dim[2] - 10;
			button3_dim[0] = button2_dim[0] + button2_dim[2] + 10;
		}

		SetWindowPos(pp_childwnd[CHILDWNDINDEX_BUTTON2], NULL, button2_dim[0], button2_dim[1], button2_dim[2], button2_dim[3], 0u);
		SetWindowPos(pp_childwnd[CHILDWNDINDEX_BUTTON3], NULL, button3_dim[0], button3_dim[1], button3_dim[2], button3_dim[3], 0u);
	}
	else
	{
		if(small_wnd) button1_dim[1] = mainwnd_height - button1_dim[3] - 40;
		else button1_dim[1] = mainwnd_height - button1_dim[3] - 60;

		button1_dim[2] = 120;
		button1_dim[0] = mainwnd_centerx - button1_dim[2]/2;
	}

	SetWindowPos(pp_childwnd[CHILDWNDINDEX_BUTTON1], NULL, button1_dim[0], button1_dim[1], button1_dim[2], button1_dim[3], 0u);
	return;
}

VOID WINAPI listbox_align(VOID)
{
	INT mainwnd_width = 0;
	INT mainwnd_height = 0;

	INT listbox_dim[4] = {0};

	BOOL small_wnd = FALSE;

	window_get_dimensions(p_mainwnd, NULL, NULL, &mainwnd_width, &mainwnd_height, NULL, NULL);

	small_wnd = ((mainwnd_width < 800) || (mainwnd_height < 600));

	if(small_wnd)
	{
		listbox_dim[0] = 20;
		listbox_dim[1] = 60;
		listbox_dim[3] = 60;
	}
	else
	{
		listbox_dim[0] = 40;
		listbox_dim[1] = 100;
		listbox_dim[3] = 100;
	}

	listbox_dim[2] = mainwnd_width - 2*listbox_dim[0];

	SetWindowPos(pp_childwnd[CHILDWNDINDEX_LISTBOX1], NULL, listbox_dim[0], listbox_dim[1], listbox_dim[2], listbox_dim[3], 0u);
	return;
}

VOID WINAPI container_align(VOID)
{
	constexpr INT BTN_CONTAINER_WIDTH = 240;
	constexpr INT TEXT2_NLINES_MIN = 6;

	INT mainwnd_width = 0;
	INT mainwnd_height = 0;
	INT mainwnd_centerx = 0;

	INT container1_dim[4] = {0};
	INT container2_dim[4] = {0};
	INT container3_dim[4] = {0};
	INT container4_dim[4] = {0};
	INT container5_dim[4] = {0};

	BOOL small_wnd = FALSE;

	INT container5_height_min = 0;

	window_get_dimensions(p_mainwnd, NULL, NULL, &mainwnd_width, &mainwnd_height, &mainwnd_centerx, NULL);

	small_wnd = ((mainwnd_width < 800) || (mainwnd_height < 600));

	container1_dim[2] = BTN_CONTAINER_WIDTH;
	container2_dim[2] = container1_dim[2];
	container3_dim[2] = container1_dim[2];
	container4_dim[2] = container1_dim[2];

	container1_dim[3] = 100;
	container2_dim[3] = container1_dim[3];
	container3_dim[3] = 60;
	container4_dim[3] = container1_dim[3];

	if(small_wnd)
	{
		container1_dim[0] = mainwnd_centerx - container1_dim[2] - 4;
		container2_dim[0] = mainwnd_centerx + 4;

		container3_dim[0] = container1_dim[0];
		container4_dim[0] = container2_dim[0];

		container4_dim[1] = mainwnd_height - container4_dim[3] - 70;
		container3_dim[1] = mainwnd_height - container3_dim[3] - 70;

		container2_dim[1] = container4_dim[1] - container2_dim[3] - 8;
		container1_dim[1] = container2_dim[1];

		container5_height_min = TEXT2_NLINES_MIN*CUSTOMFONT_NORMAL_HEIGHT;

		container5_dim[0] = 20;
		container5_dim[1] = 60;
		container5_dim[2] = mainwnd_width - 2*container5_dim[0];
		container5_dim[3] = container5_height_min;
	}
	else
	{
		container1_dim[0] = mainwnd_width - container1_dim[2] - 40;
		container2_dim[0] = container1_dim[0];
		container3_dim[0] = container1_dim[0];
		container4_dim[0] = container1_dim[0];

		container1_dim[1] = 100;
		container2_dim[1] = container1_dim[1] + container1_dim[3] + 20;
		container3_dim[1] = container2_dim[1] + container2_dim[3] + 20;
		container4_dim[1] = container3_dim[1] + container3_dim[3] + 20;

		container5_dim[0] = 40;
		container5_dim[1] = 100;
		container5_dim[2] = container1_dim[0] - container5_dim[0] - 20;
		container5_dim[3] = mainwnd_height - container5_dim[1] - 100;

		container5_height_min = TEXT2_NLINES_MIN*CUSTOMFONT_PARAMS_HEIGHT;

		if(container5_dim[3] < container5_height_min) container5_dim[3] = container5_height_min;
	}

	SetWindowPos(pp_childwnd[CHILDWNDINDEX_CONTAINER1], NULL, container1_dim[0], container1_dim[1], container1_dim[2], container1_dim[3], 0u);
	SetWindowPos(pp_childwnd[CHILDWNDINDEX_CONTAINER2], NULL, container2_dim[0], container2_dim[1], container2_dim[2], container2_dim[3], 0u);
	SetWindowPos(pp_childwnd[CHILDWNDINDEX_CONTAINER3], NULL, container3_dim[0], container3_dim[1], container3_dim[2], container3_dim[3], 0u);
	SetWindowPos(pp_childwnd[CHILDWNDINDEX_CONTAINER4], NULL, container4_dim[0], container4_dim[1], container4_dim[2], container4_dim[3], 0u);
	SetWindowPos(pp_childwnd[CHILDWNDINDEX_CONTAINER5], NULL, container5_dim[0], container5_dim[1], container5_dim[2], container5_dim[3], 0u);

	SetWindowPos(pp_childwnd[CHILDWNDINDEX_BTNGROUPBOX1], NULL, 0, 0, container1_dim[2], container1_dim[3], 0u);
	SetWindowPos(pp_childwnd[CHILDWNDINDEX_BTNGROUPBOX2], NULL, 0, 0, container2_dim[2], container2_dim[3], 0u);
	SetWindowPos(pp_childwnd[CHILDWNDINDEX_BTNGROUPBOX3], NULL, 0, 0, container3_dim[2], container3_dim[3], 0u);
	SetWindowPos(pp_childwnd[CHILDWNDINDEX_BTNGROUPBOX4], NULL, 0, 0, container4_dim[2], container4_dim[3], 0u);

	return;
}

VOID WINAPI containerchildren_align(VOID)
{
	constexpr INT BUTTON_HEIGHT = 20;
	constexpr INT TEXTBOX_HEIGHT = 20;

	INT parent_width = 0;
	INT parent_height = 0;

	INT button2_dim[4] = {0};
	INT button3_dim[4] = {0};

	INT checkbox1_dim[4] = {0};

	INT radiobutton1_dim[4] = {0};
	INT radiobutton2_dim[4] = {0};

	INT textbox1_dim[4] = {0};
	INT textbox2_dim[4] = {0};

	INT ref_status = -1;

	window_get_dimensions(pp_childwnd[CHILDWNDINDEX_CONTAINER5], NULL, NULL, &parent_width, &parent_height, NULL, NULL);

	SetWindowPos(pp_childwnd[CHILDWNDINDEX_TEXT2], NULL, 0, 0, parent_width, parent_height, 0u);

	button2_dim[3] = BUTTON_HEIGHT;
	button3_dim[3] = button2_dim[3];
	checkbox1_dim[3] = button2_dim[3];
	radiobutton1_dim[3] = button2_dim[3];
	radiobutton2_dim[3] = button2_dim[3];

	textbox1_dim[3] = TEXTBOX_HEIGHT;
	textbox2_dim[3] = textbox1_dim[3];

	window_get_dimensions(pp_childwnd[CHILDWNDINDEX_CONTAINER3], NULL, NULL, &parent_width, &parent_height, NULL, NULL);

	checkbox1_dim[0] = 10;
	checkbox1_dim[1] = parent_height - checkbox1_dim[3] - 10;
	checkbox1_dim[2] = parent_width - 2*checkbox1_dim[0];

	window_get_dimensions(pp_childwnd[CHILDWNDINDEX_CONTAINER4], NULL, NULL, &parent_width, &parent_height, NULL, NULL);

	radiobutton1_dim[0] = 10;
	radiobutton2_dim[0] = radiobutton1_dim[0];

	radiobutton1_dim[2] = parent_width - 2*radiobutton1_dim[0];
	radiobutton2_dim[2] = radiobutton1_dim[2];

	radiobutton2_dim[1] = parent_height - radiobutton2_dim[3] - 10;
	radiobutton1_dim[1] = radiobutton2_dim[1] - radiobutton1_dim[3] - 10;

	SetWindowPos(pp_childwnd[CHILDWNDINDEX_CHECKBOX1], NULL, checkbox1_dim[0], checkbox1_dim[1], checkbox1_dim[2], checkbox1_dim[3], 0u);
	SetWindowPos(pp_childwnd[CHILDWNDINDEX_RADIOBUTTON1], NULL, radiobutton1_dim[0], radiobutton1_dim[1], radiobutton1_dim[2], radiobutton1_dim[3], 0u);
	SetWindowPos(pp_childwnd[CHILDWNDINDEX_RADIOBUTTON2], NULL, radiobutton2_dim[0], radiobutton2_dim[1], radiobutton2_dim[2], radiobutton2_dim[3], 0u);

	ref_status = app_get_ref_status();

	if(ref_status != RUNTIME_STATUS_PLAYBACK_RUNNING) return;

	window_get_dimensions(pp_childwnd[CHILDWNDINDEX_CONTAINER1], NULL, NULL, &parent_width, &parent_height, NULL, NULL);

	button2_dim[0] = 10;
	textbox1_dim[0] = button2_dim[0];

	button2_dim[2] = parent_width - 2*button2_dim[0];
	textbox1_dim[2] = button2_dim[2];

	button2_dim[1] = parent_height - button2_dim[3] - 10;
	textbox1_dim[1] = button2_dim[1] - textbox1_dim[3] - 10;

	window_get_dimensions(pp_childwnd[CHILDWNDINDEX_CONTAINER2], NULL, NULL, &parent_width, &parent_height, NULL, NULL);

	button3_dim[0] = 10;
	textbox2_dim[0] = button3_dim[0];

	button3_dim[2] = parent_width - 2*button3_dim[0];
	textbox2_dim[2] = button3_dim[2];

	button3_dim[1] = parent_height - button3_dim[3] - 10;
	textbox2_dim[1] = button3_dim[1] - textbox2_dim[3] - 10;

	SetWindowPos(pp_childwnd[CHILDWNDINDEX_BUTTON2], NULL, button2_dim[0], button2_dim[1], button2_dim[2], button2_dim[3], 0u);
	SetWindowPos(pp_childwnd[CHILDWNDINDEX_BUTTON3], NULL, button3_dim[0], button3_dim[1], button3_dim[2], button3_dim[3], 0u);
	SetWindowPos(pp_childwnd[CHILDWNDINDEX_TEXTBOX1], NULL, textbox1_dim[0], textbox1_dim[1], textbox1_dim[2], textbox1_dim[3], 0u);
	SetWindowPos(pp_childwnd[CHILDWNDINDEX_TEXTBOX2], NULL, textbox2_dim[0], textbox2_dim[1], textbox2_dim[2], textbox2_dim[3], 0u);

	return;
}

VOID WINAPI ctrls_setup(BOOL redraw_mainwnd)
{
	text_choose_font();

	container_align();
	containerchildren_align();
	text_align();
	button_align();
	listbox_align();

	if(redraw_mainwnd) mainwnd_redraw();
	return;
}

BOOL WINAPI window_get_dimensions(HWND p_wnd, INT *p_xpos, INT *p_ypos, INT *p_width, INT *p_height, INT *p_centerx, INT *p_centery)
{
	RECT rect;

	if(p_wnd == NULL) return FALSE;
	if(!GetWindowRect(p_wnd, &rect)) return FALSE;

	if(p_xpos != NULL) *p_xpos = rect.left;
	if(p_ypos != NULL) *p_ypos = rect.top;
	if(p_width != NULL) *p_width = rect.right - rect.left;
	if(p_height != NULL) *p_height = rect.bottom - rect.top;
	if(p_centerx != NULL) *p_centerx = (rect.right - rect.left)/2;
	if(p_centery != NULL) *p_centery = (rect.bottom - rect.top)/2;

	return TRUE;
}

BOOL WINAPI window_enable_vscroll(HWND p_wnd, BOOL enable)
{
	DWORD wnd_style = 0u;

	if(p_wnd == NULL) return FALSE;

	wnd_style = (DWORD) GetWindowLongPtr(p_wnd, GWL_STYLE);

	if(enable) wnd_style |= WS_VSCROLL;
	else wnd_style &= ~WS_VSCROLL;

	if(SetWindowLongPtr(p_wnd, GWL_STYLE, (LONG_PTR) wnd_style)) return TRUE;

	return FALSE;
}

BOOL WINAPI window_enable_hscroll(HWND p_wnd, BOOL enable)
{
	DWORD wnd_style = 0u;

	if(p_wnd == NULL) return FALSE;

	wnd_style = (DWORD) GetWindowLongPtr(p_wnd, GWL_STYLE);

	if(enable) wnd_style |= WS_HSCROLL;
	else wnd_style &= ~WS_HSCROLL;

	if(SetWindowLongPtr(p_wnd, GWL_STYLE, (LONG_PTR) wnd_style)) return TRUE;

	return FALSE;
}

BOOL WINAPI window_set_parent(HWND p_wnd, HWND p_parent)
{
	if(p_wnd == NULL) return FALSE;

	if(SetWindowLongPtr(p_wnd, GWLP_HWNDPARENT, (LONG_PTR) p_parent)) return TRUE;

	return FALSE;
}

BOOL WINAPI mainwnd_redraw(VOID)
{
	if(p_mainwnd == NULL) return FALSE;

	return RedrawWindow(p_mainwnd, NULL, NULL, (RDW_ERASE | RDW_FRAME | RDW_INTERNALPAINT | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN));
}

VOID WINAPI childwnd_hide_all(VOID)
{
	SIZE_T n_wnd = 0u;

	for(n_wnd = 0u; n_wnd < PP_CHILDWND_LENGTH; n_wnd++) ShowWindow(pp_childwnd[n_wnd], SW_HIDE);

	return;
}

BOOL WINAPI catch_messages(VOID)
{
	MSG msg;

	while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if(msg.message == WM_QUIT) return FALSE;

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return TRUE;
}

LRESULT CALLBACK mainwnd_wndproc(HWND p_wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch(msg)
	{
		case WM_COMMAND:
			return mainwnd_event_wmcommand(p_wnd, wparam, lparam);

		case WM_DESTROY:
			return mainwnd_event_wmdestroy(p_wnd, wparam, lparam);

		case WM_SIZE:
			return mainwnd_event_wmsize(p_wnd, wparam, lparam);

		case WM_PAINT:
			return mainwnd_event_wmpaint(p_wnd, wparam, lparam);

		case WM_CTLCOLORSTATIC:
			return window_event_wmctlcolorstatic(p_wnd, wparam, lparam);

		case CUSTOM_WM_PLAYBACK_FINISHED:
			thread_stop(&p_audiothread, 0u);
			if(p_audio != NULL)
			{
				delete p_audio;
				p_audio = NULL;
			}

			runtime_status = RUNTIME_STATUS_PLAYBACK_FINISHED;
			return 0;
	}

	return DefWindowProc(p_wnd, msg, wparam, lparam);
}

LRESULT CALLBACK mainwnd_event_wmcommand(HWND p_wnd, WPARAM wparam, LPARAM lparam)
{
	SSIZE_T _ssize;

	if(p_wnd == NULL) return 0;
	if(!lparam) return 0;

	if(((ULONG_PTR) lparam) == ((ULONG_PTR) pp_childwnd[CHILDWNDINDEX_BUTTON1]))
	{
		switch(prev_status)
		{
			case RUNTIME_STATUS_CHOOSEFILE:
				if(choosefile_proc()) runtime_status = RUNTIME_STATUS_CHOOSEAUDIODEV;
				break;

			case RUNTIME_STATUS_PLAYBACK_RUNNING:
				p_audio->stopPlayback();
				break;

			case RUNTIME_STATUS_CHOOSEAUDIODEV:
				if(p_audio != NULL)
				{
					delete p_audio;
					p_audio = NULL;
				}

			case RUNTIME_STATUS_PLAYBACK_FINISHED:

				runtime_status = RUNTIME_STATUS_CHOOSEFILE;
				break;
		}
	}
	else if(((ULONG_PTR) lparam) == ((ULONG_PTR) pp_childwnd[CHILDWNDINDEX_BUTTON2]))
	{
		switch(prev_status)
		{
			case RUNTIME_STATUS_CHOOSEAUDIODEV:
				_ssize = listbox_get_sel_index(pp_childwnd[CHILDWNDINDEX_LISTBOX1]);
				if(_ssize < 0)
				{
					MessageBox(NULL, TEXT("Error: no item selected"), TEXT("ERROR"), (MB_ICONEXCLAMATION | MB_OK));
					break;
				}

				if(!chooseaudiodev_proc((SIZE_T) _ssize, FALSE)) break;
				if(!initaudioobj_proc()) break;

				p_audiothread = thread_create_default(&audiothread_proc, NULL, NULL);
				runtime_status = RUNTIME_STATUS_PLAYBACK_RUNNING;
				break;

			case RUNTIME_STATUS_PLAYBACK_RUNNING:
				attempt_update_ndelay();
				break;
		}
	}
	else if(((ULONG_PTR) lparam) == ((ULONG_PTR) pp_childwnd[CHILDWNDINDEX_BUTTON3]))
	{
		switch(prev_status)
		{
			case RUNTIME_STATUS_CHOOSEAUDIODEV:
				if(!chooseaudiodev_proc(0u, TRUE)) break;
				if(!initaudioobj_proc()) break;

				p_audiothread = thread_create_default(&audiothread_proc, NULL, NULL);
				runtime_status = RUNTIME_STATUS_PLAYBACK_RUNNING;
				break;

			case RUNTIME_STATUS_PLAYBACK_RUNNING:
				attempt_update_nfeedback();
				break;
		}
	}
	else if(((ULONG_PTR) lparam) == ((ULONG_PTR) pp_childwnd[CHILDWNDINDEX_CHECKBOX1])) update_feedbackaltpol();
	else if(((ULONG_PTR) lparam) == ((ULONG_PTR) pp_childwnd[CHILDWNDINDEX_RADIOBUTTON1])) update_cycledivincone();
	else if(((ULONG_PTR) lparam) == ((ULONG_PTR) pp_childwnd[CHILDWNDINDEX_RADIOBUTTON2])) update_cycledivincone();

	return 0;
}

LRESULT CALLBACK mainwnd_event_wmdestroy(HWND p_wnd, WPARAM wparam, LPARAM lparam)
{
	SIZE_T n_wnd = 0u;

	listbox_clear(pp_childwnd[CHILDWNDINDEX_LISTBOX1]);

	p_mainwnd = NULL;
	for(n_wnd = 0u; n_wnd < PP_CHILDWND_LENGTH; n_wnd++) pp_childwnd[n_wnd] = NULL;

	PostQuitMessage(0);
	return 0;
}

LRESULT CALLBACK mainwnd_event_wmsize(HWND p_wnd, WPARAM wparam, LPARAM lparam)
{
	if(p_wnd == NULL) return 0;

	ctrls_setup(TRUE);
	return 0;
}

LRESULT CALLBACK mainwnd_event_wmpaint(HWND p_wnd, WPARAM wparam, LPARAM lparam)
{
	HDC p_wnddc = NULL;
	PAINTSTRUCT ps;

	if(p_wnd == NULL) return 0;

	p_wnddc = BeginPaint(p_wnd, &ps);
	if(p_wnddc == NULL) return 0;

	FillRect(ps.hdc, &ps.rcPaint, pp_brush[MAINWND_BRUSHINDEX]);
	EndPaint(p_wnd, &ps);

	return 0;
}

LRESULT CALLBACK container_wndproc(HWND p_wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch(msg)
	{
		case WM_COMMAND:
			return container_wndproc_fwrdtoparent(p_wnd, msg, wparam, lparam);

		case WM_PAINT:
			return container_event_wmpaint(p_wnd, wparam, lparam);

		case WM_CTLCOLORSTATIC:
			return window_event_wmctlcolorstatic(p_wnd, wparam, lparam);
	}

	return DefWindowProc(p_wnd, msg, wparam, lparam);
}

LRESULT CALLBACK container_wndproc_fwrdtoparent(HWND p_wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	HWND p_parentwnd = NULL;

	if(p_wnd == NULL) return 0;

	p_parentwnd = (HWND) GetWindowLongPtr(p_wnd, GWLP_HWNDPARENT);
	if(p_parentwnd == NULL) return 0;

	PostMessage(p_parentwnd, msg, wparam, lparam);
	return 0;
}

LRESULT CALLBACK container_event_wmpaint(HWND p_wnd, WPARAM wparam, LPARAM lparam)
{
	HDC p_wnddc = NULL;
	PAINTSTRUCT ps;

	if(p_wnd == NULL) return 0;

	p_wnddc = BeginPaint(p_wnd, &ps);
	if(p_wnddc == NULL) return 0;

	FillRect(ps.hdc, &ps.rcPaint, pp_brush[CONTAINERWND_BRUSHINDEX]);
	EndPaint(p_wnd, &ps);

	return 0;
}

LRESULT CALLBACK window_event_wmctlcolorstatic(HWND p_wnd, WPARAM wparam, LPARAM lparam)
{
	if(p_wnd == NULL) return 0;
	if(!wparam) return 0;
	if(!lparam) return 0;

	SetTextColor((HDC) wparam, TEXTWND_TEXTCOLOR);
	SetBkColor((HDC) wparam, TEXTWND_BKCOLOR);

	return (LRESULT) pp_brush[TEXTWND_BRUSHINDEX];
}

BOOL WINAPI listbox_clear(HWND p_listbox)
{
	SIZE_T n_items = 0u;
	SSIZE_T _ssize = 0;

	if(p_listbox == NULL) return FALSE;

	_ssize = listbox_get_item_count(p_listbox);
	if(_ssize < 0) return FALSE;

	n_items = (SIZE_T) _ssize;

	while(n_items > 0u)
	{
		_ssize = listbox_remove_item(p_listbox, (n_items - 1u));
		if(_ssize < 0) return FALSE;

		n_items--;
	}

	return TRUE;
}

SSIZE_T WINAPI listbox_add_item(HWND p_listbox, const TCHAR *text)
{
	if(p_listbox == NULL) return -1;
	if(text == NULL) return -1;

	return (SSIZE_T) SendMessage(p_listbox, LB_ADDSTRING, 0, (LPARAM) text);
}

SSIZE_T WINAPI listbox_remove_item(HWND p_listbox, SIZE_T index)
{
	if(p_listbox == NULL) return -1;

	return (SSIZE_T) SendMessage(p_listbox, LB_DELETESTRING, (WPARAM) index, 0);
}

SSIZE_T WINAPI listbox_get_item_count(HWND p_listbox)
{
	if(p_listbox == NULL) return -1;

	return (SSIZE_T) SendMessage(p_listbox, LB_GETCOUNT, 0, 0);
}

SSIZE_T WINAPI listbox_get_sel_index(HWND p_listbox)
{
	if(p_listbox == NULL) return -1;

	return (SSIZE_T) SendMessage(p_listbox, LB_GETCURSEL, 0, 0);
}

VOID WINAPI fxtext_update(VOID)
{
	audiortdsp_fx_params_t fx_params;

	ShowWindow(pp_childwnd[CHILDWNDINDEX_TEXT2], SW_HIDE);

	p_audio->getFXParams(&fx_params);

	tstr = TEXT("Current Parameters:\r\n\r\n");
	tstr += TEXT("Delay Time: ") + __TOSTRING(fx_params.n_delay) + TEXT(" samples\r\n");
	tstr += TEXT("Feedback Loop Count: ") + __TOSTRING(fx_params.n_feedback) + TEXT("\r\n");
	tstr += TEXT("Feedback Alt. Polarity: ");

	if(fx_params.feedback_alt_pol) tstr += TEXT("Enabled\r\n");
	else tstr += TEXT("Disabled\r\n");

	tstr += TEXT("FB Cycle Divider Increment: ");

	if(fx_params.cyclediv_inc_one) tstr += TEXT("By 1");
	else tstr += TEXT("Exponential");

	SendMessage(pp_childwnd[CHILDWNDINDEX_TEXT2], WM_SETTEXT, 0, (LPARAM) tstr.c_str());
	ShowWindow(pp_childwnd[CHILDWNDINDEX_TEXT2], SW_SHOW);

	mainwnd_redraw();
	return;
}

BOOL WINAPI attempt_update_ndelay(VOID)
{
	INT n_delay = 0;

	SendMessage(pp_childwnd[CHILDWNDINDEX_TEXTBOX1], WM_GETTEXT, (WPARAM) TEXTBUF_SIZE_CHARS, (LPARAM) textbuf);

	try
	{
		n_delay = std::stoi(textbuf);
	}
	catch(...)
	{
		tstr = TEXT("Error: invalid value entered.");
		goto _l_attempt_update_ndelay_error;
	}

	if(n_delay < 0)
	{
		tstr = TEXT("Error: invalid value entered.");
		goto _l_attempt_update_ndelay_error;
	}

	if(!p_audio->setFXDelay((SIZE_T) n_delay))
	{
		tstr = p_audio->getLastErrorMessage();
		goto _l_attempt_update_ndelay_error;
	}

	fxtext_update();
	return TRUE;

_l_attempt_update_ndelay_error:
	MessageBox(NULL, tstr.c_str(), TEXT("ERROR"), (MB_ICONEXCLAMATION | MB_OK));
	return FALSE;
}

BOOL WINAPI attempt_update_nfeedback(VOID)
{
	INT n_feedback = 0;

	SendMessage(pp_childwnd[CHILDWNDINDEX_TEXTBOX2], WM_GETTEXT, (WPARAM) TEXTBUF_SIZE_CHARS, (LPARAM) textbuf);

	try
	{
		n_feedback = std::stoi(textbuf);
	}
	catch(...)
	{
		tstr = TEXT("Error: invalid value entered.");
		goto _l_attempt_update_nfeedback_error;
	}

	if(n_feedback < 0)
	{
		tstr = TEXT("Error: invalid value entered.");
		goto _l_attempt_update_nfeedback_error;
	}

	if(!p_audio->setFXFeedback((SIZE_T) n_feedback))
	{
		tstr = p_audio->getLastErrorMessage();
		goto _l_attempt_update_nfeedback_error;
	}

	fxtext_update();
	return TRUE;

_l_attempt_update_nfeedback_error:
	MessageBox(NULL, tstr.c_str(), TEXT("ERROR"), (MB_ICONEXCLAMATION | MB_OK));
	return FALSE;
}

VOID WINAPI update_feedbackaltpol(VOID)
{
	LRESULT btn_state = 0;

	btn_state = SendMessage(pp_childwnd[CHILDWNDINDEX_CHECKBOX1], BM_GETCHECK, 0, 0);

	if(btn_state == BST_CHECKED) p_audio->enableFeedbackAltPol(TRUE);
	else if(btn_state == BST_UNCHECKED) p_audio->enableFeedbackAltPol(FALSE);

	fxtext_update();
	return;
}

VOID WINAPI update_cycledivincone(VOID)
{
	LRESULT btn_state = 0;

	btn_state = SendMessage(pp_childwnd[CHILDWNDINDEX_RADIOBUTTON1], BM_GETCHECK, 0, 0);
	if(btn_state == BST_CHECKED)
	{
		p_audio->enableCycleDivIncOne(TRUE);
		goto _l_update_cycledivincone_updated;
	}

	btn_state = SendMessage(pp_childwnd[CHILDWNDINDEX_RADIOBUTTON2], BM_GETCHECK, 0, 0);
	if(btn_state == BST_CHECKED)
	{
		p_audio->enableCycleDivIncOne(FALSE);
		goto _l_update_cycledivincone_updated;
	}

	return;

_l_update_cycledivincone_updated:
	fxtext_update();
	return;
}

VOID WINAPI preload_ui_state_from_dsp(VOID)
{
	audiortdsp_fx_params_t fx_params;

	p_audio->getFXParams(&fx_params);

	if(fx_params.feedback_alt_pol) SendMessage(pp_childwnd[CHILDWNDINDEX_CHECKBOX1], BM_SETCHECK, (WPARAM) BST_CHECKED, 0);
	else SendMessage(pp_childwnd[CHILDWNDINDEX_CHECKBOX1], BM_SETCHECK, (WPARAM) BST_UNCHECKED, 0);

	if(fx_params.cyclediv_inc_one)
	{
		SendMessage(pp_childwnd[CHILDWNDINDEX_RADIOBUTTON2], BM_SETCHECK, (WPARAM) BST_UNCHECKED, 0);
		SendMessage(pp_childwnd[CHILDWNDINDEX_RADIOBUTTON1], BM_SETCHECK, (WPARAM) BST_CHECKED, 0);
	}
	else
	{
		SendMessage(pp_childwnd[CHILDWNDINDEX_RADIOBUTTON1], BM_SETCHECK, (WPARAM) BST_UNCHECKED, 0);
		SendMessage(pp_childwnd[CHILDWNDINDEX_RADIOBUTTON2], BM_SETCHECK, (WPARAM) BST_CHECKED, 0);
	}

	fxtext_update();
	return;
}

BOOL WINAPI choosefile_proc(VOID)
{
	SIZE_T textlen = 0u;
	INT n32 = 0;
	OPENFILENAME ofdlg;
	const TCHAR *filters = TEXT("Wave Files\0*.wav;*.WAV\0All Files\0*.*\0\0");

	ZeroMemory(&ofdlg, sizeof(OPENFILENAME));
	ZeroMemory(textbuf, TEXTBUF_SIZE_BYTES);

	ofdlg.lStructSize = sizeof(OPENFILENAME);
	ofdlg.hwndOwner = p_mainwnd;
	ofdlg.lpstrFilter = filters;
	ofdlg.nFilterIndex = 1;
	ofdlg.lpstrFile = textbuf;
	ofdlg.nMaxFile = TEXTBUF_SIZE_CHARS;
	ofdlg.Flags = (OFN_EXPLORER | OFN_ENABLESIZING);
	ofdlg.lpstrDefExt = TEXT(".wav");

	if(!GetOpenFileName(&ofdlg))
	{
		tstr = TEXT("Error: Open File Dialog Failed.");
		goto _l_choosefile_proc_error;
	}

	tstr = textbuf;
	cstr_tolower(textbuf, TEXTBUF_SIZE_CHARS);
	textlen = (SIZE_T) cstr_getlength(textbuf);

	if(textlen >= 5u)
		if(cstr_compare(TEXT(".wav"), &textbuf[textlen - 4u]))
			goto _l_choosefile_proc_fileextcheck_complete;

	n32 = MessageBox(NULL, TEXT("WARNING: Selected file does not have a \".wav\" file extension.\r\nMight be incompatible with this application.\r\nDo you wish to continue?"), TEXT("WARNING: FILE EXTENSION"), (MB_ICONEXCLAMATION | MB_YESNO));

	if(n32 != IDYES)
	{
		tstr = TEXT("Error: Bad File Extension.");
		goto _l_choosefile_proc_error;
	}

_l_choosefile_proc_fileextcheck_complete:

	if(!filein_open(tstr.c_str()))
	{
		tstr = TEXT("Error: could not open file.");
		goto _l_choosefile_proc_error;
	}

	n32 = filein_get_params();
	if(n32 < 0) goto _l_choosefile_proc_error;

	if(p_audio != NULL)
	{
		delete p_audio;
		p_audio = NULL;
	}

	pb_params.file_dir = tstr.c_str();

	switch(n32)
	{
		case PB_I16:
			p_audio = new AudioRTDSP_i16(&pb_params);
			break;

		case PB_I24:
			p_audio = new AudioRTDSP_i24(&pb_params);
			break;
	}

	if(p_audio != NULL) return TRUE;

	tstr = TEXT("Error: Failed to create audio object instance.");

_l_choosefile_proc_error:
	filein_close();
	MessageBox(NULL, tstr.c_str(), TEXT("ERROR"), (MB_ICONEXCLAMATION | MB_OK));
	return FALSE;
}

BOOL WINAPI chooseaudiodev_proc(SIZE_T sel_index, BOOL dev_default)
{
	if(p_audio == NULL) return FALSE;

	if(dev_default) goto _l_chooseaudiodev_proc_default;

_l_chooseaudiodev_proc_index:
	if(p_audio->chooseDevice(sel_index)) return TRUE;

	goto _l_chooseaudiodev_proc_error;

_l_chooseaudiodev_proc_default:
	if(p_audio->chooseDefaultDevice()) return TRUE;

_l_chooseaudiodev_proc_error:
	tstr = TEXT("Error: failed to access audio device\r\nExtended error message: ");
	tstr += p_audio->getLastErrorMessage();

	MessageBox(NULL, tstr.c_str(), TEXT("ERROR"), (MB_ICONEXCLAMATION | MB_OK));
	return FALSE;
}

BOOL WINAPI initaudioobj_proc(VOID)
{
	if(p_audio == NULL) return FALSE;

	if(p_audio->initialize()) return TRUE;

	tstr = TEXT("Error: failed to initialize audio object\r\nExtended error message: ");
	tstr += p_audio->getLastErrorMessage();

	MessageBox(NULL, tstr.c_str(), TEXT("ERROR"), (MB_ICONEXCLAMATION | MB_OK));
	return FALSE;
}

BOOL WINAPI filein_open(const TCHAR *filein_dir)
{
	if(filein_dir == NULL) return FALSE;

	filein_close(); /*Close any existing handles*/

	h_filein = CreateFile(filein_dir, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0u, NULL);

	return (h_filein != INVALID_HANDLE_VALUE);
}

VOID WINAPI filein_close(VOID)
{
	if(h_filein == INVALID_HANDLE_VALUE) return;

	CloseHandle(h_filein);
	h_filein = INVALID_HANDLE_VALUE;
	return;
}

INT WINAPI filein_get_params(VOID)
{
	const SIZE_T BUFFER_SIZE = 4096u;
	SIZE_T buffer_index = 0u;

	UINT8 *p_headerinfo = NULL;

	DWORD dummy_32;

	UINT32 u32 = 0u;
	UINT16 u16 = 0u;

	UINT16 bit_depth = 0u;

	p_headerinfo = (UINT8*) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, BUFFER_SIZE);
	if(p_headerinfo == NULL)
	{
		tstr = TEXT("Error: memory allocate failed.");
		goto _l_filein_get_params_error;
	}

	SetFilePointer(h_filein, 0, NULL, FILE_BEGIN);
	ReadFile(h_filein, p_headerinfo, (DWORD) BUFFER_SIZE, &dummy_32, NULL);
	filein_close();

	if(!compare_signature("RIFF", (const CHAR*) p_headerinfo))
	{
		tstr = TEXT("Error: file format not supported.");
		goto _l_filein_get_params_error;
	}

	if(!compare_signature("WAVE", (const CHAR*) (((SIZE_T) p_headerinfo) + 8u)))
	{
		tstr = TEXT("Error: file format not supported.");
		goto _l_filein_get_params_error;
	}

	buffer_index = 12u;

	while(TRUE)
	{
		if(buffer_index > (BUFFER_SIZE - 8u))
		{
			tstr = TEXT("Error: broken header (missing subchunk \"fmt \").\r\nFile probably corrupted.");
			goto _l_filein_get_params_error;
		}

		if(compare_signature("fmt ", (const CHAR*) (((SIZE_T) p_headerinfo) + buffer_index))) break;

		u32 = *((UINT32*) (((SIZE_T) p_headerinfo) + buffer_index + 4u));
		buffer_index += (SIZE_T) (u32 + 8u);
	}

	if(buffer_index > (BUFFER_SIZE - 24u))
	{
		tstr = TEXT("Error: broken header (error on subchunk \"fmt \").\r\nFile probably corrupted.");
		goto _l_filein_get_params_error;
	}

	u16 = *((UINT16*) (((SIZE_T) p_headerinfo) + buffer_index + 8u));

	if(u16 != 1u)
	{
		tstr = TEXT("Error: audio encoding format not supported.");
		goto _l_filein_get_params_error;
	}

	pb_params.n_channels = *((UINT16*) (((SIZE_T) p_headerinfo) + buffer_index + 10u));
	pb_params.sample_rate = *((UINT32*) (((SIZE_T) p_headerinfo) + buffer_index + 12u));
	bit_depth = *((UINT16*) (((SIZE_T) p_headerinfo) + buffer_index + 22u));

	u32 = *((UINT32*) (((SIZE_T) p_headerinfo) + buffer_index + 4u));
	buffer_index += (SIZE_T) (u32 + 8u);

	while(TRUE)
	{
		if(buffer_index > (BUFFER_SIZE - 8u))
		{
			tstr = TEXT("Error: broken header (missing subchunk \"data\").\r\nFile probably corrupted.");
			goto _l_filein_get_params_error;
		}

		if(compare_signature("data", (const CHAR*) (((SIZE_T) p_headerinfo) + buffer_index))) break;

		u32 = *((UINT32*) (((SIZE_T) p_headerinfo) + buffer_index + 4u));
		buffer_index += (SIZE_T) (u32 + 8u);
	}

	u32 = *((UINT32*) (((SIZE_T) p_headerinfo) + buffer_index + 4u));

	pb_params.audio_data_begin = (ULONG64) (buffer_index + 8u);
	pb_params.audio_data_end = pb_params.audio_data_begin + ((ULONG64) u32);

	HeapFree(p_processheap, 0u, p_headerinfo);

	p_headerinfo = NULL;

	switch(bit_depth)
	{
		case 16u:
			return PB_I16;

		case 24u:
			return PB_I24;
	}

	tstr = TEXT("Error: audio format not supported.");

_l_filein_get_params_error:
	filein_close();
	if(p_headerinfo != NULL) HeapFree(p_processheap, 0u, p_headerinfo);
	return -1;
}

BOOL WINAPI compare_signature(const CHAR *auth, const CHAR *buf)
{
	if(auth == NULL) return FALSE;
	if(buf == NULL) return FALSE;

	if(auth[0] != buf[0]) return FALSE;
	if(auth[1] != buf[1]) return FALSE;
	if(auth[2] != buf[2]) return FALSE;
	if(auth[3] != buf[3]) return FALSE;

	return TRUE;
}

DWORD WINAPI audiothread_proc(VOID *p_args)
{
	p_audio->runPlayback();

	PostMessage(p_mainwnd, CUSTOM_WM_PLAYBACK_FINISHED, 0, 0);
	return 0u;
}
