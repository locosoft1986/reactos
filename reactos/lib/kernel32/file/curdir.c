/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS system libraries
 * FILE:            lib/kernel32/file/curdir.c
 * PURPOSE:         Current directory functions
 * PROGRAMMER:      David Welch (welch@mcmail.com)
 * UPDATE HISTORY:
 *                  Created 30/09/98
 */


/* INCLUDES ******************************************************************/

#include <windows.h>

/* GLOBALS *******************************************************************/


WCHAR CurrentDirectoryW[MAX_PATH] = {0,};

WCHAR SystemDirectoryW[MAX_PATH];

WCHAR WindowsDirectoryW[MAX_PATH];

/* FUNCTIONS *****************************************************************/
 
DWORD STDCALL GetCurrentDirectoryA(DWORD nBufferLength, LPSTR lpBuffer)
{
   UINT uSize,i;
   if ( lpBuffer == NULL ) 
	return 0;
   uSize = lstrlenW(CurrentDirectoryW); 
   if ( nBufferLength > uSize ) {
	i = 0;
   	while ((CurrentDirectoryW[i])!=0 && i < MAX_PATH)
     	{
		lpBuffer[i] = (unsigned char)CurrentDirectoryW[i];
		i++;
     	}
   	lpBuffer[i] = 0;
   }
   return uSize;
}

DWORD STDCALL GetCurrentDirectoryW(DWORD nBufferLength, LPWSTR lpBuffer)
{
   UINT uSize;
   
   dprintf("CurrentDirectoryW %w\n",CurrentDirectoryW);
   
   if ( lpBuffer == NULL ) 
	return 0;
   uSize = lstrlenW(CurrentDirectoryW); 
   if ( nBufferLength > uSize )
   	lstrcpynW(lpBuffer,CurrentDirectoryW,uSize);
   
   dprintf("GetCurrentDirectoryW() = %w\n",lpBuffer);
   
   return uSize;
}

BOOL STDCALL SetCurrentDirectoryA(LPCSTR lpPathName)
{
   UINT i;

   dprintf("SetCurrentDirectoryA(lpPathName %s)\n",lpPathName);
   
   if ( lpPathName == NULL )
	return FALSE;
   if ( lstrlenA(lpPathName) > MAX_PATH )
	return FALSE;
   i = 0;
   while ((lpPathName[i])!=0 && i < MAX_PATH)
   {
	CurrentDirectoryW[i] = (unsigned short)lpPathName[i];
	i++;
   }
   CurrentDirectoryW[i] = 0;
   
   dprintf("CurrentDirectoryW = '%w'\n",CurrentDirectoryW);
   
   return(TRUE);
}


WINBOOL
STDCALL
SetCurrentDirectoryW(
    LPCWSTR lpPathName
    )
{
   if ( lpPathName == NULL )
	return FALSE;
   if ( lstrlenW(lpPathName) > MAX_PATH )
	return FALSE;
   lstrcpyW(CurrentDirectoryW,lpPathName);
   return(TRUE);
}

DWORD
STDCALL
GetTempPathA(
    DWORD nBufferLength,
    LPSTR lpBuffer
    )
{
	WCHAR BufferW[MAX_PATH];
	DWORD retCode;
	UINT i;
	retCode = GetTempPathW(nBufferLength,BufferW);
	i = 0;
   	while ((BufferW[i])!=0 && i < MAX_PATH)
     	{
		lpBuffer[i] = (unsigned char)BufferW[i];
		i++;
     	}
   	lpBuffer[i] = 0;
	return retCode;
	
}

DWORD
STDCALL
GetTempPathW(
    DWORD nBufferLength,
    LPWSTR lpBuffer
    )
{

	WCHAR EnvironmentBufferW[MAX_PATH];
	UINT i;

	EnvironmentBufferW[0] = 0;
	i = GetEnvironmentVariableW(L"TMP",EnvironmentBufferW,MAX_PATH);
	if ( i==0 )
		i = GetEnvironmentVariableW(L"TEMP",EnvironmentBufferW,MAX_PATH);
		if ( i==0 )
			i = GetCurrentDirectoryW(MAX_PATH,EnvironmentBufferW);

	return i;
}

UINT
STDCALL
GetSystemDirectoryA(
    LPSTR lpBuffer,
    UINT uSize
    )
{
   UINT uPathSize,i;
   if ( lpBuffer == NULL ) 
	return 0;
   uPathSize = lstrlenW(SystemDirectoryW); 
   if ( uSize > uPathSize ) {
   	i = 0;
   	while ((SystemDirectoryW[i])!=0 && i < uSize)
     	{
		lpBuffer[i] = (unsigned char)SystemDirectoryW[i];
		i++;
     	}
   	lpBuffer[i] = 0;
   }
   
   return uPathSize;
}

UINT
STDCALL
GetWindowsDirectoryA(
    LPSTR lpBuffer,
    UINT uSize
    )
{
   UINT uPathSize,i;
   if ( lpBuffer == NULL ) 
	return 0;
   uPathSize = lstrlenW(WindowsDirectoryW); 
   if ( uSize > uPathSize ) {
   	i = 0;
   	while ((WindowsDirectoryW[i])!=0 && i < uSize)
     	{
		lpBuffer[i] = (unsigned char)WindowsDirectoryW[i];
		i++;
     	}
   	lpBuffer[i] = 0;
   }
   return uPathSize;
}

UINT
STDCALL
GetSystemDirectoryW(
    LPWSTR lpBuffer,
    UINT uSize
    )
{
   UINT uPathSize;
   if ( lpBuffer == NULL ) 
	return 0;
   uPathSize = lstrlenW(SystemDirectoryW); 
   if ( uSize > uPathSize ) 
   	lstrcpynW(lpBuffer,SystemDirectoryW,uPathSize);
   
   
   return uPathSize;
}

UINT
STDCALL
GetWindowsDirectoryW(
    LPWSTR lpBuffer,
    UINT uSize
    )
{
   UINT uPathSize;
   if ( lpBuffer == NULL ) 
	return 0;
   uPathSize = lstrlenW(WindowsDirectoryW); 
   if ( uSize > uPathSize );
   	lstrcpynW(lpBuffer,WindowsDirectoryW,uPathSize);
   
   return uPathSize;
}
