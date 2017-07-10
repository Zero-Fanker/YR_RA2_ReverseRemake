#pragma once

//ifndef str
#define str(x) str_(x)
#define str_(x) #x
//endif

//include <YRPP.h>
#include <Helpers/Macro.h>

#define GFX_DX_HW 0x01l
#define GFX_DX_EM 0x02l

#define GFX_SU_VRAM 0x00l
#define GFX_SU_SYSTEM 0x01l

#define GFX_SU_NF3D 0x00l
#define GFX_SU_F3D 0x01l

class AbstractClass;
class AircraftTypeClass;
class TechnoTypeClass;
class CellClass;
class HouseClass;
class CCINIClass;
class MixFileClass;
class CustomPalette;
struct SHPStruct;

class Hares
{
public:
	enum class ExceptionHandlerMode {
		Default = 0,
		Full = 1,
		NoRemove = 2
	};


	//Global Options
	static HANDLE	hInstance;
	static PVOID pExceptionHandler;
	static ExceptionHandlerMode ExceptionMode;
	static const DWORD MS_VC_EXCEPTION = 0x406D1388ul;
	static void CheckProcessorFeatures();

	static void __stdcall ExeRun();
	static void __stdcall ExeTerminate();

	[[noreturn]] void static  Exit(UINT ExitCode);
	[[noreturn]] static LONG CALLBACK ExceptionHandler(PEXCEPTION_POINTERS pExs);
	static LONG CALLBACK ExceptionFilter(PEXCEPTION_POINTERS pExs);
	//General functions

};
