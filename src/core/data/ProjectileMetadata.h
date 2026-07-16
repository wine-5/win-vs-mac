#pragma once
#include <string>

namespace core::data
{
	/**
	 * @brief 弾（投射物）1種類分の定義
	 *
	 * projectileData.json の1エントリに対応する。
	 * プレイヤーのWindow投擲・敵の弾など、種類ごとにIDで引く。
	 */
	struct ProjectileMetadata
	{
		std::string m_id{};           // 弾の識別子（例: "player_window"）
		std::string m_imageId{};      // ビルボード描画に使う画像ID（resources.jsonのimagesで定義）
		float m_speed{ 0.0f };        // 弾速
		float m_damage{ 0.0f };       // 与ダメージ
		float m_lifetime{ 0.0f };     // 寿命（秒）
		float m_radius{ 0.0f };       // 接触半径（当たり判定）
		float m_scale{ 1.0f };        // 見た目スケール
		float m_spawnForward{ 0.0f }; // 発射者より前方に出す距離
		float m_spawnHeight{ 0.0f };  // 発射源の高さ（足元からのオフセット）
		float m_cooldown{ 0.0f };     // 連射間隔（秒）
	};
} // namespace core::data
