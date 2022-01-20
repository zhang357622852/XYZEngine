#include "stdafx.h"
#include "ScriptEngineRegistry.h"

#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Scene/Components.h"
#include "XYZ/Animation/Animation.h"
#include "XYZ/Animation/Animator.h"
#include "XYZ/Utils/Delegate.h"

#include "ScriptWrappers.h"
#include "Wrappers/Components/TransformComponentWrappers.h"
#include "Wrappers/Components/SpriteRendererWrappers.h"
#include "Wrappers/Components/RigidBody2DWrapper.h"

#include "Wrappers/SceneEntityNative.h"
#include "Wrappers/InputNative.h"



namespace XYZ {	

	void ScriptEngineRegistry::RegisterAll()
	{
		// Transform component
		Script::InputNative::Register();
		Script::SceneEntityNative::Register();
		Script::TransformComponentNative::Register();
		Script::RigidBody2DNative::Register();
		////////////////////////
	}
}