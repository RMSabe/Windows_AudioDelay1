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

#ifndef CONFIG_H
#define CONFIG_H

/*======================================================================================*/
/*Target Platform Definition
define __DOSWIN to target a DOS based Windows system
define __NTWIN to target a NT based Windows system
if neither is defined, __NTWIN will be defined by default*/

/*#define __DOSWIN*/

#define __NTWIN

/*======================================================================================*/
/*Text Definition
Define __USE_UTF16 to use UNICODE UTF-16 text format (NT based Windows Only)
If not defined, ANSI UTF-8 text format will be used*/

#define __USE_UTF16

/*======================================================================================*/
/*Text Buffer (textbuf) Length
Define the size (in number of characters) of the global TCHAR[] textbuf buffer.*/

#define TEXTBUF_SIZE_CHARS 1024U

#endif /*CONFIG_H*/
