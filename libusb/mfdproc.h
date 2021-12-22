#ifndef MFDPROC_H
#define MFDPROC_H

#include <QtCore>

#include <tchar.h>

#include <windows.h>
#include <thread>
#include <iostream>
#include <sstream>

#define BUFSIZE 4096

class MFDProc
{
public:
    MFDProc();
    ~MFDProc();
    BOOL start(QString path, QStringList paramters);
    void waitForFinished();
    std::string readAll();

    static bool runProc(QString exe, QStringList argument, QByteArray &output)
    {
        output.clear();
        MFDProc p;
        BOOL bSuccess = p.start(exe, argument);
        p.waitForFinished();
        output.append(p.readAll().data());
        return (bSuccess == 1);
    }

private:
    HANDLE hProcess;
    HANDLE hRead;
    HANDLE hWrite;
    std::ostringstream stream;
    friend void ReadFromPipe(MFDProc* Process);
};

void ReadFromPipe(MFDProc* Process)
{
    DWORD dwRead;
    CHAR chBuf[BUFSIZE];
    BOOL bSuccess = FALSE;
    Process->stream.clear();
    for (;;)
    {
        bSuccess = ReadFile(Process->hRead, chBuf, BUFSIZE-1, &dwRead, NULL);
        if (!bSuccess || dwRead == 0)
            break;

        chBuf[dwRead] = 0;
        Process->stream << chBuf;
    }
}

MFDProc::MFDProc()
{
    hProcess = NULL;
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;
    CreatePipe(&hRead, &hWrite, &saAttr, 0);
    SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);
}

MFDProc::~MFDProc()
{
    if(hRead)
        CloseHandle(hRead);

    if (hWrite)
        CloseHandle(hWrite);

    CloseHandle(hProcess);

    stream.clear();
}

BOOL MFDProc::start(QString path, QStringList paramters)
{
    QString args;
    for (int i = 0; i < paramters.size(); ++i)
    {
        args.append(" ");
        args.append(QString("%0").arg(paramters[i]));
    }

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    si.dwFlags = STARTF_USESTDHANDLES;
    si.wShowWindow = FALSE;
    si.hStdOutput = hWrite;
    si.hStdError = hWrite;
    BOOL ret = CreateProcess(path.toStdWString().c_str(), (wchar_t*)args.toStdWString().c_str(), NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi);
    CloseHandle(hWrite);
    hWrite = NULL;
    std::thread t(&ReadFromPipe, this);
    t.join();
    if (ret)
        hProcess = pi.hProcess;

    return ret;
}

void MFDProc::waitForFinished()
{
    if (hProcess)
        WaitForSingleObject(hProcess, INFINITE);
}

std::string MFDProc::readAll()
{
    return stream.str();
}

#endif // MFDPROC_H
