
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>

#include <iostream>
#include <string>
#include <tchar.h>
#include <stdio.h>
#include "strsafe.h"
#include <Aclapi.h>


wchar_t wtxt1[1024];
wchar_t wtxt2[1024];

uint8_t CharBuffers[10 * 102400];


const int TB_size = 20;		// max.processes
int tb_cnt = 0;				// cnt for sub-processes, except id=-1
int ID[TB_size];	    // table of sub-processes id
HANDLE HND[TB_size];    // handles of processes
PROCESS_INFORMATION piProcInfo[TB_size]; // process information
STARTUPINFO siStartInfo[TB_size];        // startinfo structure

HANDLE IN_Rd[TB_size];  // stdin
HANDLE IN_Wr[TB_size];
HANDLE OUT_Rd[TB_size]; // stdout
HANDLE OUT_Wr[TB_size];

std::string exeName;    // local datas
std::string exeArgs;
int i;
size_t outSize, size;



// returns pointer to the string buffer
extern "C" __declspec(dllexport) uint8_t *  char_100kb_buffer(int i)
{
    return &CharBuffers[i * 102400];
}


void printf2(LPCWSTR s)
{
    MessageBox(NULL, s, TEXT("ExeHandler64 message"), MB_OK);
}

void ErrorExit(PTSTR lpszFunction)

// Format a readable error message, display a message box,
// and exit from the application.
{
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0, NULL);

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
    StringCchPrintf((LPTSTR)lpDisplayBuf,
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"),
        lpszFunction, dw, lpMsgBuf);
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    //ExitProcess(1);
};

void start_process()
{
    SECURITY_ATTRIBUTES saAttr;


    IN_Rd[i] = NULL;
    IN_Wr[i] = NULL;
    OUT_Rd[i] = NULL;
    OUT_Wr[i] = NULL;

    //printf2("\n->Start of parent execution.\n");

 // Set the bInheritHandle flag so pipe handles are inherited.

    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    // Create a pipe for the child process's STDOUT.

    if (!CreatePipe(&OUT_Rd[i], &OUT_Wr[i], &saAttr, 0))
        ErrorExit( (PTSTR)("StdoutRd CreatePipe"));

    // Ensure the read handle to the pipe for STDOUT is not inherited.

    if (!SetHandleInformation(OUT_Rd[i], HANDLE_FLAG_INHERIT, 0))
        ErrorExit((PTSTR)("Stdout SetHandleInformation"));

    // Create a pipe for the child process's STDIN.

    if (!CreatePipe(&IN_Rd[i], &IN_Wr[i], &saAttr, 0))
        ErrorExit((PTSTR)("Stdin CreatePipe"));

    // Ensure the write handle to the pipe for STDIN is not inherited.

    if (!SetHandleInformation(IN_Wr[i], HANDLE_FLAG_INHERIT, 0))
        ErrorExit((PTSTR)("Stdin SetHandleInformation"));

    // Create a child process that uses the previously created pipes for STDIN and STDOUT.

    std::string szCmdline = "";
    std::string fldr = exeName;
    int a = fldr.find_last_of("\\");
    if (a >= 0) fldr = fldr.substr(0, a);

    szCmdline.append(exeName);
    szCmdline.append(" ");
    szCmdline.append(exeArgs);
    PROCESS_INFORMATION pi = piProcInfo[i];
    STARTUPINFO si = siStartInfo[i];
    BOOL bSuccess = FALSE;

    // Set up members of the PROCESS_INFORMATION structure.

    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

    // Set up members of the STARTUPINFO structure.
    // This structure specifies the STDIN and STDOUT handles for redirection.

    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);

    si.hStdError = OUT_Wr[i];
    si.hStdOutput = OUT_Wr[i];
    si.hStdInput = IN_Rd[i];

    si.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = 0;

    size = strlen(&szCmdline[0]) + 1;
    mbstowcs_s(&outSize, wtxt1, size, &szCmdline[0], size-1);

    if (a >= 0) {
        size = strlen(&fldr[0]) + 1;
        mbstowcs_s(&outSize, wtxt2, size,  &fldr[0], size - 1);
    }

    // Create the child process.

    bSuccess = CreateProcess(NULL,
        wtxt1,     // command line
        NULL,          // process security attributes
        NULL,          // primary thread security attributes
        TRUE,          // handles are inherited
        0,             // creation flags
        NULL,          // use parent's environment
        (a >= 0 ? wtxt2 : NULL),          // use directory
        &si,  // STARTUPINFO pointer
        &pi);  // receives PROCESS_INFORMATION

     // If an error occurs, exit the application.
    if (!bSuccess)
        ErrorExit((PTSTR)("CreateProcess"));
    else
    {
        // Close handles to the child process and its primary thread.
        // Some applications might keep these handles to monitor the status
        // of the child process, for example.

        ID[i] = pi.dwProcessId;
        HND[i] = pi.hProcess;
    }
    // The remaining open handles are cleaned up when this process terminates.
    // To avoid resource leaks in a larger application, close handles explicitly.
};

