#pragma once
#include <string_view>
#include <stdexcept>
#include <string>

namespace game::constant
{
	/**
	 * @brief 敵の種類
	 *
	 * ステージ配置データ（JSON）と生成処理の対応付けに使う
	 */
	enum class EnemyType
	{
		Xcode,  // 近接雑魚
		Safari, // 遠距離雑魚（飛行）
		Mac,    // ボス
	};

	/**
	 * @brief JSONのタイプ文字列をEnemyTypeへ変換する
	 * @param typeName タイプ名（"xcode" / "safari" / "mac"）
	 * @return 対応するEnemyType
	 */
	inline EnemyType toEnemyType(std::string_view typeName)
	{
		if (typeName == "xcode")  return EnemyType::Xcode;
		if (typeName == "safari") return EnemyType::Safari;
		if (typeName == "mac")    return EnemyType::Mac;
		throw std::runtime_error("未知の敵タイプです: " + std::string(typeName));
	}

	/**
	 * @brief EnemyTypeを表示用の名前へ変換する
	 * @param type 敵の種類
	 * @return 表示名
	 */
	constexpr const char* toEnemyTypeName(EnemyType type) noexcept
	{
		switch (type)
		{
		case EnemyType::Xcode: return "Xcode";
		case EnemyType::Safari: return "Safari";
		case EnemyType::Mac: return "Mac";
		}
		return "Unknown";
	}
} // namespace game::constant
