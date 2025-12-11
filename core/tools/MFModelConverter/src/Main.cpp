// Copyright (c) 2025 kong9812
#define NOMINMAX
#include <iostream>
#include <filesystem>
#include <set>
#include <chrono>
#include <unordered_set>
#include <unordered_map>

#include "iglm.h"

#include "itiny_gltf.h"
#include "iofbx.h"

#include "imeshoptimizer.h"
#include "ixatlas.h"

#include "AppInfo.h"
#include "Logger.h"
#include "Mesh.h"
#include "MFModelIo.h"
#include "RawMeshData.h"

namespace {
	// resources/models/フォルダを見て
	// 以下に存在するgltfとfbxを全部リスト化
	// resources/models/MFModels/以下の.mfmodelを全部リスト化
	// それぞれのリストに含むファイルのタイムスタンプを比較し、glfwとfbxの方が新しければ
	// mfmodelを再生成する
	// mfmodelの保存先はresources/models/MFModels/
	// mfmodelの命名規則は元ファイル名.mfmodelとする
	// 例: resources/models/character.fbx -> resources/models/MFModels/character.mfmodel
	// 例: resources/models/scene.gltf -> resources/models/MFModels/scene.mfmodel
	// mfmodelの中身はglTFやfbxを独自バイナリ形式で保存したものとする

	const char* g_fbxExt = ".fbx";
	const char* g_gltfExt = ".gltf";
	const char* g_rawModelExts[] = { g_fbxExt, g_gltfExt };

	const char* g_mfModelExt = ".mfmodel";

	struct Meshlet
	{
		std::vector<uint32_t> triangles;  // 三角形 index (tri id)
		std::vector<uint32_t> uniqueVertices;
	};
}

// 指定フォルダ下の拡張子リストを取得
static std::vector<std::filesystem::path> ListFilesRecursive(const std::filesystem::path& root, const std::set<std::string>& exts)
{
	std::vector<std::filesystem::path> out;
	if (!std::filesystem::exists(root)) return out;
	for (auto& it : std::filesystem::recursive_directory_iterator(root))
	{
		if (!it.is_regular_file()) continue;
		auto ext = it.path().extension().string();
		for (auto& e : exts)
		{
			if (std::equal(e.rbegin(), e.rend(), ext.rbegin(), ext.rend(),
				[](char a, char b) { return std::tolower(a) == std::tolower(b); }))
			{
				out.push_back(it.path());
				break;
			}
		}
	}
	return out;
}

static uint64_t FileTimestamp_ms(const std::filesystem::path& p)
{
	auto ftime = std::filesystem::last_write_time(p);
	auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
		ftime - std::filesystem::file_time_type::clock::now()
		+ std::chrono::system_clock::now()
	);
	// 精度を ms に合わせる簡易方法（portable enough）
	auto dur = sctp.time_since_epoch();
	return (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
}


