#pragma once
#include "XYZ/Script/ScriptEngine.h"

namespace XYZ {
	namespace Script {
		struct SceneEntityNative
		{
			static void Register();
			
			static uint32_t Create();
		};
	}
}