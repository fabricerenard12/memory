/**
 * @file memory.cpp
 * @brief This file contains the implementation of the Memory class which provides functionality to interact with the memory of processes.
 * @author Fabrice Renard
 */

#include "memory.hpp"

 /**
  * @brief Default constructor for Memory class.
  */
Memory::Memory() : hProcess(nullptr) {}

/**
 * @brief Constructor for Memory class that initializes with a given process handle.
 * @param hProcess Handle to the process.
 */
Memory::Memory(HANDLE hProcess) : hProcess(hProcess) {}

/**
 * @brief Constructor for Memory class that attaches to a process with a given process ID.
 * @param processID ID of the process to attach.
 */
Memory::Memory(DWORD processID) : hProcess(OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID)) {}

/**
 * @brief Constructor for Memory class that attaches to a process with a given process name (wide character string).
 * @param processName Name of the process to attach.
 */
Memory::Memory(const wchar_t* processName) {
    AttachProcess(processName);
}

/**
 * @brief Constructor for Memory class that attaches to a process with a given process name (C-style string).
 * @param processName Name of the process to attach.
 */
Memory::Memory(const char* processName) {
    AttachProcess(processName);
}

/**
 * @brief Constructor for Memory class that attaches to a process with a given process name (std::string).
 * @param processName Name of the process to attach.
 */
Memory::Memory(const std::string& processName) {
    AttachProcess(processName);
}

/**
 * @brief Destructor for Memory class.
 * Detaches from the currently attached process.
 */
Memory::~Memory() {
	DetachProcess();
}

/**
 * @brief Detaches from the currently attached process.
 */
void Memory::DetachProcess() {
	if (hProcess) {
		CloseHandle(hProcess);
		hProcess = nullptr;
	}
}

/**
 * @brief Attaches to a process with a given process name (wide character string).
 * @param processName Name of the process to attach.
 * @return true if attachment is successful, false otherwise.
 */
bool Memory::AttachProcess(const wchar_t* processName) {
	DWORD processID = GetProcessID(processName);
	if (processID == 0) return false;

	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
	return hProcess != nullptr;
}

/**
 * @brief Attaches to a process with a given process name (C-style string).
 * @param processName Name of the process to attach.
 * @return true if attachment is successful, false otherwise.
 */
bool Memory::AttachProcess(const char* processName) {
    const wchar_t* wideStrProcessName = CharToWchar(processName);
    DWORD processID = GetProcessID(wideStrProcessName);
    if (processID == 0) return false;

    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
    return hProcess != nullptr;
}

/**
 * @brief Attaches to a process with a given process name (std::string).
 * @param processName Name of the process to attach.
 * @return true if attachment is successful, false otherwise.
 */
bool Memory::AttachProcess(const std::string& processName) {
    const wchar_t* wideStrProcessName = StringToWchar(processName);
    DWORD processID = GetProcessID(wideStrProcessName);
    if (processID == 0) return false;

    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
    return hProcess != nullptr;
}

/**
 * @brief Gets the process ID of a process with a given name.
 * @param processName Name of the process.
 * @return Process ID of the process if found, 0 otherwise.
 */
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

/**
 * @brief Converts a C-style string to a wide character string.
 * @param charArray C-style string.
 * @return Wide character string.
 */
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

/**
 * @brief Converts a std::string to a wide character string.
 * @param str std::string.
 * @return Wide character string.
 */
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

/**
 * @brief Finds a pattern in the memory.
 * @param pattern Byte pattern to search for.
 * @param mask Mask for the pattern, where '?' can be used as a wildcard.
 * @param startAddress Starting address for the search.
 * @param endAddress Ending address for the search.
 * @return Address of the pattern if found, 0 otherwise.
 */
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

/**
 * @brief Gets the base address of a module in the attached process.
 * @param moduleName Name of the module.
 * @return Base address of the module if found, 0 otherwise.
 */
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

/**
 * @brief Gets the base address of a module in the attached process.
 * @param moduleName Name of the module.
 * @return Base address of the module if found, 0 otherwise.
 */
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

/**
 * @brief Gets the base address of a module in the attached process.
 * @param moduleName Name of the module.
 * @return Base address of the module if found, 0 otherwise.
 */
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

/**
 * @brief Reads memory from the attached process.
 * @param address Address to read from.
 * @param buffer Buffer to store the read data.
 * @param size Number of bytes to read.
 * @return true if reading is successful, false otherwise.
 */
bool Memory::ReadMemory(DWORD address, void* buffer, SIZE_T size) {
	if (!hProcess) return false;
	return ReadProcessMemory(hProcess, (LPCVOID)address, buffer, size, nullptr);
}

/**
 * @brief Writes memory to the attached process.
 * @param address Address to write to.
 * @param buffer Buffer containing the data to write.
 * @param size Number of bytes to write.
 * @return true if writing is successful, false otherwise.
 */
bool Memory::WriteMemory(DWORD address, const void* buffer, SIZE_T size) {
	if (!hProcess) return false;
	return WriteProcessMemory(hProcess, (LPVOID)address, buffer, size, nullptr);
}

/**
 * @brief Allocates a block of memory in the address space of the specified process.
 *
 * This function uses the `VirtualAllocEx` API to allocate a block of memory in the
 * address space of the process represented by the `hProcess` handle. The memory is
 * allocated with the specified size, allocation type, and protection flags.
 *
 * @param size The size of the memory block to allocate, in bytes.
 * @param allocationType The type of memory allocation. This can be one of the
 *                       `MEM_COMMIT`, `MEM_RESERVE`, or `MEM_RESET` values.
 * @param protect The memory protection for the allocated block. This can be one of
 *                the `PAGE_READWRITE`, `PAGE_READONLY`, or other protection values.
 * @return The starting address of the allocated memory block. If the allocation fails,
 *         the return value is zero.
 */
DWORD Memory::AllocateMemory(SIZE_T size, DWORD allocationType, DWORD protect) {
    if (!hProcess) return 0;
    return (DWORD)VirtualAllocEx(hProcess, nullptr, size, allocationType, protect);
}

/**
 * @brief Frees a block of memory in the address space of the specified process.
 *
 * This function uses the `VirtualFreeEx` API to release a previously allocated block of
 * memory in the address space of the process represented by the `hProcess` handle. The
 * memory block is identified by its starting address.
 *
 * @param address The starting address of the memory block to free.
 * @return `true` if the memory was successfully freed; otherwise, `false`.
 *         The function returns `false` if the `hProcess` handle is invalid or if the
 *         memory deallocation fails.
 */
bool Memory::FreeMemory(DWORD address) {
    if (!hProcess) return false;
    return VirtualFreeEx(hProcess, (LPVOID)address, 0, MEM_RELEASE);
}
