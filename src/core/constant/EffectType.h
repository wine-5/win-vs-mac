#pragma once
#include <array>
#include <string_view>
#include <utility>

namespace core::constant
{
	/**
	 * @brief エフェクトの種類を表す列挙型
	 */
	enum class EffectType
	{
		None,
		Enemy_HitSword,   // 敵がプレイヤーの剣（近接）で被弹
		Enemy_HitWindow,  // 敵がプレイヤーのWindow投撃弾（遠距離）で被弾
		Enemy_Spawn,      // 敵スポーン（テスト用：Tキーでプレイヤー位置に再生）
		Player_Slash1,    // プレイヤーの近接コンボ1段目の斬撃エフェクト
		Player_Slash2,    // プレイヤーの近接コンボ2段目（回転斬り）の斬撃エフェクト
		Xcode_GroundSlam, // Xcodeの地面叩きつけがダメージを与える瞬間のエフェクト

		// 今後 Player専用エフェクトを追加する場合は Player_で統一する（例: Player_HitSword）
		// 下の EFFECT_TYPE_NAMES への追加も忘れないように
	};

	/**
	 * @brief JSONに書くエフェクト名と列挙の対応表
	 */
	inline constexpr std::array<std::pair<std::string_view, EffectType>, 6> EFFECT_TYPE_NAMES{ {
		{ "Enemy_HitSword", EffectType::Enemy_HitSword },
		{ "Enemy_HitWindow", EffectType::Enemy_HitWindow },
		{ "Enemy_Spawn", EffectType::Enemy_Spawn },
		{ "Player_Slash1", EffectType::Player_Slash1 },
		{ "Player_Slash2", EffectType::Player_Slash2 },
		{ "Xcode_GroundSlam", EffectType::Xcode_GroundSlam },
	} };

	/**
	 * @brief JSONに書かれたエフェクト名を列挙へ変換する
	 * @param name エフェクト名（EFFECT_TYPE_NAMES のいずれか）
	 * @return 対応する種別。知らない名前ならNone（＝演出無し）
	 */
	[[nodiscard]] constexpr EffectType toEffectType(std::string_view name) noexcept
	{
		for (const auto& [key, type] : EFFECT_TYPE_NAMES)
		{
			if (key == name)
				return type;
		}
		return EffectType::None;
	}
} // namespace core::constant