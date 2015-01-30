#include "stdafx.h"
#include "Ouvriere.h"
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")


//static LPCTSTR pszServer = _TEXT("192.168.137.135");
static const DWORD dwPort = 8080;
static LPCTSTR pszContentTypeHeader = _TEXT("Content-Type: application/x-www-form-urlencoded");
HINTERNET hSession, hConnect;

Ouvriere::Ouvriere()
{

}

void Ouvriere::initConnection(std::wstring ip)
{
	BOOL bResults = FALSE;
	hSession = NULL;
	hConnect = NULL;

	// Use WinHttpOpen to obtain a session handle.
	hSession = WinHttpOpen(L"A WinHTTP Example Program/1.0",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS, 0);
	if (!hSession)
	{
		_tprintf(_TEXT("Failed to open WinHTTP session: %ld\n"), GetLastError());
		return;
	}
	// Specify an HTTP server.
	if (hSession)
		hConnect = WinHttpConnect(hSession, (LPCWSTR)ip.c_str(),
		dwPort, 0);

	if (!hConnect)
	{
		_tprintf(_TEXT("Failed to connect to server: %ld\n"), GetLastError());
		WinHttpCloseHandle(hSession);
		return;
	}
}

void Ouvriere::sendData(std::string form)
{
	BOOL bResults = FALSE;
	HINTERNET hRequest = NULL;
	// Create an HTTP Request handle.
	if (hConnect)
		hRequest = WinHttpOpenRequest(hConnect, _TEXT("POST"),
		L"",
		NULL, WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES,
		0);
	if (!hRequest)
	{
		_tprintf(_TEXT("Failed to open request: %ld\n"), GetLastError());
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
	}

	DWORD dwReqOpts = 0;
	DWORD dwSize = sizeof(DWORD);
	WinHttpSetOption(
		hRequest,
		WINHTTP_OPTION_SECURITY_FLAGS,
		&dwReqOpts,
		sizeof(DWORD));

	BOOL done = false;

	// Send a Request.
	if (hRequest)
		bResults = WinHttpSendRequest(hRequest,
		(LPCWSTR)pszContentTypeHeader,
		-1L, (LPVOID)form.c_str(), form.length(),
		form.length(), NULL);

	// PLACE ADDITIONAL CODE HERE.

	// Report any errors.
	if (!bResults)
		printf("Error %d has occurred.\n", GetLastError());
	//printf("No error occured\n");
	// Close any open handles.
	if (hRequest) WinHttpCloseHandle(hRequest);

}
Ouvriere::~Ouvriere()
{
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);
}


/*




 int _tmain(int argc, _TCHAR* argv[])
 {
 BOOL bResults = FALSE;
 HINTERNET hSession = NULL,
 hConnect = NULL,
 hRequest = NULL;

 // Use WinHttpOpen to obtain a session handle.
 hSession = WinHttpOpen(L"A WinHTTP Example Program/1.0",
 WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
 WINHTTP_NO_PROXY_NAME,
 WINHTTP_NO_PROXY_BYPASS, 0);
 if (!hSession)
 {
 _tprintf(_TEXT("Failed to open WinHTTP session: %ld\n"), GetLastError());
 return -1;
 }
 DWORD dwDataLen = strlen(pszPostData);
 // Specify an HTTP server.
 if (hSession)
 hConnect = WinHttpConnect(hSession, pszServer,
 dwPort, 0);

 if (!hConnect)
 {
 _tprintf(_TEXT("Failed to connect to server: %ld\n"), GetLastError());
 WinHttpCloseHandle(hSession);
 return -1;
 }

 // Create an HTTP Request handle.
 if (hConnect)
 hRequest = WinHttpOpenRequest(hConnect, _TEXT("POST"),
 L"",
 NULL, WINHTTP_NO_REFERER,
 WINHTTP_DEFAULT_ACCEPT_TYPES,
 0);
 if (!hRequest)
 {
 _tprintf(_TEXT("Failed to open request: %ld\n"), GetLastError());
 WinHttpCloseHandle(hConnect);
 WinHttpCloseHandle(hSession);
 return -1;
 }

 DWORD dwReqOpts = 0;
 DWORD dwSize = sizeof(DWORD);
 WinHttpSetOption(
 hRequest,
 WINHTTP_OPTION_SECURITY_FLAGS,
 &dwReqOpts,
 sizeof(DWORD));

 BOOL done = false;

 // Send a Request.
 if (hRequest)
 bResults = WinHttpSendRequest(hRequest,
 (LPCWSTR)pszContentTypeHeader,
 -1L, (LPVOID)pszPostData, dwDataLen,
 dwDataLen, NULL);

 // PLACE ADDITIONAL CODE HERE.

 // Report any errors.
 if (!bResults)
 printf("Error %d has occurred.\n", GetLastError());
 printf("No error occured\n");
 Sleep(10000);
 // Close any open handles.
 if (hRequest) WinHttpCloseHandle(hRequest);
 if (hConnect) WinHttpCloseHandle(hConnect);
 if (hSession) WinHttpCloseHandle(hSession);
 return 0;
 }


 */