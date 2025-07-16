// Copyright (c) 2025 kong9812
#pragma once
#include "ComponentType.h"
#include "ComponentBase.h"

namespace MyosotisFW
{
	template < class To, class From >
	inline std::shared_ptr<To> Object_Cast(const std::shared_ptr<From>& obj)
	{
		return std::dynamic_pointer_cast<To>(obj);
	}
}

#define OBJECT_CAST_FUNCTION(To)																\
    inline std::shared_ptr<To> Object_CastTo##To(const std::shared_ptr<ComponentBase>& obj) {	\
        return std::dynamic_pointer_cast<To>(obj);												\
    }