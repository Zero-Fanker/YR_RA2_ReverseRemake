#include "Hares.h"
#include "Hares.version.h"
#include "Hares.CRT.h"
//include "CallCenter.h"

#include <stdio.h>
#include <new>

//Init Statics
HANDLE Hares::hInstance = 0;
PVOID Hares::pExceptionHandler = nullptr;
Hares::ExceptionHandlerMode Hares::ExceptionMode = Hares::ExceptionHandlerMode::Default;

void __stdcall Hares::ExeRun()
{
	auto const process = GetCurrentProcess();
	DWORD_PTR const processAffinityMask = 1; // limit to first processor
	SetProcessAffinityMask(process, processAffinityMask);

#if _MSC_VER >= 1700
	// install a new exception handler, if this version of Windows supports it
	if(Hares::ExceptionMode != Hares::ExceptionHandlerMode::Default) {
		if(HINSTANCE handle = GetModuleHandle(TEXT("kernel32.dll"))) {
			if(GetProcAddress(handle, "AddVectoredExceptionHandler")) {
				Hares::pExceptionHandler = AddVectoredExceptionHandler(1, Hares::ExceptionFilter);
			}
		}
	}
#endif
}

[[noreturn]] void Hares::Exit(UINT ExitCode) {
	ExitProcess(ExitCode);
}

LONG CALLBACK Hares::ExceptionFilter(PEXCEPTION_POINTERS const pExs)
{
	switch (pExs->ExceptionRecord->ExceptionCode) {
	case EXCEPTION_BREAKPOINT:
	case MS_VC_EXCEPTION:
		return EXCEPTION_CONTINUE_SEARCH;
	}

	Hares::ExceptionHandler(pExs);
}

[[noreturn]] LONG CALLBACK Hares::ExceptionHandler(PEXCEPTION_POINTERS const pExs)
{
	Hares::Exit(pExs->ExceptionRecord->ExceptionCode);
};

void __stdcall Hares::ExeTerminate()
{
	if(Hares::pExceptionHandler && Hares::ExceptionMode != Hares::ExceptionHandlerMode::NoRemove) {
		RemoveVectoredExceptionHandler(Hares::pExceptionHandler);
		Hares::pExceptionHandler = nullptr;
	}
}

void Hares::CheckProcessorFeatures() {
	BOOL supported = FALSE;
#if _M_IX86_FP == 0 // IA32
	Debug::Log("Hares is not using enhanced instruction set.\n");
#elif _M_IX86_FP == 1 // SSE
#define INSTRUCTION_SET_NAME "SSE"
	supported = IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE);
#elif _M_IX86_FP == 2 && !__AVX__ // SEE2, not AVX
#define INSTRUCTION_SET_NAME "SSE2"
	supported = IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE);
#else // all others, untested. add more #elifs to support more
	static_assert(false, "Hares compiled using unsupported architecture.");
#endif

#ifdef INSTRUCTION_SET_NAME

	if(!supported) {
		MessageBoxA(nullptr,
			"This version of Hares requires a CPU with " INSTRUCTION_SET_NAME
			" support.\n\nYour CPU does not support " INSTRUCTION_SET_NAME ". "
			"Hares will now exit.",
			"Hares - CPU Requirements", MB_ICONERROR);
		Hares::ExeTerminate();
		Hares::Exit(553);
	}
#endif
}

//DllMain
bool __stdcall DllMain(HANDLE hInstance,DWORD dwReason,LPVOID v)
{
	switch(dwReason) {
		case DLL_PROCESS_ATTACH:
			Hares::hInstance = hInstance;
			break;
		case DLL_PROCESS_DETACH:
			break;
	}

	return true;
}

DEFINE_HOOK(7CD810, ExeRun, 9)
{
	Hares::ExeRun();
	return 0;
}

DEFINE_HOOK(7CD8EF, ExeTerminate, 9)
{
	Hares::ExeTerminate();
	GET(UINT, result, EAX);
	ExitProcess(result); //teehee
}

const DWORD YR_SIZE_1000 = 0x496110;
const DWORD YR_SIZE_1001 = 0x497110;
const DWORD YR_SIZE_1001_UC = 0x497FE0;
const DWORD YR_SIZE_NPATCH = 0x5AB000;

const DWORD YR_TIME_1000 = 0x3B846665;
const DWORD YR_TIME_1001 = 0x3BDF544E;

const DWORD YR_CRC_1000 = 0xB701D792;
const DWORD YR_CRC_1001_CD = 0x098465B3;
const DWORD YR_CRC_1001_TFD = 0xEB903080;
const DWORD YR_CRC_1001_UC = 0x1B499086;

SYRINGE_HANDSHAKE(pInfo)
{
	if(pInfo) {
		const char* AcceptMsg = "Found Yuri's Revenge %s. Applying Hares " PRODUCT_STR ".";
		const char* PatchDetectedMessage = "Found %s. Hares " PRODUCT_STR " is not compatible with other patches.";

		const char* desc = nullptr;
		const char* msg = nullptr;
		bool allowed = false;

		// accept tfd and cd version 1.001
		if(pInfo->exeTimestamp == YR_TIME_1001) {

			// don't accept expanded exes
			switch(pInfo->exeFilesize) {
			case YR_SIZE_1001:
			case YR_SIZE_1001_UC:

				// all versions allowed
				switch(pInfo->exeCRC) {
				case YR_CRC_1001_CD:
					desc = "1.001 (CD)";
					break;
				case YR_CRC_1001_TFD:
					desc = "1.001 (TFD)";
					break;
				case YR_CRC_1001_UC:
					desc = "1.001 (UC)";
					break;
				default:
					// no-cd, network port or other changes
					desc = "1.001 (modified)";
				}
				msg = AcceptMsg;
				allowed = true;
				break;

			case YR_SIZE_NPATCH:
				// known patch size
				desc = "RockPatch or an NPatch-derived patch";
				msg = PatchDetectedMessage;
				break;

			default:
				// expanded exe, unknown make
				desc = "an unknown game patch";
				msg = PatchDetectedMessage;
			}
		} else if(pInfo->exeTimestamp == YR_TIME_1000) {
			// upgrade advice for version 1.000
			desc = "1.000";
			msg = "Found Yuri's Revenge 1.000 but Hares " PRODUCT_STR " requires version 1.001. Please update your copy of Yuri's Revenge first.";
		} else {
			// does not even compute...
			msg = "Unknown executable. Hares " PRODUCT_STR " requires Command & Conquer Yuri's Revenge version 1.001 (gamemd.exe).";
		}

		// generate the output message
		if(pInfo->Message) {
			sprintf_s(pInfo->Message, pInfo->cchMessage, msg, desc);
		}

		return allowed ? S_OK : S_FALSE;
	}

	return E_POINTER;
}
