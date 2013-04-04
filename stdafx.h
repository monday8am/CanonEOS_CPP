// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// Windows Header Files:

#include <objidl.h>
#include <windows.h>
#include <gdiplus.h>
#include <string.h>

using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

// TODO: reference additional headers your program requires here

#include "EDSDK.h"
#include "EDSDKTypes.h"
#include "CameraModel.h"
#include "CameraModelLegacy.h"
#include "CameraController.h"
#include "CameraEventListener.h"
#include "FREEventObserver.h" 
#include "CanonEOSAne.h"


// TODO: reference additional headers your program requires here
