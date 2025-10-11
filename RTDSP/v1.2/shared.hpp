/*
	Audio Real-Time Delay for Windows
	Version 1.2

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#ifndef SHARED_HPP
#define SHARED_HPP

#include "globldef.h"
#include <initguid.h>

/*
PKEY_Device_FriendlyName should've been defined somewhere in a header called functiondiscoverykeys_devpkey.h
Since this header doesn't exist in my version of MinGW64, I'm defining it here...

All values used can be found in the functiondiscoverykeys_devpkey.h file:
https://github.com/EddieRingle/portaudio/blob/master/src/hostapi/wasapi/mingw-include/FunctionDiscoveryKeys_devpkey.h

Further Reference:
https://learn.microsoft.com/en-us/windows/win32/api/guiddef/ns-guiddef-guid
https://learn.microsoft.com/en-us/windows/win32/api/wtypes/ns-wtypes-propertykey

const PROPERTYKEY PKEY_Device_FriendlyName = {
	.fmtid = {
		.Data1 = 0xa45c254e,
		.Data2 = 0xdf1c,
		.Data3 = 0x4efd,
		.Data4[0] = 0x80,
		.Data4[1] = 0x20,
		.Data4[2] = 0x67,
		.Data4[3] = 0xd1,
		.Data4[4] = 0x46,
		.Data4[5] = 0xa8,
		.Data4[6] = 0x50,
		.Data4[7] = 0xe0
	},
	.pid = 14u
};
*/

const ULONG32 P_PKEY_Device_FriendlyName[] = {0xa45c254e, 0x4efddf1c, 0xd1672080, 0xe050a846, 14u};

struct _fileptr64 {
	ULONG32 l32;
	ULONG32 h32;
};

typedef struct _fileptr64 fileptr64_t;

static inline INT debug_msgbox(const TCHAR *text, UINT type)
{
	return MessageBox(NULL, text, TEXT("DEBUG"), type);
}

extern __declspec(noreturn) VOID WINAPI app_exit(UINT exit_code, const TCHAR *exit_msg);

extern BOOL WINAPI listbox_clear(HWND p_listbox);
extern SSIZE_T WINAPI listbox_add_item(HWND p_listbox, const TCHAR *text);
extern SSIZE_T WINAPI listbox_remove_item(HWND p_listbox, SIZE_T index);
extern SSIZE_T WINAPI listbox_get_item_count(HWND p_listbox);
extern SSIZE_T WINAPI listbox_get_sel_index(HWND p_listbox);

#endif /*SHARED_HPP*/
