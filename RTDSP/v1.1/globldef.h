/*
	These are some definitions to ease application development on Windows systems.
	Version 3.4

	config.h is the macro configuration file, where all macro code settings are applied.
	Settings like text format, target system and so on are set in config.h.

	globldef.h is the global definition file. It defines/undefines macros based on the definitions in config.h file.
	It should be the first #include in all subsequent header and source files.

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com

	GitHub Repository: https://github.com/RMSabe/WinLib
*/

#ifndef GLOBLDEF_H
#define GLOBLDEF_H

#include "config.h"

#ifdef __DOSWIN
#ifdef __NTWIN
#error "__DOSWIN and __NTWIN must not be defined simultaneously"
#endif
#endif

#ifndef __DOSWIN
#ifndef __NTWIN
#define __NTWIN
#endif
#endif

#ifdef __DOSWIN
#ifdef _WIN64
#error "DOS based Windows does not support 64 bit applications"
#endif
#ifdef __USE_UTF16
#error "DOS based Windows does not support UNICODE UTF-16 text format"
#endif
#endif

#ifdef __USE_UTF16
#ifndef UNICODE
#define UNICODE
#endif
#else
#ifdef UNICODE
#undef UNICODE
#endif
#endif

#include <windows.h>

#define TEXTBUF_SIZE_BYTES (TEXTBUF_SIZE_CHARS*sizeof(TCHAR))

extern HINSTANCE p_instance;
extern HANDLE p_processheap;

extern TCHAR textbuf[];

/* Get the closest power of 2 value from an input value 
_round() will round the result
_floor() will return the closest power of 2 below the input value
_ceil() will return the closest power of 2 above the input value 
These functions are intended primarily for size of buffer allocations (SIZE_T), (ULONG_PTR), (UINT_PTR) 
If the function fails or if input == 0, it returns 0 */

extern SIZE_T WINAPI _get_closest_power2_round(SIZE_T input);
extern SIZE_T WINAPI _get_closest_power2_floor(SIZE_T input);
extern SIZE_T WINAPI _get_closest_power2_ceil(SIZE_T input);

/* Check if input is a power of 2 */

extern BOOL WINAPI _is_power2(SIZE_T input);

/* Returns the most significant bit value of SIZE_T type. (SIZE_T is the same as ULONG_PTR and UINT_PTR) */

extern SIZE_T WINAPI _get_sizet_msb_value(VOID);

#endif /*GLOBLDEF_H*/
