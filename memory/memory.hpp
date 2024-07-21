#ifndef MEMORY_H
#define MEMORY_H

#include <Windows.h>
#include <TlHelp32.h>
#include <cwchar>
#include <cstdlib>
#include <string>

class Memory {
public:
    Memory();
    Memory(HANDLE hProcess);
    Memory(DWORD processID);
    Memory(const wchar_t* processName);
    Memory(const char* processName);
    Memory(const std::string& processName);
    ~Memory();

    bool AttachProcess(const wchar_t* processName);
    bool AttachProcess(const char* processName);
    bool AttachProcess(const std::string& processName);
    void DetachProcess();
    bool ReadMemory(DWORD address, void* buffer, SIZE_T size);
    bool WriteMemory(DWORD address, const void* buffer, SIZE_T size);

private:
    HANDLE hProcess;
    DWORD GetProcessID(const wchar_t* processName);
    const wchar_t* CharToWchar(const char* charArray);
    const wchar_t* StringToWchar(const std::string& str);
};

#endif // !MEMORY_H
