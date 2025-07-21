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

#include "thread.h"

HANDLE WINAPI thread_create_default(DWORD (WINAPI *p_thread_start_routine)(VOID*), VOID *p_args, DWORD *p_threadid)
{
	if(p_thread_start_routine == NULL) return NULL;

	return CreateThread(NULL, 0u, (LPTHREAD_START_ROUTINE) p_thread_start_routine, p_args, 0u, p_threadid);
}

BOOL WINAPI thread_wait(HANDLE *pp_thread)
{
	INT32 n_ret = 0;

	if(pp_thread == NULL) return FALSE;
	if(*pp_thread == NULL) return FALSE;

	n_ret = (INT32) WaitForSingleObject(*pp_thread, INFINITE);

	if(n_ret < 0) return FALSE;

	*pp_thread = NULL;
	return TRUE;
}

BOOL WINAPI thread_stop(HANDLE *pp_thread, DWORD exit_code)
{
	if(pp_thread == NULL) return FALSE;
	if(*pp_thread == NULL) return FALSE;
	if(!TerminateThread(*pp_thread, exit_code)) return FALSE;

	*pp_thread = NULL;
	return TRUE;
}
