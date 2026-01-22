// Copyright (c) 2025 kong9812
#pragma once
#include "StaticMesh.h"
#include "ComponentCast.h"
#include "PrimitiveGeometryShape.h"
#include "istduuid.h"

namespace MyosotisFW::System::Render
{
	class PrimitiveGeometry : public StaticMesh
	{
	public:
		PrimitiveGeometry(const uint32_t objectID, const std::function<void(void)>& meshChangedCallback);
		~PrimitiveGeometry() {}

		struct MeshComponentInfo
		{
			Shape::PrimitiveGeometryShape primitiveGeometryShape = Shape::PrimitiveGeometryShape::UNDEFINED;
		};

		const ComponentType GetType() const override { return ComponentType::PrimitiveGeometryMesh; }

		void PrepareForRender(const RenderDevice_ptr& device, const RenderResources_ptr& resources) override;
		void Update(const UpdateData& updateData, const Camera::CameraBase_ptr& camera) override;
		void SetPrimitiveGeometryShape(const Shape::PrimitiveGeometryShape shape) { m_meshComponentInfo.primitiveGeometryShape = shape; loadAssets(); }
		const Shape::PrimitiveGeometryShape& GetPrimitiveGeometryShape() const { return m_meshComponentInfo.primitiveGeometryShape; }
		virtual rapidjson::Value Serialize(rapidjson::Document::AllocatorType& allocator) const override;
		virtual void Deserialize(const rapidjson::Value& doc) override;
	private:
		void loadAssets() override;
		MeshComponentInfo m_meshComponentInfo;

	public:
		static constexpr PropertyDesc::EnumItem PrimitiveGeometryShapeEnumItem[] = {
			{"UNDEFINED", static_cast<int32_t>(Shape::PrimitiveGeometryShape::UNDEFINED)},
			{"Quad", static_cast<int32_t>(Shape::PrimitiveGeometryShape::Quad)},
			{"Plane", static_cast<int32_t>(Shape::PrimitiveGeometryShape::Plane)},
			{"Circle", static_cast<int32_t>(Shape::PrimitiveGeometryShape::Circle)},
			{"Sphere", static_cast<int32_t>(Shape::PrimitiveGeometryShape::Sphere)},
			{"PlaneWithHole", static_cast<int32_t>(Shape::PrimitiveGeometryShape::PlaneWithHole)},
			{"Cylinder", static_cast<int32_t>(Shape::PrimitiveGeometryShape::Cylinder)},
			{"Capsule", static_cast<int32_t>(Shape::PrimitiveGeometryShape::Capsule)},
			{"Cone", static_cast<int32_t>(Shape::PrimitiveGeometryShape::Cone) },
			{"Arrow", static_cast<int32_t>(Shape::PrimitiveGeometryShape::Arrow) }
		};

		// ComponentProperty
		static const PropertyTable& StaticPropertyTable()
		{
			static const PropertyDesc props[] = {
				MakeProperty(uuids::hashMaker(), "Shape", "PrimitiveGeometry",
					PrimitiveGeometryShapeEnumItem, std::size(PrimitiveGeometryShapeEnumItem),
					PropertyDesc::PropertyFlags::None,
					+[](const void* obj)->PropertyDesc::PropertyValue
					{
						auto* o = static_cast<const PrimitiveGeometry*>(obj);
						return static_cast<int32_t>(o->m_meshComponentInfo.primitiveGeometryShape);
					},
					+[](void* obj, const PropertyDesc::PropertyValue& v, PropertyDesc::ChangeReason cr)
					{
						static_cast<PrimitiveGeometry*>(obj)->SetPrimitiveGeometryShape(static_cast<Shape::PrimitiveGeometryShape>(std::get<int32_t>(v)));
					}),
			};
			static const PropertyTable table{ &ComponentBase::StaticPropertyTable(), props, std::size(props) };
			return table;
		}
		const PropertyTable& GetPropertyTable() const override { return StaticPropertyTable(); }

	protected:
		virtual void OnPropertyChanged(uuids::uuid propertyID) {}

	};
	TYPEDEF_SHARED_PTR_ARGS(PrimitiveGeometry);
	OBJECT_CAST_FUNCTION(PrimitiveGeometry);
}