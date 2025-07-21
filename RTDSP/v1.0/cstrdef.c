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

#include "cstrdef.h"

BOOL WINAPI cstr_compare(const TCHAR *str1, const TCHAR *str2)
{
	SIZE_T len1 = 0u;
	SIZE_T len2 = 0u;

	if(str1 == NULL) return FALSE;
	if(str2 == NULL) return FALSE;

	len1 = (SIZE_T) cstr_getlength(str1);
	len2 = (SIZE_T) cstr_getlength(str2);

	if(len1 != len2) return FALSE;

	for(len1 = 0u; len1 < len2; len1++) if(str1[len1] != str2[len1]) return FALSE;

	return TRUE;
}

BOOL WINAPI cstr_copy(const TCHAR *input_str, TCHAR *output_str, SIZE_T bufferout_length)
{
	SIZE_T input_len = 0u;
	SIZE_T stop_index = 0u;
	SIZE_T n_char = 0u;

	if(input_str == NULL) return FALSE;
	if(output_str == NULL) return FALSE;
	if(bufferout_length < 1u) return FALSE;

	input_len = (SIZE_T) cstr_getlength(input_str);

	if(input_len < bufferout_length) stop_index = input_len;
	else stop_index = bufferout_length - 1u;

	for(n_char = 0u; n_char < stop_index; n_char++) output_str[n_char] = input_str[n_char];

	output_str[stop_index] = (TCHAR) '\0';
	return (input_len < bufferout_length);
}

BOOL WINAPI cstr_tolower(TCHAR *str, SIZE_T buffer_length)
{
	SIZE_T len = 0u;
	SIZE_T nchar = 0u;

	if(str == NULL) return FALSE;
	if(buffer_length < 1u) return FALSE;

	str[buffer_length - 1u] = (TCHAR) '\0';

	len = (SIZE_T) cstr_getlength(str);

	for(nchar = 0u; nchar < len; nchar++)
		if((str[nchar] >= 0x41) && (str[nchar] <= 0x5a)) 
			str[nchar] = (TCHAR) (((UINT) str[nchar]) | 0x20);

	return TRUE;
}

BOOL WINAPI cstr_toupper(TCHAR *str, SIZE_T buffer_length)
{
	SIZE_T len = 0u;
	SIZE_T nchar = 0u;

	if(str == NULL) return FALSE;
	if(buffer_length < 1u) return FALSE;

	str[buffer_length - 1u] = (TCHAR) '\0';

	len = (SIZE_T) cstr_getlength(str);

	for(nchar = 0u; nchar < len; nchar++)
		if((str[nchar] >= 0x61) && (str[nchar] <= 0x7a)) 
			str[nchar] = (TCHAR) (((UINT) str[nchar]) & 0xdf);

	return TRUE;
}

SSIZE_T WINAPI cstr_getlength(const TCHAR *str)
{
	SIZE_T len = 0u;

	if(str == NULL) return -1;

	while(str[len] != '\0') len++;

	return (SSIZE_T) len;
}

BOOL WINAPI _cstr_str8_to_str16(const CHAR *str8, WCHAR *str16, SIZE_T bufferout_length)
{
	SIZE_T input_len = 0u;
	SIZE_T stop_index = 0u;
	SIZE_T n_char = 0u;

	if(str8 == NULL) return FALSE;
	if(str16 == NULL) return FALSE;
	if(bufferout_length < 1u) return FALSE;

	while(str8[input_len] != '\0') input_len++;

	if(input_len < bufferout_length) stop_index = input_len;
	else stop_index = bufferout_length - 1u;

	for(n_char = 0u; n_char < stop_index; n_char++) str16[n_char] = (WCHAR) str8[n_char];

	str16[stop_index] = '\0';

	return (input_len < bufferout_length);
}

BOOL WINAPI _cstr_str16_to_str8(const WCHAR *str16, CHAR *str8, SIZE_T bufferout_length)
{
	SIZE_T input_len = 0u;
	SIZE_T stop_index = 0u;
	SIZE_T n_char = 0u;

	if(str16 == NULL) return FALSE;
	if(str8 == NULL) return FALSE;
	if(bufferout_length < 1u) return FALSE;

	while(str16[input_len] != '\0') input_len++;

	if(input_len < bufferout_length) stop_index = input_len;
	else stop_index = bufferout_length - 1u;

	for(n_char = 0u; n_char < stop_index; n_char++) str8[n_char] = (CHAR) str16[n_char];

	str8[stop_index] = '\0';

	return (input_len < bufferout_length);
}

BOOL WINAPI cstr_char_to_tchar(const CHAR *str8, TCHAR *str, SIZE_T bufferout_length)
{

#ifdef __USE_UTF16
	return _cstr_str8_to_str16(str8, (WCHAR*) str, bufferout_length);
#else
	return cstr_copy((const TCHAR*) str8, str, bufferout_length);
#endif

}

BOOL WINAPI cstr_wchar_to_tchar(const WCHAR *str16, TCHAR *str, SIZE_T bufferout_length)
{

#ifdef __USE_UTF16
	return cstr_copy((const TCHAR*) str16, str, bufferout_length);
#else
	return _cstr_str16_to_str8(str16, (CHAR*) str, bufferout_length);
#endif

}

BOOL WINAPI cstr_tchar_to_char(const TCHAR *str, CHAR *str8, SIZE_T bufferout_length)
{

#ifdef __USE_UTF16
	return _cstr_str16_to_str8((const WCHAR*) str, str8, bufferout_length);
#else
	return cstr_copy(str, (TCHAR*) str8, bufferout_length);
#endif

}

BOOL WINAPI cstr_tchar_to_wchar(const TCHAR *str, WCHAR *str16, SIZE_T bufferout_length)
{

#ifdef __USE_UTF16
	return cstr_copy(str, (TCHAR*) str16, bufferout_length);
#else
	return _cstr_str8_to_str16((const CHAR*) str, str16, bufferout_length);
#endif

}

