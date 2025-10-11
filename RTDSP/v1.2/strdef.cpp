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

#include "strdef.hpp"

__string WINAPI str_tolower(__string input)
{
	__string output = TEXT("");
	INT nchar = 0;
	INT len = 0;

	len = input.length();

	for(nchar = 0; nchar < len; nchar++)
	{
		if((input[nchar] >= 0x41) && (input[nchar] <= 0x5a)) output += (TCHAR) (((UINT) input[nchar]) | 0x20);
		else output += input[nchar];
	}

	return output;
}

__string WINAPI str_toupper(__string input)
{
	__string output = TEXT("");
	INT nchar = 0;
	INT len = 0;

	len = input.length();

	for(nchar = 0; nchar < len; nchar++)
	{
		if((input[nchar] >= 0x61) && (input[nchar] <= 0x7a)) output += (TCHAR) (((UINT) input[nchar]) & 0xdf);
		else output += input[nchar];
	}

	return output;
}
