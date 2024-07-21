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

bool Memory::ReadMemory(DWORD address, void* buffer, SIZE_T size) {
	if (!hProcess) return false;
	return ReadProcessMemory(hProcess, (LPCVOID)address, buffer, size, nullptr);
}

bool Memory::WriteMemory(DWORD address, const void* buffer, SIZE_T size) {
	if (!hProcess) return false;
	return WriteProcessMemory(hProcess, (LPVOID)address, buffer, size, nullptr);
}