static void CreateMFModel(const char* name, MyosotisFW::RawMeshData& rawMeshData)
{
	// UV1の作成
	glm::ivec2 atlasSize = xatlas::BuildLightmapUV(rawMeshData.vertex, rawMeshData.index);

	// メッシュレット数の上限
	size_t maxMeshlets = meshopt_buildMeshletsBound(
		rawMeshData.index.size(),
		MyosotisFW::AppInfo::g_maxMeshletVertices,
		MyosotisFW::AppInfo::g_maxMeshletPrimitives);

	// meshoptimizer用データ作成
	std::vector<meshopt_Meshlet> meshlets(maxMeshlets);
	std::vector<uint32_t> meshletVertices(maxMeshlets * MyosotisFW::AppInfo::g_maxMeshletVertices);
	std::vector<uint8_t> meshletTriangles(maxMeshlets * MyosotisFW::AppInfo::g_maxMeshletPrimitives * 3);

	// メッシュレット生成
	size_t meshletCount = meshopt_buildMeshlets(
		meshlets.data(),
		meshletVertices.data(),
		meshletTriangles.data(),
		rawMeshData.index.data(),
		rawMeshData.index.size(),
		&rawMeshData.vertex[0].position.x,
		rawMeshData.vertex.size(),
		sizeof(MyosotisFW::VertexData),
		MyosotisFW::AppInfo::g_maxMeshletVertices,
		MyosotisFW::AppInfo::g_maxMeshletPrimitives,
		0.0f);

	glm::vec3 meshAABBMin(FLT_MAX);
	glm::vec3 meshAABBMax(-FLT_MAX);

	MyosotisFW::Mesh meshData{};
	meshData.vertex = rawMeshData.vertex;
	meshData.meshlet.reserve(meshletCount);
	for (size_t i = 0; i < meshletCount; i++)
	{
		const meshopt_Meshlet& src = meshlets[i];
		MyosotisFW::Meshlet dst{};

		// UniqueIndex (GlobalIndex)
		dst.uniqueIndex.reserve(src.vertex_count);
		for (size_t v = 0; v < src.vertex_count; v++)
		{
			uint32_t vertexIndex = meshletVertices[src.vertex_offset + v];
			dst.uniqueIndex.push_back(vertexIndex);
		}

		// Primitives (LocalIndex)
		dst.primitives.reserve(src.triangle_count * 3);
		for (size_t t = 0; t < src.triangle_count * 3; t++)
		{
			uint32_t triangleIndex = static_cast<uint32_t>(meshletTriangles[src.triangle_offset + t]);
			dst.primitives.push_back(triangleIndex);
		}

		// AABB
		glm::vec3 p0 = rawMeshData.vertex[meshletVertices[src.vertex_offset + 0]].position;
		dst.meshletInfo.AABBMin = glm::vec4(p0, 0.0f);
		dst.meshletInfo.AABBMax = glm::vec4(p0, 0.0f);
		meshAABBMin = glm::min(meshAABBMin, p0);
		meshAABBMax = glm::max(meshAABBMax, p0);
		for (size_t v = 1; v < src.vertex_count; v++)
		{
			uint32_t vertexIndex = meshletVertices[src.vertex_offset + v];
			const glm::vec3& pos = rawMeshData.vertex[vertexIndex].position;
			dst.meshletInfo.AABBMin = glm::min(dst.meshletInfo.AABBMin, glm::vec4(pos, 0.0f));
			dst.meshletInfo.AABBMax = glm::max(dst.meshletInfo.AABBMax, glm::vec4(pos, 0.0f));
			meshAABBMin = glm::min(meshAABBMin, pos);
			meshAABBMax = glm::max(meshAABBMax, pos);
		}

		dst.meshletInfo.vertexCount = src.vertex_count;
		dst.meshletInfo.primitiveCount = src.triangle_count;

		meshData.meshlet.push_back(dst);
	}
	meshData.meshInfo.AABBMin = glm::vec4(meshAABBMin, 0.0f);
	meshData.meshInfo.AABBMax = glm::vec4(meshAABBMax, 0.0f);
	meshData.meshInfo.meshletCount = meshletCount;
	meshData.meshInfo.vertexFloatCount = static_cast<uint32_t>(rawMeshData.vertex.size()) * (sizeof(MyosotisFW::VertexData) / sizeof(float));
	meshData.meshInfo.atlasSize = atlasSize;

	Utility::Loader::SerializeMFModel(name, meshData);
}

