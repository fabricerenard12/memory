#ifndef MEMORY_H
#define MEMORY_H

#include <windows.h>
#include <tlhelp32.h>

class Memory {
public:
    Memory();
    Memory(HANDLE hProcess);
    Memory(DWORD processID);
    ~Memory();

    bool AttachProcess(const wchar_t* processName);
    void DetachProcess();
    bool ReadMemory(DWORD address, void* buffer, SIZE_T size);
    bool WriteMemory(DWORD address, const void* buffer, SIZE_T size);

private:
    HANDLE hProcess;
    DWORD GetProcessID(const wchar_t* processName);
};

#endif // !MEMORY_H
