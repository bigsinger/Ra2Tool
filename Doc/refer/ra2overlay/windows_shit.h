#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
// Disable min/max macros in minwindef.h. They are sucks!
#ifndef NOMINMAX  
#define NOMINMAX  
#endif
#include <windows.h>
#include <winsock2.h>

#ifdef _DEBUG
#include <Dbghelp.h>
#include <tlhelp32.h>
#include <winternl.h>
#endif
