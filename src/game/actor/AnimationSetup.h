#pragma once
#include <vector>
#include "core/data/ModelMetadata.h"
#include "core/interface/IResourceManager.h"
#include "game/component/visual/AnimationComponent.h"

namespace game::actor
{
	/**
	 * @brief JSONのアニメーションクリップ定義からAnimationComponentを組み立てる
	 *
	 * 状態名・優先度名の文字列からgame層の列挙へ変換し、アニメーションを読み込む。
	 * プレイヤーも敵も同じ形式のJSONからクリップを定義するため、変換はここへ集約する。
	 * @param defs クリップ定義の一覧（JSONの animations 配列）
	 * @param resourceManager アニメーション読み込み用のIResourceManager
	 * @return 組み立てたAnimationComponent（defsが空なら空のクリップ表を持つ）
	 */
	[[nodiscard]] component::visual::AnimationComponent buildAnimationComponent(
	    const std::vector<core::data::AnimationClipDef>& defs,
	    core::iface::IResourceManager& resourceManager);
} // namespace game::actor
