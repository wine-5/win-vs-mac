#include "SafariEnemy.h"
#include "game/actor/EnemyBehaviors.h"

namespace game::actor
{
	void SafariEnemy::setupAnimation()
	{
		// safariData.json に animations が無ければ何もしない（Safariはアニメを持たない）
		installEnemyAnimations(m_componentManager, m_entity.getId(), m_enemyData, m_resourceManager);
	}

	void SafariEnemy::setupAI()
	{
		// 積む振る舞いは safariData.json の behaviors（["rangeKeep","patrol"]）で決まる。
		// 距離維持・浮遊高度・発射間隔・正面軸補正はgameplayから各インストーラが読む
		installEnemyBehaviors(m_componentManager, m_entity.getId(), m_enemyData);
	}
} // namespace game::actor
