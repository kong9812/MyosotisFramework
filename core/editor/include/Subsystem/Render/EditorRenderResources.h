// Copyright (c) 2025 kong9812
#pragma once
#include "RenderResources.h"

namespace MyosotisFW::System::Render
{
	class EditorRenderResources : public RenderResources
	{
	public:
		EditorRenderResources(const RenderDevice_ptr& device, const RenderDescriptors_ptr& descriptors) :
			RenderResources(device, descriptors),
			m_editorRenderTarget{} {
		}
		~EditorRenderResources();

		void Initialize(const uint32_t& width, const uint32_t& height) override;
		void Resize(const uint32_t& width, const uint32_t& height) override;

		VMAImage& GetEditorRenderTarget() { return m_editorRenderTarget; }

	private:
		VMAImage m_editorRenderTarget;
	};
	TYPEDEF_SHARED_PTR_ARGS(EditorRenderResources);
}