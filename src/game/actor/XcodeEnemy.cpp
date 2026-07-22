#include "XcodeEnemy.h"
#include "game/actor/EnemyBehaviors.h"

namespace game::actor
{
	void XcodeEnemy::setupAnimation()
	{
		// クリップ定義は xcodeData.json の animations（状態・アニメID・優先度・速度）で決まる
		installEnemyAnimations(m_componentManager, m_entity.getId(), m_enemyData, m_resourceManager);
	}

	void XcodeEnemy::setupAI()
	{
		// 積む振る舞いは xcodeData.json の behaviors（["meleeChase","patrol"]）で決まる。
		// 攻撃のワインドアップ等のパラメータは gameplay から各インストーラが読む
		installEnemyBehaviors(m_componentManager, m_entity.getId(), m_enemyData);
	}
} // namespace game::actor
