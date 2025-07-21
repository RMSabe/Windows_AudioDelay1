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

/*
	These are generic create thread/wait thread/stop thread functions for Windows threads.

	Thread handles/processes in these functions use the CreateThread(), TerminateThread(), WaitForSingleObject() Windows functions.

	https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createthread
	https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-terminatethread
	https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitforsingleobject
*/

#ifndef THREAD_H
#define THREAD_H

#include "globldef.h"

/*This would be equivalent to pthread_create() in pthread.h
Create a new thread with default settings. 
For more customized settings, look over CreateThread() documentation.
p_thread_start_routine is a pointer to the thread procedure function.
Note: Unlike pthread.h which uses "void* (*)(void*)", CreateThread uses "DWORD (WINAPI*)(VOID*)".
p_args is a pointer to the function argument. (This parameter can be NULL).
p_threadid is a pointer to the thread_id variable. (This parameter can be NULL).
Function returns NULL if it fails, thread handle otherwise.*/

extern HANDLE WINAPI thread_create_default(DWORD (WINAPI *p_thread_start_routine)(VOID*), VOID *p_args, DWORD *p_threadid);

/*This would be equivalent to pthread_join() in pthread.h
Pause main thread's execution, wait for the given thread to finish it's process
Returns TRUE if successful, FALSE otherwise*/

extern BOOL WINAPI thread_wait(HANDLE *pp_thread);

/*This would be equivalent to pthread_cancel() in pthread.h
Force Quit the given thread's process
Returns TRUE if successful, FALSE otherwise*/

extern BOOL WINAPI thread_stop(HANDLE *pp_thread, DWORD exit_code);

#endif /*THREAD_H*/
