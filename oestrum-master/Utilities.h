#pragma once

// Includes
#include <time.h>
#include <Windows.h>
#include <string>

// Colors for the console
//Define extra colours
#define FOREGROUND_WHITE		    (FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN)
#define FOREGROUND_YELLOW       	(FOREGROUND_RED | FOREGROUND_GREEN)
#define FOREGROUND_CYAN		        (FOREGROUND_BLUE | FOREGROUND_GREEN)
#define FOREGROUND_MAGENTA	        (FOREGROUND_RED | FOREGROUND_BLUE)
#define FOREGROUND_BLACK		    0

#define FOREGROUND_INTENSE_RED		(FOREGROUND_RED | FOREGROUND_INTENSITY)
#define FOREGROUND_INTENSE_GREEN	(FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define FOREGROUND_INTENSE_BLUE		(FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define FOREGROUND_INTENSE_WHITE	(FOREGROUND_WHITE | FOREGROUND_INTENSITY)
#define FOREGROUND_INTENSE_YELLOW	(FOREGROUND_YELLOW | FOREGROUND_INTENSITY)
#define FOREGROUND_INTENSE_CYAN		(FOREGROUND_CYAN | FOREGROUND_INTENSITY)
#define FOREGROUND_INTENSE_MAGENTA	(FOREGROUND_MAGENTA | FOREGROUND_INTENSITY)

// Utilities Namespace
// Purpose: Contains misc functionality for memory related functionality
namespace Utilities
{
	// Debug console controls
	void OpenConsole(std::string Title);
	void CloseConsole();

	void EnableLogFile(std::string filename);

	std::string GetModuleFilePath(HMODULE hModule);

	std::string GetModuleBaseDir(HMODULE hModule);

	void SetConsoleColor(WORD color);

	// Misc Shizz
	std::string GetTimeString();


	// Memory utils
	// Purpose: Provides memeory related functionality (pattern scanning ect)
	namespace Memory
	{
		// Waits for a module to be available, before returning it's base address
		DWORD WaitOnModuleHandle(std::string moduleName);

		uint64_t FindPatternIDA(const char * szModule, const char * szSignature);

		// Attempts to locate the given signature and mask in a memory range
		// Returns the address at which it has been found
		DWORD FindPattern(std::string moduleName, BYTE* Mask, char* szMask);

		// Attempts to locate the given text in a memory range
		// Returns the address at which it has been found
		DWORD FindTextPattern(std::string moduleName, char* string);

		class VMTManager
		{
		private:
			DWORD	*CustomTable;
			bool	initComplete;
			DWORD	*OriginalTable;
			DWORD	*Instance;

			int		MethodCount(DWORD* InstancePointer);

		public:
			bool	Initialise(DWORD* InstancePointer); // Pass a &class

			DWORD	HookMethod(DWORD NewFunction, int Index);
			void	UnhookMethod(int Index);

			void	RestoreOriginal();
			void	RestoreCustom();

			DWORD	GetOriginalFunction(int Index);
		};
	};
};

