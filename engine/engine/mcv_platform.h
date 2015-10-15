#ifndef INC_MCV_PLATFORM_H_
#define INC_MCV_PLATFORM_H_

#define _WIN32_WINNT  _WIN32_WINNT_WIN7
#define	 DIRECTINPUT_VERSION		0X0800

#include "targetver.h"

#define _USE_MATH_DEFINES
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#define NOMINMAX
#include <windows.h>

// C RunTime Header Files
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cinttypes>
#include <cmath>
#include <time.h>
#include <sstream>
#include <thread>
#include <memory>

// C++
#include <map>
#include <vector>

#include <LuaBridge.h>

extern "C" {
# include "lua.h"
# include "lauxlib.h"
# include "lualib.h"
}


#include <DirectXMath.h>

#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")
#include <dinput.h>

using DirectX::XMMATRIX;
using DirectX::XMVECTOR;
using DirectX::XMFLOAT2;
using DirectX::XMFLOAT3;
using DirectX::XMFLOAT4;

#include "utils/utils.h"
#include "utils/data_provider.h"
#include "utils/input.h"
#include "render/render.h"

#include "components/components.h"
#include "handles/handle.h"

#include <PxPhysicsAPI.h>
#include <foundation\PxFoundation.h>
#include "pxtask/PxCudaContextManager.h"
#include <physxprofilesdk\PxProfileZoneManager.h>



#endif

#ifdef _DEBUG
#pragma comment(lib, "PhysX3DEBUG_x64.lib")
#pragma comment(lib, "PhysX3CommonDEBUG_x64.lib")
#pragma comment(lib, "PhysX3ExtensionsDEBUG.lib")
#pragma comment(lib, "PhysXVisualDebuggerSDKDEBUG.lib") 
#pragma comment(lib, "PhysX3CharacterKinematicDEBUG_x64") 
#pragma comment(lib, "PhysX3CookingDEBUG_x64.lib")
//#pragma comment(lib, "PxToolkitDEBUG.lib")
#pragma comment(lib, "PxTaskDEBUG.lib")
#pragma comment(lib, "PhysXProfileSDKDEBUG.lib")
#else
#pragma comment(lib, "PhysX3_x64.lib")
#pragma comment(lib, "PhysX3Common_x64.lib")
#pragma comment(lib, "PhysX3Cooking_x64.lib")
#pragma comment(lib, "PhysX3Extensions.lib")
#pragma comment(lib, "PhysX3CharacterKinematic_x64.lib")
#pragma comment(lib, "PxTask.lib")
#pragma comment(lib, "PhysXProfileSDK.lib")

#endif
