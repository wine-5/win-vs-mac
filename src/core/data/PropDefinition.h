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
		std::string m_collider{};   // "box" | "ground" | "none"

		// 特別な役割。空なら普通の配置物。"bossGate" はボス出現で閉じる扉として扱う。
		// 種類（id）ではなく役割で判定することで、見た目違いの扉を何種類でも用意できる
		std::string m_role{};
		// テクスチャ1枚が受け持つ実寸（ユニット）。0なら繰り返さず面いっぱいに引き伸ばす。
		// 引き伸ばした壁で窓が間延びしないよう、実寸に応じて模様を繰り返させる
		float m_textureTile{ 0.0f };

		// 坂を滑り落ちる加速度。0なら滑らない普通の足場。
		// 歩き速度より滑りが速くなるよう設定すると「ダッシュしないと登れない坂」になる
		float m_slideAccel{ 0.0f };

		// テクスチャを流す速さ（1.0でテクスチャ1枚ぶん/秒）。0なら流れない。
		// 壁の情報が流れる「サーバー内部」の演出に使う。縦横それぞれ指定できるので、
		// 縦に流れる壁と横に流れる壁を配置物の種類として作り分けられる
		float m_scrollU{ 0.0f };
		float m_scrollV{ 0.0f };
	};
} // namespace core::data
