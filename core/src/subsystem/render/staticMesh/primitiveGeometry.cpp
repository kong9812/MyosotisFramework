// Copyright (c) 2025 kong9812
#include "primitiveGeometry.h"

namespace MyosotisFW::System::Render
{
	PrimitiveGeometry::PrimitiveGeometry(RenderDevice_ptr device, RenderResources_ptr resources, VkRenderPass renderPass, VkPipelineCache pipelineCache) :
		StaticMesh(device, resources, renderPass, pipelineCache)
	{
		// プリミティブジオメトリの作成
		loadAssets();
		prepareUniformBuffers();
		prepareShaderStorageBuffers();
		prepareDescriptors();
		prepareRenderPipeline();

	}

	PrimitiveGeometry::~PrimitiveGeometry()
	{

	}

	void PrimitiveGeometry::BindCommandBuffer()
	{

	}

	void PrimitiveGeometry::loadAssets()
	{
		
	}

	void PrimitiveGeometry::prepareUniformBuffers()
	{
		
	}

	void PrimitiveGeometry::prepareDescriptors()
	{
		__super::prepareDescriptors();
	}

	void PrimitiveGeometry::prepareRenderPipeline()
	{
		__super::prepareRenderPipeline();
	}
}