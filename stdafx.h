#pragma once

#pragma comment(lib, "winmm.lib")

#pragma comment(lib, "DbgHelp.Lib")
#pragma comment(lib, "ImageHlp")
#pragma comment(lib, "psapi")


#include <stdio.h>
#include <tchar.h>
#include <Windows.h>
#include <process.h>
#include <Windows.h>
#include <vector>
#include <winnt.h>

#include <psapi.h>
#include <dbghelp.h>
#include <crtdbg.h>
#include <tlhelp32.h>
#include <strsafe.h>


//------------------------------------------

#include "Stack(LockFree).h"
#include "MemoryPool(LockFree).h"