static void CreateMFModelFromFBX(const std::filesystem::path& fbxPath, const std::filesystem::path& mfModelPath)
{
	// FBX読み込み
	std::ifstream file(fbxPath, std::ios::ate | std::ios::binary);
	ASSERT(file.is_open(), "Failed to open fbx file: " + fbxPath.string());
	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<char> buf(fileSize);
	file.seekg(0);
	file.read(buf.data(), fileSize);
	file.close();
	ofbx::IScene* scene = ofbx::load(
		reinterpret_cast<ofbx::u8*>(buf.data()),
		static_cast<ofbx::usize>(fileSize),
		static_cast<ofbx::u16>(ofbx::LoadFlags::NONE));

	// メッシュデータ抽出
	MyosotisFW::RawMeshData rawMeshData{};
	uint32_t meshCount = scene->getMeshCount();
	for (uint32_t meshIndex = 0; meshIndex < meshCount; ++meshIndex)
	{
		const ofbx::Mesh* mesh = scene->getMesh(meshIndex);
		const ofbx::GeometryData& geomData = mesh->getGeometryData();
		const ofbx::Vec3Attributes positions = geomData.getPositions();
		const ofbx::Vec3Attributes normal = geomData.getNormals();
		const ofbx::Vec2Attributes uv = geomData.getUVs();
		const ofbx::Vec4Attributes color = geomData.getColors();

		for (uint32_t partitionIdx = 0; partitionIdx < geomData.getPartitionCount(); partitionIdx++)
		{
			const ofbx::GeometryPartition& geometryPartition = geomData.getPartition(partitionIdx);
			for (uint32_t polygonIdx = 0; polygonIdx < geometryPartition.polygon_count; polygonIdx++)
			{
				const ofbx::GeometryPartition::Polygon& polygon = geometryPartition.polygons[polygonIdx];

				for (uint32_t vertexIdx = polygon.from_vertex; vertexIdx < polygon.from_vertex + polygon.vertex_count; vertexIdx++)
				{
					glm::vec3 v = ToGlmVec3(positions.get(vertexIdx));
					glm::vec3 n = glm::vec3(0.0f);
					glm::vec2 u0 = glm::vec2(0.0f);
					glm::vec2 u1 = glm::vec2(0.0f);
					glm::vec4 c = glm::vec4(1.0f);

					// Normal
					if (normal.count > vertexIdx)
					{
						n = ToGlmVec3(normal.get(vertexIdx));
					}

					// UV0
					if (uv.count > vertexIdx)
					{
						u0 = ToGlmVec2(uv.get(vertexIdx));
					}

					// Color
					if (color.count > vertexIdx)
					{
						c = ToGlmVec4(color.get(vertexIdx));
					}

					rawMeshData.vertex.insert(rawMeshData.vertex.end(), { v, n, u0, u1, c });
				}
			}

			for (uint32_t polygonIdx = 0; polygonIdx < geometryPartition.polygon_count; polygonIdx++)
			{
				const ofbx::GeometryPartition::Polygon& polygon = geometryPartition.polygons[polygonIdx];
				int triangle[3]{};
				triangulate(geomData, polygon, triangle);
				for (uint32_t i = 0; i < 3; i++)
				{
					rawMeshData.index.push_back(static_cast<uint32_t>(triangle[i]));
				}
			}
		}
	}
	scene->destroy();

	// mfmodel作成
	CreateMFModel(fbxPath.stem().string().c_str(), rawMeshData);
}

