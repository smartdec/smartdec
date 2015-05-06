#include <Windows.h>
#include <strsafe.h>

#pragma comment(linker, "/nodefaultlib:msvcrt.lib")
#pragma comment(linker, "/nodefaultlib:msvcrtd.lib")

int myitoa(LPWSTR s) {
  size_t length = 0;
  int result = 0;
  unsigned int i;
  
  StringCchLength(s, 20, &length);
  for(i = 0; i < length; i++)
    result = result * 10 + (s[i] - L'0');
  return result;
}

void ErrorExit(LPWSTR lpszFunction, LPWSTR lpszParam) { 
  LPWSTR lpMsgBuf;
  LPWSTR lpDisplayBuf;
  DWORD dwError = GetLastError(); 

  FormatMessageW(
    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
    FORMAT_MESSAGE_FROM_SYSTEM |
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    dwError,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    (LPWSTR) &lpMsgBuf,
    0, 
    NULL 
  );

  lpDisplayBuf = (LPWSTR) LocalAlloc(
    LMEM_ZEROINIT, 
    (lstrlen(lpMsgBuf) + lstrlen(lpszFunction) + 4096) * sizeof(TCHAR)
  ); 

  wsprintfW(
    lpDisplayBuf, 
    TEXT(
"При обновлении произошла ошибка. Пожалуйста, загрузите обновление самостоятельно. \n\
\n\
Дополнительная информация о произошедшей ошибке:\n\
%s(%s) failed with error %d: %s\n\
Пожалуйста, сообщите об этом разработчикам. Спасибо."
    ), 
    lpszFunction, 
    lpszParam,
    dwError, 
    lpMsgBuf
  ); 
  lpDisplayBuf[1024] = 0;
  MessageBoxW(NULL, lpDisplayBuf, TEXT("Error"), MB_OK); 

  LocalFree(lpMsgBuf);
  LocalFree(lpDisplayBuf);
  ExitProcess(dwError); 
}


static BOOL SafeDeleteFile(LPCTSTR filename) {
  int count = 0;
  int iterations = 0;

  if(DeleteFileW(filename) != 0)
    return TRUE;

  SetFileAttributesW(filename, FILE_ATTRIBUTE_NORMAL);
  if(DeleteFileW(filename) != 0)
    return TRUE;

  do {
    BOOL result = DeleteFileW(filename);
    WIN32_FIND_DATA findFileData;
    HANDLE hFind;

    if(result != 0)
      return TRUE;

    hFind = FindFirstFileW(filename, &findFileData);
    if(hFind == INVALID_HANDLE_VALUE) {
      if(GetLastError() == ERROR_FILE_NOT_FOUND)
        return TRUE;
    } else {
      FindClose(hFind);
    }

    Sleep(100);
    iterations++;
  } while(iterations < 10);

  return 0;
}


int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  /* Command line:
   *
   * pid source-file target-file reserve-target-file
   */

  LPWSTR *argv;
  int argc;
  HANDLE hHandle;
  LPWSTR targetFileName;
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  DWORD pid;
  int i;

  /* Check command line. */
  argv = CommandLineToArgvW(GetCommandLineW(), &argc);
  if(argv == NULL)
    ErrorExit(L"CommandLineToArgvW", L"");
  if(argc != 5)
    return 1000 + argc;

  /* Kill parent. */
  pid = myitoa(argv[1]);
  hHandle = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
  if(hHandle == NULL)
    ErrorExit(L"OpenProcess", argv[1]);
  if(TerminateProcess(hHandle, 0) == 0)
    ErrorExit(L"TerminateProcess", L"");
  CloseHandle(hHandle);

  /* Try delete parent's executable. */
  if(SafeDeleteFile(argv[3]) == 0) {
    targetFileName = argv[4];
  } else {
    targetFileName = argv[3];
  }

  /* Move source to target. */
  if(MoveFileW(argv[2], targetFileName) == 0)
    ErrorExit(L"MoveFileW", argv[2]);

  /* Start target. */
  RtlSecureZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  RtlSecureZeroMemory(&pi, sizeof(pi));

  if(!CreateProcessW( 
    NULL,      /* No module name (use command line). */
    targetFileName, /* Command line. */
    NULL,      /* Process handle not inheritable. */
    NULL,      /* Thread handle not inheritable. */
    FALSE,     /* Set handle inheritance to FALSE. */
    0,         /* No creation flags. */
    NULL,      /* Use parent's environment block. */
    NULL,      /* Use parent's starting directory. */
    &si,       /* Pointer to STARTUPINFO structure. */
    &pi        /* Pointer to PROCESS_INFORMATION structure. */
  )) {
    ErrorExit(L"CreateProcessW", targetFileName);
  }

  /* If we haven't deleted original executable - try again. */
  if(targetFileName != argv[3])
    for(i = 0; i < 100; i++)
      SafeDeleteFile(argv[3]);

  return 0;
}
