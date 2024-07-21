#include "memory.hpp"

Memory::Memory() : hProcess(nullptr) {}

Memory::Memory(HANDLE hProcess) : hProcess(hProcess) {}

Memory::Memory(DWORD processID) : hProcess(OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID)) {}

Memory::Memory(const wchar_t* processName) {
    AttachProcess(processName);
}

Memory::Memory(const char* processName) {
    AttachProcess(processName);
}

Memory::Memory(const std::string& processName) {
    AttachProcess(processName);
}

Memory::~Memory() {
	DetachProcess();
}

void Memory::DetachProcess() {
	if (hProcess) {
		CloseHandle(hProcess);
		hProcess = nullptr;
	}
}

bool Memory::AttachProcess(const wchar_t* processName) {
	DWORD processID = GetProcessID(processName);
	if (processID == 0) return false;

	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
	return hProcess != nullptr;
}

bool Memory::AttachProcess(const wchar_t* processName) {
    DWORD processID = GetProcessID(processName);
    if (processID == 0) return false;

    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
    return hProcess != nullptr;
}

bool Memory::AttachProcess(const char* processName) {
    const wchar_t* wideStrProcessName = CharToWchar(processName);
    DWORD processID = GetProcessID(wideStrProcessName);
    if (processID == 0) return false;

    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
    return hProcess != nullptr;
}

bool Memory::AttachProcess(const std::string& processName) {
    const wchar_t* wideStrProcessName = StringToWchar(processName);
    DWORD processID = GetProcessID(wideStrProcessName);
    if (processID == 0) return false;

    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
    return hProcess != nullptr;
}

DWORD Memory::GetProcessID(const wchar_t* processName) {
    DWORD processID = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hSnapshot, &pe)) {
            do {
                if (_wcsicmp(pe.szExeFile, processName) == 0) {
                    processID = pe.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnapshot, &pe));
        }
        CloseHandle(hSnapshot);
    }

    return processID;
}

const wchar_t* Memory::CharToWchar(const char* charArray) {
    size_t len = std::mbstowcs(nullptr, charArray, 0);
    if (len == static_cast<size_t>(-1)) {
        return nullptr;
    }

    wchar_t* wcharArray = new wchar_t[len + 1];
    std::mbstowcs(wcharArray, charArray, len);
    wcharArray[len] = L'\0';

    return wcharArray;
}

const wchar_t* Memory::StringToWchar(const std::string& str) {
    size_t len = std::mbstowcs(nullptr, str.c_str(), 0);
    if (len == static_cast<size_t>(-1)) {
        return nullptr;
    }

    wchar_t* wcharArray = new wchar_t[len + 1];
    std::mbstowcs(wcharArray, str.c_str(), len);
    wcharArray[len] = L'\0';

    return wcharArray;
}

DWORD Memory::FindPattern(const BYTE* pattern, const char* mask, DWORD startAddress, DWORD endAddress) {
    SIZE_T patternLength = std::strlen(mask);
    BYTE* buffer = new BYTE[endAddress - startAddress];
    ReadMemory(startAddress, buffer, endAddress - startAddress);

    for (SIZE_T i = 0; i < endAddress - startAddress - patternLength; ++i) {
        bool found = true;
        for (SIZE_T j = 0; j < patternLength; ++j) {
            if (mask[j] != '?' && pattern[j] != buffer[i + j]) {
                found = false;
                break;
            }
        }
        if (found) {
            delete[] buffer;
            return startAddress + i;
        }
    }
    delete[] buffer;
    return 0;
}

DWORD Memory::GetModuleBaseAddress(const wchar_t* moduleName) {
    DWORD moduleBaseAddress = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, GetProcessId(hProcess));

    if (hSnapshot != INVALID_HANDLE_VALUE) {
        MODULEENTRY32 me;
        me.dwSize = sizeof(MODULEENTRY32);
        if (Module32First(hSnapshot, &me)) {
            do {
                if (_wcsicmp(me.szModule, moduleName) == 0) {
                    moduleBaseAddress = (DWORD)me.modBaseAddr;
                    break;
                }
            } while (Module32Next(hSnapshot, &me));
        }
        CloseHandle(hSnapshot);
    }

    return moduleBaseAddress;
}

DWORD Memory::GetModuleBaseAddress(const char* moduleName) {
    const wchar_t* wideStrModuleName = CharToWchar(moduleName);
    DWORD moduleBaseAddress = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, GetProcessId(hProcess));

    if (hSnapshot != INVALID_HANDLE_VALUE) {
        MODULEENTRY32 me;
        me.dwSize = sizeof(MODULEENTRY32);
        if (Module32First(hSnapshot, &me)) {
            do {
                if (_wcsicmp(me.szModule, wideStrModuleName) == 0) {
                    moduleBaseAddress = (DWORD)me.modBaseAddr;
                    break;
                }
            } while (Module32Next(hSnapshot, &me));
        }
        CloseHandle(hSnapshot);
    }

    return moduleBaseAddress;
}

DWORD Memory::GetModuleBaseAddress(const std::string& moduleName) {
    const wchar_t* wideStrModuleName = StringToWchar(moduleName);
    DWORD moduleBaseAddress = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, GetProcessId(hProcess));

    if (hSnapshot != INVALID_HANDLE_VALUE) {
        MODULEENTRY32 me;
        me.dwSize = sizeof(MODULEENTRY32);
        if (Module32First(hSnapshot, &me)) {
            do {
                if (_wcsicmp(me.szModule, wideStrModuleName) == 0) {
                    moduleBaseAddress = (DWORD)me.modBaseAddr;
                    break;
                }
            } while (Module32Next(hSnapshot, &me));
        }
        CloseHandle(hSnapshot);
    }

    return moduleBaseAddress;
}

bool Memory::ReadMemory(DWORD address, void* buffer, SIZE_T size) {
	if (!hProcess) return false;
	return ReadProcessMemory(hProcess, (LPCVOID)address, buffer, size, nullptr);
}

bool Memory::WriteMemory(DWORD address, const void* buffer, SIZE_T size) {
	if (!hProcess) return false;
	return WriteProcessMemory(hProcess, (LPVOID)address, buffer, size, nullptr);
}

DWORD Memory::AllocateMemory(SIZE_T size, DWORD allocationType, DWORD protect) {
    if (!hProcess) return 0;
    return (DWORD)VirtualAllocEx(hProcess, nullptr, size, allocationType, protect);
}

bool Memory::FreeMemory(DWORD address) {
    if (!hProcess) return false;
    return VirtualFreeEx(hProcess, (LPVOID)address, 0, MEM_RELEASE);
}
