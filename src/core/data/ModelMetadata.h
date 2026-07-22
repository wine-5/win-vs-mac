#pragma once
#include "core/utility/Vector3.h"
#include "core/data/MacMetadata.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

namespace core::data
{
	/**
	 * @brief 1アニメーションクリップの定義（JSONの animations 配列の1要素に対応）
	 *
	 * 状態名・優先度名は文字列で持ち、game層でAnimationState/優先度の列挙へ変換する
	 * （core層はgameのアニメ列挙を知らないため）。
	 */
	struct AnimationClipDef
	{
		std::string state;                    // "Idle"/"Walk"/"Run"/"Attack1"/"Attack2"/"Hit"/"Dying"/"Jump"
		std::string animId;                   // resources.jsonのアニメID（例 "anim_xcode_idle"）
		bool loop{ true };                    // 単発再生（攻撃・死亡等）はfalse
		std::string onComplete{ "Idle" };     // 非ループ終了後に遷移する状態名
		std::string priority{ "locomotion" }; // "dying"/"hit"/"attack"/"jump"/"locomotion"
		float speed{ 1.0f };                  // 再生速度倍率
	};

	/**
	 * @brief モデルのメタデータ（JSONに依存しない純粋な構造体）
	 */
	struct ModelMetadata
	{
		std::string id;
		std::string category;
		std::string modelPath;
		core::Vector3 scale{1.0f, 1.0f, 1.0f};
		core::Vector3 position{0.0f, 0.0f, 0.0f};      // Transform情報（全Entity共通）
		core::Vector3 rotation{0.0f, 0.0f, 0.0f};      // Transform情報（全Entity共通）
		core::Vector3 colliderSize{ 0.0f, 0.0f, 0.0f };   // コライダーサイズ（全成分0ならModelRepositoryがモデルのAABBから自動算出する）
		core::Vector3 colliderOffset{ 0.0f, 0.0f, 0.0f }; // コライダー中心オフセット（size自動算出時にAABB中心から併せて算出される）

		std::unordered_map<std::string, float> floatProperties;
		std::unordered_map<std::string, std::string> stringProperties; // 例: {"idleAnim": "path/to/anim.mv1"}

		// 敵の振る舞いレシピ：積むAI振る舞いの名前リスト（例: ["rangeKeep","patrol"]）。
		// パラメータはfloatProperties等から各インストーラが読む。敵をデータの組み合わせで定義するために使う
		std::vector<std::string> behaviors;

		// アニメーションクリップ定義の一覧（JSONの animations 配列）。空ならアニメ無し（Safari等）
		std::vector<AnimationClipDef> animations;

		std::optional<MacMetadata> mac; // ボスの挙動定義（macData.jsonなどにmac要素がある場合のみ）
	};
} // namespace core::data