//-- (C) eraser, 2005
//-- project termination procedures for DLL

BOOL enable_token_privilege(HANDLE htok,
    LPCTSTR szPrivilege, TOKEN_PRIVILEGES* tpOld)
{
    TOKEN_PRIVILEGES tp;
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (LookupPrivilegeValue(0, szPrivilege, &tp.Privileges[0].Luid))
    {
        DWORD cbOld = sizeof(*tpOld);

        if (AdjustTokenPrivileges(htok, FALSE, &tp, cbOld, tpOld, &cbOld))
        {
            return (ERROR_NOT_ALL_ASSIGNED != GetLastError());
        }
        else
        {
            //printf2("enable_token_privilege\n");

            return (FALSE);
        }
    }
    else
    {
        //printf2("enable_token_privilege\n");

        return (FALSE);
    }
};

BOOL adjust_dacl(HANDLE h, DWORD dwDesiredAccess)
{
    SID world = { SID_REVISION, 1, SECURITY_WORLD_SID_AUTHORITY, 0 };

    EXPLICIT_ACCESS ea =
    {
        0,
            SET_ACCESS,
            NO_INHERITANCE,
        {
            0, NO_MULTIPLE_TRUSTEE,
                TRUSTEE_IS_SID,
                TRUSTEE_IS_USER,
                0
        }
    };

    ACL* pdacl = 0;
    DWORD err = SetEntriesInAcl(1, &ea, 0, &pdacl);

    ea.grfAccessPermissions = dwDesiredAccess;
    ea.Trustee.ptstrName = (LPTSTR)(&world);

    if (err == ERROR_SUCCESS)
    {
        err = SetSecurityInfo(h, SE_KERNEL_OBJECT,
            DACL_SECURITY_INFORMATION, 0, 0, pdacl, 0);
        LocalFree(pdacl);

        return (err == ERROR_SUCCESS);
    }
    else
    {
        //printf2("adjust_dacl\n");

        return(FALSE);
    }
};

HANDLE adv_open_process(DWORD pid, DWORD dwAccessRights)
{
    HANDLE hProcess = OpenProcess(dwAccessRights, FALSE, pid);

    if (hProcess == NULL)
    {
        HANDLE hpWriteDAC = OpenProcess(WRITE_DAC, FALSE, pid);

        if (hpWriteDAC == NULL)
        {
            HANDLE htok;
            TOKEN_PRIVILEGES tpOld;

            if (!OpenProcessToken(GetCurrentProcess(),
                TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &htok))
            {
                return(FALSE);
            }

            if (enable_token_privilege(htok, SE_TAKE_OWNERSHIP_NAME, &tpOld))
            {
                HANDLE hpWriteOwner = OpenProcess(WRITE_OWNER, FALSE, pid);

                if (hpWriteOwner != NULL)
                {
                    BYTE buf[512];
                    DWORD cb = sizeof buf;

                    if (GetTokenInformation(htok, TokenUser, buf, cb, &cb))
                    {
                        DWORD err = SetSecurityInfo(hpWriteOwner, SE_KERNEL_OBJECT,
                            OWNER_SECURITY_INFORMATION,
                            ((TOKEN_USER*)(buf))->User.Sid, 0, 0, 0);

                        if (err == ERROR_SUCCESS)
                        {
                            if (!DuplicateHandle(GetCurrentProcess(), hpWriteOwner,
                                GetCurrentProcess(), &hpWriteDAC,
                                WRITE_DAC, FALSE, 0))
                            {
                                hpWriteDAC = NULL;
                            }
                        }
                    }

                    CloseHandle(hpWriteOwner);
                }

                AdjustTokenPrivileges(htok, FALSE, &tpOld, 0, 0, 0);
            }

            CloseHandle(htok);
        }

        if (hpWriteDAC)
        {
            adjust_dacl(hpWriteDAC, dwAccessRights);

            if (!DuplicateHandle(GetCurrentProcess(), hpWriteDAC,
                GetCurrentProcess(), &hProcess, dwAccessRights, FALSE, 0))
            {
                hProcess = NULL;
            }

            CloseHandle(hpWriteDAC);
        }
    }

    return (hProcess);
};

BOOL kill_the_process(DWORD pid)
{
    HANDLE hp = adv_open_process(pid, PROCESS_TERMINATE);

    if (hp != NULL)
    {
        BOOL bRet = TerminateProcess(hp, 1);
        CloseHandle(hp);

        return (bRet);
    }

    return (FALSE);
};

//------- end of process termination

