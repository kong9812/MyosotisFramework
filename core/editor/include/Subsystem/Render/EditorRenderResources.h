// Copyright (c) 2025 kong9812
#pragma once
#include "RenderResources.h"

namespace MyosotisFW::System::Render
{
	class EditorRenderResources : public RenderResources
	{
	public:
		EditorRenderResources(const RenderDevice_ptr& device) :
			RenderResources(device),
			m_editorRenderTarget{} {
		}
		~EditorRenderResources();

		void Initialize(const uint32_t width, const uint32_t height) override;
		void Resize(const uint32_t width, const uint32_t height) override;

		Image& GetEditorRenderTarget() { return m_editorRenderTarget; }

	private:
		Image m_editorRenderTarget;
	};
	TYPEDEF_SHARED_PTR_ARGS(EditorRenderResources);
}