// Copyright (c) 2025 kong9812
#pragma once
#include "RenderResources.h"

namespace MyosotisFW::System::Render
{
	class EditorRenderResources : public RenderResources
	{
	public:
		EditorRenderResources(const RenderDevice_ptr& device, const RenderDescriptors_ptr& renderDescriptors) :
			RenderResources(device, renderDescriptors) {
		}
		~EditorRenderResources();

		void Initialize(const glm::ivec2& screenSize) override;
		void Resize(const glm::ivec2& screenSize) override;

	private:

	};
	TYPEDEF_SHARED_PTR_ARGS(EditorRenderResources);
}