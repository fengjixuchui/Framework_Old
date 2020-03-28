#pragma once

//Includes

#define INCLUDE_UTILITY          1
#define INCLUDE_MEMORY           1
#define INCLUDE_MEMORY_INTERNAL  1
#define INCLUDE_MEMORY_EXTERNAL  1
#define INCLUDE_FUNCTION_MANAGER 1
#define INCLUDE_API              1
#define INCLUDE_DIRECTX          0
#define INCLUDE_DIRECTX9         0

//Definitions
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__) && !defined(linux)
#define WIN
#elif defined(linux)
#define LINUX
#endif

#if defined(_M_IX86) || defined(__i386__)
#define ARCH_X86
#elif defined(_M_X64) || defined(__LP64__) || defined(_LP64)
#define ARCH_X64
#endif

#if defined(WIN) || defined(LINUX)
#define FRAMEWORK
#endif

//Includes / OS specific
#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>
#include <functional>

#if defined(WIN)
#include <iostream>
#include <vector>
#include <Windows.h>
#include <TlHelp32.h>
#if INCLUDE_DIRECTX9
#include <d3d9.h>
#include <d3dx9.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#endif
typedef DWORD pid_t;
typedef uintptr_t mem_t;

#elif defined(LINUX)
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <string>
#include <string.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/mman.h>
#define INVALID_PID -1
#define MAX_FILENAME 256
typedef off_t mem_t;
#endif

typedef unsigned char byte_t;
#define BYTE_SIZE 1
#if defined(ARCH_X86)
#define HOOK_MIN_SIZE 5
const byte_t JMP = 0xE9;
#elif defined(ARCH_X64)
#define HOOK_MIN_SIZE 12
const byte_t MOV_RAX[] = { 0x48, 0xB8 };
const byte_t JMP_RAX[] = { 0xFF, 0xE0 };
#endif

#define BAD_RETURN 0
#define BAD_FUNCTION 0

//Framework

#ifdef FRAMEWORK
namespace Framework
{
#	if INCLUDE_API
	namespace API
	{
#		if defined(WIN)
		namespace Windows
		{
			HWND GetCurrentWindow();
		}
#		endif //Windows
#		if INCLUDE_DIRECTX
		namespace D3D
		{
#			if INCLUDE_DIRECTX9
			namespace DX9
			{
				bool GetCurrentDevice(void** vtable, size_t size);
				namespace Draw
				{
					void FilledRectangle(int x, int y, int w, int h, D3DCOLOR color, LPDIRECT3DDEVICE9 pdevice);
					void Rectangle(int x, int y, int w, int h, int thickness, D3DCOLOR color, LPDIRECT3DDEVICE9 pdevice);
				}
			}
#			endif //DX9
		}
#		endif //D3D
	}
#	endif //API

#	if INCLUDE_UTILITY
	namespace Utility
	{
		size_t FileToArrayOfBytes(std::string filepath, char*& pbuffer);
		void MultiByteToWideChar(char mbstr[], wchar_t wcbuf[], size_t max_size);
		void WideCharToMultiByte(wchar_t wcstr[], char mbbuf[], size_t max_size);
	}
#	endif //Utility

#	if INCLUDE_FUNCTION_MANAGER
	namespace FunctionManager
	{
		template <class type_t>
		std::unordered_map<std::string, std::function<type_t()>> function_arr;
		template <class type_t>
		bool Register(std::string strName, std::function<type_t()> func, bool overwrite = true)
		{
			if (!overwrite && function_arr<type_t>.count(strName) > 0) return false;
			function_arr<type_t>.insert(std::pair<std::string, std::function<type_t()>>(strName, func));
			return true;
		}
		template <class type_t>
		type_t Call(std::string strName)
		{
			if (function_arr<type_t>.count(strName) == 0 || function_arr<type_t>[strName] == BAD_FUNCTION) return (type_t)BAD_RETURN;

			try
			{
				type_t ret = (type_t)function_arr<type_t>[strName]();
				return ret;
			}

			catch (const std::exception & e)
			{
				function_arr<type_t>[strName] = BAD_FUNCTION;
				return (type_t)BAD_RETURN;
			}
		}
	}
#	endif //FunctionManager

#	if INCLUDE_MEMORY
	namespace Memory
	{
		void ZeroMem(void* src, size_t size);
		bool IsBadPointer(void* pointer);

		namespace Ex
		{
#			if defined(WIN)
			pid_t GetProcessIdByName(LPCWSTR processName);
			pid_t GetProcessIdByWindow(LPCSTR windowName);
			pid_t GetProcessIdByWindow(LPCSTR windowClass, LPCSTR windowName);
			HANDLE GetProcessHandle(pid_t pid);
			mem_t GetModuleAddress(LPCWSTR moduleName, pid_t pid);
			mem_t GetPointer(HANDLE hProc, mem_t ptr, std::vector<mem_t> offsets);
			BOOL WriteBuffer(HANDLE hProc, mem_t address, const void* value, SIZE_T size);
			BOOL ReadBuffer(HANDLE hProc, mem_t address, void* buffer, SIZE_T size);
#			elif defined(LINUX)
			pid_t GetProcessIdByName(std::string processName);
			void ReadBuffer(pid_t pid, mem_t address, void* buffer, size_t size);
			void WriteBuffer(pid_t pid, mem_t address, void* value, size_t size);
			bool IsProcessRunning(pid_t pid);
#			endif //Ex
		}

		namespace In
		{
			pid_t GetCurrentProcessID();
			template <class type_t>
			type_t Read(mem_t address)
			{
				if (IsBadPointer((void*)address)) return (type_t)BAD_RETURN;
				return *(type_t*)(address);
			}
			template <class type_t>
			bool Write(mem_t address, type_t value)
			{
				if (IsBadPointer((void*)address)) return false;
				*(type_t*)(address) = value;
				return true;
			}
#			if defined(WIN)
			HANDLE GetCurrentProcessHandle();
			mem_t GetModuleAddress(LPCWSTR moduleName);
			mem_t GetPointer(mem_t baseAddress, std::vector<mem_t> offsets);
			bool WriteBuffer(mem_t address, const void* value, SIZE_T size);
			bool ReadBuffer(mem_t address, void* buffer, SIZE_T size);
#			elif defined(LINUX)
			bool ReadBuffer(mem_t address, void* buffer, size_t size);
			bool WriteBuffer(mem_t address, void* value, size_t size);
#			endif //In

			namespace Hook
			{
				extern std::map<mem_t, std::vector<byte_t>> restore_arr;
				bool Restore(mem_t address);
#				if defined(WIN)
				bool Detour(byte_t* src, byte_t* dst, size_t size);
				byte_t* TrampolineHook(byte_t* src, byte_t* dst, size_t size);
#				endif //Hook
			}
		}
	}
#	endif //Memory
}
#endif //Framework