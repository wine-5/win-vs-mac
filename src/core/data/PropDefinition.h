#pragma once
#include <string>
#include "core/utility/Vector3.h"

namespace core::data
{
	/**
	 * @brief 配置物の種類定義（stageCatalog.jsonのprops[]要素1つ分）
	 *
	 * PropMetadata.m_type からこの定義を引き、モデルパスと素材実寸（baseSize）・
	 * コライダー種別を解決する。エディタとゲームの両方がstageCatalog.jsonを読むが、
	 * ゲーム側が必要とするのはモデル解決に使うこれらの項目のみ。
	 */
	struct PropDefinition
	{
		std::string m_id{};
		std::string m_modelPath{};
		core::Vector3 m_baseSize{}; // モデル素材の実寸。size ÷ baseSize がモデルスケールになる
		std::string m_collider{};   // "box" | "none"
	};
} // namespace core::data
