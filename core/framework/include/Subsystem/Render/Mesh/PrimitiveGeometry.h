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

		const ComponentType GetType() const override { return ComponentType::PrimitiveGeometryMesh; }

		void PrepareForRender(const RenderDevice_ptr& device, const RenderResources_ptr& resources) override;
		void Update(const UpdateData& updateData, const Camera::CameraBase_ptr& camera) override;
		void SetPrimitiveGeometryShape(const Shape::PrimitiveGeometryShape shape) { m_primitiveGeometryShape = shape; loadAssets(); }
		const Shape::PrimitiveGeometryShape& GetPrimitiveGeometryShape() const { return m_primitiveGeometryShape; }
		virtual rapidjson::Value Serialize(rapidjson::Document::AllocatorType& allocator) const override;
		virtual void Deserialize(const rapidjson::Value& doc) override;
	private:
		void loadAssets() override;

		Shape::PrimitiveGeometryShape m_primitiveGeometryShape;

	public:
		static constexpr EnumItem PrimitiveGeometryShapeEnumItem[] = {
			{"UNDEFINED", static_cast<int32_t>(Shape::PrimitiveGeometryShape::UNDEFINED)},
			{"Quad", static_cast<int32_t>(Shape::PrimitiveGeometryShape::Quad)},
			{"Plane", static_cast<int32_t>(Shape::PrimitiveGeometryShape::Plane)},
			{"Circle", static_cast<int32_t>(Shape::PrimitiveGeometryShape::Circle)},
			{"Sphere", static_cast<int32_t>(Shape::PrimitiveGeometryShape::Sphere)},
			{"PlaneWithHole", static_cast<int32_t>(Shape::PrimitiveGeometryShape::PlaneWithHole)},
			{"Cylinder", static_cast<int32_t>(Shape::PrimitiveGeometryShape::Cylinder)},
			{"Capsule", static_cast<int32_t>(Shape::PrimitiveGeometryShape::Capsule)}
		};

		// ComponentProperty
		static const PropertyTable& StaticPropertyTable()
		{
			static const PropertyDesc props[] = {
				MakeEnumProp<PrimitiveGeometry, Shape::PrimitiveGeometryShape, &PrimitiveGeometry::m_primitiveGeometryShape>(
					uuids::hashMaker(), "Shape", "PrimitiveGeometry",
					PrimitiveGeometryShapeEnumItem,
					std::size(PrimitiveGeometryShapeEnumItem), PropertyFlags::None,
					+[](void* obj, const PropertyValue& v, ChangeReason cr)
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