bool IsRunning()
{
    BOOL r = false;
    DWORD dwExitCode;

    if ((HND[i] != NULL) && GetExitCodeProcess(HND[i], &dwExitCode))
    {
        if (dwExitCode == STILL_ACTIVE) r = true; //still running
    }
    //else query failed,
    // handle probably doesn't have the PROCESS_QUERY_INFORMATION access
    return r;
};

void CloseProcess()
{
    PROCESS_INFORMATION pi = piProcInfo[i];

    kill_the_process(ID[i]);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    CloseHandle(IN_Rd[i]);
    CloseHandle(IN_Wr[i]);
    CloseHandle(OUT_Rd[i]);
    CloseHandle(OUT_Wr[i]);

    // Unfortunately, ExitProcess terminates DLL
    // ExitProcess(1);

};


extern "C" int __declspec(dllexport) add_process(const LPCSTR sName, const LPCSTR sArgs)
{
    for (i = 0;i < tb_cnt;i++) if (ID[i] == -1) break;
    if (i == tb_cnt)
    {
        if (tb_cnt == TB_size) return -1;
        tb_cnt++;
    }
    ID[i] = -1;
    exeName = (LPCSTR)sName;
    exeArgs = (LPCSTR)sArgs;
    start_process();
    return ID[i];
};

extern "C" int __declspec(dllexport) kill_process(int id)
{
    int n = 0;
    for (i = 0;i < tb_cnt;i++)
        if (ID[i] == id)
        {
            CloseProcess();
            ID[i] = -1;
            n++;
            break;
        }
    return n;
};

extern "C" int __declspec(dllexport) release_all()
{
    int n = 0;
    for (int j = 0;j < tb_cnt;j++)
        if (ID[j] != -1) n += kill_process(ID[j]);
    tb_cnt = 0;
    return n;
};


int put_stdin_route(int id, char* b) {
    int l = -1;
    for (i = 0;i < tb_cnt;i++)
        if ((ID[i] == id) && IsRunning() && (IN_Wr[i] != NULL))
        {
            DWORD dwWritten;
            std::string s = b;
            DWORD dwCnt = s.length();
            BOOL bSuccess = WriteFile(IN_Wr[i], b, dwCnt, &dwWritten, NULL);
            if (bSuccess) l = (int)dwWritten;
            break;
        }
    return l;
}

extern "C" int __declspec(dllexport) put_stdin_i(int id, int Iat)
{
    uint8_t*b = &CharBuffers[Iat*102400];
    return put_stdin_route(id, (char*)b);
}

extern "C" int __declspec(dllexport) put_stdin(int id, char* b)
{
    return put_stdin_route(id, b);
}

int get_stdout_route(int id, char* b)
{
    int l = -1;
    for (i = 0;i < tb_cnt;i++)
        if ((ID[i] == id) && (OUT_Rd[i] != NULL))
        {
            BOOL bSuccess = TRUE;
            DWORD dwRead;
            DWORD lpTotalBytesAvail;
            DWORD lpBytesLeftThisMessage;
            for (l = 0;;l += (int)dwRead)
            {
                // are there datas in the pipe?
                bSuccess = PeekNamedPipe(OUT_Rd[i], &b[l], 1024, &dwRead,
                    &lpTotalBytesAvail, &lpBytesLeftThisMessage);
                if (lpTotalBytesAvail == NULL) break;

                bSuccess = ReadFile(OUT_Rd[i], &b[l], 1024, &dwRead, NULL);
                if ((!bSuccess) || (dwRead == 0)) break;
            }
            break;
        }
    return l;
};

extern "C" int __declspec(dllexport) assign_memory_i_n(int Iat, int n, int C)
{
    CharBuffers[(Iat * 102400) + n] = C;
    return 1;
}
extern "C" int __declspec(dllexport) getfrom_memory_i_n(int Iat, int n)
{
    int r = CharBuffers[(Iat * 102400) + n];
    return r;
}


extern "C" int __declspec(dllexport) get_stdout_i(int id, int Iat)
{
    uint8_t* b = &CharBuffers[Iat * 102400];
    return get_stdout_route(id, (char*)b);
}

extern "C" int __declspec(dllexport) get_stdout(int id, char* b)
{
    return get_stdout_route(id, b);
}

extern "C" int __declspec(dllexport) get_status(int id)
{
    int r = 0;
    for (int i = 0;i < tb_cnt;i++)
        if (ID[i] == id)
        {
            r |= 1;
            r |= (IsRunning() ? 2 /*still running*/ : 4 /*not running anymore*/);
            break;
        }
    return r;
};


extern "C" __declspec(dllexport) BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        // attach to process
        // return FALSE to fail DLL load
        break;

    case DLL_PROCESS_DETACH:
        // detach from process
        release_all();
        break;

    case DLL_THREAD_ATTACH:
        // attach to thread
        break;

    case DLL_THREAD_DETACH:
        // detach from thread
        break;
    }
    return TRUE; // succesful
};



