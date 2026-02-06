// Copyright (c) 2025 kong9812
#pragma once
#include "iqt.h"
#include <vector>
#include "MObject.h"

using namespace MyosotisFW;

class OverviewMObjectModel : public QAbstractItemModel
{
public:
	OverviewMObjectModel(QObject* parent = nullptr) : QAbstractItemModel(parent) {}

	// 現在のTopLevel MObjectを受け取る
	void setTopLevelObjects(std::vector<MObject_ptr> objects)
	{
		beginResetModel();
		m_topLevelObjects = objects;
		endResetModel();
	}

	// 行数
	int rowCount(const QModelIndex& parent = QModelIndex()) const override
	{
		// TopLevelの行数
		if (!parent.isValid())
		{
			return static_cast<int>(m_topLevelObjects.size());
		}

		// 特定のオブジェクト(Parent)子の行数
		MObject* parentObj = static_cast<MObject*>(parent.internalPointer());
		return static_cast<int>(parentObj->GetChildren().size());
	}

	// 列数
	int columnCount(const QModelIndex& parent = QModelIndex()) const override
	{
		// 名前の表示のみ 1列でOK
		return 1;
	}

	QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override
	{
		if (!hasIndex(row, column, parent)) return QModelIndex();

		MObject_ptr childObj = nullptr;

		if (!parent.isValid())
		{
			// トップレベルリストから取得
			childObj = m_topLevelObjects[row];
		}
		else
		{
			// 親オブジェクトの子リストから取得
			MObject* parentObj = static_cast<MObject*>(parent.internalPointer());
			childObj = parentObj->GetChildren()[row];
		}

		return childObj ? createIndex(row, column, childObj.get()) : QModelIndex();
	}

	QModelIndex parent(const QModelIndex& index) const override
	{
		if (!index.isValid()) return QModelIndex();

		MObject* childObj = static_cast<MObject*>(index.internalPointer());

		// MObjectにGetParent()が必要
		// もし無い場合は、全探索が必要になりパフォーマンスが低下します
		MObject_ptr parentObj = childObj->GetParent();

		if (!parentObj) return QModelIndex(); // 親がいなければ自身がトップレベル

		// 親が「さらにその親（おじいちゃん）」の何番目の子かを探す
		MObject_ptr grandParent = parentObj->GetParent();
		int row = 0;
		if (!grandParent)
		{
			// 親自身がトップレベルの場合、m_topLevelObjects内での位置を探す
			auto it = std::find_if(m_topLevelObjects.begin(), m_topLevelObjects.end(),
				[&](MObject_ptr ptr) { return ptr == parentObj; });
			row = std::distance(m_topLevelObjects.begin(), it);
		}
		else
		{
			// 親のリスト内でのインデックスを探す
			std::vector<MObject_ptr> siblings = grandParent->GetChildren();
			auto it = std::find_if(siblings.begin(), siblings.end(),
				[&](MObject_ptr ptr) { return ptr == parentObj; });
			row = std::distance(siblings.begin(), it);
		}

		return createIndex(row, 0, parentObj.get());
	}

	QVariant data(const QModelIndex& index, int role) const override
	{
		if (!index.isValid()) return QVariant();

		MObject* obj = static_cast<MObject*>(index.internalPointer());

		switch (role)
		{
		case Qt::DisplayRole:
		case Qt::EditRole:
			return QString::fromStdString(obj->GetName());

		case Qt::DecorationRole: // アイコン
			if (obj->IsCamera()) return m_cameraIcon;
			return m_objectIcon;

		case Qt::ToolTipRole:
			return QString::fromStdString(uuids::to_string(obj->GetUUID()));
		}
		return QVariant();
	}

	QModelIndex addObject(const QModelIndex& parentIndex, MObject_ptr newChild)
	{
		// 親オブジェクトのポインタを取得
		MObject* parentObj = parentIndex.isValid()
			? static_cast<MObject*>(parentIndex.internalPointer())
			: nullptr;

		// 挿入位置(最後の行)を計算
		int newRow = rowCount(parentIndex);

		// Qtに行が増えると宣言
		beginInsertRows(parentIndex, newRow, newRow);

		// 実際のデータ構造(MObject)を更新
		if (parentObj)
		{
			parentObj->AddChild(newChild);
		}
		else
		{
			m_topLevelObjects.push_back(newChild);
		}

		// Qtに完了通知
		endInsertRows();

		// 追加したアイテムのインデックスを作成して返す
		return index(newRow, 0, parentIndex);
	}

	QModelIndex indexFromObject(MObject* object, const QModelIndex& parent = QModelIndex()) const
	{
		if (!object) return QModelIndex();

		for (int i = 0; i < rowCount(parent); i++)
		{
			QModelIndex idx = index(i, 0, parent);
			if (idx.internalPointer() == object)
			{
				return idx;
			}
			if (hasChildren(idx))
			{
				QModelIndex childIdx = indexFromObject(object, idx);
				if (childIdx.isValid()) return childIdx;
			}
		}
		return QModelIndex();
	}

	void ClearAllData()
	{
		beginResetModel();
		m_topLevelObjects.clear();
		endResetModel();
	}

private:
	std::vector<MObject_ptr> m_topLevelObjects;
	QIcon m_cameraIcon;
	QIcon m_objectIcon;
};