static const float* GetGLTFFloatData(const tinygltf::Model& glTFModel, const tinygltf::Primitive& primitive, const char* attributeName, size_t* count = nullptr)
{
	if (primitive.attributes.find(attributeName) == primitive.attributes.end()) return nullptr;

	const tinygltf::Accessor& accessor = glTFModel.accessors[primitive.attributes.find(attributeName)->second];
	const tinygltf::BufferView& view = glTFModel.bufferViews[accessor.bufferView];
	if (count) *count = accessor.count;
	return reinterpret_cast<const float*>(&(glTFModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
}
static void CreateMFModelFromGLTF(const std::filesystem::path& gltfPath, const std::filesystem::path& mfModelPath)
{
	// GLTF読み込み
	tinygltf::Model glTFModel{};
	tinygltf::TinyGLTF glTFLoader{};
	std::string error{};
	std::string warning{};

	bool fileLoaded = glTFLoader.LoadASCIIFromFile(&glTFModel, &error, &warning, gltfPath.string());
	ASSERT(fileLoaded, "Failed to open gltf file: " + gltfPath.string() + "\nerror: " + error);

	// メッシュデータ抽出
	MyosotisFW::RawMeshData rawMeshData{};
	for (const tinygltf::Mesh& mesh : glTFModel.meshes)
	{
		for (const tinygltf::Primitive& primitive : mesh.primitives)
		{
			size_t vertexCount = 0;
			const float* positionBuffer = GetGLTFFloatData(glTFModel, primitive, "POSITION", &vertexCount);
			const float* normalBuffer = GetGLTFFloatData(glTFModel, primitive, "NORMAL");
			const float* uvBuffer = GetGLTFFloatData(glTFModel, primitive, "TEXCOORD_0");
			const float* colorBuffer = GetGLTFFloatData(glTFModel, primitive, "COLOR_0");

			// Vertex
			for (size_t vertex = 0; vertex < vertexCount; vertex++)
			{
				glm::vec3 v = glm::vec3(0.0f);
				glm::vec3 n = glm::vec3(0.0f);
				glm::vec2 u0 = glm::vec2(0.0f);
				glm::vec2 u1 = glm::vec2(0.0f);
				glm::vec4 c = glm::vec4(1.0f);

				if (positionBuffer != nullptr)
				{
					v = glm::make_vec3(&positionBuffer[vertex * 3]);
				}

				if (normalBuffer != nullptr)
				{
					n = glm::normalize(glm::make_vec3(&normalBuffer[vertex * 3]));
				}

				if (uvBuffer != nullptr)
				{
					u0 = glm::make_vec2(&uvBuffer[vertex * 2]);
				}

				if (colorBuffer != nullptr)
				{
					c = glm::vec4(glm::make_vec3(&colorBuffer[vertex * 3]), 1.0f);
				}

				rawMeshData.vertex.insert(rawMeshData.vertex.end(), { v, n, u0, u1, c });
			}

			// Index
			const tinygltf::Accessor& accessor = glTFModel.accessors[primitive.indices];
			const tinygltf::BufferView& view = glTFModel.bufferViews[accessor.bufferView];
			const tinygltf::Buffer& buffer = glTFModel.buffers[view.buffer];
			const uint32_t trianglesCount = accessor.count / 3;
			switch (accessor.componentType) {
			case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
				const uint32_t* buf = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + view.byteOffset]);
				for (size_t primitiveIndex = 0; primitiveIndex < trianglesCount; primitiveIndex++) {
					const uint32_t triangle[3] = {
						static_cast<uint32_t>(buf[primitiveIndex * 3 + 0]),
						static_cast<uint32_t>(buf[primitiveIndex * 3 + 1]),
						static_cast<uint32_t>(buf[primitiveIndex * 3 + 2])
					};
					rawMeshData.index.insert(rawMeshData.index.end(), { triangle[0], triangle[1], triangle[2] });
				}
				break;
			}
			case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
				const uint16_t* buf = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + view.byteOffset]);
				for (size_t primitiveIndex = 0; primitiveIndex < trianglesCount; primitiveIndex++) {
					const uint32_t triangle[3] = {
						static_cast<uint32_t>(buf[primitiveIndex * 3 + 0]),
						static_cast<uint32_t>(buf[primitiveIndex * 3 + 1]),
						static_cast<uint32_t>(buf[primitiveIndex * 3 + 2])
					};
					rawMeshData.index.insert(rawMeshData.index.end(), { triangle[0], triangle[1], triangle[2] });
				}
				break;
			}
			case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
				const uint8_t* buf = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + view.byteOffset]);
				for (size_t primitiveIndex = 0; primitiveIndex < trianglesCount; primitiveIndex++) {
					const uint32_t triangle[3] = {
						static_cast<uint32_t>(buf[primitiveIndex * 3 + 0]),
						static_cast<uint32_t>(buf[primitiveIndex * 3 + 1]),
						static_cast<uint32_t>(buf[primitiveIndex * 3 + 2])
					};
					rawMeshData.index.insert(rawMeshData.index.end(), { triangle[0], triangle[1], triangle[2] });
				}
				break;
			}
			default:
				ASSERT(false, "Index component type not supported! File: " + gltfPath.string());
				break;
			}
		}
	}

	// mfmodel作成
	CreateMFModel(gltfPath.stem().string().c_str(), rawMeshData);
}

