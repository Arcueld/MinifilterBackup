#pragma once
#include <Windows.h>
#include <iostream>
#include <fltUser.h>

#pragma comment(lib,"fltLib.lib")

#define R3portName L"\\TestPort"


EXTERN_C{
	HRESULT FilterConnectCommunicationPort(
		LPCWSTR               lpPortName,
		DWORD                 dwOptions,
		LPCVOID               lpContext,
		WORD                  wSizeOfContext,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		HANDLE* hPort
	);

	HRESULT FilterSendMessage(
		HANDLE  hPort,
		LPVOID  lpInBuffer,
		DWORD   dwInBufferSize,
		LPVOID  lpOutBuffer,
		DWORD   dwOutBufferSize,
		LPDWORD lpBytesReturned
	);
}