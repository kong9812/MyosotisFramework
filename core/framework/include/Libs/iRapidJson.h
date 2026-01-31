// Copyright (c) 2025 kong9812
// For include RapidJSON
#pragma once
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/PrettyWriter.h>
#include <rapidjson/writer.h>

template <typename T>
inline void SerializeVec4ToJson(const char* key, const T& vec, rapidjson::Value& doc, rapidjson::Document::AllocatorType& allocator)
{
	rapidjson::Value vecArray(rapidjson::Type::kArrayType);
	vecArray.PushBack(vec.x, allocator);
	vecArray.PushBack(vec.y, allocator);
	vecArray.PushBack(vec.z, allocator);
	vecArray.PushBack(vec.w, allocator);
	doc.AddMember(rapidjson::Value(key, allocator), vecArray, allocator);
}

template <typename T>
inline void SerializeVec3ToJson(const char* key, const T& vec, rapidjson::Value& doc, rapidjson::Document::AllocatorType& allocator)
{
	rapidjson::Value vecArray(rapidjson::Type::kArrayType);
	vecArray.PushBack(vec.x, allocator);
	vecArray.PushBack(vec.y, allocator);
	vecArray.PushBack(vec.z, allocator);
	doc.AddMember(rapidjson::Value(key, allocator), vecArray, allocator);
}

template <typename T>
inline void SerializeVec2ToJson(const char* key, const T& vec, rapidjson::Value& doc, rapidjson::Document::AllocatorType& allocator)
{
	rapidjson::Value vecArray(rapidjson::Type::kArrayType);
	vecArray.PushBack(vec.x, allocator);
	vecArray.PushBack(vec.y, allocator);
	doc.AddMember(rapidjson::Value(key, allocator), vecArray, allocator);
}

template <typename T>
inline void DeserializeVec4FromJson(const char* key, T& vec, const rapidjson::Value& doc)
{
	const auto& vecArray = doc[key].GetArray();
	vec.x = vecArray[0].GetFloat();
	vec.y = vecArray[1].GetFloat();
	vec.z = vecArray[2].GetFloat();
	vec.z = vecArray[3].GetFloat();
}

template <typename T>
inline void DeserializeVec3FromJson(const char* key, T& vec, const rapidjson::Value& doc)
{
	const auto& vecArray = doc[key].GetArray();
	vec.x = vecArray[0].GetFloat();
	vec.y = vecArray[1].GetFloat();
	vec.z = vecArray[2].GetFloat();
}

template <typename T>
inline void DeserializeVec2FromJson(const char* key, T& vec, const rapidjson::Value& doc)
{
	const auto& vecArray = doc[key].GetArray();
	vec.x = vecArray[0].GetFloat();
	vec.y = vecArray[1].GetFloat();
}