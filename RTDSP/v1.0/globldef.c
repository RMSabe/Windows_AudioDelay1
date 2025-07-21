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

#include "globldef.h"

HINSTANCE p_instance = NULL;
HANDLE p_processheap = NULL;

TCHAR textbuf[TEXTBUF_SIZE_CHARS] = {'\0'};

SIZE_T WINAPI _get_closest_power2_round(SIZE_T input)
{
	SIZE_T _limit = 0u;
	SIZE_T _max = 0u;
	SIZE_T _size1 = 0u;
	SIZE_T _size2 = 0u;

	if(!input) return 0u;

	if(_is_power2(input)) return input;

	_limit = _get_sizet_msb_value();

	_max = 0u;
	_max = ~_max;

	if(input > _limit)
	{
		_size2 = _max - input;
		_size1 = input - _limit;

		if(_size1 < _size2) return _limit;
		return 0u;
	}

	_size2 = _get_closest_power2_ceil(input);

	_size1 = (_size2 >> 1);

	if((input - _size1) < (_size2 - input)) return _size1;

	return _size2;
}

SIZE_T WINAPI _get_closest_power2_floor(SIZE_T input)
{
	SIZE_T _limit = 0u;
	SIZE_T _size = 0u;

	if(!input) return 0u;

	if(_is_power2(input)) return input;

	_limit = _get_sizet_msb_value();

	if(input > _limit) return _limit;

	_size = _get_closest_power2_ceil(input);
	_size = (_size >> 1);

	return _size;
}

SIZE_T WINAPI _get_closest_power2_ceil(SIZE_T input)
{
	SIZE_T _limit = 0u;
	SIZE_T _size = 0u;

	if(!input) return 0u;

	if(_is_power2(input)) return input;

	_limit = _get_sizet_msb_value();

	if(input > _limit) return 0u;

	_size = 1u;

	while(TRUE)
	{
		if(!(input/_size)) break;
		_size = (_size << 1);
	}

	return _size;
}

BOOL WINAPI _is_power2(SIZE_T input)
{
	SIZE_T _limit = 0u;
	SIZE_T _size = 0u;

	if(!input) return FALSE;

	_limit = _get_sizet_msb_value();

	if(input == _limit) return TRUE;
	if(input > _limit) return FALSE;

	_size = 1u;

	while(_size < _limit)
	{
		if(input == _size) return TRUE;
		_size = (_size << 1);
	}

	return FALSE;
}

SIZE_T WINAPI _get_sizet_msb_value(VOID)
{
	SIZE_T _size = 0u;

	_size = 0u;		/*_size = 0x000...*/
	_size = ~_size;		/*_size = 0xfff...*/
	_size = (_size >> 1);	/*_size = 0x7ff...*/
	_size = ~_size;		/*_size = 0x800...*/

	return _size;
}