int main()
{
	// デバッグ用出力 デバッグ用出力 デバッグ用出力 デバッグ用出力
	std::cout << "========================================CurrentPath========================================" << std::endl;
	std::cout << std::filesystem::current_path() << std::endl;
	// デバッグ用出力 デバッグ用出力 デバッグ用出力 デバッグ用出力

	std::vector<std::filesystem::path> rawModelFiles = ListFilesRecursive(MyosotisFW::AppInfo::g_modelFolder, { g_fbxExt, g_gltfExt });
	std::vector<std::filesystem::path> mfModelFiles = ListFilesRecursive(MyosotisFW::AppInfo::g_mfModelFolder, { g_mfModelExt });

	// デバッグ用出力 デバッグ用出力 デバッグ用出力 デバッグ用出力
	std::cout << "========================================RawModel========================================" << std::endl;
	for (const auto& rawModelFile : rawModelFiles)
	{
		std::cout << rawModelFile.filename() << std::endl;
	}
	std::cout << "========================================MFModel========================================" << std::endl;
	for (const auto& mfModelFile : mfModelFiles)
	{
		std::cout << mfModelFile.filename() << std::endl;
	}
	// デバッグ用出力 デバッグ用出力 デバッグ用出力 デバッグ用出力

	// mfModelファイル名リスト作成
	std::unordered_set<std::filesystem::path> mfModelFileName{};
	for (const std::filesystem::path& mfModelName : mfModelFiles)
	{
		if (!mfModelFileName.insert(mfModelName.filename()).second)
		{
			std::cout << "[Warning] Duplicate MFModel file name: " << mfModelName.filename() << std::endl;
		}
	}
	// mfModel生成リスト作成
	std::cout << "========================================mfModelCreateList========================================" << std::endl;
	std::vector<std::filesystem::path> mfModelCreateList{};
	for (const std::filesystem::path& rawModelPath : rawModelFiles)
	{
		std::filesystem::path mfModelPath = std::filesystem::path(MyosotisFW::AppInfo::g_mfModelFolder) / rawModelPath.filename().replace_extension(g_mfModelExt);
		uint64_t rawModelTimestamp = FileTimestamp_ms(rawModelPath);
		uint64_t mfModelTimestamp = 0;
		if (std::filesystem::exists(mfModelPath))
		{
			mfModelTimestamp = FileTimestamp_ms(mfModelPath);
		}
		if (rawModelTimestamp > mfModelTimestamp)
		{
			std::cout << "[Info] MFModel needs to be regenerated for: " << rawModelPath.filename() << std::endl;
			mfModelCreateList.push_back(rawModelPath);
		}
		else
		{
			std::cout << "[Info] MFModel is up to date for: " << rawModelPath.filename() << std::endl;
		}
	}

	// mfModel作成！
	std::cout << "========================================MFModel Generation Start========================================" << std::endl;
	for (const std::filesystem::path& path : mfModelCreateList)
	{
		std::cout << "[Info] Generating MFModel: " << path.filename() << std::endl;
		// 拡張子取得
		std::string extension = path.extension().string();
		std::filesystem::path mfModelPath = std::filesystem::path(MyosotisFW::AppInfo::g_mfModelFolder) / path.filename().replace_extension(g_mfModelExt);
		if ((extension == ".fbx") || (extension == ".FBX"))
		{
			CreateMFModelFromFBX(path, mfModelPath);
		}
		else if ((extension == ".gltf") || (extension == ".GLTF"))
		{
			CreateMFModelFromGLTF(path, mfModelPath);
		}
	}
	return 0;
}