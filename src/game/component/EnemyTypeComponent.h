#pragma once
#include "game/constant/EnemyType.h"

namespace game::component
{
	/**
	 * @brief スポーン時に確定する敵の種類
	 *
	 * 表示名・撃破ログ・死に方の分岐は全てここを唯一の情報源とする。
	 * AIマーカーコンポーネント（MacAIComponent等）の有無から敵種を推測すると、
	 * AI構成を変えた瞬間に判定が壊れるため、種別は明示的に持たせる。
	 */
	struct EnemyTypeComponent
	{
		constant::EnemyType m_type{ constant::EnemyType::Xcode };
	};
} // namespace game::component
