#include "MacEnemy.h"
#include "game/actor/EnemyBehaviors.h"

namespace game::actor
{
	void MacEnemy::setupAnimation()
	{
		// クリップ定義は macData.json の animations（状態・アニメID・優先度・速度）で決まる
		installEnemyAnimations(m_componentManager, m_entity.getId(), m_enemyData, m_resourceManager);
	}

	void MacEnemy::setupAI()
	{
		// 積む振る舞いは macData.json の behaviors（["boss"]）で決まる。
		// bossインストーラがmac定義（フェーズ・攻撃）を読み込み、攻撃クールダウンをFSMへ委ねる
		installEnemyBehaviors(m_componentManager, m_entity.getId(), m_enemyData);
	}
} // namespace game::actor
