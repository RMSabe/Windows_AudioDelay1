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

#ifndef CSTRDEF_H
#define CSTRDEF_H

#include "globldef.h"
#include <stdio.h>

#ifdef __USE_UTF16
#define __SPRINTF(output, size, input, ...) swprintf(output, size, input, ## __VA_ARGS__)
#define __CSTRTOINT32(str) _wtol(str)
#define __CSTRTOINT64(str) _wtoi64(str)
#define __CSTRTODOUBLE(str) _wtof(str)
#else
#define __SPRINTF(output, size, input, ...) snprintf(output, size, input, ## __VA_ARGS__)
#define __CSTRTOINT32(str) atol(str)
#define __CSTRTODOUBLE(str) atof(str)
#ifdef __NTWIN
#define __CSTRTOINT64(str) _atoi64(str)
#endif
#endif

/*compare two strings.
returns TRUE if they're equal, FALSE otherwise.*/

extern BOOL WINAPI cstr_compare(const TCHAR *str1, const TCHAR *str2);

/*Copy an input string to an output buffer.
Returns TRUE if copy is successful.
Returns FALSE if copy failed or input string is bigger than output buffer*/

extern BOOL WINAPI cstr_copy(const TCHAR *input_str, TCHAR *output_str, SIZE_T bufferout_length);

/*converts all characters to upper/lower case.
returns TRUE if successful, FALSE otherwise.
Might not work too well with some specific UTF-16 characters.*/

extern BOOL WINAPI cstr_tolower(TCHAR *str, SIZE_T buffer_length);
extern BOOL WINAPI cstr_toupper(TCHAR *str, SIZE_T buffer_length);

/*get string length.
returns length of string or -1 if failed.*/

extern SSIZE_T WINAPI cstr_getlength(const TCHAR *str);

/*Converts all characters from input format to output format (CHAR, WCHAR, TCHAR).
If output format is smaller than input format, the higher bits will be truncated.
Returns TRUE if conversion is successful
Returns FALSE if conversion fails, or if input string is longer than output buffer.*/

extern BOOL WINAPI cstr_char_to_tchar(const CHAR *str8, TCHAR *str, SIZE_T bufferout_length);
extern BOOL WINAPI cstr_wchar_to_tchar(const WCHAR *str16, TCHAR *str, SIZE_T bufferout_length);
extern BOOL WINAPI cstr_tchar_to_char(const TCHAR *str, CHAR *str8, SIZE_T bufferout_length);
extern BOOL WINAPI cstr_tchar_to_wchar(const TCHAR *str, WCHAR *str16, SIZE_T bufferout_length);

#endif /*CSTRDEF_H*/
