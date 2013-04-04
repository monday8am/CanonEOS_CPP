#include "FlashRuntimeExtensions.h"


extern "C"
{
	__declspec(dllexport) FREObject initCamera	  (FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
	__declspec(dllexport) FREObject executeCommand(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
	__declspec(dllexport) FREObject getEVF		  (FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
	__declspec(dllexport) FREObject releaseCamera (FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
	__declspec(dllexport) FREObject isSupported	  (FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);

	__declspec(dllexport) void initializer(void** extData, FREContextInitializer* ctxInitializer, FREContextFinalizer* ctxFinalizer);
	__declspec(dllexport) void finalizer(void* extData);
}