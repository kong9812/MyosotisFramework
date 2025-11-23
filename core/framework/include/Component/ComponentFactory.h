// Copyright (c) 2025 kong9812
#pragma once
#include "ComponentType.h"
#include "ClassPointer.h"

namespace MyosotisFW {
	class ComponentBase;
	TYPEDEF_SHARED_PTR_FWD(ComponentBase);
}

namespace MyosotisFW::System::ComponentFactory
{
	ComponentBase_ptr CreateComponent(const uint32_t objectID, const ComponentType& objectType);
}