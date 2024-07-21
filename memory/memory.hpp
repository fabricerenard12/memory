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
    DWORD GetProcessID(const wchar_t* processName);
    DWORD FindPattern(const BYTE* pattern, const char* mask, DWORD startAddress, DWORD endAddress);
    DWORD GetModuleBaseAddress(const wchar_t* moduleName);
    DWORD GetModuleBaseAddress(const char* moduleName);
    DWORD GetModuleBaseAddress(const std::string& moduleName);
    bool ReadMemory(DWORD address, void* buffer, SIZE_T size);
    bool WriteMemory(DWORD address, const void* buffer, SIZE_T size);
    DWORD AllocateMemory(SIZE_T size, DWORD allocationType, DWORD protect);
    bool FreeMemory(DWORD address);

private:
    HANDLE hProcess;
    const wchar_t* CharToWchar(const char* charArray);
    const wchar_t* StringToWchar(const std::string& str);
};

#endif // !MEMORY_H
