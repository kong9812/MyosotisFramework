// Copyright (c) 2025 kong9812
#include <iostream>
#include <filesystem>
#include <set>
#include <chrono>
#include <unordered_set>

#include "iglm.h"
#include "iofbx.h"
#include "Logger.h"
#include <unordered_map>

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

	const char* g_modelsFolder = "resources/models/";
	const char* g_mfModelsFolder = "resources/models/MFModels/";

	const char* g_fbxExt = ".fbx";
	const char* g_gltfExt = ".gltf";
	const char* g_rawModelExts[] = { g_fbxExt, g_gltfExt };

	const char* g_mfModelExt = ".mfmodel";

	struct RawMeshData
	{
		std::vector<glm::vec3> position;
		std::vector<glm::vec3> normal;
		std::vector<glm::vec2> uv;
		std::vector<glm::vec4> color;

		std::vector<uint32_t> index;
	};

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


static void CreateMFModel(const RawMeshData& rawMeshData)
{
	// 貪欲法でメッシュレット分割
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
	RawMeshData rawMeshData{};
	uint32_t meshCount = scene->getMeshCount();
	for (uint32_t meshIndex = 0; meshIndex < meshCount; ++meshIndex)
	{
		const ofbx::Mesh* mesh = scene->getMesh(meshIndex);
		const ofbx::GeometryData& geomData = mesh->getGeometryData();
		const ofbx::Vec3Attributes positions = geomData.getPositions();

		for (uint32_t partitionIdx = 0; partitionIdx < geomData.getPartitionCount(); partitionIdx++)
		{
			const ofbx::GeometryPartition& geometryPartition = geomData.getPartition(partitionIdx);
			for (uint32_t polygonIdx = 0; polygonIdx < geometryPartition.polygon_count; polygonIdx++)
			{
				const ofbx::GeometryPartition::Polygon& polygon = geometryPartition.polygons[polygonIdx];

				for (uint32_t vertexIdx = polygon.from_vertex; vertexIdx < polygon.from_vertex + polygon.vertex_count; vertexIdx++)
				{
					ofbx::Vec3 v = positions.get(vertexIdx);
					rawMeshData.position.push_back(glm::vec3(v.x, v.y, v.z));
					// 仮normal
					rawMeshData.normal.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
					// 仮uv
					rawMeshData.uv.push_back(glm::vec2(0.0f, 0.0f));
					// 仮color
					rawMeshData.color.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
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
	CreateMFModel(rawMeshData);
}

int main()
{
	// デバッグ用出力 デバッグ用出力 デバッグ用出力 デバッグ用出力
	std::cout << "========================================CurrentPath========================================" << std::endl;
	std::cout << std::filesystem::current_path() << std::endl;
	// デバッグ用出力 デバッグ用出力 デバッグ用出力 デバッグ用出力

	std::vector<std::filesystem::path> rawModelFiles = ListFilesRecursive(g_modelsFolder, { g_fbxExt, g_gltfExt });
	std::vector<std::filesystem::path> mfModelFiles = ListFilesRecursive(g_mfModelsFolder, { g_mfModelExt });

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
		std::filesystem::path mfModelPath = std::filesystem::path(g_mfModelsFolder) / rawModelPath.filename().replace_extension(g_mfModelExt);
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
		std::filesystem::path mfModelPath = std::filesystem::path(g_mfModelsFolder) / path.filename().replace_extension(g_mfModelExt);
		if ((extension == ".fbx") || (extension == ".FBX"))
		{
			CreateMFModelFromFBX(path, mfModelPath);
		}
		else if ((extension == ".gltf") || (extension == ".GLTF"))
		{

		}
	}


	return 0;
}