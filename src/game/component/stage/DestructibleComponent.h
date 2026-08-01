#pragma once
#include <vector>

namespace game::component::stage
{
	/**
	 * @brief 殴って壊せる配置物のコンポーネント
	 *
	 * 壊れるまでの寿命をHPではなく「打撃回数」で持つ。
	 * 攻撃力が伸びても壊すのに必要な手数が変わらないため、
	 * ひびの段階が必ず全部見える。HP制にすると育ったビルドで一撃になり、
	 * 段階を用意した意味が無くなる。
	 */
	struct DestructibleComponent
	{
		// 破壊に必要な打撃回数（stageCatalog.jsonのhitsToBreak）
		int m_hitsToBreak{ 0 };

		// これまでに受けた打撃回数
		int m_hitCount{ 0 };

		// 無傷のあいだ描くモデル
		int m_intactHandle{ -1 };

		// あらかじめ割ってあるモデル。破壊した瞬間からこちらへ切り替える
		int m_fracturedHandle{ -1 };

		// ひび段階のテクスチャ。[0]が無傷で、以降が段階1〜のひび
		std::vector<int> m_crackTextures{};

		/**
		 * @brief 現在の被弾回数に対応するひび段階を返す
		 *
		 * 打撃回数とひびの枚数は一致しないため（例: 3回で壊れるブロックへ
		 * ひび3枚）、最後の1発手前が一番派手なひびになるよう割り振る。
		 * 素直に「回数＝段階」にすると、3回で壊れるブロックでは最後のひびが
		 * 表示される前に破壊が走り、一度も見られない
		 * @return テクスチャ配列の添字（0=無傷）
		 */
		[[nodiscard]] int crackStage() const noexcept
		{
			if (m_crackTextures.size() <= 1 || m_hitsToBreak <= 1 || m_hitCount <= 0)
				return 0;

			// 破壊する打撃（m_hitsToBreak回目）はひびを進めないので、
			// ひびが進むのは m_hitsToBreak - 1 回ぶん
			const int lastStage{ static_cast<int>(m_crackTextures.size()) - 1 };
			const int crackHits{ m_hitsToBreak - 1 };
			const int stage{ (m_hitCount * lastStage + crackHits - 1) / crackHits };
			return stage < lastStage ? stage : lastStage;
		}

		/**
		 * @brief 次の打撃で壊れるか
		 * @return 壊れるならtrue
		 */
		[[nodiscard]] bool isBroken() const noexcept
		{
			return m_hitsToBreak > 0 && m_hitCount >= m_hitsToBreak;
		}
	};
} // namespace game::component::stage
