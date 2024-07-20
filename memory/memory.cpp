#include "memory.hpp"

Memory::Memory() : hProcess(nullptr) {}

Memory::Memory(HANDLE hProcess) : hProcess(hProcess) {}

Memory::Memory(DWORD processID) : hProcess(OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID)) {}

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

bool Memory::ReadMemory(DWORD address, void* buffer, SIZE_T size) {
	if (!hProcess) return false;
	return ReadProcessMemory(hProcess, (LPCVOID)address, buffer, size, nullptr);
}

bool Memory::WriteMemory(DWORD address, const void* buffer, SIZE_T size) {
	if (!hProcess) return false;
	return WriteProcessMemory(hProcess, (LPVOID)address, buffer, size, nullptr);
}
